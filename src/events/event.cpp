//
// Created by doralitze on 11/14/23.
//

#include <ostream>
#include "event.hpp"

namespace dmxfish::events {

    static event_id_t next_event_id = 1;

    event::event() : type(event_type::INVALID), event_arguments{}, event_id(0), sender_id{} {}

    event::event(dmxfish::events::event_type _type, event_sender_t _sender_id) : type(_type), sender_id(_sender_id),
    event_id(next_event_id), event_arguments() {
        next_event_id++;
    }

    event::event(const event& other) : type(other.type), sender_id(other.sender_id), event_id(other.event_id),
                                       event_arguments(other.event_arguments) {

    }

    event::event(event_type _type, const event& other) : type(_type), sender_id(other.sender_id),
    event_id(next_event_id), event_arguments(other.event_arguments) {
        next_event_id++;
    }

    std::ostream& operator<<(std::ostream& os, const event& ev) {
        if(!ev.is_valid()) {
            return os << "Invalid Event";
        }
        os << "Event " << ev.get_event_id() << " Type: " << ev.get_type();
        os << " From: " << ev.get_event_sender().decoded_representation.sender << ':';
        os << ev.get_event_sender().decoded_representation.sender_function;
        os << " ARGS:" << std::hex;
        for (auto& c : ev.get_args()) {
            os << " " << c;
        }
        os << std::dec;
        return os;
    }
}