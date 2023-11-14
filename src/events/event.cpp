//
// Created by leondietrich on 11/14/23.
//

#include "event.hpp"

namespace dmxfish::events {

    static event_id_t next_event_id = 0;

    event::event(dmxfish::events::event_type _type, event_sender_t _sender_id) : type(_type), sender_id(_sender_id),
    event_id(next_event_id) {
        next_event_id++;
    }
    // TODO implement out stream helpers and registry for senders
}