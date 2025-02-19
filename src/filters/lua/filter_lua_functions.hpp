//
// Created by leondietrich on 2/11/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <sol/sol.hpp>

namespace dmxfish::filters::lua {

    /**
     * Initialize the given lua context with the required API
     * @param lua The lua context to initialize
     */
    void init_lua(sol::state &lua);
}