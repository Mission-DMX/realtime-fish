//
// Created by doralitze on 11/14/23.
//

#pragma once

#include <array>

#include "event_type.hpp"

namespace dmxfish::events {

    using event_id_t = unsigned long;
    using event_sender_t = unsigned long;

    class event {
    private:
        event_type type;
        std::array<uint8_t, 7> event_arguments;
        event_id_t event_id;
        event_sender_t sender_id;
    public:
        event(event_type _type, event_sender_t sender_id);

        inline event_id_t get_event_id() {
            return this->event_id;
        }

        inline event_sender_t get_event_sender() {
            return this->sender_id;
        }
    };

}

