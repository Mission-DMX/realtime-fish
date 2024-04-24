#include "filters/filter_cue.hpp"

#include <cmath>
#include <string>
#include <sstream>

#include "main.hpp"
#include "lib/macros.hpp"
#include "lib/logging.hpp"
#include "dmx/pixel.hpp"
#include "filters/util.hpp"

#include "proto_src/MessageTypes.pb.h"
#include "proto_src/FilterMode.pb.h"

// Todo: Color Transisiton via RGB colorspace not with HSI

int count_occurence_of(const std::string &base_string, std::string pattern, size_t start, size_t end) {
    int occurrences = 0;
    while ((start = base_string.find(pattern, start)) != std::string::npos && start <= end) {
        ++occurrences;
        start += pattern.length();
    }
    return occurrences;
}

namespace dmxfish::filters {

    void filter_cue::update_parameter_gui() {
        if (auto iomanager = get_iomanager_instance(); iomanager != nullptr) {
            if (auto s = iomanager->get_active_show(); s != nullptr) {

                std::string run_state_str;
                switch (this->running_state) {
                    case STOP: {
                        run_state_str = "stop";
                        break;
                    }
                    case PLAY: {
                        run_state_str = "play";
                        break;
                    }
                    case PAUSE: {
                        run_state_str = "pause";
                        break;
                    }
                    default: {
                        run_state_str = "error";
                        break;
                    }
                }

                auto update_message = missiondmx::fish::ipcmessages::update_parameter();
                update_message.set_filter_id(this->own_filter_id);
                update_message.set_parameter_key("actual_state");
                update_message.set_scene_id(s->get_active_scene());
                std::stringstream params;
                params <<
                       run_state_str << ";" <<
                       std::to_string(this->actual_values.cue) << ";" <<
                       std::to_string((*this->time - this->start_time) / 1000) << ";";
                if (this->cues.at(this->actual_values.cue).timestamps.size() > 0) {
                    params << std::to_string(
                            (this->cues.at(this->actual_values.cue).timestamps.at(this->cues.at(this->actual_values.cue).timestamps.size() - 1)) /
                            1000);
                } else {
                    params << std::to_string(0);
                }
                params << ";" << std::to_string(this->time_scale);

                update_message.set_parameter_value(params.str());
                iomanager->push_msg_to_all_gui(update_message, ::missiondmx::fish::ipcmessages::MSGT_UPDATE_PARAMETER);
            }
        }
    }

    template <typename T>
    void filter_cue::reserve_init_out(int amount){
        if constexpr (std::is_same<T, uint8_t>::value) {
            this->channel_names_eight.reserve(amount);
            this->actual_values.eight_bit_channels.reserve(amount);
            this->last_values.eight_bit_channels.reserve(amount);
        } else if constexpr (std::is_same<T, uint16_t>::value) {
            this->channel_names_sixteen.reserve(amount);
            this->actual_values.sixteen_bit_channels.reserve(amount);
            this->last_values.sixteen_bit_channels.reserve(amount);
        } else if constexpr (std::is_same<T, double>::value) {
            this->channel_names_float.reserve(amount);
            this->actual_values.float_channels.reserve(amount);
            this->last_values.float_channels.reserve(amount);
        } else {
            this->channel_names_color.reserve(amount);
            this->actual_values.color_channels.reserve(amount);
            this->last_values.color_channels.reserve(amount);
        }
    }

    template <typename T>
    void filter_cue::init_values_out(std::string &channel_name){
        if constexpr (std::is_same<T, uint8_t>::value) {
            this->channel.push_back(channel_str(EIGHT_BIT, this->actual_values.eight_bit_channels.size()));
            this->channel_names_eight.push_back(channel_name);
            this->actual_values.eight_bit_channels.push_back(0);
            this->last_values.eight_bit_channels.push_back(0);
        } else if constexpr (std::is_same<T, uint16_t>::value) {
            this->channel.push_back(channel_str(SIXTEEN_BIT, this->actual_values.sixteen_bit_channels.size()));
            this->channel_names_sixteen.push_back(channel_name);
            this->actual_values.sixteen_bit_channels.push_back(0);
            this->last_values.sixteen_bit_channels.push_back(0);
        } else if constexpr (std::is_same<T, double>::value) {
            this->channel.push_back(channel_str(FLOAT, this->actual_values.float_channels.size()));
            this->channel_names_float.push_back(channel_name);
            this->actual_values.float_channels.push_back(0);
            this->last_values.float_channels.push_back(0);
        } else {
            this->channel.push_back(channel_str(COLOR, this->actual_values.color_channels.size()));
            this->channel_names_color.push_back(channel_name);
            this->actual_values.color_channels.push_back(dmxfish::dmx::pixel());
            this->last_values.color_channels.push_back(dmxfish::dmx::pixel());
        }
    }

