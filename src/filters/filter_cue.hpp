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

        std::vector<std::string> channel_names_eight;
        std::vector<std::string> channel_names_sixteen;
        std::vector<std::string> channel_names_float;
        std::vector<std::string> channel_names_color;

    public:
        filter_cue() : filter() {}
        virtual ~filter_cue() {}

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override {
//            MARK_UNUSED(initial_parameters);
            MARK_UNUSED(configuration);
            if(!input_channels.float_channels.contains("time") || !input_channels.eight_bit_channels.contains("running_state")) {
                throw filter_config_exception("Unable to link input of cue filter: channel mapping does not contain channel 'time' of type 'double' or 'running_state' of type 8bit");
            }
            this->time = input_channels.float_channels.at("time");
            this->running_state = input_channels.eight_bit_channels.at("running_state");

//            if(!initial_parameters.contains("value")){
//                throw filter_config_exception("Unable to set value of constant filter: initial configuration does not contain value parameter");
//            } else if (!this->receive_update_from_gui("value", initial_parameters.at("value"))) {
//                throw filter_config_exception("Unable to set value of constant filter: unable to parse parameter.");
//            }

            if(!configuration.contains("mapping")){
                throw filter_config_exception("cue filter: unable to setup the mapping");
            }

            std::string mapping = configuration.at("mapping");
            size_t start_pos = 0;
            auto next_pos = mapping.find(",");
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

                if (next_pos >= mapping.size()){
                    break;
                }
                start_pos = next_pos + 1;
                next_pos = mapping.find(",", start_pos);
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