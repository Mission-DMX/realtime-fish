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
        std::vector<std::pair<size_t, keyframe<uint8_t>>> frames_8bit;
        std::vector<std::pair<size_t, keyframe<uint16_t>>> frames_16bit;
        std::vector<std::pair<size_t, keyframe<double>>> frames_float;
        std::vector<std::pair<size_t, keyframe<dmxfish::dmx::pixel>>> frames_color;
    public:
        // TODO implement application methods and parsing using separate source file
        // TODO implement constructor with string as argument for parsing
    };
}