    inline bool
    filter_cue::do_with_substr(const std::string &str, size_t start, const size_t end, const char sep, size_t min_loops,
                               const std::function<bool(const std::string &, size_t, size_t, size_t)> func) {
        size_t next_pos = str.find(sep, start);
        size_t number = 0;
        while (true) {
            if (!func(str, start, std::min(end, next_pos), number)) {
                return false;
            }

            number++;
            if (next_pos >= end) {
                if (number < min_loops) {
                    return false;
                }
                break;
            }
            start = next_pos + 1;
            next_pos = str.find(sep, start);
        }
        return true;
    }

    bool filter_cue::handle_frame(size_t cue, const std::string &str, size_t start, size_t end, size_t nr_channel) {
        const size_t sep = str.find('@', start);
        if (nr_channel >= this->channel.size()) {
            return false;
        }
        transition_t tr = LINEAR;
        if (!str.substr(sep + 1, end - sep - 1).compare("edg")) {
            tr = EDGE;
//            } else if (!str.substr(start, sep-start).compare("lin")){
//                tr = LINEAR;
        } else if (!str.substr(sep + 1, end - sep - 1).compare("sig")) {
            tr = SIGMOIDAL;
        } else if (!str.substr(sep + 1, end - sep - 1).compare("e_i")) {
            tr = EASE_IN;
        } else if (!str.substr(sep + 1, end - sep - 1).compare("e_o")) {
            tr = EASE_OUT;
        }
        try {
            switch (this->channel.at(nr_channel).channel_type) {
                case EIGHT_BIT: {
                    this->cues.at(cue).eight_bit_frames.push_back(
                            key_frame<uint8_t>((uint8_t) std::max(std::min(std::stoi(str.substr(start, sep - start)),(int) std::numeric_limits<uint8_t>::max()), 0), tr));
                    break;
                }
                case SIXTEEN_BIT: {
                    this->cues.at(cue).sixteen_bit_frames.push_back(
                            key_frame<uint16_t>((uint16_t) std::max(std::min(std::stoi(str.substr(start, sep - start)),(int) std::numeric_limits<uint16_t>::max()), 0), tr));
                    break;
                }
                case FLOAT: {
                    this->cues.at(cue).float_frames.push_back(
                            key_frame<double>(std::stod(str.substr(start, sep - start)), tr));
                    break;
                }
                case COLOR: {
                    const auto first_position = str.find(',', start);
                    const auto second_position = str.find(",", first_position + 1);
                    this->cues.at(cue).color_frames.push_back(key_frame<dmxfish::dmx::pixel>(dmxfish::dmx::pixel(std::stod(
                            str.substr(start, first_position - start)), std::stod(
                            str.substr(first_position + 1, second_position - first_position - 1)), std::stod(
                            str.substr(second_position + 1, end - second_position - 1))), tr));
                    break;
                }
                default: {
                    return false;
                }
            }
            return true;
        } catch (const std::invalid_argument &ex) {
            MARK_UNUSED(ex);
            return false;
        } catch (const std::out_of_range &ex) {
            MARK_UNUSED(ex);
            return false;
        }
    }

