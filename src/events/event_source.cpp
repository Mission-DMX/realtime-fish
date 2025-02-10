//
// Created by doralitze on 11/15/23.
//

#include "event_source.hpp"
#include "event_storage.hpp"

namespace dmxfish::events {
    event_source::event_source() : sender_id{} {}

    event_source::event_source(event_source* other) : sender_id(other->sender_id) {}

    void event_source::deregister() {

    }

    event_source::~event_source() {
        this->deregister();
    }
} // dmxfish