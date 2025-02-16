//
// Created by Leon Dietrich on 13.02.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <algorithm>
#include <deque>
#include <numeric>
#include <unordered_map>
#include <tuple>
#include <type_traits>

#include "dmx/pixel.hpp"
#include "filters/sequencer/keyframe.hpp"
#include "filters/sequencer/time.hpp"
#include "filters/sequencer/transition.hpp"

namespace dmxfish::filters::sequencer {

    enum class interleaving_method : uint8_t {
        AVERAGE,
        MIN,
        MAX
    };

    template <typename T>
    struct channel {
        T current_value;
        T default_value;

        /**
         * The structure of this container is as follows:
         * map: contains all active transitions, keys: transition id (location in the vector of available
         * transitions), value: the transitions. If they're empty, they'll be removed from the map. Inserting an already
         * running transition overwrites (and therefore resets) it.
         *  -> A tuple of the keyframe start time, the value when it started and a deque of keyframes define the running
         *     transition. The start time stores the time when the current key frame started. If the keyframe expired
         *     it will be removed from the deque.
         *
         * During the update process, the channel value will be updated by the average of the active transition
         * keyframes.
         */
        std::unordered_map<size_t, std::tuple<sequencer_time_t, T, std::deque<keyframe<T>>>> upcomming_keyframes;
        bool apply_default_value_on_empty_transition_queue = false;
        interleaving_method i_method = interleaving_method::AVERAGE;
    public:
        channel() = default;

        void apply_update(sequencer_time_t current_time, double time_scale) {
            if(this->upcomming_keyframes.empty()) {
                if (this->apply_default_value_on_empty_transition_queue) {
                    current_value = default_value;
                }
                return;
            }
            std::vector<size_t> transitions_to_remove;
            std::vector<T> requested_values;
            requested_values.reserve(this->upcomming_keyframes.size());
            for (auto& [trans_id, trans] : this->upcomming_keyframes) {
                auto& keyframe_start_time = std::get<0>(trans);
                auto& keyframe_start_value = std::get<1>(trans);
                auto& keyframe_queue = std::get<2>(trans);
                if (keyframe_start_time == 0) {
                    keyframe_start_time = current_time;
                }
                do {
                    if (keyframe_queue.empty()) {
                        transitions_to_remove.push_back(trans_id);
                        break;
                    }
                    auto &current_frame = keyframe_queue.front();
                    if (current_time * time_scale >= keyframe_start_time + current_frame.duration) {
                        keyframe_start_time = current_time;
                        keyframe_start_value = current_frame.value;
                        keyframe_queue.pop_front();
                        continue;
                    }
                    requested_values.push_back(current_frame.calculate_update(current_time - keyframe_start_time, keyframe_start_value, time_scale));
                    break;
                } while (true);
            }
            this->perform_update_arbiting(requested_values);
            for (auto i : transitions_to_remove) {
                // TODO test if this is really removing the key and not the position
                this->upcomming_keyframes.erase(i);
            }
        }

        bool transition_active(size_t transition_id) {
            return this->upcomming_keyframes.contains(transition_id);
        }

        void insert_keyframes(std::deque<keyframe<T>> frames, size_t transition_id) {
            this->upcomming_keyframes[transition_id] = std::make_tuple(0, this->current_value, frames);
        }
    private:
        void perform_update_arbiting(const std::vector<T>& values) {
            if (values.empty()) [[unlikely]] {
                return;
            }
            switch(this->i_method) {
                default:
                case interleaving_method::AVERAGE:
                    {
                        if constexpr (std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value) {
                            this->current_value = std::accumulate(values.begin(), values.end(), 0) / values.size();
                        } else if constexpr (std::is_same<T, double>::value) {
                            double acc = 0.0;
                            double size = (double) values.size();
                            for (const auto& v : values) {
                                acc += v / size;
                            }
                            this->current_value = acc;
                        } else {
                            if (values.size() == 1) {
                                this->current_value = values[0];
                                return;
                            }
                            double r = 0.0;
                            double g = 0.0;
                            double b = 0.0;
                            double i = 0.0;
                            for (auto v : values) {
                                const auto r2 = v.getRed();
                                const auto g2 = v.getGreen();
                                const auto b2 = v.getBlue();
                                r += r2*r2;
                                g += g2*g2;
                                b += b2*b2;
                                i = std::max(i, v.getIluminance());
                            }
                            r = std::sqrt(r);
                            g = std::sqrt(g);
                            b = std::sqrt(b);
                            i = std::min(i, 1.0);
                            const auto vlen = std::sqrt(r*r+g*g+b*b);
                            this->current_value.setRed((r/vlen)*65535*i);
                            this->current_value.setGreen((g/vlen)*65535*i);
                            this->current_value.setBlue((b/vlen)*65535*i);
                        }
                    }
                    break;
                case interleaving_method::MAX:
                    {
                        if constexpr (std::is_same<T, dmxfish::dmx::pixel>::value) {
                            unsigned int max_r = 0, max_g = 0, max_b = 0;
                            for (auto& color : values) {
                                max_r = std::max(max_r, color.getRed());
                                max_g = std::max(max_g, color.getGreen());
                                max_b = std::max(max_b, color.getBlue());
                            }
                            this->current_value = {max_r, max_g, max_b};
                        } else {
                            this->current_value = *std::max_element(values.begin(), values.end());
                        }
                    }
                    break;
                case interleaving_method::MIN:
                    if constexpr (std::is_same<T, dmxfish::dmx::pixel>::value) {
                        if constexpr (std::is_same<T, dmxfish::dmx::pixel>::value) {
                            unsigned int min_r = 65535, min_g = 65535, min_b = 65535;
                            for (auto& color : values) {
                                min_r = std::min(min_r, color.getRed());
                                min_g = std::min(min_g, color.getGreen());
                                min_b = std::min(min_b, color.getBlue());
                            }
                            this->current_value = {min_r, min_g, min_b};
                        } else {
                            this->current_value = *std::max_element(values.begin(), values.end());
                        }
		            } else {
			            this->current_value = *std::min_element(values.begin(), values.end());
		            }
		            break;
            }
        }
    };
}
