#pragma once

/*
 * The filters hold a cue sequence
 */
#include <cmath>
#include <string>
#include <vector>

#include "filters/filter.hpp"
#include "lib/macros.hpp"
#include "dmx/pixel.hpp"


int count_occurence_of(std::string& base_string, std::string pattern){
    int occurrences = 0;
    std::string::size_type start = 0;

    while ((start = base_string.find(pattern, start)) != std::string::npos) {
        ++occurrences;
        start += pattern.length();
    }
    return occurrences;
}

namespace dmxfish::filters {
    COMPILER_SUPRESS("-Weffc++")


    class filter_cue: public filter {
    private:
        enum transition_t{
            EDGE,
            LINEAR,
            SIGMOIDAL,
            EASE_IN,
            EASE_OUT
        };
        enum handling_at_the_end{
            HOLD,
            START_AGAIN,
            NEXT_CUE
        };
        enum handling_at_restart{
            DO_NOTHING,
            START_FROM_BEGIN
        };
        enum run_state{
            STOP,
            PLAY,
            PAUSE,
            TO_NEXT_CUE
        };
        enum channel_t{
            EIGHT_Bit,
            SIXTEEN_BIT,
            FLOAT,
            COLOR
        };


        template <typename T>
        struct KeyFrame{
            T value;
            transition_t transition;
        };

        struct Cue{
            std::vector<double> timestamps;
            std::vector<KeyFrame<uint8_t>> eight_bit_frames;
            std::vector<KeyFrame<uint16_t>> sixteen_bit_frames;
            std::vector<KeyFrame<double>> float_frames;
            std::vector<KeyFrame<dmxfish::dmx::pixel>> color_frames;
            handling_at_the_end end_handling;
            handling_at_restart restart_handling;

        };

        double* time = nullptr;
        double start_time = 0;
        double pause_time = 0;
        uint16_t frame = 0;
        bool already_updated = false;
        uint16_t active_cue = 0;
        uint16_t next_cue = 0xffff;
        handling_at_the_end handle_end;
        run_state running_state = STOP;
        std::vector<Cue> cues;

        std::vector<uint8_t> eight_bit_channels;
        std::vector<uint16_t> sixteen_bit_channels;
        std::vector<double> float_channels;
        std::vector<dmxfish::dmx::pixel> color_channels;

        double last_timestamp = 0;
        std::vector<uint8_t> last_eight_bit_channels;
        std::vector<uint16_t> last_sixteen_bit_channels;
        std::vector<double> last_float_channels;
        std::vector<dmxfish::dmx::pixel> last_color_channels;

        std::vector<std::string> channel_names_eight;
        std::vector<std::string> channel_names_sixteen;
        std::vector<std::string> channel_names_float;
        std::vector<std::string> channel_names_color;

    public:
        filter_cue() : filter() {}
        virtual ~filter_cue() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            if(!input_channels.float_channels.contains("time")) {
                throw filter_config_exception("Unable to link input of cue filter: channel mapping does not contain channel 'time' of type 'double'");
            }
            this->time = input_channels.float_channels.at("time");

            if(!configuration.contains("mapping")){
                throw filter_config_exception("cue filter: unable to setup the mapping");
            }

