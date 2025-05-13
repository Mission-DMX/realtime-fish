//
// Created by Doralitze on 5/13/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <array>

#include <fftw3.h>

namespace dmxfish::audio {

    constexpr auto fft_size = 1024;
    constexpr auto complex_fft_size = fft_size * 2;

    void train_fft();

    struct fft_context {
        std::array<double, complex_fft_size> fft_buffer;
        std::array<double, complex_fft_size> out_buffer;
        fftw_plan p;

        fft_context();
        ~fft_context();
    };

    void fft(fft_context& ctx);

}
