#pragma once

namespace dmxfish::filters {
    enum class filter_type : int {
        constants_8bit = 0,
        constants_16bit = 1,
        constants_float = 2,
        constants_pixel = 3
    };
}
