#include "filters/filter_lua_script.hpp"


#include "lib/macros.hpp"
#include "lib/logging.hpp"
#include "dmx/pixel.hpp"
#include "filters/util.hpp"

#include <iostream>


namespace dmxfish::filters {

    void filter_lua_script::pre_setup(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters) {
        if (!configuration.contains("out_mapping")) {
            throw filter_config_exception("lua filter: unable to setup the out_mapping");
        }
        if (!configuration.contains("in_mapping")) {
            throw filter_config_exception("lua filter: unable to setup the in_mapping");
        }

        // getting output channels
        std::string out_mapping = configuration.at("out_mapping");
        //::spdlog::debug("pre-setup: out_mapping: {}", out_mapping);

        int count_channel_type = util::count_occurence_of(out_mapping, ":8bit", 0, out_mapping.size());
        out_eight_bit.reserve(count_channel_type);
        names_out_eight_bit.reserve(count_channel_type);

        count_channel_type = util::count_occurence_of(out_mapping, ":16bit", 0, out_mapping.size());
        out_sixteen_bit.reserve(count_channel_type);
        names_out_sixteen_bit.reserve(count_channel_type);

        count_channel_type = util::count_occurence_of(out_mapping, ":float", 0, out_mapping.size());
        out_float.reserve(count_channel_type);
        names_out_float.reserve(count_channel_type);

        count_channel_type = util::count_occurence_of(out_mapping, ":color", 0, out_mapping.size());
        out_color.reserve(count_channel_type);
        names_out_color.reserve(count_channel_type);

        size_t start_pos = 0;
        auto next_pos = out_mapping.find(";");
        while (true) {
            const auto sign = out_mapping.find(":", start_pos);

            std::string channel_type = out_mapping.substr(sign + 1, next_pos - sign - 1);
            std::string channel_name = out_mapping.substr(start_pos, sign - start_pos);
            if (!channel_type.compare("8bit")) {
                out_eight_bit.push_back(0);
                names_out_eight_bit.push_back(channel_name);
            } else if (!channel_type.compare("16bit")) {
                out_sixteen_bit.push_back(0);
                names_out_sixteen_bit.push_back(channel_name);
            } else if (!channel_type.compare("float")) {
                out_float.push_back(0);
                names_out_float.push_back(channel_name);
            } else if (!channel_type.compare("color")) {
                out_color.push_back(dmxfish::dmx::pixel());
                names_out_color.push_back(channel_name);
            } else {
                throw filter_config_exception(std::string("can not recognise channel type: ") +
                                              out_mapping.substr(sign + 1, next_pos - sign - 1));
            }
            if (next_pos >= out_mapping.length()) {
                break;
            }
            start_pos = next_pos + 1;
            next_pos = out_mapping.find(";", start_pos);
        }


        // getting input channels
        std::string in_mapping = configuration.at("in_mapping");
        count_channel_type = util::count_occurence_of(in_mapping, ":8bit", 0, in_mapping.size());
        in_eight_bit.reserve(count_channel_type);
        names_in_eight_bit.reserve(count_channel_type);

        count_channel_type = util::count_occurence_of(in_mapping, ":16bit", 0, in_mapping.size());
        in_sixteen_bit.reserve(count_channel_type);
        names_in_sixteen_bit.reserve(count_channel_type);

        count_channel_type = util::count_occurence_of(in_mapping, ":float", 0, in_mapping.size());
        in_float.reserve(count_channel_type);
        names_in_float.reserve(count_channel_type);

        count_channel_type = util::count_occurence_of(in_mapping, ":color", 0, in_mapping.size());
        in_color.reserve(count_channel_type);
        names_in_color.reserve(count_channel_type);

        start_pos = 0;
        next_pos = in_mapping.find(";");
        while (true) {
            const auto sign = in_mapping.find(":", start_pos);

            std::string channel_type = in_mapping.substr(sign + 1, next_pos - sign - 1);
            std::string channel_name = in_mapping.substr(start_pos, sign - start_pos);
            if (!channel_type.compare("8bit")) {
                in_eight_bit.push_back(NULL);
                names_in_eight_bit.push_back(channel_name);
            } else if (!channel_type.compare("16bit")) {
                in_sixteen_bit.push_back(NULL);
                names_in_sixteen_bit.push_back(channel_name);
            } else if (!channel_type.compare("float")) {
                in_float.push_back(NULL);
                names_in_float.push_back(channel_name);
            } else if (!channel_type.compare("color")) {
                in_color.push_back(NULL);
                names_in_color.push_back(channel_name);
            } else {
                throw filter_config_exception(std::string("can not recognise channel type: ") +
                                              in_mapping.substr(sign + 1, next_pos - sign - 1));
            }

            if (next_pos >= in_mapping.length()) {
                break;
            }
            start_pos = next_pos + 1;
            next_pos = in_mapping.find(";", start_pos);
        }
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
