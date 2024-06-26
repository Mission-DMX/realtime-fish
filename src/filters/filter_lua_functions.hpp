#pragma once

/*
 * This holds functions for the lua filter
 */
#include <sol/sol.hpp>

namespace dmxfish::filters::lua {

    std::tuple<uint8_t, uint8_t, uint8_t> hsi_to_rgb_color(dmxfish::dmx::pixel& color){
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        color.pixel_to_rgb(r, g, b);
        return std::make_tuple(r, g, b);
    }

    std::tuple<uint8_t, uint8_t, uint8_t> hsi_to_rgb_table(sol::table color){
        dmxfish::dmx::pixel color_local = dmxfish::dmx::pixel((double) color["h"], (double) color["s"], (double) color["i"]);
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
        dmxfish::dmx::pixel color_local = dmxfish::dmx::pixel((double) color["h"], (double) color["s"], (double) color["i"]);
        return hsi_to_rgbw_color(color_local);
    }

    void init_lua(sol::state& lua){
        // prepare libraries and prerequirements in lua
        lua.open_libraries(sol::lib::base, sol::lib::package);
        lua.open_libraries(sol::lib::math, sol::lib::table, sol::lib::string);

        lua.set_function("update", []() {
        });
        lua.set_function("scene_activated", []() {
        });
        lua.set_function("receive_update_from_gui", [](const std::string& key, const std::string& value) {
            MARK_UNUSED(key);
            MARK_UNUSED(value);
            return false;
        });

        lua.create_named_table("output");

        lua.set_function( "hsi_to_rgb", sol::overload(
                dmxfish::filters::lua::hsi_to_rgb_color,
                dmxfish::filters::lua::hsi_to_rgb_table
        ) );

        lua.set_function( "hsi_to_rgbw", sol::overload(
                dmxfish::filters::lua::hsi_to_rgbw_color,
                dmxfish::filters::lua::hsi_to_rgbw_table
        ) );
    }

}
