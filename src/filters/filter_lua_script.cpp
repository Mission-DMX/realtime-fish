#include "filters/filter_lua_script.hpp"


#include "lib/macros.hpp"
#include "lib/logging.hpp"
#include "dmx/pixel.hpp"
#include "filters/util.hpp"

#include <iostream>


namespace dmxfish::filters {
    template <typename T>
    void filter_lua_script::reserve_init_out(int amount){
        if constexpr (std::is_same<T, uint8_t>::value) {
            out_eight_bit.reserve(amount);
            names_out_eight_bit.reserve(amount);
        } else if constexpr (std::is_same<T, uint16_t>::value) {
            out_sixteen_bit.reserve(amount);
            names_out_sixteen_bit.reserve(amount);
        } else if constexpr (std::is_same<T, double>::value) {
            out_float.reserve(amount);
            names_out_float.reserve(amount);
        } else {
            out_color.reserve(amount);
            names_out_color.reserve(amount);
        }
    }

    template <typename T>
    void filter_lua_script::reserve_init_in(int amount){
        if constexpr (std::is_same<T, uint8_t>::value) {
            in_eight_bit.reserve(amount);
            names_in_eight_bit.reserve(amount);
        } else if constexpr (std::is_same<T, uint16_t>::value) {
            in_sixteen_bit.reserve(amount);
            names_in_sixteen_bit.reserve(amount);
        } else if constexpr (std::is_same<T, double>::value) {
            in_float.reserve(amount);
            names_in_float.reserve(amount);
        } else {
            in_color.reserve(amount);
            names_in_color.reserve(amount);
        }
    }


    template <typename T>
    void filter_lua_script::init_values_out(std::string &channel_name){
        if constexpr (std::is_same<T, uint8_t>::value) {
            out_eight_bit.push_back(0);
            names_out_eight_bit.push_back(channel_name);
        } else if constexpr (std::is_same<T, uint16_t>::value) {
            out_sixteen_bit.push_back(0);
            names_out_sixteen_bit.push_back(channel_name);
        } else if constexpr (std::is_same<T, double>::value) {
            out_float.push_back(0);
            names_out_float.push_back(channel_name);
        } else {
            out_color.push_back(dmxfish::dmx::pixel());
            names_out_color.push_back(channel_name);
        }
    }


    template <typename T>
    void filter_lua_script::init_values_in(std::string &channel_name){
        if constexpr (std::is_same<T, uint8_t>::value) {
            in_eight_bit.push_back(NULL);
            names_in_eight_bit.push_back(channel_name);
        } else if constexpr (std::is_same<T, uint16_t>::value) {
            in_sixteen_bit.push_back(NULL);
            names_in_sixteen_bit.push_back(channel_name);
        } else if constexpr (std::is_same<T, double>::value) {
            in_float.push_back(NULL);
            names_in_float.push_back(channel_name);
        } else {
            in_color.push_back(NULL);
            names_in_color.push_back(channel_name);
        }
    }

