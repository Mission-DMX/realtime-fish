//
// Created by Leon Dietrich on 2026-10-22.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "filters/filter_dimmer_director.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "filters/types.hpp"
#include "io/iomanager.hpp"
#include "lib/logging.hpp"
#include "main.hpp"
#include "utils.hpp"

#include "proto_src/FilterMode.pb.h"
#include "proto_src/MessageTypes.pb.h"

namespace dmxfish::filters {

    namespace {

        constexpr filter_type own_type = filter_type::filter_dimmer_director;

        [[nodiscard]] double clamp01(double v) noexcept {
            return std::clamp(v, 0.0, 1.0);
        }

        [[nodiscard]] double sig_dimmer(double x) noexcept {
            // Logistic centered at 0.5 with slope 12; matches a textbook S-curve
            // on [0, 1] that is near-identity at the endpoints.
            return 1.0 / (1.0 + std::exp(-12.0 * (x - 0.5)));
        }

        [[nodiscard]] double bezier_dimmer(const std::array<double, 4>& cp, double t) noexcept {
            const double one_minus_t = 1.0 - t;
            const double b0 = one_minus_t * one_minus_t * one_minus_t;
            const double b1 = 3.0 * one_minus_t * one_minus_t * t;
            const double b2 = 3.0 * one_minus_t * t * t;
            const double b3 = t * t * t;
            return cp[0] * b0 + cp[1] * b1 + cp[2] * b2 + cp[3] * b3;
        }

        [[nodiscard]] std::pair<std::string, std::string> split_once(const std::string& value, char delim) {
            const auto pos = value.find(delim);
            if (pos == std::string::npos) {
                return {value, ""};
            }
            return {value.substr(0, pos), value.substr(pos + 1)};
        }

        // Specialised REPEAT inner loop. Hoists the dimmer-curve dispatch out of the
        // per-cell path so the compiler can SIMD-vectorise the arithmetic for each
        // curve type independently. The modulo correction is branchless so GCC
        // produces straight-line code across the X extent.
        template <dimmer_curve::type_t Curve>
        [[gnu::flatten]] void run_repeat_body(dd_group& g, int64_t base, double f0,
                                              const std::array<double, 4>& params,
                                              const double* samples, int32_t length) noexcept {
            const int32_t X = static_cast<int32_t>(g.dim_x);
            const int32_t Y = static_cast<int32_t>(g.dim_y);
            const int32_t Z = static_cast<int32_t>(g.dim_z);
            const int64_t phase_x = g.phase_x;
            const int64_t phase_y = g.phase_y;
            const int64_t phase_z = g.phase_z;
            const double a = params[0];
            const double b = params[1];
            uint16_t* __restrict__ out = g.outputs.data();

            for (int32_t z = 0; z < Z; z++) {
                for (int32_t y = 0; y < Y; y++) {
                    const int64_t row_base = base + phase_y * y + phase_z * z;
                    uint16_t* __restrict__ row = out + static_cast<size_t>(y) * X + static_cast<size_t>(z) * X * Y;
                    for (int32_t x = 0; x < X; x++) {
                        const int64_t idx = row_base + phase_x * x;
                        int64_t m = idx % length;
                        // Branchless `m += (m < 0 ? length : 0)` — the signed arithmetic shift
                        // yields -1 for negatives, 0 otherwise.
                        m += length & (m >> 63);
                        double f = std::clamp(f0 * samples[m], 0.0, 1.0);
                        if constexpr (Curve == dimmer_curve::type_t::LIN) {
                            // Final clamp below enforces [0, 1]; omit a redundant mid-clamp.
                            f = a * f + b;
                        } else if constexpr (Curve == dimmer_curve::type_t::SIG) {
                            f = sig_dimmer(f);
                        } else if constexpr (Curve == dimmer_curve::type_t::BEZIER) {
                            f = bezier_dimmer(params, f);
                        }
                        row[x] = static_cast<uint16_t>(std::clamp(f, 0.0, 1.0) * 65535.0);
                    }
                }
            }
        }

    }