            std::string mapping = configuration.at("mapping");
            size_t start_pos = 0;
            auto next_pos = mapping.find(";");
            int count_channel_type = count_occurence_of(mapping, ":8bit");
            channel_names_eight.reserve(count_channel_type);
            eight_bit_channels.reserve(count_channel_type);
            last_eight_bit_channels.reserve(count_channel_type);
            count_channel_type = count_occurence_of(mapping, ":16bit");
            channel_names_sixteen.reserve(count_channel_type);
            sixteen_bit_channels.reserve(count_channel_type);
            last_sixteen_bit_channels.reserve(count_channel_type);
            count_channel_type = count_occurence_of(mapping, ":float");
            channel_names_float.reserve(count_channel_type);
            float_channels.reserve(count_channel_type);
            last_float_channels.reserve(count_channel_type);
            count_channel_type = count_occurence_of(mapping, ":color");
            channel_names_color.reserve(count_channel_type);
            color_channels.reserve(count_channel_type);
            last_color_channels.reserve(count_channel_type);
            while(true){
                const auto sign = mapping.find(":", start_pos);
                std::string channel_type = mapping.substr(sign+1, next_pos-sign-1);
                std::string channel_name = mapping.substr(start_pos, sign-start_pos);
                if (channel_type.compare("8bit") == 0){
                    channel_names_eight.push_back(channel_name);
                    eight_bit_channels.push_back(0);
                    last_eight_bit_channels.push_back(0);
                } else if(channel_type.compare("16bit") == 0){
                    channel_names_sixteen.push_back(channel_name);
                    sixteen_bit_channels.push_back(0);
                    last_sixteen_bit_channels.push_back(0);
                } else if(channel_type.compare("float") == 0){
                    channel_names_float.push_back(channel_name);
                    float_channels.push_back(0);
                    last_float_channels.push_back(0);
                } else if(channel_type.compare("color") == 0){
                    channel_names_color.push_back(channel_name);
                    color_channels.push_back(dmxfish::dmx::pixel());
                    last_color_channels.push_back(dmxfish::dmx::pixel());
                } else {
                    throw filter_config_exception(std::string("can not recognise channel type: ") + mapping.substr(sign, next_pos-sign));
                }

                if (next_pos >= mapping.length()){
                    break;
                }
                start_pos = next_pos + 1;
                next_pos = mapping.find(";", start_pos);
            }

            if(!initial_parameters.contains("frames")){
                throw filter_config_exception("cue filter: unable to setup the frames");
            }

