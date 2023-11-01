#pragma once

/*
 * The filter runs lua scripts
 */

#include <vector>
#include "filters/filter.hpp"
#include <sol/sol.hpp>
#include <string>


namespace dmxfish::filters {
    COMPILER_SUPRESS("-Weffc++")


    class filter_lua_script: public filter {
    private:
        template <typename T>
        void reserve_init_out(int amount);
        template <typename T>
        void init_values_out(std::string &channel_name);
        template <typename T>
        void reserve_init_in(int amount);
        template <typename T>
        void init_values_in(std::string &channel_name);

        void get_direct_out_channels();
        void get_output_values_from_lua();
        void send_input_values_to_lua();

        sol::state lua;

        sol::function scene_activated_lua;
        sol::function update_lua;
        sol::function receive_update;

        std::vector<uint8_t*> in_eight_bit;
        std::vector<uint16_t*> in_sixteen_bit;
        std::vector<double*> in_float;
        std::vector<dmxfish::dmx::pixel*> in_color;

        std::vector<uint8_t> out_eight_bit;
        std::vector<uint16_t> out_sixteen_bit;
        std::vector<double> out_float;
        std::vector<dmxfish::dmx::pixel> out_color;

        // channel names for the input map
        std::vector<std::string> names_in_eight_bit;
        std::vector<std::string> names_in_sixteen_bit;
        std::vector<std::string> names_in_float;
        std::vector<std::string> names_in_color;

        // channel names for the output map
        std::vector<std::string> names_out_eight_bit;
        std::vector<std::string> names_out_sixteen_bit;
        std::vector<std::string> names_out_float;
        std::vector<std::string> names_out_color;

    public:
        filter_lua_script() : filter() {}
        virtual ~filter_lua_script() {}


        virtual void pre_setup(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const std::string& own_id) override;

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override;

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override;

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override;

        virtual void update() override;

        virtual void scene_activated() override;

    };

    COMPILER_RESTORE("-Weffc++")

}
