//
// Created by leondietrich on 1/28/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//
#pragma once

#include "events/event_source.hpp"

namespace dmxfish::io {

class ioboard_midi_event_source : public dmxfish::events::event_source {
public:
    ioboard_midi_event_source();
    virtual ~ioboard_midi_event_source();
protected:
    virtual void deregister() override;

    };


}