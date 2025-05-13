//
// Created by Doralitze on 5/13/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <array>

namespace dmxfish::audio {

    constexpr auto fft_size = 1024;

    void fft(const std::array<double, fft_size> &in_buffer, std::array<double, fft_size> &real,
             std::array<double, fft_size> &imag);

}
