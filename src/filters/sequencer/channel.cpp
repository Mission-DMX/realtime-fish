//
// Created by leondietrich on 2/18/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "filters/sequencer/channel.hpp"

namespace dmxfish::filters::sequencer {
    interleaving_method interleaving_method_from_string(const std::string& s) {
        if(auto upper_s = utils::toupper(s); s == "MAX") {
            return interleaving_method::MAX;
        } else if(upper_s == "MIN") {
            return interleaving_method::MIN;
        } else {
            return interleaving_method::AVERAGE;
        }
    }
}