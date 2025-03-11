//
// Created by leondietrich on 1/28/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "io/ioboard/ioboard_midi_event_source.hpp"

namespace dmxfish::io {
    ioboard_midi_event_source::ioboard_midi_event_source() : dmxfish::events::event_source(this) {

    }

    ioboard_midi_event_source::~ioboard_midi_event_source() {}

    void ioboard_midi_event_source::deregister() {

    }
}