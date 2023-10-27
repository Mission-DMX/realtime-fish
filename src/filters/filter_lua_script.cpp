#include "filters/filter_lua_script.hpp"

#include "filters/filter_lua_functions.hpp"
#include "lib/macros.hpp"
#include "lib/logging.hpp"
#include "dmx/pixel.hpp"
#include "filters/util.hpp"
#include "io/universe_sender.hpp"
#include <iostream>


namespace dmxfish::filters {

    inline void filter_lua_script::send_input_values_to_lua(){
        for (size_t i = 0; i < in_eight_bit.size(); i++) {
            lua[names_in_eight_bit.at(i)] = *in_eight_bit.at(i);
        }
        for (size_t i = 0; i < in_sixteen_bit.size(); i++) {
            lua[names_in_sixteen_bit.at(i)] = *in_sixteen_bit.at(i);
        }
        for (size_t i = 0; i < in_float.size(); i++) {
            lua[names_in_float.at(i)] = *in_float.at(i);
        }
        // we dont need pixel, because it was transmitted as reference
//        for (size_t i = 0; i < in_color.size(); i++) {
//            sol::table color = lua.create_table_with("h", in_color.at(i)->hue,
//                                                     "s", in_color.at(i)->saturation,
//                                                     "i", in_color.at(i)->iluminance
//            );
//            lua[names_in_color.at(i)] = color;
//        }
    }

