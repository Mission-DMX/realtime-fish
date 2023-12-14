//
// Created by doralitze on 11/15/23.
//

#pragma once

#include <memory>

#include "event_storage.hpp"

namespace dmxfish::events {

class event_source: public std::enable_shared_from_this<event_source> {
        friend class event_storage;

        event_sender_t sender_id;
    public:
        explicit event_source(std::shared_ptr<event_storage> storage_to_register_with);

        [[nodiscard]] inline uint32_t get_sender_id() const {
            return this->sender_id.decoded_representation.sender;
        }
    private:

        /**
         * This method needs to be called by the event storage if it is being taken down.
         */
        void deregister();
    };

} // dmxfish

