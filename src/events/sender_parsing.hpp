//
// Created by Doralitze on 11.05.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <string>

#include "events/event.hpp"

namespace dmxfish::events {

    /**
     * Parse a sender specification of the form "sender:function". The sender
     * part may be either a numeric sender id (fully supported, used by the
     * editor's automated filter generation) or a registered sender name. The
     * function part is always numeric. Names are resolved via the global event
     * storage singleton.
     *
     * @param s The specification string, e.g. "17:3" or "audio-in:0".
     * @return The resolved event_sender_t.
     * @throws std::invalid_argument If the sender part is a name but no
     *         matching source is registered, or if the specification is
     *         syntactically invalid.
     */
    event_sender_t parse_sender_representation(const std::string &s);

}