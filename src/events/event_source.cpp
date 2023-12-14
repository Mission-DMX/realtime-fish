//
// Created by doralitze on 11/15/23.
//

#include "event_source.hpp"
#include "event_storage.hpp"

namespace dmxfish::events {
    event_source::event_source(std::shared_ptr<event_storage> storage_to_register_with) : sender_id{} {
        this->sender_id = storage_to_register_with->register_event_source(this->shared_from_this());
    }

    void event_source::deregister() {

    }
} // dmxfish