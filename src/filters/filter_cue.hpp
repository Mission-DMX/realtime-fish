#pragma once

/*
 * The filters hold a cue sequence
 */

#include "string"
#include "vector"

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
            NOTHING,
            RESTART
        };
//        enum run_state{
//            PLAY = 1,
//            PAUSE = 2,
//            STOP = 0,
//            TO_NEXT_FRAME = 3
//        };
//        enum channel_t{
//            EIGHT_Bit,
//            SIXTEEN_BIT,
//            FLOAT,
//            COLOR
//        };

        template <typename T>
        struct KeyFrame{
            T value;
            transition_t transition;
//            KeyFrame(T val, transition_t trans) : value(val), transition(trans) {}
        };

        uint8_t* running_state = nullptr;
        double* time = nullptr;
        double start_time = 0;

        std::vector<double> timestamps;

        std::vector<uint8_t> eight_bit_channels;
        std::vector<uint16_t> sixteen_bit_channels;
        std::vector<double> float_channels;
        std::vector<dmxfish::dmx::pixel> color_channels;

        std::vector<KeyFrame<uint8_t>> eight_bit_frames;
        std::vector<KeyFrame<uint16_t>> sixteen_bit_frames;
        std::vector<KeyFrame<double>> float_frames;
        std::vector<KeyFrame<dmxfish::dmx::pixel>> color_frames;

        std::vector<std::string> channel_names_eight;
        std::vector<std::string> channel_names_sixteen;
        std::vector<std::string> channel_names_float;
        std::vector<std::string> channel_names_color;

    public:
        filter_cue() : filter() {}
        virtual ~filter_cue() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
            if(!input_channels.float_channels.contains("time") || !input_channels.eight_bit_channels.contains("running_state")) {
                throw filter_config_exception("Unable to link input of cue filter: channel mapping does not contain channel 'time' of type 'double' or 'running_state' of type 8bit");
            }
            this->time = input_channels.float_channels.at("time");
            this->running_state = input_channels.eight_bit_channels.at("running_state");

            if(!configuration.contains("mapping")){
                throw filter_config_exception("cue filter: unable to setup the mapping");
            }

            std::string mapping = configuration.at("mapping");
            size_t start_pos = 0;
            auto next_pos = mapping.find(";");
            int count_channel_type = count_occurence_of(mapping, "8bit");
            channel_names_eight.reserve(count_channel_type);
            eight_bit_channels.reserve(count_channel_type);
            count_channel_type = count_occurence_of(mapping, "16bit");
            channel_names_sixteen.reserve(count_channel_type);
            sixteen_bit_channels.reserve(count_channel_type);
            count_channel_type = count_occurence_of(mapping, "float");
            channel_names_float.reserve(count_channel_type);
            float_channels.reserve(count_channel_type);
            count_channel_type = count_occurence_of(mapping, "color");
            channel_names_color.reserve(count_channel_type);
            color_channels.reserve(count_channel_type);
            while(true){
                const auto sign = mapping.find(":", start_pos);
                std::string channel_type = mapping.substr(sign+1, next_pos-sign-1);
                std::string channel_name = mapping.substr(start_pos, sign-start_pos);
                if (channel_type.compare("8bit") == 0){
                    channel_names_eight.push_back(channel_name);
                    eight_bit_channels.push_back(0);
                } else if(channel_type.compare("16bit") == 0){
                    channel_names_sixteen.push_back(channel_name);
                    sixteen_bit_channels.push_back(0);
                } else if(channel_type.compare("float") == 0){
                    channel_names_float.push_back(channel_name);
                    float_channels.push_back(0);
                } else if(channel_type.compare("color") == 0){
                    channel_names_color.push_back(channel_name);
                    color_channels.push_back(dmxfish::dmx::pixel());
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

            int frames_count = std::count(frames.begin(), frames.end(), ':');
            eight_bit_frames.reserve(frames_count*eight_bit_channels.size());
            sixteen_bit_frames.reserve(frames_count*sixteen_bit_channels.size());
            float_frames.reserve(frames_count*float_channels.size());
            color_frames.reserve(frames_count*color_channels.size());
            size_t start_pos_channel = 0;
            auto next_pos_channel = frames.find(";");
            while(true){
                const auto sign = frames.find(":", start_pos_channel);
                std::string timestamp = frames.substr(start_pos_channel, sign-start_pos_channel);
                std::string frame_channels = frames.substr(sign+1, next_pos_channel-sign-1);
                size_t start_pos_frame = 0;
                auto next_pos_frame = frame_channels.find("|");
                try {
                    timestamps.push_back(std::stod(timestamp));

                } catch (const std::invalid_argument& ex) {
                    MARK_UNUSED(ex);
                    throw filter_config_exception("cue filter: cant parse timestamp");
                } catch (const std::out_of_range& ex) {
                    MARK_UNUSED(ex);
                    throw filter_config_exception("cue filter: cant parse timestamp ");
                }
                size_t i = 0;
                while(true) {
                    try {
                        const auto sign_transition = frame_channels.find("@", start_pos_frame);
                        std::string tran = frame_channels.substr(sign_transition+1, next_pos_frame-sign_transition-1);
                        transition_t transition;
                        if(tran.compare("lin")==0){
                            transition = LINEAR;
                        } else if(tran.compare("edge")==0){
                            transition = EDGE;
                        } else if(tran.compare("sigm")==0){
                            transition = SIGMOIDAL;
                        } else if(tran.compare("e_in")==0){
                            transition = EASE_IN;
                        } else if(tran.compare("e_out")==0){
                            transition = EASE_OUT;
                        } else {
                            throw filter_config_exception("cue filter: could not resolve transition type");
                        }

                        if (i < eight_bit_channels.size()) {
                            eight_bit_frames.push_back(KeyFrame<uint8_t>(std::stoi(
                                        frame_channels.substr(start_pos_frame, next_pos_frame - start_pos_frame)),
                                    transition));
                        } else if (i < eight_bit_channels.size() + sixteen_bit_channels.size()) {
                            sixteen_bit_frames.push_back(KeyFrame<uint16_t>(std::stoi(
                                        frame_channels.substr(start_pos_frame, next_pos_frame - start_pos_frame)),
                                     transition));
                        } else if (i <
                                   eight_bit_channels.size() + sixteen_bit_channels.size() + float_channels.size()) {
                            float_frames.push_back(KeyFrame<double>(std::stod(
                                        frame_channels.substr(start_pos_frame, next_pos_frame - start_pos_frame)),
                                    transition));
                        } else if (i < eight_bit_channels.size() + sixteen_bit_channels.size() + float_channels.size() +
                                       color_channels.size()) {
                            const auto first_position = frame_channels.find(",", start_pos_frame);
                            double hue = std::stod(
                                    frame_channels.substr(start_pos_frame, first_position - start_pos_frame));
                            const auto second_position = frame_channels.find(",", first_position + 1);
                            double saturation = std::stod(
                                    frame_channels.substr(first_position + 1, second_position - first_position - 1));
                            double iluminance = std::stod(
                                    frame_channels.substr(second_position + 1, next_pos_frame - second_position - 1));
                            color_frames.push_back(KeyFrame<dmxfish::dmx::pixel>(dmxfish::dmx::pixel(hue, saturation, iluminance),
                                                   transition));
                        } else {
                            throw filter_config_exception("cue filter: too many channels for a frame");
                        }
                    }
                    catch (const std::invalid_argument& ex) {
                        MARK_UNUSED(ex);
                        throw filter_config_exception("cue filter: can not parse the value");
                    } catch (const std::out_of_range& ex) {
                        MARK_UNUSED(ex);
                        throw filter_config_exception("cue filter: can not parse the value ");
                    }
                    i++;

                    if (next_pos_frame >= frame_channels.length()) {
                        if (i < eight_bit_channels.size() + sixteen_bit_channels.size() + float_channels.size() + color_channels.size()){
                            throw filter_config_exception("cue filter: too less channels for a frame");
                        }
                        break;
                    }
                    start_pos_frame = next_pos_frame + 1;
                    next_pos_frame = frame_channels.find("|", start_pos_frame);
                }
                if (next_pos_channel >= frames.length()){
                    break;
                }
                start_pos_channel = next_pos_channel + 1;
                next_pos_channel = frames.find(";", start_pos_channel);
            }
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(_value);
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
            for(int i = 0; i<eight_bit_channels.size(); i++){
                map.eight_bit_channels[name + ":" + channel_names_eight.at(i)] = &eight_bit_channels.at(i);
            }
            for(int i = 0; i<sixteen_bit_channels.size(); i++){
                map.sixteen_bit_channels[name + ":" + channel_names_sixteen.at(i)] = &sixteen_bit_channels.at(i);
            }
            for(int i = 0; i<float_channels.size(); i++){
                map.float_channels[name + ":" + channel_names_float.at(i)] = &float_channels.at(i);
            }
            for(int i = 0; i<color_channels.size(); i++){
                map.color_channels[name + ":" + channel_names_color.at(i)] = &color_channels.at(i);
            }
        }

        virtual void update() override {
//            this->output = (*input);
        }

        virtual void scene_activated() override {}

    };

    COMPILER_RESTORE("-Weffc++")

}