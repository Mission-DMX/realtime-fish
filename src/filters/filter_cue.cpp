#include "filters/filter_cue.hpp"

#include <cmath>
#include <string>

#include "lib/macros.hpp"
#include "lib/logging.hpp"
#include "dmx/pixel.hpp"


int count_occurence_of(const std::string &base_string, std::string pattern, size_t start, size_t end) {
    int occurrences = 0;
    while ((start = base_string.find(pattern, start)) != std::string::npos && start <= end) {
        ++occurrences;
        start += pattern.length();
    }
    return occurrences;
}

namespace dmxfish::filters {
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
        if (nr_channel >= channel.size()) {
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
            switch (channel.at(nr_channel).channel_type) {
                case EIGHT_BIT:
                    cues.at(cue).eight_bit_frames.push_back(
                            key_frame<uint8_t>(std::stoi(str.substr(start, sep - start)), tr));
                    break;
                case SIXTEEN_BIT:
                    cues.at(cue).sixteen_bit_frames.push_back(
                            key_frame<uint16_t>(std::stoi(str.substr(start, sep - start)), tr));
                    break;
                case FLOAT:
                    cues.at(cue).float_frames.push_back(
                            key_frame<double>(std::stod(str.substr(start, sep - start)), tr));
                    break;
                case COLOR:
                    cues.at(cue).color_frames.push_back(key_frame<dmxfish::dmx::pixel>(dmxfish::dmx::pixel(), tr));
                    const auto first_position = str.find(',', start);
                    cues.at(cue).color_frames.at(channel.at(nr_channel).index).value.hue = std::stod(
                            str.substr(start, first_position - start));
                    const auto second_position = str.find(",", first_position + 1);
                    cues.at(cue).color_frames.at(channel.at(nr_channel).index).value.saturation = std::stod(
                            str.substr(first_position + 1, second_position - first_position - 1));
                    cues.at(cue).color_frames.at(channel.at(nr_channel).index).value.iluminance = std::stod(
                            str.substr(second_position + 1, end - second_position - 1));
                    break;
//                    default:
//                        return false;
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

    bool
    filter_cue::handle_timestamps(size_t cue, const std::string &str, size_t start, size_t end, size_t nr_timestamp) {
        const size_t end_ts = str.find(':', start);
        try {
            cues.at(cue).timestamps.push_back(std::stod(str.substr(start, end_ts - start)) * 1000);
        } catch (const std::invalid_argument &ex) {
            MARK_UNUSED(ex);
            return false;
        } catch (const std::out_of_range &ex) {
            MARK_UNUSED(ex);
            return false;
        }
        return do_with_substr(str, end_ts + 1, end, '&', channel.size(),
                              std::bind(&dmxfish::filters::filter_cue::handle_frame, this, cue, std::placeholders::_1,
                                        std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    }

    bool filter_cue::handle_cue_conf(size_t cue, const std::string &str, size_t start, size_t end, size_t number) {
    
        if (!str.substr(start, end - start).compare("hold")) {
            cues.at(cue).end_handling = HOLD;
        } else if (!str.substr(start, end - start).compare("start_again")) {
            cues.at(cue).end_handling = START_AGAIN;
        } else if (!str.substr(start, end - start).compare("next_cue")) {
            cues.at(cue).end_handling = NEXT_CUE;
        } else if (!str.substr(start, end - start).compare("do_nothing")) {
            cues.at(cue).restart_handling = DO_NOTHING;
        } else if (!str.substr(start, end - start).compare("restart")) {
            cues.at(cue).restart_handling = START_FROM_BEGIN;
        } else {
            return false;
        }
        return true;
    }

    bool filter_cue::handle_cue(const std::string &str, size_t start, size_t end, size_t cue) {
        cues.push_back(cue_st());
        cues.at(cue).end_handling = NEXT_CUE;
        cues.at(cue).restart_handling = DO_NOTHING;
        const size_t end_conf = str.find('#', start);
        const size_t nr_of_ts = count_occurence_of(str, "|", start, end_conf) + 1;
        cues.at(cue).timestamps.reserve(nr_of_ts);
        cues.at(cue).eight_bit_frames.reserve(nr_of_ts * eight_bit_channels.size());
        cues.at(cue).sixteen_bit_frames.reserve(nr_of_ts * sixteen_bit_channels.size());
        cues.at(cue).float_frames.reserve(nr_of_ts * float_channels.size());
        cues.at(cue).color_frames.reserve(nr_of_ts * color_channels.size());
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
                    eight_bit_channels.at(ind) = (rel_time < 0.5) ? start_value : end_value;
                } else if constexpr(std::is_same<T, uint16_t>::value)
                {
                    sixteen_bit_channels.at(ind) = (rel_time < 0.5) ? start_value : end_value;
                } else if constexpr(std::is_same<T, double>::value)
                {
                    float_channels.at(ind) = (rel_time < 0.5) ? start_value : end_value;
                } else if constexpr(std::is_same<T, dmxfish::dmx::pixel>::value)
                {
                    color_channels.at(ind) = (rel_time < 0.5) ? start_value : end_value;
                }
                already_updated_act = false;
                return;
            case LINEAR:
                break;
            case SIGMOIDAL:
                rel_time = 1.0 / (1 + std::exp(6 - rel_time * 12));
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
            eight_bit_channels.at(ind) = (uint8_t) std::round((end_value - start_value) * rel_time + start_value);
        } else if constexpr(std::is_same<T, uint16_t>::value)
        {
            sixteen_bit_channels.at(ind) = (uint16_t) std::round((end_value - start_value) * rel_time + start_value);
        } else if constexpr(std::is_same<T, double>::value)
        {
            float_channels.at(ind) = (end_value - start_value) * rel_time + start_value;
        } else if constexpr(std::is_same<T, dmxfish::dmx::pixel>::value)
        {
            color_channels.at(ind) = dmxfish::dmx::pixel((end_value.hue - start_value.hue) * rel_time + start_value.hue,
                                                         (end_value.saturation - start_value.saturation) * rel_time +
                                                         start_value.saturation,
                                                         (end_value.iluminance - start_value.iluminance) * rel_time +
                                                         start_value.iluminance);
        }
        already_updated_act = false;
    }

    void filter_cue::update_hold_values() {
        if (!already_updated_act) {
            for (size_t i = 0; i < eight_bit_channels.size(); i++) {
                eight_bit_channels.at(i) = last_eight_bit_channels.at(i);
            }
            for (size_t i = 0; i < sixteen_bit_channels.size(); i++) {
                sixteen_bit_channels.at(i) = last_sixteen_bit_channels.at(i);
            }
            for (size_t i = 0; i < float_channels.size(); i++) {
                float_channels.at(i) = last_float_channels.at(i);
            }
            for (size_t i = 0; i < color_channels.size(); i++) {
                color_channels.at(i) = last_color_channels.at(i);
            }
            already_updated_act = true;
        }
    }

    void filter_cue::update_last_values() {
        if (!already_updated_last) {
            for (size_t i = 0; i < eight_bit_channels.size(); i++) {
                last_eight_bit_channels.at(i) = eight_bit_channels.at(i);
            }
            for (size_t i = 0; i < sixteen_bit_channels.size(); i++) {
                last_sixteen_bit_channels.at(i) = sixteen_bit_channels.at(i);
            }
            for (size_t i = 0; i < float_channels.size(); i++) {
                last_float_channels.at(i) = float_channels.at(i);
            }
            for (size_t i = 0; i < color_channels.size(); i++) {
                last_color_channels.at(i) = color_channels.at(i);
            }
            already_updated_last = true;
        }
    }

    void filter_cue::calc_values() {
        if (*time >= cues.at(active_cue).timestamps.at(frame) + start_time) { // Next Frame?
            if (!already_updated_last) {
                for (size_t i = 0; i < eight_bit_channels.size(); i++) {
                    last_eight_bit_channels.at(i) = cues.at(active_cue).eight_bit_frames.at(
                            eight_bit_channels.size() * frame + i).value;
                }
                for (size_t i = 0; i < sixteen_bit_channels.size(); i++) {
                    last_sixteen_bit_channels.at(i) = cues.at(active_cue).sixteen_bit_frames.at(
                            sixteen_bit_channels.size() * frame + i).value;
                }
                for (size_t i = 0; i < float_channels.size(); i++) {
                    last_float_channels.at(i) = cues.at(active_cue).float_frames.at(
                            float_channels.size() * frame + i).value;
                }
                for (size_t i = 0; i < color_channels.size(); i++) {
                    last_color_channels.at(i) = cues.at(active_cue).color_frames.at(
                            color_channels.size() * frame + i).value;
                }
                already_updated_last = true;
            }
            last_timestamp = *time;
            if (frame < cues.at(active_cue).timestamps.size() - 1) { // Not the last Frame of the cue?
                frame++;
            } else { // last frame of cue
//                ::spdlog::debug("now the val is: {} at {}", cue_end_handling_real, *time);
                switch (cue_end_handling_real) { // end of cue handling
                    case START_AGAIN:
                        break;
                    case HOLD:
                        update_hold_values();
                        pause_time = *time;
                        running_state = PAUSE;
                        cue_end_handling_real = HOLDING;
                        return;
                    case HOLDING:
                        return;
                    case NEXT_CUE:
                        if (active_cue < cues.size() - 1) { // Not last cue?
                            active_cue++;
                        } else {
                            switch (handle_end) { // end of cuelist handling
                                case START_AGAIN:
                                    active_cue = 0;
                                    start_time = *time;
                                    start_time = *time;
                                    frame = 0;
                                    break;
                                case HOLD:
                                    update_hold_values();
                                    return;
                                case NEXT_CUE:
                                    ::spdlog::warn("should not have reached NEXT CUE at the end of the cuelist");
                                    return;
                                default:
                                    ::spdlog::warn("should not have reached default end_handling at the end of the cuelist");
                                    return;
                            }
                        }
                        break;
                    default:
                        ::spdlog::warn("should not have reached default end_handling at the end of the cue");
                        return;
                }
                if (next_cue < cues.size()) { // if next cue is set, start this cue
                    active_cue = next_cue;
                    next_cue = 0xffff;
                }
                start_time = *time;
                frame = 0;
                cue_end_handling_real = cues.at(active_cue).end_handling;
            }
        }
        already_updated_last = false;

        double rel_time =
                (*time - last_timestamp) / (cues.at(active_cue).timestamps.at(frame) - last_timestamp + start_time);

        for (size_t i = 0; i < eight_bit_channels.size(); i++) {
            size_t frame_index = frame * eight_bit_channels.size() + i;
            calc_transition<uint8_t>(rel_time, cues.at(active_cue).eight_bit_frames.at(frame_index).transition, last_eight_bit_channels.at(i),
                                     cues.at(active_cue).eight_bit_frames.at(frame_index).value, i);
        }
        for (size_t i = 0; i < sixteen_bit_channels.size(); i++) {
            size_t frame_index = frame * sixteen_bit_channels.size() + i;
            calc_transition<uint16_t>(rel_time, cues.at(active_cue).sixteen_bit_frames.at(frame_index).transition, last_sixteen_bit_channels.at(i),
                                      cues.at(active_cue).sixteen_bit_frames.at(frame_index).value, i);
        }
        for (size_t i = 0; i < float_channels.size(); i++) {
            size_t frame_index = frame * float_channels.size() + i;
            calc_transition<double>(rel_time, cues.at(active_cue).float_frames.at(frame_index).transition, last_float_channels.at(i),
                                    cues.at(active_cue).float_frames.at(frame_index).value, i);
        }
        for (size_t i = 0; i < color_channels.size(); i++) {
            size_t frame_index = frame * color_channels.size() + i;
            calc_transition<dmxfish::dmx::pixel>(rel_time, cues.at(active_cue).color_frames.at(frame_index).transition, last_color_channels.at(i),
                                                 cues.at(active_cue).color_frames.at(frame_index).value, i);
        }
    }


    void filter_cue::setup_filter(const std::map <std::string, std::string> &configuration,
                                  const std::map <std::string, std::string> &initial_parameters,
                                  const channel_mapping &input_channels) {
      
        if (already_setup_filter_called){
            if (!input_channels.float_channels.contains("time")) {
                throw filter_config_exception(
                        "Unable to link input of cue filter: channel mapping does not contain channel 'time' of type 'double'. This input should come from the scenes global time node.");
            }
            this->time = input_channels.float_channels.at("time");
            return;
        }
        already_setup_filter_called = true;

        if (!initial_parameters.contains("mapping")) {
            throw filter_config_exception("cue filter: unable to setup the mapping");
        }

        std::string mapping = initial_parameters.at("mapping");
        ::spdlog::debug("setup_filter: mapping: {}", mapping);
        size_t start_pos = 0;
        auto next_pos = mapping.find(";");
        int count_channel_type = count_occurence_of(mapping, ":8bit", 0, mapping.size());
        channel_names_eight.reserve(count_channel_type);
        eight_bit_channels.reserve(count_channel_type);
        last_eight_bit_channels.reserve(count_channel_type);
        count_channel_type = count_occurence_of(mapping, ":16bit", 0, mapping.size());
        channel_names_sixteen.reserve(count_channel_type);
        sixteen_bit_channels.reserve(count_channel_type);
        last_sixteen_bit_channels.reserve(count_channel_type);
        count_channel_type = count_occurence_of(mapping, ":float", 0, mapping.size());
        channel_names_float.reserve(count_channel_type);
        float_channels.reserve(count_channel_type);
        last_float_channels.reserve(count_channel_type);
        count_channel_type = count_occurence_of(mapping, ":color", 0, mapping.size());
        channel_names_color.reserve(count_channel_type);
        color_channels.reserve(count_channel_type);
        last_color_channels.reserve(count_channel_type);
        while (true) {
            const auto sign = mapping.find(":", start_pos);
            
            std::string channel_type = mapping.substr(sign + 1, next_pos - sign - 1);
            std::string channel_name = mapping.substr(start_pos, sign - start_pos);
            if (!channel_type.compare("8bit")) {
                channel.push_back(channel_str(EIGHT_BIT, eight_bit_channels.size()));
                channel_names_eight.push_back(channel_name);
                eight_bit_channels.push_back(0);
                last_eight_bit_channels.push_back(0);
            } else if (!channel_type.compare("16bit")) {
                channel.push_back(channel_str(SIXTEEN_BIT, sixteen_bit_channels.size()));
                channel_names_sixteen.push_back(channel_name);
                sixteen_bit_channels.push_back(0);
                last_sixteen_bit_channels.push_back(0);
            } else if (!channel_type.compare("float")) {
                channel.push_back(channel_str(FLOAT, float_channels.size()));
                channel_names_float.push_back(channel_name);
                float_channels.push_back(0);
                last_float_channels.push_back(0);
            } else if (!channel_type.compare("color")) {
                channel.push_back(channel_str(COLOR, color_channels.size()));
                channel_names_color.push_back(channel_name);
                color_channels.push_back(dmxfish::dmx::pixel());
                last_color_channels.push_back(dmxfish::dmx::pixel());
            } else {
                throw filter_config_exception(std::string("can not recognise channel type: ") + mapping.substr(sign + 1, next_pos - sign - 1));
            }

            if (next_pos >= mapping.length()) {
                break;
            }
            start_pos = next_pos + 1;
            next_pos = mapping.find(";", start_pos);
        }


        this->handle_end = HOLD;
        if (initial_parameters.contains("end_handling")) {
            if (!initial_parameters.at("end_handling").compare("start_again")) {
                this->handle_end = START_AGAIN;
            }
        }

        if (!initial_parameters.contains("cuelist")) {
            throw filter_config_exception("cue filter: unable to setup the cuelist");
        }

        const std::string frames = initial_parameters.at("cuelist");
        cues.reserve(count_occurence_of(frames, "$", 0, frames.size()) + 1);
        if (!do_with_substr(frames, 0, frames.length(), '$', 1,
                               std::bind(&dmxfish::filters::filter_cue::handle_cue,
                                         this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                                         std::placeholders::_4))) {
            throw filter_config_exception("cue filter: unable to parse the cuelist");
        }
    }

    bool filter_cue::receive_update_from_gui(const std::string &key, const std::string &_value) {
        if (!key.compare("run_mode")) {
            if (!_value.compare("play") || !_value.compare("to_next_cue")) {
//                if (cue_end_handling_real == HOLDING){
//                    if (cues.at(active_cue).end_handling == HOLD) {
//                        cue_end_handling_real = NEXT_CUE;
//                    } else {
//                        cue_end_handling_real = cues.at(active_cue).end_handling;
//                    }
//                    return true;
//                } else {
//                    cue_end_handling_real = cues.at(active_cue).end_handling;
//                }
                switch (running_state) {
                    case STOP:
                        update_last_values();
                        active_cue = 0;
                        if (next_cue < cues.size()) { // if next cue is set, start this cue
                            active_cue = next_cue;
                            next_cue = 0xffff;
                        }
                        start_time = *time;
                        last_timestamp = *time;
                        frame = 0;
                        break;
                    case PLAY:
                        switch (cues.at(active_cue).restart_handling) {
                            case DO_NOTHING:
                                break;
                            case START_FROM_BEGIN:
                                update_last_values();
                                start_time = *time;
                                last_timestamp = *time;
                                frame = 0;
                                break;
                            default:
                                return false;
                        }
                        break;
                    case PAUSE:
                        start_time = start_time + (*time - pause_time);
                        last_timestamp = last_timestamp + (*time - pause_time);
                        if(*time >= cues.at(active_cue).timestamps.at(cues.at(active_cue).timestamps.size() - 1) + start_time){
                            if (cues.at(active_cue).end_handling == NEXT_CUE || cues.at(active_cue).end_handling == HOLD){
                                if(active_cue < cues.size()-1){
                                    active_cue++;
                                } else {
                                    active_cue = 0;

                                }
                                if (next_cue < cues.size()) { // if next cue is set, start this cue
                                    active_cue = next_cue;
                                    next_cue = 0xffff;
                                }
                            }
                            start_time = *time;
                            last_timestamp = *time;
                            frame = 0;
                        }
                        break;
                    default:
                        return false;
                }
                running_state = PLAY;
                cue_end_handling_real = cues.at(active_cue).end_handling;
                if (!_value.compare("to_next_cue")) {
                    cue_end_handling_real = HOLD;
                }
                return true;
            }
            if (!_value.compare("pause")) {
                switch (running_state) {
                    case STOP:
                        return false;
                    case PLAY:
                        break;
                    case PAUSE:
                        return true;
                    default:
                        return false;
                }
                running_state = PAUSE;
                pause_time = *time;
                return true;
            }
            if (!_value.compare("stop")) {
                running_state = STOP;
                return true;
            }
        }
        if (!key.compare("run_cue")) {
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
            if (next > cues.size()) {
                return false;
            }
            start_time = *time;
            frame = 0;
            active_cue = next;
            for (size_t i = 0; i < eight_bit_channels.size();i++) {
                last_eight_bit_channels.
                        at(i) = eight_bit_channels.at(i);
            }
            for (size_t i = 0; i < sixteen_bit_channels.size(); i++) {
                last_sixteen_bit_channels.
                        at(i) = sixteen_bit_channels.at(i);
            }
            for (size_t i = 0; i < float_channels.size(); i++) {
                last_float_channels.
                        at(i) = float_channels.at(i);
            }
            for (size_t i = 0; i < color_channels.size(); i++) {
                last_color_channels.at(i) = color_channels.at(i);
            }
            last_timestamp = *time;
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
            if (next > cues.size()) {
                return false;
            }
            next_cue = next;
            return true;
        }
        return false;
    }

    void filter_cue::get_output_channels(channel_mapping &map, const std::string &name) {
        ::spdlog::debug("Cue filter get outputchannels: 8bit: {}, 16bit:{}, fl:{}, col:{}", eight_bit_channels.size(), sixteen_bit_channels.size(), float_channels.size(), color_channels.size());
        for (size_t i = 0; i < eight_bit_channels.size(); i++) {
            map.eight_bit_channels[name + ":" + channel_names_eight.at(i)] = &eight_bit_channels.at(i);
        }
        for (size_t i = 0; i < sixteen_bit_channels.size(); i++) {
            map.sixteen_bit_channels[name + ":" + channel_names_sixteen.at(i)] = &sixteen_bit_channels.at(i);
        }
        for (size_t i = 0; i < float_channels.size(); i++) {
            map.float_channels[name + ":" + channel_names_float.at(i)] = &float_channels.at(i);
        }
        for (size_t i = 0; i < color_channels.size(); i++) {
            map.color_channels[name + ":" + channel_names_color.at(i)] = &color_channels.at(i);
        }
    }

    void filter_cue::update() {
        switch (running_state) {
            case STOP: {
                return;
            }
            case PLAY: {
                break;
            }
            case PAUSE: {
                return;
            }
            default: {
                break;
            }
        }

        calc_values();

    }

    void filter_cue::scene_activated() {
    }

}