    void filter_lua_script::pre_setup(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters) {
        if (!configuration.contains("out_mapping")) {
            throw filter_config_exception("lua filter: unable to setup the out_mapping");
        }
        if (!configuration.contains("in_mapping")) {
            throw filter_config_exception("lua filter: unable to setup the in_mapping");
        }

        //::spdlog::debug("pre-setup: out_mapping: {}", out_mapping);
        util::init_mapping(
                configuration.at("out_mapping"),
                std::bind(&dmxfish::filters::filter_lua_script::reserve_init_out<uint8_t>, this, std::placeholders::_1),
                std::bind(&dmxfish::filters::filter_lua_script::reserve_init_out<uint16_t>, this,
                          std::placeholders::_1),
                std::bind(&dmxfish::filters::filter_lua_script::reserve_init_out<double>, this, std::placeholders::_1),
                std::bind(&dmxfish::filters::filter_lua_script::reserve_init_out<dmxfish::dmx::pixel>, this,
                          std::placeholders::_1),
                std::bind(&dmxfish::filters::filter_lua_script::init_values_out<uint8_t>, this, std::placeholders::_1),
                std::bind(&dmxfish::filters::filter_lua_script::init_values_out<uint16_t>, this, std::placeholders::_1),
                std::bind(&dmxfish::filters::filter_lua_script::init_values_out<double>, this, std::placeholders::_1),
                std::bind(&dmxfish::filters::filter_lua_script::init_values_out<dmxfish::dmx::pixel>, this,
                          std::placeholders::_1)
        );

        // getting input channels
        util::init_mapping(
                configuration.at("in_mapping"),
                std::bind(&dmxfish::filters::filter_lua_script::reserve_init_in<uint8_t>, this,
                          std::placeholders::_1),
                std::bind(&dmxfish::filters::filter_lua_script::reserve_init_in<uint16_t>, this,
                          std::placeholders::_1),
                std::bind(&dmxfish::filters::filter_lua_script::reserve_init_in<double>, this, std::placeholders::_1),
                std::bind(&dmxfish::filters::filter_lua_script::reserve_init_in<dmxfish::dmx::pixel>, this,
                          std::placeholders::_1),
                std::bind(&dmxfish::filters::filter_lua_script::init_values_in<uint8_t>, this, std::placeholders::_1),
                std::bind(&dmxfish::filters::filter_lua_script::init_values_in<uint16_t>, this,
                          std::placeholders::_1),
                std::bind(&dmxfish::filters::filter_lua_script::init_values_in<double>, this, std::placeholders::_1),
                std::bind(&dmxfish::filters::filter_lua_script::init_values_in<dmxfish::dmx::pixel>, this,
                          std::placeholders::_1)
        );
    }