    double dimmer_curve::apply(double x) const noexcept {
        switch (this->type) {
        case type_t::LIN:
            return clamp01(this->params[0] * x + this->params[1]);
        case type_t::SIG:
            return sig_dimmer(x);
        case type_t::BEZIER:
            return bezier_dimmer(this->params, x);
        default:
            return x;
        }
    }

    filter_dimmer_director::filter_dimmer_director()
        : filter(),
          groups(), effect_curves(), dimmer_curves(),
          global_mixins(), global_offsets(),
          group_index_by_name(),
          tick(0),
          own_filter_id() {}

    filter_dimmer_director::~filter_dimmer_director() {}

    void filter_dimmer_director::parse_groups(const std::map<std::string, std::string>& configuration, const std::string& own_id) {
        if (!configuration.contains("groups")) {
            throw filter_config_exception("Dimmer director requires a 'groups' configuration entry.", own_type, own_id);
        }
        const auto& groups_str = configuration.at("groups");
        if (groups_str.empty()) [[unlikely]] {
            ::spdlog::warn("Dimmer director '{}' has an empty 'groups' configuration.", own_id);
            return;
        }
        for (const auto& group_str : utils::split(groups_str, ';')) {
            if (group_str.empty()) {
                continue;
            }
            auto parts = utils::split(group_str, ':');
            if (parts.size() < 9) {
                throw filter_config_exception(std::string("Group definition is underspecified: '") + group_str + "'.", own_type, own_id);
            }
            dd_group g;
            try {
                g.name = parts.front(); parts.pop_front();
                g.dim_x = static_cast<uint32_t>(std::stoul(parts.front())); parts.pop_front();
                g.dim_y = static_cast<uint32_t>(std::stoul(parts.front())); parts.pop_front();
                g.dim_z = static_cast<uint32_t>(std::stoul(parts.front())); parts.pop_front();
                g.n_mix_group = static_cast<uint32_t>(std::stoul(parts.front())); parts.pop_front();
                g.n_off_group = static_cast<uint32_t>(std::stoul(parts.front())); parts.pop_front();
                g.phase_x = std::stoi(parts.front()); parts.pop_front();
                g.phase_y = std::stoi(parts.front()); parts.pop_front();
                g.phase_z = std::stoi(parts.front()); parts.pop_front();
            } catch (const std::invalid_argument& e) {
                MARK_UNUSED(e);
                throw filter_config_exception(std::string("Failed to parse group '") + g.name + "': non-numeric value.", own_type, own_id);
            } catch (const std::out_of_range& e) {
                MARK_UNUSED(e);
                throw filter_config_exception(std::string("Failed to parse group '") + g.name + "': value out of range.", own_type, own_id);
            }

            if (g.dim_x == 0 || g.dim_y == 0 || g.dim_z == 0) {
                throw filter_config_exception(std::string("Group '") + g.name + "' has a zero dimension.", own_type, own_id);
            }
            if (this->group_index_by_name.contains(g.name)) {
                throw filter_config_exception(std::string("Duplicate group name '") + g.name + "'.", own_type, own_id);
            }

            const size_t cells = static_cast<size_t>(g.dim_x) * g.dim_y * g.dim_z;
            g.outputs.assign(cells, 0);
            g.group_mixins.assign(g.n_mix_group, nullptr);
            g.group_offsets.assign(g.n_off_group, nullptr);
            g.trigger_tick.assign(cells, 0);
            g.triggered_running.assign(cells, 0);
            g.value_window_pos.assign(cells, 0);

            this->group_index_by_name[g.name] = this->groups.size();
            this->groups.push_back(std::move(g));
        }
    }

