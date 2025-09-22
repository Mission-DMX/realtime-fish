//
// Created by Doralitze on 5/13/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "sound/fft.hpp"

#include <stdexcept>
#include <thread>

#include <fftw3.h>

#include "lib/logging.hpp"

namespace dmxfish::audio {

    static bool fft_ready = false;
    static fftw_plan fft_plan_template;

    void _train() {
        std::array<double, complex_fft_size> in_arr;
        std::array<double, complex_fft_size> out_arr;
        fft_plan_template = fftw_plan_dft_1d(fft_size, (fftw_complex *) in_arr.data(),
                                             (fftw_complex *) out_arr.data(), FFTW_FORWARD, FFTW_MEASURE);
        fft_ready = true;
        ::spdlog::info("FFT training successful.");
    }

    void train_fft(bool online) {
        ::spdlog::info("Starting FFT training.");
        if(!online) {
            std::thread t([]() {
                _train();
            });
            t.detach();
        } else {
            _train();
        }
    }

    fft_context::fft_context() : fft_buffer(), out_buffer() {
        this->p = fftw_plan_dft_1d(fft_size, (fftw_complex*) fft_buffer.data(), (fftw_complex*) out_buffer.data(), FFTW_FORWARD, FFTW_MEASURE);
        for(auto& r : this->fft_buffer) {
            r = 0;
        }
        for(auto& r : this->out_buffer) {
            r = 0;
        }
    }

    fft_context::~fft_context() {
        fftw_destroy_plan(this->p);
    }

    void fft(fft_context& ctx) {
        if (!fft_ready) {
            ::spdlog::warn("FFT isn't trained yet.");
        }
        fftw_execute(ctx.p);
    }
}