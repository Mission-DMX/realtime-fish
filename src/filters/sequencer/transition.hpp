//
// Created by leondietrich on 2/14/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <cstddef>
#include <cstdint>

// Having ordered maps gives us a little bit more overhead at construction but yields better cache access during runtime.
#if __cplusplus >= 202302
#include <flat_map>
#define selected_map_impl std::flat_map
#else
#include <map>
#define selected_map_impl std::map
#endif

#include <list>
#include <string>
#include <vector>

#include "dmx/pixel.hpp"
#include "filters/sequencer/keyframe.hpp"
#include "filters/sequencer/name_maps.h"

namespace dmxfish::filters::sequencer {

    struct transition {
    private:
        /**
         * If this is set to false, the transition will not be applied to the channels, if it is already running
         */
        bool reset_allowed = true;
        size_t id;

    public:
        /**
         * Set of channels and their transition content
         */
        selected_map_impl<size_t, std::vector<keyframe<uint8_t>>> frames_8bit;
        selected_map_impl<size_t, std::vector<keyframe<uint16_t>>> frames_16bit;
        selected_map_impl<size_t, std::vector<keyframe<double>>> frames_float;
        selected_map_impl<size_t, std::vector<keyframe<dmxfish::dmx::pixel>>> frames_color;

        /**
         * This will be filled in by the constructor and is sorted. It is used to quickly check if the
         * transition is still running. It does not distinguish between the type of channels and channel
         * 8bit/1 and color/1 will both be reduced to 1. But for every channel id, all types will be queried.
         */
        std::vector<size_t> affected_channel_ids;
    public:
        transition();
        transition(const std::list<std::string>& s, const name_maps& nm);

        [[nodiscard]] inline bool is_reset_allowed() const {
            return this->reset_allowed;
        }

        [[nodiscard]] inline size_t get_transition_id() const {
            return this->id;
        }
    };
}

#undef selected_map_impl