#pragma once

/*
 * The filters hold a cue sequence
 */

#include "string"
#include "vector"

#include "filters/filter.hpp"
#include "lib/macros.hpp"
#include "dmx/pixel.hpp"


namespace dmxfish::filters {
    COMPILER_SUPRESS("-Weffc++")


    class filter_cue: public filter {
    private:
        uint8_t* running_state = nullptr;
        double* time = nullptr;
        double start_time = 0;

        std::vector<double> timestamps;

        std::vector<uint8_t> eight_bit_channels;
        std::vector<uint16_t> sixteen_bit_channels;
        std::vector<double> float_channels;
        std::vector<dmxfish::dmx::pixel> color_channels;

//        std::vector<std::vector<uint8_t>> eight_bit_frames;
//        std::vector<std::vector<uint16_t>> sixteen_bit_frames;
//        std::vector<std::vector<double>> float_frames;
//        std::vector<std::vector<dmxfish::dmx::pixel>> color_frames;
        std::vector<uint8_t> eight_bit_frames;
        std::vector<uint16_t> sixteen_bit_frames;
        std::vector<double> float_frames;
        std::vector<dmxfish::dmx::pixel> color_frames;

//        std::vector<std::string> channel_type;

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
            while(true){
                const auto sign = mapping.find(":", start_pos);
                std::string channel_type = mapping.substr(sign+1, next_pos-sign-1);
                std::string channel_name = mapping.substr(start_pos, sign-start_pos);
                if (channel_type.compare("8bit") == 0){
                    channel_names_eight.push_back(channel_name);
                    eight_bit_channels.push_back(0);
//                    eight_bit_frames.push_back(std::vector<uint8_t>);
                } else if(channel_type.compare("16bit") == 0){
                    channel_names_sixteen.push_back(channel_name);
                    sixteen_bit_channels.push_back(0);
//                    sixteen_bit_frames.push_back(std::vector<uint16_t>);
                } else if(channel_type.compare("float") == 0){
                    channel_names_float.push_back(channel_name);
                    float_channels.push_back(0);
//                    float_frames.push_back(std::vector<double>);
                } else if(channel_type.compare("color") == 0){
                    channel_names_color.push_back(channel_name);
                    color_channels.push_back(dmxfish::dmx::pixel());
//                    color_frames.push_back(std::vector<dmxfish::dmx::pixel>);
                } else {
                    throw filter_config_exception(std::string("can not recognise channel type: ") + mapping.substr(sign, next_pos-sign));
                }

                if (next_pos >= mapping.size()){
                    break;
                }
                start_pos = next_pos + 1;
                next_pos = mapping.find(";", start_pos);
            }

            if(!initial_parameters.contains("frames")){
                throw filter_config_exception("cue filter: unable to setup the frames");
            }

            std::string frames = configuration.at("frames");
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
                    size_t i = 0;
                    while(true) {
                        if (i < eight_bit_channels.size()){
                            eight_bit_frames.push_back(std::stoi(frame_channels.substr(start_pos_frame, next_pos_frame-start_pos_frame)));
                        }
                        else if (i < eight_bit_channels.size() + sixteen_bit_channels.size()){
                            sixteen_bit_frames.push_back(std::stoi(frame_channels.substr(start_pos_frame, next_pos_frame-start_pos_frame)));
                        }
                        else if (i < eight_bit_channels.size() + sixteen_bit_channels.size() + float_channels.size()){
                            float_frames.push_back(std::stod(frame_channels.substr(start_pos_frame, next_pos_frame-start_pos_frame)));
                        }
                        else if (i < eight_bit_channels.size() + sixteen_bit_channels.size() + float_channels.size() + color_channels.size()){
                            const auto first_position = frame_channels.find(",", start_pos_frame);
                            double hue = std::stod(frame_channels.substr(start_pos_frame, first_position-start_pos_frame));
                            const auto second_position = frame_channels.find(",", first_position + 1);
                            double saturation = std::stod(frame_channels.substr(first_position + 1, second_position - first_position - 1));
                            double iluminance = std::stod(frame_channels.substr(second_position + 1, next_pos_frame - second_position - 1));
                            color_frames.push_back(dmxfish::dmx::pixel(hue, saturation, iluminance));
                        } else {
                            throw filter_config_exception("cue filter: too many channels for a frame");
                        }
                        i++;

                        if (next_pos_frame >= frame_channels.size()) {
                            if (i < eight_bit_channels.size() + sixteen_bit_channels.size() + float_channels.size() + color_channels.size()){
                                throw filter_config_exception("cue filter: too less channels for a frame");
                            }
                            break;
                        }
                        start_pos_frame = next_pos_frame + 1;
                        next_pos_frame = mapping.find("|", start_pos_frame);
                    }
                } catch (const std::invalid_argument& ex) {
                    MARK_UNUSED(ex);
                    throw filter_config_exception("cue filter: cant parse timestamp");
                } catch (const std::out_of_range& ex) {
                    MARK_UNUSED(ex);
                    throw filter_config_exception("cue filter: cant parse timestamp ");
                }
                if (next_pos_channel >= frames.size()){
                    break;
                }
                start_pos_channel = next_pos_channel + 1;
                next_pos_channel = mapping.find(";", start_pos_channel);
            }
        }

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override {
            MARK_UNUSED(key);
            MARK_UNUSED(_value);
            return false;
        }

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override {
//            map.float_channels[name + ":value"] = &output;
        }

        virtual void update() override {
//            this->output = (*input);
        }

        virtual void scene_activated() override {}

    };

    COMPILER_RESTORE("-Weffc++")

}