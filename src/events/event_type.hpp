//
// Created by doralitze on 11/14/23.
//

#pragma once

#include <cstdint>

namespace dmxfish::events {

    enum class event_type : uint8_t {
        SINGLE_TRIGGER = 0,
        START = 1,
        RELEASE = 2
    };

}
