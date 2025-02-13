//
// Created by Leon Dietrich on 13.02.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <cstdint>

namespace dmxfish::filters {
    enum class transition_t : uint8_t {
        EDGE,
        LINEAR,
        SIGMOIDAL,
        EASE_IN,
        EASE_OUT
    };

    template <typename T>
    struct keyframe{
        T value;
        transition_t transition;
        keyframe(T val, transition_t tr): value(val), transition(tr) {}
    };
}