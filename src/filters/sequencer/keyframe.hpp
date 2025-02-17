//
// Created by Leon Dietrich on 13.02.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "dmx/pixel.hpp"
#include "filters/sequencer/time.hpp"

namespace dmxfish::filters::sequencer {
    enum class transition_t : uint8_t {
        EDGE,
        LINEAR,
        SIGMOIDAL,
        EASE_IN,
        EASE_OUT
    };

    template <typename T>
    class keyframe{
        const T value;
        const transition_t transition;
        const sequencer_time_t duration;
    public:
        keyframe(T& val, transition_t tr, sequencer_time_t fduration): value(val), transition(tr), duration(fduration) {}

        T calculate_update(const sequencer_time_t time_since_start, const T& start_value, const double time_scale) const {
            const double interleave_point = this->compute_interleave_point(time_since_start, time_scale);
            if constexpr (std::is_same<T, dmxfish::dmx::pixel>::value) {
                return dmxfish::dmx::mix_color_interleaving(start_value, value, interleave_point);
            } else {
                return (T) ((((double) start_value) * (1-interleave_point)) + (((double) value) * interleave_point));
            }
        }

        [[nodiscard]] inline sequencer_time_t get_duration() const {
            return this->duration;
        }
    private:
        double compute_interleave_point(sequencer_time_t elapsed_time, double time_scale) const {
            switch(this->transition) {
                default:
                case transition_t::LINEAR:
                    return std::min(std::max((elapsed_time * time_scale) / duration, 0.0), 1.0);
                case transition_t::EDGE: {
                    constexpr double switch_point = 0.95;
                    if (elapsed_time * time_scale >= this->duration * switch_point) {
                        return 1.0;
                    } else {
                        return 0.0;
                    }
                }
                case transition_t::SIGMOIDAL: {
#if __cplusplus >= 202602L
                    constexpr double e = std::exp(1.0);
#else
                    const double e = std::exp(1.0);
#endif
                    return (std::exp(elapsed_time * time_scale / this->duration) - 1.0) / (e - 1.0);
                }
                case transition_t::EASE_IN: {
                    const auto adv = (elapsed_time * time_scale) / this->duration;
                    return adv*adv;
                }
                case transition_t::EASE_OUT: {
                    const auto adv = (elapsed_time * time_scale) / this->duration;
                    return 1 - ((1-adv) * (1-adv));
                }
            }
        }
    };
}
