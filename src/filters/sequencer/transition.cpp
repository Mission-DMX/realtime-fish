//
// Created by Leon Dietrich on 18.02.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "filters/sequencer/transition.hpp"

namespace dmxfish::filters::sequencer {
    transition::transition() : frames_8bit(), frames_16bit(), frames_float(), frames_color(), affected_channel_ids() {
        // Nothing to do here
    }

    transition::transition(const std::list<std::string>& s, const name_maps& nm) : transition() {
        // TODO implement constructor with string as argument for parsing
    }
}