            std::string frames = configuration.at("frames");

//            int frames_count = std::count(frames.begin(), frames.end(), ':');
//            eight_bit_frames.reserve(frames_count*eight_bit_channels.size());
//            sixteen_bit_frames.reserve(frames_count*sixteen_bit_channels.size());
//            float_frames.reserve(frames_count*float_channels.size());
//            color_frames.reserve(frames_count*color_channels.size());
//            size_t start_pos_channel = 0;
//            auto next_pos_channel = frames.find(";");
//            while(true){
//                const auto sign = frames.find(":", start_pos_channel);
//                std::string timestamp = frames.substr(start_pos_channel, sign-start_pos_channel);
//                std::string frame_channels = frames.substr(sign+1, next_pos_channel-sign-1);
//                size_t start_pos_frame = 0;
//                auto next_pos_frame = frame_channels.find("|");
//                try {
//                    timestamps.push_back(std::stod(timestamp));
//
//                } catch (const std::invalid_argument& ex) {
//                    MARK_UNUSED(ex);
//                    throw filter_config_exception("cue filter: cant parse timestamp");
//                } catch (const std::out_of_range& ex) {
//                    MARK_UNUSED(ex);
//                    throw filter_config_exception("cue filter: cant parse timestamp ");
//                }
//                size_t i = 0;
//                while(true) {
//                    try {
//                        const auto sign_transition = frame_channels.find("@", start_pos_frame);
//                        std::string tran = frame_channels.substr(sign_transition+1, next_pos_frame-sign_transition-1);
//                        transition_t transition;
//                        if(tran.compare("lin")==0){
//                            transition = LINEAR;
//                        } else if(tran.compare("edge")==0){
//                            transition = EDGE;
//                        } else if(tran.compare("sigm")==0){
//                            transition = SIGMOIDAL;
//                        } else if(tran.compare("e_in")==0){
//                            transition = EASE_IN;
//                        } else if(tran.compare("e_out")==0){
//                            transition = EASE_OUT;
//                        } else {
//                            throw filter_config_exception("cue filter: could not resolve transition type");
//                        }
//
//                        if (i < eight_bit_channels.size()) {
//                            eight_bit_frames.push_back(KeyFrame<uint8_t>(std::stoi(
//                                        frame_channels.substr(start_pos_frame, next_pos_frame - start_pos_frame)),
//                                    transition));
//                        } else if (i < eight_bit_channels.size() + sixteen_bit_channels.size()) {
//                            sixteen_bit_frames.push_back(KeyFrame<uint16_t>(std::stoi(
//                                        frame_channels.substr(start_pos_frame, next_pos_frame - start_pos_frame)),
//                                     transition));
//                        } else if (i <
//                                   eight_bit_channels.size() + sixteen_bit_channels.size() + float_channels.size()) {
//                            float_frames.push_back(KeyFrame<double>(std::stod(
//                                        frame_channels.substr(start_pos_frame, next_pos_frame - start_pos_frame)),
//                                    transition));
//                        } else if (i < eight_bit_channels.size() + sixteen_bit_channels.size() + float_channels.size() +
//                                       color_channels.size()) {
//                            const auto first_position = frame_channels.find(",", start_pos_frame);
//                            double hue = std::stod(
//                                    frame_channels.substr(start_pos_frame, first_position - start_pos_frame));
//                            const auto second_position = frame_channels.find(",", first_position + 1);
//                            double saturation = std::stod(
//                                    frame_channels.substr(first_position + 1, second_position - first_position - 1));
//                            double iluminance = std::stod(
//                                    frame_channels.substr(second_position + 1, next_pos_frame - second_position - 1));
//                            color_frames.push_back(KeyFrame<dmxfish::dmx::pixel>(dmxfish::dmx::pixel(hue, saturation, iluminance),
//                                                   transition));
//                        } else {
//                            throw filter_config_exception("cue filter: too many channels for a frame");
//                        }
//                    }
//                    catch (const std::invalid_argument& ex) {
//                        MARK_UNUSED(ex);
//                        throw filter_config_exception("cue filter: can not parse the value");
//                    } catch (const std::out_of_range& ex) {
//                        MARK_UNUSED(ex);
//                        throw filter_config_exception("cue filter: can not parse the value ");
//                    }
//                    i++;
//
//                    if (next_pos_frame >= frame_channels.length()) {
//                        if (i < eight_bit_channels.size() + sixteen_bit_channels.size() + float_channels.size() + color_channels.size()){
//                            throw filter_config_exception("cue filter: too less channels for a frame");
//                        }
//                        break;
//                    }
//                    start_pos_frame = next_pos_frame + 1;
//                    next_pos_frame = frame_channels.find("|", start_pos_frame);
//                }
//                if (next_pos_channel >= frames.length()){
//                    break;
//                }
//                start_pos_channel = next_pos_channel + 1;
//                next_pos_channel = frames.find(";", start_pos_channel);
//            }
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            if (key.compare("run_mode") == 0){
                if (_value.compare("play") == 0){
                    switch (running_state) {
                        case STOP:
                            active_cue = 0;
                            start_time = *time;
                            frame = 0;
                            break;
                        case PLAY:
                            break;
                        case PAUSE:
                            start_time = start_time + (*time-pause_time);
                            return true;
                        case TO_NEXT_CUE:
                            break;
                        default:
                            return false;
                    }
                    switch (cues.at(active_cue).restart_handling) {
                        case DO_NOTHING:
                            break;
                        case START_FROM_BEGIN:
                            start_time = *time;
                            frame = 0;
                            break;
                        default:
                            return false;
                    }
                    switch (cues.at(active_cue).end_handling) {
                        case HOLD:
                            running_state = TO_NEXT_CUE;
                            break;
                        case START_AGAIN:
                            running_state = PLAY;
                            break;
                        case NEXT_CUE:
                            running_state = PLAY;
                            break;
                        default:
                            break;
                    }
                    return true;
                }
                if (_value.compare("pause") == 0){
                    switch (running_state) {
                        case STOP:
                            return false;
                        case PLAY:
                            break;
                        case PAUSE:
                            return true;
                        case TO_NEXT_CUE:
                            break;
                        default:
                            return false;
                    }
                    running_state = PAUSE;
                    pause_time = *time;
                    return true;
                }
                if (_value.compare("to_next_cue") == 0){
                    switch (running_state) {
                        case STOP:
                            start_time = *time;
                            frame = 0;
                            break;
                        case PLAY:
                            break;
                        case PAUSE:
                            start_time = start_time + (*time-pause_time);
                            return true;
                        case TO_NEXT_CUE:
                            break;
                        default:
                            return false;
                    }
                    switch (cues.at(active_cue).restart_handling) {
                        case DO_NOTHING:
                            break;
                        case START_FROM_BEGIN:
                            start_time = *time;
                            frame = 0;
                            break;
                        default:
                            return false;
                    }
                    running_state = TO_NEXT_CUE;
                    return true;
                }
                if (_value.compare("stop")==0){
                    running_state = STOP;
                    return true;
                }
            }
            if (key.compare("run_cue") == 0){
                uint16_t next;
                try {
                    next = (uint16_t) std::stoi(_value);
                } catch (const std::invalid_argument& ex) {
                    MARK_UNUSED(ex);
                    return false;
                } catch (const std::out_of_range& ex) {
                    MARK_UNUSED(ex);
                    return false;
                }
                if (next > cues.size()){
                    return false;
                }
                start_time = *time;
                frame = 0;
                active_cue = next;
                for(size_t i = 0; i<eight_bit_channels.size(); i++){
                    last_eight_bit_channels.at(i) = eight_bit_channels.at(i);
                }
                for(size_t i = 0; i<sixteen_bit_channels.size(); i++){
                    last_sixteen_bit_channels.at(i) = sixteen_bit_channels.at(i);
                }
                for(size_t i = 0; i<float_channels.size(); i++){
                    last_float_channels.at(i) = float_channels.at(i);
                }
                for(size_t i = 0; i<color_channels.size(); i++){
                    last_color_channels.at(i) = color_channels.at(i);
                }
                return true;
            }
            if (key.compare("next_cue") == 0){
                // Todo handling next cue not implemented yet
                uint16_t next;
                try {
                    next = (uint16_t) std::stoi(_value);
                } catch (const std::invalid_argument& ex) {
                    MARK_UNUSED(ex);
                    return false;
                } catch (const std::out_of_range& ex) {
                    MARK_UNUSED(ex);
                    return false;
                }
                if (next > cues.size()){
                    return false;
                }
                next_cue = next;
                return true;
            }
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            for(size_t i = 0; i<eight_bit_channels.size(); i++){
                map.eight_bit_channels[name + ":" + channel_names_eight.at(i)] = &eight_bit_channels.at(i);
            }
            for(size_t i = 0; i<sixteen_bit_channels.size(); i++){
                map.sixteen_bit_channels[name + ":" + channel_names_sixteen.at(i)] = &sixteen_bit_channels.at(i);
            }
            for(size_t i = 0; i<float_channels.size(); i++){
                map.float_channels[name + ":" + channel_names_float.at(i)] = &float_channels.at(i);
            }
            for(size_t i = 0; i<color_channels.size(); i++){
                map.color_channels[name + ":" + channel_names_color.at(i)] = &color_channels.at(i);
            }
        }

