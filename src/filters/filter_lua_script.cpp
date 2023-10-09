#include "filters/filter_lua_script.hpp"


#include "lib/macros.hpp"
#include "lib/logging.hpp"
#include "dmx/pixel.hpp"
#include "filters/util.hpp"
#include "io/universe_sender.hpp"
#include <iostream>


std::tuple<uint8_t, uint8_t, uint8_t> hsi_to_rgb_color(dmxfish::dmx::pixel& color){
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    color.pixel_to_rgb(r, g, b);
    return std::make_tuple(r, g, b);
}

std::tuple<uint8_t, uint8_t, uint8_t> hsi_to_rgb_table(sol::table color){
    dmxfish::dmx::pixel color_local = dmxfish::dmx::pixel(color["h"], color["s"], color["i"]);
    return hsi_to_rgb_color(color_local);
}

std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> hsi_to_rgbw_color(dmxfish::dmx::pixel& color){
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t w = 0;
    color.pixel_to_rgbw(r, g, b, w);
    return std::make_tuple(r, g, b, w);
}

std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> hsi_to_rgbw_table(sol::table color){
    dmxfish::dmx::pixel color_local = dmxfish::dmx::pixel(color["h"], color["s"], color["i"]);
    return hsi_to_rgbw_color(color_local);
}


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
        for (size_t i = 0; i < names_in_eight_bit.size(); i++) {
            if(!input_channels.eight_bit_channels.contains(names_in_eight_bit.at(i))) {
                throw filter_config_exception("Unable to link input of lua filter: channel mapping does not contain channel '" + names_in_eight_bit.at(i) + "' of type 'uint8_t'.");
            }
            in_eight_bit.at(i) = input_channels.eight_bit_channels.at(names_in_eight_bit.at(i));
        }
        for (size_t i = 0; i < names_in_sixteen_bit.size(); i++) {
            if(!input_channels.sixteen_bit_channels.contains(names_in_sixteen_bit.at(i))) {
                throw filter_config_exception("Unable to link input of lua filter: channel mapping does not contain channel '" + names_in_sixteen_bit.at(i) + "' of type 'uint16_t'.");
            }
            in_sixteen_bit.at(i) = input_channels.sixteen_bit_channels.at(names_in_sixteen_bit.at(i));
        }
        for (size_t i = 0; i < names_in_float.size(); i++) {
            if(!input_channels.float_channels.contains(names_in_float.at(i))) {
                throw filter_config_exception("Unable to link input of lua filter: channel mapping does not contain channel '" + names_in_float.at(i) + "' of type 'double'.");
            }
            in_float.at(i) = input_channels.float_channels.at(names_in_float.at(i));
        }
        for (size_t i = 0; i < names_in_color.size(); i++) {
            if(!input_channels.color_channels.contains(names_in_color.at(i))) {
                throw filter_config_exception("Unable to link input of lua filter: channel mapping does not contain channel '" + names_in_color.at(i) + "' of type 'pixel'.");
            }
            in_color.at(i) = input_channels.color_channels.at(names_in_color.at(i));
            // Todo delete in_color;
            lua[names_in_color.at(i)] = std::ref(input_channels.color_channels.at(names_in_color.at(i)));
        }

        if (!initial_parameters.contains("script")) {
            throw filter_config_exception("lua filter: unable to setup the script");
        }

//        if (!initial_parameters.contains("patching")) {
//            throw filter_config_exception("lua filter: unable to setup the patching");
//        }

        // initial_parameters.at("patching") divide
        // for each fixture
        //      fix_univ
        //      fix_name
        //      fix_ch_size
        //      fixture local = fixture(fix_univ, fix_name, fix_ch_size);
        //      ? dynamisch verschieden viele Argumente übergeben?
//              lua.new_usertype<fixture>("Fixture", "pan", &fixture.channel_values.at(0), "s", &dmxfish::dmx::pixel::saturation, "i", &dmxfish::dmx::pixel::iluminance);
        //      fixtures.append(local);



        lua.open_libraries(sol::lib::base, sol::lib::package);
        lua.open_libraries(sol::lib::math);
        lua.set_function("update", []() {
        });
        lua.set_function("scene_activated", []() {
        });