    bool filter_cue::handle_timestamps(size_t cue, const std::string &str, size_t start, size_t end, size_t nr_timestamp) {
        MARK_UNUSED(nr_timestamp);
        const size_t end_ts = str.find(':', start);
        if (start < end) {
            try {
                this->cues.at(cue).timestamps.push_back(std::stod(str.substr(start, end_ts - start)) * 1000);
            } catch (const std::invalid_argument &ex) {
                MARK_UNUSED(ex);
                return false;
            } catch (const std::out_of_range &ex) {
                MARK_UNUSED(ex);
                return false;
            }
            return do_with_substr(str, end_ts + 1, end, '&', this->channel.size(),
                        std::bind(&dmxfish::filters::filter_cue::handle_frame, this, cue, std::placeholders::_1,
                        std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        }
        else {
            ::spdlog::warn("cue " + std::to_string(cue) + " of filter " + this->own_filter_id + " is empty");
            return true;
        }
    }

    bool filter_cue::handle_cue_conf(size_t cue, const std::string &str, size_t start, size_t end, size_t number) {
        MARK_UNUSED(number);
        if (!str.substr(start, end - start).compare("hold")) {
            this->cues.at(cue).end_handling = HOLD;
        } else if (!str.substr(start, end - start).compare("start_again")) {
            this->cues.at(cue).end_handling = START_AGAIN;
        } else if (!str.substr(start, end - start).compare("next_cue")) {
            this->cues.at(cue).end_handling = NEXT_CUE;
        } else if (!str.substr(start, end - start).compare("do_nothing")) {
            this->cues.at(cue).restart_handling = DO_NOTHING;
        } else if (!str.substr(start, end - start).compare("restart")) {
            this->cues.at(cue).restart_handling = START_FROM_BEGIN;
        } else {
            return false;
        }
        return true;
    }

    bool filter_cue::handle_cue(const std::string &str, size_t start, size_t end, size_t cue) {
        this->cues.push_back(cue_st());
        this->cues.at(cue).end_handling = NEXT_CUE;
        this->cues.at(cue).restart_handling = DO_NOTHING;
        const size_t end_conf = str.find('#', start);
        const size_t nr_of_ts = count_occurence_of(str, "|", start, end_conf) + 1;
        this->cues.at(cue).timestamps.reserve(nr_of_ts);
        this->cues.at(cue).eight_bit_frames.reserve(nr_of_ts * this->actual_values.eight_bit_channels.size());
        this->cues.at(cue).sixteen_bit_frames.reserve(nr_of_ts * this->actual_values.sixteen_bit_channels.size());
        this->cues.at(cue).float_frames.reserve(nr_of_ts * this->actual_values.float_channels.size());
        this->cues.at(cue).color_frames.reserve(nr_of_ts * this->actual_values.color_channels.size());
        if (!do_with_substr(str, start, std::min(end_conf, end), '|', 1,
                            std::bind(&dmxfish::filters::filter_cue::handle_timestamps, this, cue,
                                      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                                      std::placeholders::_4))) {
            return false;
        }
        if (end_conf < end) {
            return do_with_substr(str, end_conf + 1, end, '#', 1,
                                  std::bind(&dmxfish::filters::filter_cue::handle_cue_conf, this, cue,
                                            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                                            std::placeholders::_4));
        }
        return true;
    }


    template<typename T>
    void filter_cue::calc_transition(double rel_time, transition_t transition, T start_value, T end_value, size_t ind) {
        switch (transition) {
            case EDGE:
                if constexpr(std::is_same<T, uint8_t>::value)
                {
                    this->actual_values.eight_bit_channels.at(ind) = (rel_time < 1) ? start_value : end_value;
                } else if constexpr(std::is_same<T, uint16_t>::value)
                {
                    this->actual_values.sixteen_bit_channels.at(ind) = (rel_time < 1) ? start_value : end_value;                } else if constexpr(std::is_same<T, double>::value)
                {
                    this->actual_values.float_channels.at(ind) = (rel_time < 1) ? start_value : end_value;
                } else if constexpr(std::is_same<T, dmxfish::dmx::pixel>::value)
                {
                    this->actual_values.color_channels.at(ind) = (rel_time < 1) ? start_value : end_value;
                }
                this->actual_values.updated = false;
                return;
            case LINEAR:
                break;
            case SIGMOIDAL:
                rel_time = (1.0 / (1 + std::exp(6 - rel_time * 12))) * 2 * 0.5024726231566347743340599073722557624510930726498587540880605924 - 0.0024726231566347743340599073722557624510930726498587540880605924;
                break;
            case EASE_IN:
                rel_time = rel_time * rel_time;
                break;
            case EASE_OUT:
                rel_time = 1 - ((1 - rel_time) * (1 - rel_time));
                break;
            default:
                ::spdlog::warn("should not have reached default transition type");
                break;
        }
        if constexpr(std::is_same<T, uint8_t>::value)
        {
            this->actual_values.eight_bit_channels.at(ind) = (uint8_t) std::round((end_value - start_value) * rel_time + start_value);
        } else if constexpr(std::is_same<T, uint16_t>::value)
        {
            this->actual_values.sixteen_bit_channels.at(ind) = (uint16_t) std::round((end_value - start_value) * rel_time + start_value);
        } else if constexpr(std::is_same<T, double>::value)
        {
            this->actual_values.float_channels.at(ind) = (end_value - start_value) * rel_time + start_value;
        } else if constexpr(std::is_same<T, dmxfish::dmx::pixel>::value)
        {
            this->actual_values.color_channels.at(ind) = dmxfish::dmx::pixel((end_value.hue - start_value.hue) * rel_time + start_value.hue,
                                                         (end_value.saturation - start_value.saturation) * rel_time +
                                                         start_value.saturation,
                                                         (end_value.iluminance - start_value.iluminance) * rel_time +
                                                         start_value.iluminance);
        }
        this->actual_values.updated = false;
    }

    void filter_cue::update_hold_values() {
        if (!this->actual_values.updated) {
            for (size_t i = 0; i < this->actual_values.eight_bit_channels.size(); i++) {
                this->actual_values.eight_bit_channels.at(i) = this->last_values.eight_bit_channels.at(i);
            }
            for (size_t i = 0; i < this->actual_values.sixteen_bit_channels.size(); i++) {
                this->actual_values.sixteen_bit_channels.at(i) = this->last_values.sixteen_bit_channels.at(i);
            }
            for (size_t i = 0; i < this->actual_values.float_channels.size(); i++) {
                this->actual_values.float_channels.at(i) = this->last_values.float_channels.at(i);
            }
            for (size_t i = 0; i < this->actual_values.color_channels.size(); i++) {
                this->actual_values.color_channels.at(i) = this->last_values.color_channels.at(i);
            }
            this->actual_values.updated = true;
        }
    }

    inline void filter_cue::update_last_values() {
        this->last_values.time_stamp = *this->time;
        if (!this->last_values.updated) {
            for (size_t i = 0; i < this->actual_values.eight_bit_channels.size(); i++) {
                this->last_values.eight_bit_channels.at(i) = this->actual_values.eight_bit_channels.at(i);
            }
            for (size_t i = 0; i < this->actual_values.sixteen_bit_channels.size(); i++) {
                this->last_values.sixteen_bit_channels.at(i) = this->actual_values.sixteen_bit_channels.at(i);
            }
            for (size_t i = 0; i < this->actual_values.float_channels.size(); i++) {
                this->last_values.float_channels.at(i) = this->actual_values.float_channels.at(i);
            }
            for (size_t i = 0; i < this->actual_values.color_channels.size(); i++) {
                this->last_values.color_channels.at(i) = this->actual_values.color_channels.at(i);
            }
            this->last_values.frame = this->actual_values.frame;
            this->last_values.cue = this->actual_values.cue;
            this->last_values.updated = true;
        }
    }

    void filter_cue::start_new_cue() {
        update_last_values();
        this->start_time = *this->time;
        this->actual_values.frame = 0;
        if (this->actual_values.cue >= this->cues.size()) {
            this->actual_values.cue = this->cues.size() - 1;
        }
        if(this->actual_values.cue < this->cues.size()) {
                this->cue_end_handling_real = this->cues.at(this->actual_values.cue).end_handling;
        }
    }

    bool filter_cue::last_frame_handling() {
        switch (this->cue_end_handling_real) { // end of cue handling
            case START_AGAIN:
                break;
            case HOLD:
                update_hold_values();
                this->pause_time = *this->time;
                this->running_state = PAUSE;
                this->cue_end_handling_real = HOLDING;
                update_parameter_gui();
                return false;
            case HOLDING:
                return false;
            case NEXT_CUE:
                if (this->actual_values.cue < this->cues.size() - 1) { // Not last cue?
                    this->actual_values.cue++;
                } else {
                    switch (this->handle_end) { // end of cuelist handling
                        case START_AGAIN:
                            this->actual_values.cue = 0;
                            break;
                        case HOLD:
                            this->running_state = STOP;
                            update_hold_values();
                            update_parameter_gui();
                            return false;
                        case NEXT_CUE:
                            ::spdlog::warn("should not have reached NEXT CUE at the end of the cuelist");
                            return false;
                        case HOLDING:
                            ::spdlog::warn("should not have reached HOLDING at the end of the cuelist");
                            return false;
                        default:
                            ::spdlog::warn("should not have reached default end_handling at the end of the cuelist");
                            return false;
                    }
                }
                break;
            default:
                ::spdlog::warn("should not have reached default end_handling at the end of the cue");
                return false;
        }
        if (this->next_cue < this->cues.size()) { // if next cue is set, start this cue
            this->actual_values.cue = this->next_cue;
            this->next_cue = 0xffff;
        }
        start_new_cue();
        this->cue_end_handling_real = this->cues.at(this->actual_values.cue).end_handling;
        return true;
    }

    void filter_cue::calc_values() {
        if (this->actual_values.frame >= this->cues.at(this->actual_values.cue).timestamps.size()){
            last_frame_handling();
            return;
        }
        if (this->last_values.frame != this->actual_values.frame - 1 || this->last_values.cue != this->actual_values.cue) {
            this->last_values.updated = false;
        }
        if (*this->time >= this->cues.at(this->actual_values.cue).timestamps.at(this->actual_values.frame) / this->time_scale + this->start_time) { // Next Frame?
            if (!this->last_values.updated) {
                for (size_t i = 0; i < this->actual_values.eight_bit_channels.size(); i++) {
                    this->last_values.eight_bit_channels.at(i) = this->cues.at(this->actual_values.cue).eight_bit_frames.at(
                            this->actual_values.eight_bit_channels.size() * this->actual_values.frame + i).value;
                }
                for (size_t i = 0; i < this->actual_values.sixteen_bit_channels.size(); i++) {
                    this->last_values.sixteen_bit_channels.at(i) = this->cues.at(this->actual_values.cue).sixteen_bit_frames.at(
                            this->actual_values.sixteen_bit_channels.size() * this->actual_values.frame + i).value;
                }
                for (size_t i = 0; i < this->actual_values.float_channels.size(); i++) {
                    this->last_values.float_channels.at(i) = this->cues.at(this->actual_values.cue).float_frames.at(
                            this->actual_values.float_channels.size() * this->actual_values.frame + i).value;
                }
                for (size_t i = 0; i < this->actual_values.color_channels.size(); i++) {
                    this->last_values.color_channels.at(i) = this->cues.at(this->actual_values.cue).color_frames.at(
                            this->actual_values.color_channels.size() * this->actual_values.frame + i).value;
                }
                this->last_values.frame = this->actual_values.frame;
                this->last_values.cue = this->actual_values.cue;
                this->last_values.updated = true;
            }
            this->last_values.time_stamp = this->start_time + this->cues.at(this->actual_values.cue).timestamps.at(this->actual_values.frame) / this->time_scale;
            if (this->actual_values.frame < this->cues.at(this->actual_values.cue).timestamps.size() - 1) { // Not the last Frame of the cue?
                this->actual_values.frame++;
            } else { // last frame of cue
                if (!last_frame_handling()){
                    return;
                }
            }
            update_parameter_gui();
        }
        if (this->actual_values.frame < this->cues.at(this->actual_values.cue).timestamps.size()) {
            this->last_values.updated = false;
            double rel_time = (this->cues.at(this->actual_values.cue).timestamps.at(this->actual_values.frame) / this->time_scale - this->last_values.time_stamp + this->start_time) <= 0 ? 1 :
                    (*this->time - this->last_values.time_stamp) / (this->cues.at(this->actual_values.cue).timestamps.at(this->actual_values.frame) / this->time_scale - this->last_values.time_stamp + this->start_time);

            for (size_t i = 0; i < this->actual_values.eight_bit_channels.size(); i++) {
                size_t frame_index = this->actual_values.frame * this->actual_values.eight_bit_channels.size() + i;
                calc_transition<uint8_t>(rel_time,
                                         this->cues.at(this->actual_values.cue).eight_bit_frames.at(frame_index).transition,
                                         this->last_values.eight_bit_channels.at(i),
                                         this->cues.at(this->actual_values.cue).eight_bit_frames.at(frame_index).value, i);
            }
            for (size_t i = 0; i < this->actual_values.sixteen_bit_channels.size(); i++) {
                size_t frame_index = this->actual_values.frame * this->actual_values.sixteen_bit_channels.size() + i;
                calc_transition<uint16_t>(rel_time,
                                          this->cues.at(this->actual_values.cue).sixteen_bit_frames.at(frame_index).transition,
                                          this->last_values.sixteen_bit_channels.at(i),
                                          this->cues.at(this->actual_values.cue).sixteen_bit_frames.at(frame_index).value, i);
            }
            for (size_t i = 0; i < this->actual_values.float_channels.size(); i++) {
                size_t frame_index = this->actual_values.frame * this->actual_values.float_channels.size() + i;
                calc_transition<double>(rel_time,
                                        this->cues.at(this->actual_values.cue).float_frames.at(frame_index).transition,
                                        this->last_values.float_channels.at(i),
                                        this->cues.at(this->actual_values.cue).float_frames.at(frame_index).value, i);
            }
            for (size_t i = 0; i < this->actual_values.color_channels.size(); i++) {
                size_t frame_index = this->actual_values.frame * this->actual_values.color_channels.size() + i;
                calc_transition<dmxfish::dmx::pixel>(rel_time,
                                                     this->cues.at(this->actual_values.cue).color_frames.at(frame_index).transition,
                                                     this->last_values.color_channels.at(i),
                                                     this->cues.at(this->actual_values.cue).color_frames.at(frame_index).value, i);
            }
        }
    }


    void filter_cue::pre_setup(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const std::string& own_id) {
        MARK_UNUSED(initial_parameters);
        this->own_filter_id = own_id;
        if (!configuration.contains("mapping")) {
            throw filter_config_exception("cue filter: unable to setup the mapping", filter_type::filter_cue, own_id);
        }
        util::init_mapping(
                configuration.at("mapping"),
                std::bind(&dmxfish::filters::filter_cue::reserve_init_out<uint8_t>, this, std::placeholders::_1),
                std::bind(&dmxfish::filters::filter_cue::reserve_init_out<uint16_t>, this,
                          std::placeholders::_1),
                std::bind(&dmxfish::filters::filter_cue::reserve_init_out<double>, this, std::placeholders::_1),
                std::bind(&dmxfish::filters::filter_cue::reserve_init_out<dmxfish::dmx::pixel>, this,
                          std::placeholders::_1),
                std::bind(&dmxfish::filters::filter_cue::init_values_out<uint8_t>, this, std::placeholders::_1),
                std::bind(&dmxfish::filters::filter_cue::init_values_out<uint16_t>, this, std::placeholders::_1),
                std::bind(&dmxfish::filters::filter_cue::init_values_out<double>, this, std::placeholders::_1),
                std::bind(&dmxfish::filters::filter_cue::init_values_out<dmxfish::dmx::pixel>, this,
                          std::placeholders::_1),
                filter_type::filter_cue, own_id
        );
    }

    void filter_cue::setup_filter(const std::map <std::string, std::string> &configuration,
                                  const std::map <std::string, std::string> &initial_parameters,
                                  const channel_mapping &input_channels,
                                  const std::string& own_id) {
        MARK_UNUSED(initial_parameters);
        if (!input_channels.float_channels.contains("time")) {
            throw filter_config_exception("Unable to link input of cue filter: channel mapping does not contain channel "
                                          "'time' of type 'double'. This input should come from the scenes global time "
                                          "node.", filter_type::filter_cue, own_id);
        }
        this->time = input_channels.float_channels.at("time");

        if (!input_channels.float_channels.contains("time_scale")) {
            throw filter_config_exception("Unable to link input of cue filter: channel mapping does not contain channel "
                                          "'time_scale' of type 'double'. If you dont want to change the speed you can link a constant with value 1", filter_type::filter_cue, own_id);
        }
        this->time_scale_input = input_channels.float_channels.at("time_scale");

        this->handle_end = HOLD;
        if (configuration.contains("end_handling")) {
            if (!configuration.at("end_handling").compare("start_again")) {
                this->handle_end = START_AGAIN;
            }
        }

        if (!configuration.contains("cuelist")) {
            throw filter_config_exception("cue filter: unable to setup the cuelist. Property 'cuelist' is missing.",
                                          filter_type::filter_cue, own_id);
        }

        const std::string frames = configuration.at("cuelist");
        this->cues.reserve(count_occurence_of(frames, "$", 0, frames.size()) + 1);
        if (!do_with_substr(frames, 0, frames.length(), '$', 1,
                            std::bind(&dmxfish::filters::filter_cue::handle_cue,
                                      this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                                      std::placeholders::_4))) {
            throw filter_config_exception("cue filter: unable to parse the cuelist", filter_type::filter_cue, own_id);
        }

        if(configuration.contains("default_cue")) {
            long new_default_cue;
            try {
                new_default_cue = stol(configuration.at("default_cue"));
            } catch (
                    const std::invalid_argument &ex
            ) {
                MARK_UNUSED(ex);
                throw filter_config_exception("cue filter: unable to setup the cuelist. Property 'default_cue' is not parseable as long.",
                                              filter_type::filter_cue, own_id);
            } catch (
                    const std::out_of_range &ex
            ) {
                MARK_UNUSED(ex);
                throw filter_config_exception("cue filter: unable to setup the cuelist. Property 'default_cue' was too large.",
                                              filter_type::filter_cue, own_id);
            }
            if (new_default_cue >= (long) this->cues.size()) {
                this->default_cue = -1;
            } else {
                this->default_cue = new_default_cue;
            }
        }
    }

    bool filter_cue::receive_update_from_gui(const std::string &key, const std::string &_value) {
        if (!key.compare("run_mode")) {
            if ((!_value.compare("play") || !_value.compare("to_next_cue")) && this->scale_valid) {
                switch (this->running_state) {
                    case STOP:
                        this->actual_values.cue = 0;
                        if (this->next_cue < this->cues.size()) { // if next cue is set, start this cue
                            this->actual_values.cue = this->next_cue;
                            this->next_cue = 0xffff;
                        }
                        start_new_cue();
                        break;
                    case PLAY:
                        switch (this->cues.at(this->actual_values.cue).restart_handling) {
                            case DO_NOTHING:
                                break;
                            case START_FROM_BEGIN:
                                start_new_cue();
                                break;
                            default:
                                return false;
                        }
                        break;
                    case PAUSE:
                        this->start_time = this->start_time + (*this->time - this->pause_time);
                        this->last_values.time_stamp = this->last_values.time_stamp + (*this->time - this->pause_time);
                        if(*this->time >= this->cues.at(this->actual_values.cue).timestamps.at(this->cues.at(this->actual_values.cue).timestamps.size() - 1) + this->start_time){ // start new cue if cue was finished
                            if (this->cues.at(this->actual_values.cue).end_handling == NEXT_CUE || this->cues.at(this->actual_values.cue).end_handling == HOLD){
                                if(this->actual_values.cue < this->cues.size()-1){
                                    this->actual_values.cue++;
                                } else {
                                    this->actual_values.cue = 0;
                                }
                                if (this->next_cue < this->cues.size()) { // if next cue is set, start this cue
                                    this->actual_values.cue = this->next_cue;
                                    this->next_cue = 0xffff;
                                }
                            }
                            start_new_cue();
                        }
                        break;
                    default:
                        return false;
                }
                this->running_state = PLAY;
                this->cue_end_handling_real = this->cues.at(this->actual_values.cue).end_handling;
                if (!_value.compare("to_next_cue")) {
                    this->cue_end_handling_real = HOLD;
                }
                update_parameter_gui();
                return true;
            }
            if (!_value.compare("pause")) {
                switch (this->running_state) {
                    case STOP:
                        return false;
                    case PLAY:
                        break;
                    case PAUSE:
                        return true;
                    default:
                        return false;
                }
                this->running_state = PAUSE;
                this->pause_time = *this->time;
                update_parameter_gui();
                return true;
            }
            if (!_value.compare("stop")) {
                this->running_state = STOP;
                update_parameter_gui();
                return true;
            }
        }
        if (!key.compare("run_cue") && this->time_scale > 0) {
            uint16_t next;
            try {
                next = (uint16_t) std::stoi(_value);
            } catch (
                    const std::invalid_argument &ex
            ) {
                MARK_UNUSED(ex);
                return false;
            } catch (
                    const std::out_of_range &ex
            ) {
                MARK_UNUSED(ex);
                return false;
            }
            if (next > this->cues.size()) {
                return false;
            }
            start_new_cue();
            this->actual_values.cue = next;
            this->running_state = PLAY;
            this->cue_end_handling_real = this->cues.at(this->actual_values.cue).end_handling;
            update_parameter_gui();
            return true;
        }
        if (!key.compare("next_cue")) {
            uint16_t next;
            try {
                next = (uint16_t) std::stoi(_value);
            } catch (
                    const std::invalid_argument &ex
            ) {
                MARK_UNUSED(ex);
                return false;
            } catch (
                    const std::out_of_range &ex
            ) {
                MARK_UNUSED(ex);
                return false;
            }
            if (next > this->cues.size()) {
                return false;
            }
            this->next_cue = next;
            return true;
        }
        if (!key.compare("set_default_cue")) {
            long new_default_cue;
            try {
                new_default_cue = std::stol(_value);
            } catch (
                    const std::invalid_argument &ex
            ) {
                MARK_UNUSED(ex);
                return false;
            } catch (
                    const std::out_of_range &ex
            ) {
                MARK_UNUSED(ex);
                return false;
            }
            if (new_default_cue >= (long) this->cues.size()) {
                this->default_cue = -1;
                ::spdlog::info("removed autoplay for this scene");
            } else {
                this->default_cue = new_default_cue;
            }
            return true;
        }
        return false;
    }

    void filter_cue::get_output_channels(channel_mapping &map, const std::string &name) {
        for (size_t i = 0; i < this->actual_values.eight_bit_channels.size(); i++) {
            map.eight_bit_channels[name + ":" + this->channel_names_eight.at(i)] = &this->actual_values.eight_bit_channels.at(i);
        }
        for (size_t i = 0; i < this->actual_values.sixteen_bit_channels.size(); i++) {
            map.sixteen_bit_channels[name + ":" + this->channel_names_sixteen.at(i)] = &this->actual_values.sixteen_bit_channels.at(i);
        }
        for (size_t i = 0; i < this->actual_values.float_channels.size(); i++) {
            map.float_channels[name + ":" + this->channel_names_float.at(i)] = &this->actual_values.float_channels.at(i);
        }
        for (size_t i = 0; i < this->actual_values.color_channels.size(); i++) {
            map.color_channels[name + ":" + this->channel_names_color.at(i)] = &this->actual_values.color_channels.at(i);
        }
    }

    void filter_cue::update() {
        switch (this->running_state) {
            case STOP: {
                break;
            }
            case PLAY: {
                calc_values();
                break;
            }
            case PAUSE: {
                break;
            }
            default: {
                calc_values();
                break;
            }
        }
        if (*this->time_scale_input != this->time_scale) {
            if(*this->time_scale_input <= 0 && this->scale_valid){
                spdlog::info("cue list paused cause scale factor is <=0");
                receive_update_from_gui("run_mode", "pause");
                this->scale_valid = false;
                return;
            } else if (*this->time_scale_input > 0) {
                if (!this->scale_valid) {
                    spdlog::info("cue list started again cause scale factor is >0");
                    this->scale_valid = true;
                    receive_update_from_gui("run_mode", "play");
                }
                this->start_time = *this->time - (*this->time - this->start_time) * this->time_scale / *this->time_scale_input;
                this->pause_time = *this->time - (*this->time - this->pause_time) * this->time_scale / *this->time_scale_input;
                this->last_values.time_stamp = *this->time - (*this->time - this->last_values.time_stamp) * this->time_scale / *this->time_scale_input;
                this->time_scale = *this->time_scale_input;
            }
        }
    }

    void filter_cue::scene_activated() {
        if (this->default_cue > -1) {
            this->running_state = PLAY;
            this->actual_values.cue = (uint16_t) this->default_cue;
            start_new_cue();
            ::spdlog::info("Switched to Cue {}.", this->default_cue);
        }
    }

}