        template <typename T>
        T calc_transition(double rel_time, transition_t transition, T start_value, T end_value){
            switch (transition) {
                case EDGE:
                    if (rel_time<0.5){
                        return start_value;
                    } else{
                        return end_value;
                    }
                case LINEAR:
                    break;
                case SIGMOIDAL:
                    rel_time = 1.0-1.0/(1 + std::exp(rel_time*12-6));
                    break;
                case EASE_IN:
                    rel_time = rel_time * rel_time;
                    break;
                case EASE_OUT:
                    rel_time = 1 - ((1-rel_time) * (1-rel_time));
                    break;
                default:
                    ::spdlog::warn("should not have reached default transition type");
                    break;
            }
            if constexpr (std::is_same<T, uint8_t>::value){
                return (uint8_t) std::round((end_value - start_value) * rel_time + start_value);
            } else if constexpr (std::is_same<T, uint16_t>::value){
                return (uint16_t) std::round((end_value - start_value) * rel_time + start_value);
            } else if constexpr (std::is_same<T, double>::value){
                return (end_value - start_value) * rel_time + start_value;
            } else if constexpr (std::is_same<T, dmxfish::dmx::pixel>::value){
                return dmxfish::dmx::pixel((end_value.hue - start_value.hue) * rel_time + start_value.hue, (end_value.saturation - start_value.saturation) * rel_time + start_value.saturation, (end_value.iluminance - start_value.iluminance) * rel_time + start_value.iluminance);
            }
        }