//        for(fixture fix: fixtures){
//            lua.create_named_table(fix.name, "universe", fix.universe,
//                                   "first_channel", fix.first_channel);
//        }
        lua.create_named_table("output");

        lua.set_function( "hsi_to_rgb", sol::overload(
                hsi_to_rgb_color,
                hsi_to_rgb_table
        ) );

        lua.set_function( "hsi_to_rgbw", sol::overload(
                hsi_to_rgbw_color,
                hsi_to_rgbw_table
        ) );

        try {
            lua.script(initial_parameters.at("script"));
        } catch (std::exception &e){
            throw filter_config_exception(std::string("setup the filter threw an error: ") + e.what());
        }

        sol::object scene_activated_obj = lua["scene_activated"];
        if (scene_activated_obj.get_type() == sol::type::function && scene_activated_obj.is<std::function<void()>>()){
            scene_activated_lua = scene_activated_obj;
        } else {
            throw filter_config_exception("scene_activated is not a function or has the wrong signature");
        }

        sol::object update_obj = lua["update"];
        if (update_obj.get_type() == sol::type::function && update_obj.is<std::function<void()>>()){
            update_lua = update_obj;
        } else {
            throw filter_config_exception("update is not a function or has the wrong signature");
        }
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
//        sol::protected_function_result script_update_res = script_update();
        try {
            update_lua();
        } catch (const std::exception& e) {
            ::spdlog::warn("Update of lua has failed: {}", e.what());
            throw filter_config_exception(std::string("update script in lua had an error: ") + e.what());
        }

//        // optionally, check if it worked
//        if (!script_update_res.valid()){
//            sol::error err = script_update_res;
//            ::spdlog::warn("Output of lua update has failed: {}", err.what());
//        }
        // receive output data from lua
        for (size_t i = 0; i < out_eight_bit.size(); i++) {
            out_eight_bit.at(i) = std::max(std::min(std::round(lua[names_out_eight_bit.at(i)].get_or((double) out_eight_bit.at(i))), 255.0), 0.0);
        }
        for (size_t i = 0; i < out_sixteen_bit.size(); i++) {
            out_sixteen_bit.at(i) = std::max(std::min(std::round(lua[names_out_sixteen_bit.at(i)].get_or((double) out_sixteen_bit.at(i))), 65535.0), 0.0);
        }
        for (size_t i = 0; i < out_float.size(); i++) {
            out_float.at(i) = lua.get_or(names_out_float.at(i), out_float.at(i));
        }
        for (size_t i = 0; i < out_color.size(); i++) {
            out_color.at(i).hue = lua[names_out_color.at(i)]["h"].get_or(out_color.at(i).hue);
            out_color.at(i).saturation = lua[names_out_color.at(i)]["s"].get_or(out_color.at(i).saturation);
            out_color.at(i).iluminance = lua[names_out_color.at(i)]["i"].get_or(out_color.at(i).iluminance);
        }

        // Going through the table and set output channels
        sol::object outputs = lua["output"];
        if (outputs.get_type() == sol::type::table) {
            for(int universe_id = 0; universe_id <= 2 * ((sol::table) outputs).size(); universe_id++){
                sol::object universe = ((sol::table) outputs)[universe_id];
                if(auto uptr = dmxfish::io::get_universe(universe_id); uptr != nullptr) {
                    if (universe.get_type() == sol::type::table) {
                        for (uint16_t chan = 0; chan < 512; chan++) {
                            sol::object channel = ((sol::table) universe)[chan];
                            if (channel.get_type() == sol::type::number) {
                                uint8_t value = ((sol::table) universe)[chan];
                                (*uptr)[chan] = value;
//                                std::cout << "test : " << (int) value << std::endl;
//                            } else if (channel.get_type() != sol::type::nil) {
//                                std::cout << " test : is not nil : " << std::endl;
                            }
                        }
                    }
                }
//            for (const auto &univ_kvpair: (sol::table) outputs) {
//                sol::object uni_key = univ_kvpair.first;
//                sol::object universe = univ_kvpair.second;
//                if (uni_key.get_type() == sol::type::number) {
//                    std::cout << "number" << std::endl;
//                    int i = ((sol::lua_value) uni_key).value();
//
//                    std::cout << "b1 " << i << std::endl;
//                }
//                if (uni_key.get_type() == sol::type::string) {
//                    std::cout << "string" << std::endl;
////                    int str = (uni_key).get<sol::lua_value>();
////                    std::cout << "b " << str << std::endl;
//                }
//                int i = ((sol::lua_value) uni_key).get<int>();
//                if (universe.get_type() == sol::type::table) {
//                    if(auto uptr = dmxfish::io::get_universe((sol::lua_value) uni_key); uptr != nullptr) {
//                        for (const auto &channel_kv_pair: (sol::table) universe) {
//                            sol::object ch_name = channel_kv_pair.first;
//                            sol::object ch_value = channel_kv_pair.second;
//                            (*uptr)[ch_name] = ch_value;
//                            std::cout << "c " << std::endl;
//                        }
//                    } else {
//                            throw std::invalid_argument("The requested universe with id " + std::to_string(uni_key) +
//                                                        " does not exist anymore");
//                    }
//                }
            }
        }
    }

    void filter_lua_script::scene_activated() {
        try {
            scene_activated_lua();
        } catch (const std::exception& e) {
            ::spdlog::warn("Scene activated of lua has failed: {}", e.what());
            throw filter_config_exception(std::string("scene_sctivated script in lua had an error: ") + e.what());
        }
    }

}