    void filter_dimmer_director::parse_effect_curves(const std::map<std::string, std::string>& configuration, const std::string& own_id) {
        if (!configuration.contains("effect_curves")) {
            throw filter_config_exception("Dimmer director requires an 'effect_curves' configuration entry.", own_type, own_id);
        }
        const auto& curves_str = configuration.at("effect_curves");
        for (const auto& curve_str : utils::split(curves_str, ';')) {
            if (curve_str.empty()) {
                continue;
            }
            auto parts = utils::split(curve_str, ':');
            if (parts.size() < 5) {
                throw filter_config_exception(std::string("Effect curve definition is underspecified: '") + curve_str + "'.", own_type, own_id);
            }
            dd_effect_curve ec;
            try {
                ec.name = parts.front(); parts.pop_front();
                const auto mode = utils::toupper(parts.front()); parts.pop_front();
                if (mode == "REPEAT") {
                    ec.triggered = false;
                } else if (mode == "TRIGGER") {
                    ec.triggered = true;
                } else {
                    throw filter_config_exception(std::string("Unknown effect curve mode '") + mode + "'.", own_type, own_id);
                }
                ec.threshold = std::stof(parts.front()); parts.pop_front();
                ec.window_size = static_cast<uint32_t>(std::stoul(parts.front())); parts.pop_front();
                if (ec.window_size == 0) {
                    ec.window_size = 1;
                }
                constexpr uint32_t max_allowed_window = 65535;
                if (ec.window_size > max_allowed_window) {
                    throw filter_config_exception(std::string("Effect curve '") + ec.name + "' has window exceeding " +
                        std::to_string(max_allowed_window) + " ticks.", own_type, own_id);
                }

                const auto samples_str = parts.front(); parts.pop_front();
                for (const auto& s : utils::split(samples_str, ',')) {
                    if (s.empty()) {
                        continue;
                    }
                    ec.samples.push_back(std::stof(s));
                }
            } catch (const std::invalid_argument& e) {
                MARK_UNUSED(e);
                throw filter_config_exception(std::string("Failed to parse effect curve '") + ec.name + "': non-numeric value.", own_type, own_id);
            } catch (const std::out_of_range& e) {
                MARK_UNUSED(e);
                throw filter_config_exception(std::string("Failed to parse effect curve '") + ec.name + "': value out of range.", own_type, own_id);
            }
            if (ec.samples.empty()) {
                throw filter_config_exception(std::string("Effect curve '") + ec.name + "' has no samples.", own_type, own_id);
            }
            this->effect_curves.push_back(std::move(ec));
        }
        if (this->effect_curves.empty()) {
            throw filter_config_exception("Dimmer director has no effect curves configured.", own_type, own_id);
        }
    }

