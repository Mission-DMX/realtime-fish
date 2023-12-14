//
// Created by doralitze on 11/14/23.
//

#pragma once

#include <cstdint>
#include <ostream>

namespace dmxfish::events {

    enum class event_type : uint8_t {
        SINGLE_TRIGGER = 0,
        START = 1,
        RELEASE = 2,
        ONGOING_EVENT = 3,
        INVALID = 255
    };

    std::ostream& operator<<(std::ostream& os, const event_type& et);

}
