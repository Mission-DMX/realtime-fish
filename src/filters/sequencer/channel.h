//
// Created by Leon Dietrich on 13.02.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <deque>

#include "filters/sequencer/keyframe.hpp"

namespace dmxfish::filters::sequencer {

    using sequencer_time_t = double;
    template <typename T>
    struct channel {
        T current_value;
        T last_transition_value;
        T default_value;
        sequencer_time_t last_transition_time = 0.0;
        std::deque<keyframe<T>> upcomming_keyframes;
        bool apply_default_value_on_emty_transition_queue = false;
    public:
        channel() = default;
    };
}
