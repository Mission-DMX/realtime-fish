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
        filter_pixel_to_rgb_channels = 15,
        filter_pixel_to_rgbw_channels = 16,
        filter_pixel_to_rgbwa_channels = 17,
        filter_floats_to_pixel = 18,
        filter_sine = 19,
        filter_cosine = 20,
        filter_tangent = 21,
        filter_arcsine = 22,
        filter_arccosine = 23,
        filter_arctangent = 24,
        filter_square = 25,
        filter_triangle = 26,
        filter_sawtooth = 27,
        filter_logarithm = 28,
        filter_exponential = 29,
        filter_minimum = 30,
        filter_maximum = 31,
        filter_time = 32,
        delay_switch_on_8bit = 33,
        delay_switch_on_16bit = 34,
        delay_switch_on_float = 35,
        delay_switch_off_8bit = 36,
        delay_switch_off_16bit = 37,
        delay_switch_off_float = 38,
        filter_fader_column_raw = 39,
        filter_fader_column_hsi = 40,
        filter_fader_column_hsia = 41,
        filter_fader_column_hsiu = 42,
        filter_fader_column_hsiau = 43,
        filter_cue = 44,
        filter_shift_8bit = 45,
        filter_shift_16bit = 46,
        filter_shift_float = 47,
        filter_shift_color = 48,
        filter_main_brightness_fader = 49,
        filter_lua_script = 50,
        filter_8bit_to_float = 51,
        filter_16bit_to_float = 52,
        filter_pixel_to_floats = 53
    };
}
