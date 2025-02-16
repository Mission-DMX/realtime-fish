//
// Created by leondietrich on 2/14/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "dmx/pixel.hpp"
#include "filters/sequencer/keyframe.hpp"

namespace dmxfish::filters::sequencer {
    class transition {
    private:
        /**
         * If this is set to false, the transition will not be applied to the channels, if it is already running
         */
        bool reset_allowed = true;
        size_t id;
        std::vector<std::pair<size_t, keyframe<uint8_t>>> frames_8bit;
        std::vector<std::pair<size_t, keyframe<uint16_t>>> frames_16bit;
        std::vector<std::pair<size_t, keyframe<double>>> frames_float;
        std::vector<std::pair<size_t, keyframe<dmxfish::dmx::pixel>>> frames_color;

        /**
         * This will be filled in by the constructor and is sorted. It is used to quickly check if the
         * transition is still running. It does not distinguish between the type of channels and channel
         * 8bit/1 and color/1 will both be reduced to 1. But for every channel id, all types will be queried.
         */
        std::vector<size_t> affected_channel_ids;
    public:
        // TODO implement application methods and parsing using separate source file
        // TODO implement constructor with string as argument for parsing
    };
}