    void filter_dimmer_director::parse_dimmer_curves(const std::map<std::string, std::string>& configuration, const std::string& own_id) {
        if (!configuration.contains("dimmer_curves")) {
            throw filter_config_exception("Dimmer director requires a 'dimmer_curves' configuration entry.", own_type, own_id);
        }
        const auto& curves_str = configuration.at("dimmer_curves");
        for (const auto& curve_str : utils::split(curves_str, ';')) {
            if (curve_str.empty()) {
                continue;
            }
            auto parts = utils::split(curve_str, ':');
            if (parts.size() < 2) {
                throw filter_config_exception(std::string("Dimmer curve definition is underspecified: '") + curve_str + "'.", own_type, own_id);
            }
            dimmer_curve dc;
            parts.pop_front(); // name is informational, discarded
            const auto type_str = utils::toupper(parts.front()); parts.pop_front();
            if (type_str == "LIN") {
                dc.type = dimmer_curve::type_t::LIN;
                // Optional "a,b" parameters: default to the identity f(x) = x when omitted.
                if (!parts.empty()) {
                    const auto param_str = parts.front(); parts.pop_front();
                    if (!param_str.empty()) {
                        const auto cp_list = utils::split(param_str, ',');
                        if (cp_list.size() != 2) {
                            throw filter_config_exception("LIN dimmer curve expects exactly two parameters 'a,b' or none at all.", own_type, own_id);
                        }
                        try {
                            auto it = cp_list.begin();
                            dc.params[0] = std::stod(*it++);
                            dc.params[1] = std::stod(*it);
                        } catch (const std::invalid_argument& e) {
                            MARK_UNUSED(e);
                            throw filter_config_exception("LIN parameter is not a double.", own_type, own_id);
                        } catch (const std::out_of_range& e) {
                            MARK_UNUSED(e);
                            throw filter_config_exception("LIN parameter is out of range.", own_type, own_id);
                        }
                    }
                }
            } else if (type_str == "SIG") {
                dc.type = dimmer_curve::type_t::SIG;
            } else if (type_str == "BEZIER") {
                dc.type = dimmer_curve::type_t::BEZIER;
                if (parts.empty()) {
                    throw filter_config_exception("BEZIER dimmer curve requires control point parameters.", own_type, own_id);
                }
                const auto cp_str = parts.front(); parts.pop_front();
                const auto cp_list = utils::split(cp_str, ',');
                if (cp_list.size() != 4) {
                    throw filter_config_exception("BEZIER dimmer curve requires exactly four control-point y-values.", own_type, own_id);
                }
                try {
                    size_t i = 0;
                    for (const auto& cp : cp_list) {
                        dc.params[i++] = std::stod(cp);
                    }
                } catch (const std::invalid_argument& e) {
                    MARK_UNUSED(e);
                    throw filter_config_exception("BEZIER control point is not a double.", own_type, own_id);
                } catch (const std::out_of_range& e) {
                    MARK_UNUSED(e);
                    throw filter_config_exception("BEZIER control point is out of range.", own_type, own_id);
                }
            } else {
                throw filter_config_exception(std::string("Unknown dimmer curve type '") + type_str + "'.", own_type, own_id);
            }
            this->dimmer_curves.push_back(dc);
        }
        if (this->dimmer_curves.empty()) {
            throw filter_config_exception("Dimmer director has no dimmer curves configured.", own_type, own_id);
        }
    }

    void filter_dimmer_director::pre_setup(const std::map<std::string, std::string>& configuration,
                                           const std::map<std::string, std::string>& initial_parameters,
                                           const std::string& filter_id) {
        MARK_UNUSED(initial_parameters);
        this->own_filter_id = filter_id;

        this->parse_groups(configuration, filter_id);
        this->parse_effect_curves(configuration, filter_id);
        this->parse_dimmer_curves(configuration, filter_id);

        // Size per-cell trigger window buffers to the largest window across ALL effect
        // curves. Checking every curve (not only TRIGGER ones) keeps the inner-loop
        // invariant `window <= max_window` intact even if a REPEAT curve ever gets
        // reclassified or if the dispatch logic changes later — the previous version
        // could underflow `(head + max_window - window) % max_window` the moment a
        // TRIGGER-mode window was larger than any observed-at-pre_setup TRIGGER window.
        uint32_t global_max_window = 1;
        for (const auto& ec : this->effect_curves) {
            if (ec.window_size > global_max_window) {
                global_max_window = ec.window_size;
            }
        }
        for (auto& g : this->groups) {
            g.max_window = global_max_window;
            const size_t cells = g.outputs.size();
            g.value_window.assign(cells * g.max_window, 0.0f);
        }
    }

