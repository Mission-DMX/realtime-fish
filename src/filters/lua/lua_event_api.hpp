//
// Created by leondietrich on 2/12/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//
#pragma once

#include <sol/sol.hpp>

namespace dmxfish::filters::lua {
    void init_lua_event_api(sol::state& lua);
}