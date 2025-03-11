//
// Created by leondietrich on 2/18/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "filters/sequencer/keyframe.hpp"

#include <stdexcept>

#include "utils.hpp"

namespace dmxfish::filters::sequencer {

    transition_t stotransition(const std::string& s) {
        if (auto s_upper = utils::toupper(s); s_upper == "EDG") {
            return transition_t::EDGE;
        } else if (s_upper == "LIN") {
            return transition_t::LINEAR;
        } else if (s_upper == "SIG") {
            return transition_t::SIGMOIDAL;
        } else if (s_upper == "E_I") {
            return transition_t::EASE_IN;
        } else if (s_upper == "E_O") {
            return transition_t::EASE_OUT;
        } else {
            throw std::invalid_argument("Expected transition description instead of '" + s + "'.");
        }
    }

}