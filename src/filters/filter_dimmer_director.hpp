//
// Created by Leon Dietrich on 2026-10-22.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

/*
 * The dimmer director filter fans a single 16-bit dimmer input per group out
 * to an X*Y*Z grid of 16-bit outputs. Each cell's effect-curve sample is
 * phase-shifted by its (x, y, z) coordinate so a shared modulation shape
 * "sweeps" across the group geometry. See
 * Docs/Editor/Filters/Filter_Types/effects.md and
 * Docs/Editor/Filters/DimmerDirector.md for the complete signal flow.
 */

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "filters/filter.hpp"
#include "lib/macros.hpp"

namespace dmxfish::filters {

    struct dimmer_curve {
        enum class type_t : uint8_t { LIN, SIG, BEZIER };
        type_t type;
        // Shared parameter storage. For LIN params[0]=a, params[1]=b (others unused).
        // For BEZIER params[0..3] are the four control-point y-values. SIG does not use it.
        std::array<double, 4> params;

        dimmer_curve() : type(type_t::LIN), params{1.0, 0.0, 0.0, 0.0} {}

        [[nodiscard]] double apply(double x) const noexcept;
    };

    struct dd_effect_curve {
        std::string name;
        std::vector<double> samples;
        bool triggered;
        double threshold;
        uint32_t window_size;

        dd_effect_curve() : name(), samples(), triggered(false), threshold(0.1f), window_size(1) {}
    };

    COMPILER_SUPRESS("-Weffc++")
    struct dd_group {
        std::string name;
        uint32_t dim_x, dim_y, dim_z;
        int32_t phase_x, phase_y, phase_z;

        uint16_t* dimmer_input;

        std::vector<uint16_t*> group_mixins;
        std::vector<uint16_t*> group_offsets;

        std::vector<uint16_t> outputs;

        size_t active_effect_idx;
        size_t active_dimmer_idx;

        uint32_t max_window;
        std::vector<uint64_t> trigger_tick;
        std::vector<uint8_t> triggered_running;
        std::vector<double> value_window;
        std::vector<uint32_t> value_window_pos;

        uint32_t n_mix_group;
        uint32_t n_off_group;

        // Per-group virtual time for REPEAT curves. Advances once per non-paused
        // update; pause/resume just gates the increment, so the effect index
        // continues seamlessly after a resume.
        bool paused;
        int64_t virtual_tick;

        dd_group()
            : name(), dim_x(0), dim_y(0), dim_z(0),
              phase_x(0), phase_y(0), phase_z(0),
              dimmer_input(nullptr),
              group_mixins(), group_offsets(), outputs(),
              active_effect_idx(0), active_dimmer_idx(0),
              max_window(1), trigger_tick(), triggered_running(),
              value_window(), value_window_pos(),
              n_mix_group(0), n_off_group(0),
              paused(false), virtual_tick(0) {}
    };
    COMPILER_RESTORE("-Weffc++")

    class filter_dimmer_director : public filter {
    private:
        std::vector<dd_group> groups;
        std::vector<dd_effect_curve> effect_curves;
        std::vector<dimmer_curve> dimmer_curves;
        std::vector<uint16_t*> global_mixins;
        std::vector<uint16_t*> global_offsets;
        std::unordered_map<std::string, size_t> group_index_by_name;
        uint64_t tick;
        std::string own_filter_id;

    public:
        filter_dimmer_director();
        virtual ~filter_dimmer_director();

        virtual void pre_setup(const std::map<std::string, std::string>& configuration,
                               const std::map<std::string, std::string>& initial_parameters,
                               const std::string& filter_id) override;

        virtual void setup_filter(const std::map<std::string, std::string>& configuration,
                                  const std::map<std::string, std::string>& initial_parameters,
                                  const channel_mapping& input_channels,
                                  const std::string& filter_id) override;

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override;

        virtual void update() override;

        virtual void scene_activated() override;

        virtual bool receive_update_from_gui(const std::string& key, const std::string& value) override;

    private:
        void parse_groups(const std::map<std::string, std::string>& configuration, const std::string& own_id);
        void parse_effect_curves(const std::map<std::string, std::string>& configuration, const std::string& own_id);
        void parse_dimmer_curves(const std::map<std::string, std::string>& configuration, const std::string& own_id);
        void link_input_channels(const channel_mapping& input_channels, const std::string& own_id);
        void update_group(dd_group& g);
        void emit_effect_state(const dd_group& g) const;
        void emit_dimmer_state(const dd_group& g) const;
    };

}