    void filter_lua_script::setup_filter(const std::map <std::string, std::string> &configuration,
                                  const std::map <std::string, std::string> &initial_parameters,
                                  const channel_mapping &input_channels) {
        MARK_UNUSED(configuration);
        MARK_UNUSED(initial_parameters);
        for (size_t i = 0; i < in_eight_bit.size(); i++) {
            if(!input_channels.eight_bit_channels.contains(names_in_eight_bit.at(i))) {
                throw filter_config_exception("Unable to link input of lua filter: channel mapping does not contain channel '" + names_in_eight_bit.at(i) + "' of type 'uint8_t'.");
            }
            in_eight_bit.at(i) = input_channels.eight_bit_channels.at(names_in_eight_bit.at(i));
        }
        for (size_t i = 0; i < in_sixteen_bit.size(); i++) {
            if(!input_channels.sixteen_bit_channels.contains(names_in_sixteen_bit.at(i))) {
                throw filter_config_exception("Unable to link input of lua filter: channel mapping does not contain channel '" + names_in_sixteen_bit.at(i) + "' of type 'uint16_t'.");
            }
            in_sixteen_bit.at(i) = input_channels.sixteen_bit_channels.at(names_in_sixteen_bit.at(i));
        }
        for (size_t i = 0; i < in_float.size(); i++) {
            if(!input_channels.float_channels.contains(names_in_float.at(i))) {
                throw filter_config_exception("Unable to link input of lua filter: channel mapping does not contain channel '" + names_in_float.at(i) + "' of type 'double'.");
            }
            in_float.at(i) = input_channels.float_channels.at(names_in_float.at(i));
        }
        for (size_t i = 0; i < in_color.size(); i++) {
            if(!input_channels.color_channels.contains(names_in_color.at(i))) {
                throw filter_config_exception("Unable to link input of lua filter: channel mapping does not contain channel '" + names_in_color.at(i) + "' of type 'pixel'.");
            }
            in_color.at(i) = input_channels.color_channels.at(names_in_color.at(i));
        }

        if (!initial_parameters.contains("script")) {
            throw filter_config_exception("lua filter: unable to setup the script");
        }

        lua.open_libraries(sol::lib::base, sol::lib::package);
        lua.set_function("update", []() {
        });
        lua.set_function("scene_activated", []() {
        });

        lua.script("function hsi_to_rgb(color)\n"
                   "    help_h = color.h % 360;\n"
                   "    help_h = 3.14159*help_h / 180;\n"
                   "    help_s = color.s>0 and (color.s<1 and color.s or 1) or 0;\n"
                   "    help_i = color.i>0 and (color.i<1 and color.i or 1) or 0;\n"
                   "    if help_h < 2.09439\n"
                   "    then\n"
                   "        cos_h = math.cos(help_h)\n"
                   "        cos_1047_h = math.cos(1.047196667-help_h)\n"
                   "        return {\n"
                   "            r = 255*help_i/3*(1+help_s*cos_h/cos_1047_h),\n"
                   "            g = 255*help_i/3*(1+help_s*(1-cos_h/cos_1047_h)),\n"
                   "            b = 255*help_i/3*(1-help_s)\n"
                   "        }\n"
                   "    elseif help_h < 4.188787\n"
                   "    then\n"
                   "        help_h = help_h - 2.09439\n"
                   "        cos_h = math.cos(help_h)\n"
                   "        cos_1047_h = math.cos(1.047196667-help_h)\n"
                   "        return {\n"
                   "            g = 255*help_i/3*(1+help_s*cos_h/cos_1047_h),\n"
                   "            b = 255*help_i/3*(1+help_s*(1-cos_h/cos_1047_h)),\n"
                   "            r = 255*help_i/3*(1-help_s)\n"
                   "        }\n"
                   "    else\n"
                   "        help_h = help_h - 4.188787\n"
                   "        cos_h = math.cos(help_h)\n"
                   "        cos_1047_h = math.cos(1.047196667-help_h)\n"
                   "        return {\n"
                   "            b = 255*help_i/3*(1+help_s*cos_h/cos_1047_h),\n"
                   "            r = 255*help_i/3*(1+help_s*(1-cos_h/cos_1047_h)),\n"
                   "            g = 255*help_i/3*(1-help_s)\n"
                   "        }\n"
                   "    end\n"
                   "end\n"
                   "\n"
                   "function hsi_to_rgbw(color)\n"
                   "    help_h = color.h % 360;\n"
                   "    help_h = 3.14159*help_h / 180;\n"
                   "    help_s = color.s>0 and (color.s<1 and color.s or 1) or 0;\n"
                   "    help_i = color.i>0 and (color.i<1 and color.i or 1) or 0;\n"
                   "    if help_h < 2.09439\n"
                   "    then \n"
                   "        cos_h = math.cos(help_h)\n"
                   "        cos_1047_h = math.cos(1.047196667-help_h)\n"
                   "        return {\n"
                   "            r = help_s*255*help_i/3*(1+cos_h/cos_1047_h),\n"
                   "            g = help_s*255*help_i/3*(1+(1-cos_h/cos_1047_h)),\n"
                   "            b = 0,\n"
                   "            w = 255*help_i*(1-help_s)\n"
                   "        }\n"
                   "    elseif help_h < 4.188787\n"
                   "    then\n"
                   "        help_h = help_h - 2.09439\n"
                   "        cos_h = math.cos(help_h)\n"
                   "        cos_1047_h = math.cos(1.047196667-help_h)\n"
                   "        return {\n"
                   "            g = help_s*255*help_i/3*(1+cos_h/cos_1047_h),\n"
                   "            b = help_s*255*help_i/3*(1+(1-cos_h/cos_1047_h)),\n"
                   "            r = 0,\n"
                   "            w = 255*help_i*(1-help_s)\n"
                   "        }\n"
                   "    else\n"
                   "        help_h = help_h - 4.188787\n"
                   "        cos_h = math.cos(help_h)\n"
                   "        cos_1047_h = math.cos(1.047196667-help_h)\n"
                   "        return {\n"
                   "            b = help_s*255*help_i/3*(1+cos_h/cos_1047_h),\n"
                   "            r = help_s*255*help_i/3*(1+(1-cos_h/cos_1047_h)),\n"
                   "            g = 0,\n"
                   "            w = 255*help_i*(1-help_s)\n"
                   "        }\n"
                   "    end\n"
                   "end");

        lua.script(initial_parameters.at("script"));

        // run scene_activated
        lua.script("scene_activated()");
        // load script without execute
        script_update = lua.load("update()");
    }

