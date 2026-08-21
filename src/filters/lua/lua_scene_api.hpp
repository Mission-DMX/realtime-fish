//
// Created by leondietrich on 2/11/26.
// SPDX-License-Identifier: GPL-3.0-or-later
//
#pragma once

#include <sol/sol.hpp>

namespace dmxfish::filters::lua {
    void init_lua_scene_api(sol::state& lua);
}
