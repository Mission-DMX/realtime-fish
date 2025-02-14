//
// Created by Leon Dietrich on 13.02.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <cstdint>

#include "filters/sequencer/time.hpp"

namespace dmxfish::filters::sequencer {
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
        sequencer_time_t duration;
        keyframe(T val, transition_t tr): value(val), transition(tr) {}

        T calculate_update(sequencer_time_t time_since_start, T start_value) {
            // TODO implement transition
        }
    };
}