    bool filter_lua_script::receive_update_from_gui(const std::string &key, const std::string &_value) {

    }

    void filter_lua_script::get_output_channels(channel_mapping &map, const std::string &name) {
        for (size_t i = 0; i < out_eight_bit.size(); i++) {
            map.eight_bit_channels[name + ":" + names_out_eight_bit.at(i)] = &out_eight_bit.at(i);
        }
        for (size_t i = 0; i < out_sixteen_bit.size(); i++) {
            map.sixteen_bit_channels[name + ":" + names_out_sixteen_bit.at(i)] = &out_sixteen_bit.at(i);
        }
        for (size_t i = 0; i < out_float.size(); i++) {
            map.float_channels[name + ":" + names_out_float.at(i)] = &out_float.at(i);
        }
        for (size_t i = 0; i < out_color.size(); i++) {
            map.color_channels[name + ":" + names_out_color.at(i)] = &out_color.at(i);
        }
    }

    void filter_lua_script::update() {

        // transmit input data to lua
        for (size_t i = 0; i < in_eight_bit.size(); i++) {
            lua[names_in_eight_bit.at(i)] = *in_eight_bit.at(i);
        }
        for (size_t i = 0; i < in_sixteen_bit.size(); i++) {
            lua[names_in_sixteen_bit.at(i)] = *in_sixteen_bit.at(i);
        }
        for (size_t i = 0; i < in_float.size(); i++) {
            lua[names_in_float.at(i)] = *in_float.at(i);
        }
        for (size_t i = 0; i < in_color.size(); i++) {
            sol::table color = lua.create_table_with("h", in_color.at(i)->hue,
                                                           "s", in_color.at(i)->saturation,
                                                           "i", in_color.at(i)->iluminance
            );
            lua[names_in_color.at(i)] = color;
        }

        // execute update script in lua
        sol::protected_function_result script_update_res = script_update();
//        // optionally, check if it worked
//        std::cout << "test : " << script_update_res.valid() << std::endl;

        // receive output data from lua
        for (size_t i = 0; i < out_eight_bit.size(); i++) {
            out_eight_bit.at(i) = std::max(std::min(lua[names_out_eight_bit.at(i)].get_or((double) out_eight_bit.at(i)), 255.0), 0.0);
        }
        for (size_t i = 0; i < out_sixteen_bit.size(); i++) {
            out_sixteen_bit.at(i) = std::max(std::min(lua[names_out_sixteen_bit.at(i)].get_or((double) out_sixteen_bit.at(i)), 65535.0), 0.0);
        }
        for (size_t i = 0; i < out_float.size(); i++) {
            out_float.at(i) = lua.get_or(names_out_float.at(i), out_float.at(i));
        }
        for (size_t i = 0; i < out_color.size(); i++) {
            out_color.at(i).hue = lua[names_out_color.at(i)]["h"].get_or(out_color.at(i).hue);
            out_color.at(i).saturation = lua[names_out_color.at(i)]["s"].get_or(out_color.at(i).saturation);
            out_color.at(i).iluminance = lua[names_out_color.at(i)]["i"].get_or(out_color.at(i).iluminance);
        }
    }

    void filter_lua_script::scene_activated() {
    }

}
