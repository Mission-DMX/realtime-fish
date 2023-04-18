#pragma once

namespace dmxfish::filters {
    enum class filter_type : unsigned int {
        constants_8bit = 0,
        constants_16bit = 1,
        constants_float = 2,
        constants_pixel = 3,
        debug_8bit = 4,
        debug_16bit = 5,
        debug_float = 6,
        debug_pixel = 7,
        filter_16bit_to_dual_byte = 8,
        filter_16bit_to_bool = 9,
        filter_multiply_add = 10,
        filter_universe_output = 11,
        filter_float_to_16bit = 12,
        filter_float_to_8bit = 13,
        filter_round_number = 14,
    };
}
