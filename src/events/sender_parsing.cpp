//
// Created by Doralitze on 11.05.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "events/sender_parsing.hpp"

#include "utils.hpp"

namespace dmxfish::events {

    dmxfish::events::event_sender_t parse_sender_representation(const std::string& s) {
        auto parts = utils::split(s, ':');
        dmxfish::events::event_sender_t es;
        es.decoded_representation.sender = std::stoul(parts.front());
        parts.pop_front();
        es.decoded_representation.sender_function = std::stoul(parts.front());
        return es;
    }
}