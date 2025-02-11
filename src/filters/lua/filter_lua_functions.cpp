//
// Created by ludwig rahlff
// Modified by leon dietrich
// SPDX-License-Identifier: GPL-3.0-or-later
//

/*
 * This holds functions for the lua filter
 */
#include <string>

#include <sol/sol.hpp>
#include "lib/macros.hpp"
#include "filters/lua/lua_color_api.hpp"


namespace dmxfish::filters::lua {

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

        init_lua_color_api(lua);

        // TODO make events available here
        // TODO make function to send updates to GUI available here
        // TODO allow sending of filter parameter updates to own scene here
        
        // TODO send exceptions, thrown in scene_actived, update, etc. to gui as log entries
    }

}
