#pragma once

/*
 * The filter runs lua scripts
 */

#include <vector>
#include "filters/filter.hpp"


namespace dmxfish::filters {
    COMPILER_SUPRESS("-Weffc++")


    class filter_lua_script: public filter {
    private:
//        enum channel_t{
//            EIGHT_BIT,
//            SIXTEEN_BIT,
//            FLOAT,
//            COLOR
//        };

        std::vector<uint8_t*> in_eight_bit;
        std::vector<uint16_t*> in_sixteen_bit;
        std::vector<double*> in_float;
        std::vector<dmxfish::dmx::pixel*> in_color;

        std::vector<uint8_t> eight_bit_channels;
        std::vector<uint16_t> sixteen_bit_channels;
        std::vector<double> float_channels;
        std::vector<dmxfish::dmx::pixel> color_channels;

        // channel names for the output map
        std::vector<std::string> channel_names_eight;
        std::vector<std::string> channel_names_sixteen;
        std::vector<std::string> channel_names_float;
        std::vector<std::string> channel_names_color;


    public:
        filter_lua_script() : filter() {}
        virtual ~filter_lua_script() {}


        virtual void pre_setup(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters) override;

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override;

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override;

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override;

        virtual void update() override;

        virtual void scene_activated() override;

    };

    COMPILER_RESTORE("-Weffc++")

}