        virtual void calc_values(){
            if (*time >= cues.at(active_cue).timestamps.at(frame)){ // Next Frame?
                if (!already_updated) {
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
                    already_updated = true;
                }
                if(frame < cues.at(active_cue).timestamps.size() - 1){ // Not the last Frame of the cue?
                    frame++;
                } else { // last frame of cue
                    switch (running_state) {
                        case PLAY:
                            switch (cues.at(active_cue).end_handling) { // end of cue handling
                                case START_AGAIN:
                                    break;
                                case HOLD:
                                    return;
                                case NEXT_CUE:
                                    if (active_cue < cues.size()-1) { // Not last cue?
                                        active_cue++;
                                    } else {
                                        switch (handle_end) { // end of cuelist handling
                                            case START_AGAIN:
                                                active_cue = 0;
                                                start_time = *time;
                                                frame = 0;
                                                break;
                                            case HOLD:
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
                            if (next_cue < cues.size()){ // if next cue is set, start this cue
                                active_cue = next_cue;
                                next_cue = 0xffff;
                            }
                            start_time = *time;
                            frame = 0;
                            break;
                        case TO_NEXT_CUE:
                            return;
                        case STOP:
                            ::spdlog::warn("should not have reached STOP running mode when calculating values");
                            return;
                        case PAUSE:
                            ::spdlog::warn("should not have reached PAUSE running mode when calculating values");
                            return;
                        default:
                            ::spdlog::warn("should not have reached default running mode when calculating values");
                            return;
                    }
                }
            }

            already_updated = false;

            double rel_time = (*time-last_timestamp)/(cues.at(active_cue).timestamps.at(frame)-last_timestamp);

            for(size_t i = 0; i < eight_bit_channels.size(); i++){
                size_t frame_index = frame*eight_bit_channels.size()+i;
                eight_bit_channels.at(i) = calc_transition<uint8_t>(rel_time, cues.at(active_cue).eight_bit_frames.at(frame_index).transition, last_eight_bit_channels.at(i), cues.at(active_cue).eight_bit_frames.at(frame_index).value);
            }
            for(size_t i = 0; i < sixteen_bit_channels.size(); i++){
                size_t frame_index = frame*sixteen_bit_channels.size()+i;
                sixteen_bit_channels.at(i) = calc_transition<uint16_t>(rel_time, cues.at(active_cue).sixteen_bit_frames.at(frame_index).transition, last_sixteen_bit_channels.at(i), cues.at(active_cue).sixteen_bit_frames.at(frame_index).value);
            }
            for(size_t i = 0; i < float_channels.size(); i++){
                size_t frame_index = frame*float_channels.size()+i;
                float_channels.at(i) = calc_transition<double>(rel_time, cues.at(active_cue).float_frames.at(frame_index).transition, last_float_channels.at(i), cues.at(active_cue).float_frames.at(frame_index).value);
            }
            for(size_t i = 0; i < color_channels.size(); i++){
                size_t frame_index = frame*color_channels.size()+i;
                color_channels.at(i) = calc_transition<dmxfish::dmx::pixel>(rel_time, cues.at(active_cue).color_frames.at(frame_index).transition, last_color_channels.at(i), cues.at(active_cue).color_frames.at(frame_index).value);
            }
        }

        virtual void update() override {
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
                case TO_NEXT_CUE:{
                    break;
                }
                default: {
                }
                calc_values();
            }
        }

        virtual void scene_activated() override {}

    };

    COMPILER_RESTORE("-Weffc++")

}