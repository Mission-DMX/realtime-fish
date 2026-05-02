#pragma once

#include <cstdint>

namespace dmxfish::filters::chaserlayers {
    enum class color_channel_target : uint8_t {
        R,
        G,
        B,
        H,
        S,
        I
    };
}