    inline void filter_lua_script::get_output_values_from_lua() {
        for (size_t i = 0; i < out_eight_bit.size(); i++) {
            out_eight_bit.at(i) = std::max(
                    std::min(std::round(lua[names_out_eight_bit.at(i)].get_or((double) out_eight_bit.at(i))), 255.0),
                    0.0);
        }
        for (size_t i = 0; i < out_sixteen_bit.size(); i++) {
            out_sixteen_bit.at(i) = std::max(
                    std::min(std::round(lua[names_out_sixteen_bit.at(i)].get_or((double) out_sixteen_bit.at(i))),
                             65535.0), 0.0);
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

    inline void filter_lua_script::get_direct_out_channels() {
        sol::object outputs = lua["output"];
        if (outputs.get_type() == sol::type::table) {
            // Todo: improve check all! existing universes and (only?) patched channels
            for (int universe_id = 0; universe_id <= 2 * ((sol::table) outputs).size(); universe_id++) {
                sol::object universe = ((sol::table) outputs)[universe_id];
                if (auto uptr = dmxfish::io::get_universe(universe_id); uptr != nullptr) {
                    if (universe.get_type() == sol::type::table) {
                        for (uint16_t chan = 0; chan < 512; chan++) {
                            sol::object channel = ((sol::table) universe)[chan];
                            if (channel.get_type() == sol::type::number) {
                                uint8_t value = ((sol::table) universe)[chan];
                                (*uptr)[chan] = value;
                            }
                        }
                    }
                }
            }
        }
    }

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

    void filter_lua_script::pre_setup(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const std::string& own_id) {
        if (!configuration.contains("out_mapping")) {
            throw filter_config_exception("lua filter: unable to setup the out_mapping",
                                          filter_type::filter_lua_script, own_id);
        }
        if (!configuration.contains("in_mapping")) {
            throw filter_config_exception("lua filter: unable to setup the in_mapping",
                                          filter_type::filter_lua_script, own_id);
        }

        lua.new_usertype<dmxfish::dmx::pixel>("Pixel", "h", &dmxfish::dmx::pixel::hue, "s", &dmxfish::dmx::pixel::saturation, "i", &dmxfish::dmx::pixel::iluminance);

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
                          std::placeholders::_1),
                          filter_type::filter_lua_script, own_id
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
                          std::placeholders::_1),
                          filter_type::filter_lua_script, own_id
        );
    }


    void filter_lua_script::setup_filter(const std::map <std::string, std::string> &configuration,
                                  const std::map <std::string, std::string> &initial_parameters,
                                  const channel_mapping &input_channels,
                                  const std::string& own_id) {
        MARK_UNUSED(configuration);
        MARK_UNUSED(initial_parameters);
        for (size_t i = 0; i < names_in_eight_bit.size(); i++) {
            if(!input_channels.eight_bit_channels.contains(names_in_eight_bit.at(i))) {
                throw filter_config_exception("Unable to link input of lua filter: channel mapping does not contain "
                                              "channel '" + names_in_eight_bit.at(i) + "' of type 'uint8_t'.",
                                              filter_type::filter_lua_script, own_id);
            }
            in_eight_bit.at(i) = input_channels.eight_bit_channels.at(names_in_eight_bit.at(i));
        }
        for (size_t i = 0; i < names_in_sixteen_bit.size(); i++) {
            if(!input_channels.sixteen_bit_channels.contains(names_in_sixteen_bit.at(i))) {
                throw filter_config_exception("Unable to link input of lua filter: channel mapping does not contain "
                                              "channel '" + names_in_sixteen_bit.at(i) + "' of type 'uint16_t'.",
                                              filter_type::filter_lua_script, own_id);
            }
            in_sixteen_bit.at(i) = input_channels.sixteen_bit_channels.at(names_in_sixteen_bit.at(i));
        }
        for (size_t i = 0; i < names_in_float.size(); i++) {
            if(!input_channels.float_channels.contains(names_in_float.at(i))) {
                throw filter_config_exception("Unable to link input of lua filter: channel mapping does not contain "
                                              "channel '" + names_in_float.at(i) + "' of type 'double'.",
                                              filter_type::filter_lua_script, own_id);
            }
            in_float.at(i) = input_channels.float_channels.at(names_in_float.at(i));
        }
        for (size_t i = 0; i < names_in_color.size(); i++) {
            if(!input_channels.color_channels.contains(names_in_color.at(i))) {
                throw filter_config_exception("Unable to link input of lua filter: channel mapping does not contain "
                                              "channel '" + names_in_color.at(i) + "' of type 'pixel'.",
                                              filter_type::filter_lua_script, own_id);
            }
            in_color.at(i) = input_channels.color_channels.at(names_in_color.at(i));
            // Todo delete in_color;
            // setting color as reference in lua
            lua[names_in_color.at(i)] = std::ref(input_channels.color_channels.at(names_in_color.at(i)));
        }

        if (!initial_parameters.contains("script")) {
            throw filter_config_exception("lua filter: unable to setup the script. Parameter missing.",
                                          filter_type::filter_lua_script, own_id);
        }


        dmxfish::filters::lua::init_lua(lua);



        try {
            lua.script(initial_parameters.at("script"));
        } catch (std::exception &e){
            throw filter_config_exception(std::string("setup the filter threw an error: ") + e.what(),
                                          filter_type::filter_lua_script, own_id);
        }


        // Todo: checking signature does not work?
        sol::object scene_activated_obj = lua["scene_activated"];
        if (scene_activated_obj.get_type() == sol::type::function && scene_activated_obj.is<std::function<void()>>()){
            scene_activated_lua = scene_activated_obj;
        } else {
            throw filter_config_exception("scene_activated is not a function or has the wrong signature",
                                          filter_type::filter_lua_script, own_id);
        }

        sol::object update_obj = lua["update"];
        if (update_obj.get_type() == sol::type::function && update_obj.is<std::function<void()>>()){
            update_lua = update_obj;
        } else {
            throw filter_config_exception("update is not a function or has the wrong signature",
                                          filter_type::filter_lua_script, own_id);
        }

        sol::object receive_update_obj = lua["receive_update_from_gui"];
        if (receive_update_obj.get_type() == sol::type::function && receive_update_obj.is<std::function<bool(std::string, std::string)>>()){
            receive_update = receive_update_obj;
        } else {
            throw filter_config_exception("receive_update_from_gui is not a function or has the wrong signature",
                                          filter_type::filter_lua_script, own_id);
        }
    }

    bool filter_lua_script::receive_update_from_gui(const std::string &key, const std::string &_value) {
        // execute receive_update_from_gui script in lua
        try {
            return receive_update(key, _value);
        } catch (const std::exception& e) {
            ::spdlog::warn("receive_update_from_gui of lua has failed: {}", e.what());
            return false;
        }
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
        send_input_values_to_lua();

        // execute update script in lua
        try {
            update_lua();
        } catch (const std::exception& e) {
            ::spdlog::warn("Update of lua has failed: {}", e.what());
            throw filter_runtime_exception(std::string("update script in lua had an error: ") + e.what(), filter_type::filter_lua_script);
        }

        // receive output data from lua
        get_output_values_from_lua();

        // Going through the table and set output channels
        get_direct_out_channels();
    }

    void filter_lua_script::scene_activated() {
        try {
            scene_activated_lua();
        } catch (const std::exception& e) {
            ::spdlog::warn("Scene activated of lua has failed: {}", e.what());
            throw filter_runtime_exception(std::string("scene_sctivated script in lua had an error: ") + e.what(), filter_type::filter_lua_script);
        }
    }

}