    void filter_dimmer_director::link_input_channels(const channel_mapping& input_channels, const std::string& own_id) {
        // Global mixin / offset counts come from the channel_mapping itself: the config
        // declared them, but the actual pointers are discovered here by name.
        for (uint32_t i = 0;; i++) {
            const auto name = "mixin_" + std::to_string(i);
            if (!input_channels.sixteen_bit_channels.contains(name)) {
                break;
            }
            this->global_mixins.push_back(input_channels.sixteen_bit_channels.at(name));
        }
        for (uint32_t i = 0;; i++) {
            const auto name = "offset_" + std::to_string(i);
            if (!input_channels.sixteen_bit_channels.contains(name)) {
                break;
            }
            this->global_offsets.push_back(input_channels.sixteen_bit_channels.at(name));
        }

        for (auto& g : this->groups) {
            if (!input_channels.sixteen_bit_channels.contains(g.name)) {
                throw filter_config_exception(std::string("Missing dimmer input channel '") + g.name + "'.", own_type, own_id);
            }
            g.dimmer_input = input_channels.sixteen_bit_channels.at(g.name);

            for (uint32_t j = 0; j < g.n_mix_group; j++) {
                const auto name = "mixin_" + g.name + "_" + std::to_string(j);
                if (!input_channels.sixteen_bit_channels.contains(name)) {
                    throw filter_config_exception(std::string("Missing group mixin input '") + name + "'.", own_type, own_id);
                }
                g.group_mixins[j] = input_channels.sixteen_bit_channels.at(name);
            }
            for (uint32_t l = 0; l < g.n_off_group; l++) {
                const auto name = "offset_" + g.name + "_" + std::to_string(l);
                if (!input_channels.sixteen_bit_channels.contains(name)) {
                    throw filter_config_exception(std::string("Missing group offset input '") + name + "'.", own_type, own_id);
                }
                g.group_offsets[l] = input_channels.sixteen_bit_channels.at(name);
            }
        }
    }

    void filter_dimmer_director::setup_filter(const std::map<std::string, std::string>& configuration,
                                              const std::map<std::string, std::string>& initial_parameters,
                                              const channel_mapping& input_channels,
                                              const std::string& filter_id) {
        MARK_UNUSED(configuration);
        this->link_input_channels(input_channels, filter_id);
        for (const auto& [k, v] : initial_parameters) {
            this->receive_update_from_gui(k, v);
        }
    }

    void filter_dimmer_director::get_output_channels(channel_mapping& map, const std::string& name) {
        for (auto& g : this->groups) {
            for (uint32_t z = 0; z < g.dim_z; z++) {
                for (uint32_t y = 0; y < g.dim_y; y++) {
                    for (uint32_t x = 0; x < g.dim_x; x++) {
                        const size_t idx = x + y * g.dim_x + z * g.dim_x * g.dim_y;
                        const auto channel_name = name + ":" + g.name + "_" + std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(z);
                        map.sixteen_bit_channels[channel_name] = &g.outputs[idx];
                    }
                }
            }
        }
    }

