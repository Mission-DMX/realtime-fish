//
// Created by doralitze on 12/14/23.
//

#include "event_type.hpp"

namespace dmxfish::events {
    std::ostream& operator<<(std::ostream& os, const event_type& et) {
        switch (et) {
            case event_type::SINGLE_TRIGGER:
                return os << "single";
            case event_type::START:
                return os << "positive edge";
            case event_type::RELEASE:
                return os << "negative edge";
            case event_type::ONGOING_EVENT:
                return os << "continuous";
            case event_type::INVALID:
                return os << "invalid";
        }
    }
}