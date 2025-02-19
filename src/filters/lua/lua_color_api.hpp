//
// Created by leondietrich on 2/11/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//
#pragma once

#include <sol/sol.hpp>

namespace dmxfish::filters::lua {
    void init_lua_color_api(sol::state& lua);
}