    void filter_dimmer_director::update_group(dd_group& g) {
        const dd_effect_curve& ec = this->effect_curves[g.active_effect_idx];
        const dimmer_curve& dc = this->dimmer_curves[g.active_dimmer_idx];

        // Reduce mixins and offsets once per group per tick.
        double f0 = static_cast<double>(*g.dimmer_input) * (1.0 / 65535.0);
        for (const uint16_t* mix : this->global_mixins) {
            f0 *= static_cast<double>(*mix) * (1.0 / 65535.0);
        }
        for (const uint16_t* mix : g.group_mixins) {
            f0 *= static_cast<double>(*mix) * (1.0 / 65535.0);
        }
        for (const uint16_t* off : this->global_offsets) {
            f0 += static_cast<double>(*off) * (1.0 / 65535.0);
        }
        for (const uint16_t* off : g.group_offsets) {
            f0 += static_cast<double>(*off) * (1.0 / 65535.0);
        }
        f0 = clamp01(f0);

        const int32_t length = static_cast<int32_t>(ec.samples.size());
        const dimmer_curve::type_t dc_type = dc.type;
        const std::array<double, 4> params = dc.params;
        const double* samples = ec.samples.data();

        const int32_t X = static_cast<int32_t>(g.dim_x);
        const int32_t Y = static_cast<int32_t>(g.dim_y);
        const int32_t Z = static_cast<int32_t>(g.dim_z);

        if (!ec.triggered) {
            // virtual_tick advances in update() only on non-paused ticks and is incremented
            // AFTER the base is sampled. While paused, replay the previous (virtual_tick - 1)
            // index so the output holds its last value regardless of how many paused ticks pass.
            const int64_t base = g.paused ? std::max<int64_t>(0, g.virtual_tick - 1) : g.virtual_tick;
            switch (dc_type) {
            case dimmer_curve::type_t::LIN:
                run_repeat_body<dimmer_curve::type_t::LIN>(g, base, f0, params, samples, length);
                break;
            case dimmer_curve::type_t::SIG:
                run_repeat_body<dimmer_curve::type_t::SIG>(g, base, f0, params, samples, length);
                break;
            case dimmer_curve::type_t::BEZIER:
                run_repeat_body<dimmer_curve::type_t::BEZIER>(g, base, f0, params, samples, length);
                break;
            default:
                break;
            }
            return;
        }

        // TRIGGER mode: per-cell state driven by input delta over the window.
        const uint32_t window = ec.window_size;
        const double threshold = ec.threshold;
        const uint32_t max_window = g.max_window;
        for (int32_t z = 0; z < Z; z++) {
            for (int32_t y = 0; y < Y; y++) {
                for (int32_t x = 0; x < X; x++) {
                    const size_t cell = static_cast<size_t>(x) + static_cast<size_t>(y) * X + static_cast<size_t>(z) * X * Y;
                    const size_t ring_base = cell * max_window;
                    const uint32_t head = g.value_window_pos[cell];
                    const uint32_t oldest = (head + max_window - window) % max_window;
                    const double old_value = g.value_window[ring_base + oldest];
                    const double delta = std::abs(f0 - old_value);

                    g.value_window[ring_base + head] = f0;
                    g.value_window_pos[cell] = (head + 1) % max_window;

                    if (!g.triggered_running[cell]) {
                        if (delta > threshold) {
                            g.triggered_running[cell] = 1;
                            g.trigger_tick[cell] = this->tick;
                        }
                    }

                    int64_t idx;
                    if (g.triggered_running[cell]) {
                        const int64_t elapsed = static_cast<int64_t>(this->tick - g.trigger_tick[cell]);
                        const int64_t phase_shift = static_cast<int64_t>(g.phase_x) * x + static_cast<int64_t>(g.phase_y) * y + static_cast<int64_t>(g.phase_z) * z;
                        idx = std::clamp<int64_t>(elapsed + phase_shift, 0, length - 1);
                        if (idx >= length - 1 && delta <= threshold) {
                            // Curve has finished and the input has settled across the window: ready for a fresh trigger.
                            g.triggered_running[cell] = 0;
                        }
                    } else {
                        idx = length - 1; // hold the final sample until a new trigger
                    }

                    const double sample = samples[idx];
                    double f = clamp01(f0 * sample);
                    switch (dc_type) {
                    case dimmer_curve::type_t::LIN:
                        // Final `clamp01(f) * 65535.0` below enforces the [0, 1] clip; skip the redundant clamp here.
                        f = params[0] * f + params[1];
                        break;
                    case dimmer_curve::type_t::SIG:
                        f = sig_dimmer(f);
                        break;
                    case dimmer_curve::type_t::BEZIER:
                        f = bezier_dimmer(params, f);
                        break;
                    default:
                        break;
                    }
                    g.outputs[cell] = static_cast<uint16_t>(clamp01(f) * 65535.0);
                }
            }
        }
    }

    void filter_dimmer_director::update() {
        for (auto& g : this->groups) {
            this->update_group(g);
            if (!g.paused) {
                g.virtual_tick++;
            }
        }
        this->tick++;
    }

    void filter_dimmer_director::scene_activated() {
        this->tick = 0;
        for (auto& g : this->groups) {
            std::fill(g.outputs.begin(), g.outputs.end(), static_cast<uint16_t>(0));
            std::fill(g.trigger_tick.begin(), g.trigger_tick.end(), 0u);
            std::fill(g.triggered_running.begin(), g.triggered_running.end(), static_cast<uint8_t>(0));
            std::fill(g.value_window.begin(), g.value_window.end(), 0.0f);
            std::fill(g.value_window_pos.begin(), g.value_window_pos.end(), 0u);
            g.paused = false;
            g.virtual_tick = 0;
        }
    }

    void filter_dimmer_director::emit_effect_state(const dd_group& g) const {
        auto iomanager = get_iomanager_instance();
        if (iomanager == nullptr) {
            return;
        }
        const auto active_show = iomanager->get_active_show();
        if (active_show == nullptr) {
            return;
        }
        auto msg = missiondmx::fish::ipcmessages::update_parameter();
        msg.set_filter_id(this->own_filter_id);
        msg.set_parameter_key("effect_state");
        msg.set_scene_id(active_show->get_current_scene_id());
        msg.set_parameter_value(g.name + ":" + std::to_string(g.active_effect_idx) + ":" + (g.paused ? "paused" : "running"));
        iomanager->push_msg_to_all_gui(msg, ::missiondmx::fish::ipcmessages::MSGT_UPDATE_PARAMETER);
    }

    void filter_dimmer_director::emit_dimmer_state(const dd_group& g) const {
        auto iomanager = get_iomanager_instance();
        if (iomanager == nullptr) {
            return;
        }
        const auto active_show = iomanager->get_active_show();
        if (active_show == nullptr) {
            return;
        }
        auto msg = missiondmx::fish::ipcmessages::update_parameter();
        msg.set_filter_id(this->own_filter_id);
        msg.set_parameter_key("dimmer_state");
        msg.set_scene_id(active_show->get_current_scene_id());
        msg.set_parameter_value(g.name + ":" + std::to_string(g.active_dimmer_idx));
        iomanager->push_msg_to_all_gui(msg, ::missiondmx::fish::ipcmessages::MSGT_UPDATE_PARAMETER);
    }

    bool filter_dimmer_director::receive_update_from_gui(const std::string& key, const std::string& value) {
        if (key == "pause_effect" || key == "resume_effect") {
            const auto git = this->group_index_by_name.find(value);
            if (git == this->group_index_by_name.end()) {
                return false;
            }
            dd_group& g = this->groups[git->second];
            if (key == "pause_effect") {
                if (g.active_effect_idx >= this->effect_curves.size()) {
                    return false;
                }
                if (this->effect_curves[g.active_effect_idx].triggered) {
                    return false;
                }
                if (g.paused) {
                    return false;
                }
                g.paused = true;
                this->emit_effect_state(g);
                return true;
            }
            // resume_effect: no-op (but reported as success) if not currently paused.
            if (!g.paused) {
                return true;
            }
            g.paused = false;
            this->emit_effect_state(g);
            return true;
        }

        if (key != "active_effect" && key != "active_dimmer") {
            return false;
        }
        const auto [group_name, index_str] = split_once(value, ':');
        if (group_name.empty() || index_str.empty()) {
            return false;
        }
        const auto it = this->group_index_by_name.find(group_name);
        if (it == this->group_index_by_name.end()) {
            return false;
        }
        size_t index;
        try {
            index = std::stoul(index_str);
        } catch (const std::invalid_argument& e) {
            MARK_UNUSED(e);
            return false;
        } catch (const std::out_of_range& e) {
            MARK_UNUSED(e);
            return false;
        }
        dd_group& g = this->groups[it->second];
        if (key == "active_effect") {
            if (index >= this->effect_curves.size()) {
                return false;
            }
            g.active_effect_idx = index;
            // Switching curves resumes playback: pause state is tied to the previously-active curve.
            g.paused = false;
            this->emit_effect_state(g);
        } else {
            if (index >= this->dimmer_curves.size()) {
                return false;
            }
            g.active_dimmer_idx = index;
            this->emit_dimmer_state(g);
        }
        return true;
    }

}
