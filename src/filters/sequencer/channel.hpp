//
// Created by Leon Dietrich on 13.02.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <unordered_map>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include "dmx/pixel.hpp"
#include "filters/sequencer/frame_queue.hpp"
#include "filters/sequencer/keyframe.hpp"
#include "filters/sequencer/time.hpp"
#include "filters/sequencer/transition.hpp"

#include "lib/logging.hpp"
#include "lib/macros.hpp"

#include "utils.hpp"

namespace dmxfish::filters::sequencer {

    enum class interleaving_method : uint8_t {
        AVERAGE,
        MIN,
        MAX
    };

    interleaving_method interleaving_method_from_string(const std::string& s);

    template <typename T>
    class channel {
        T current_value;
        const T default_value;

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
        std::unordered_map<size_t, frame_queue<T>> upcomming_keyframes;
        const bool apply_default_value_on_empty_transition_queue = false;
        const bool apply_default_value_on_clear_command = false;
        const interleaving_method i_method = interleaving_method::AVERAGE;
        const std::string channel_name;
    public:
        channel(const std::string& name, T dv, bool default_on_empty, bool default_on_clear,
                interleaving_method interleavingMethod) :
        current_value(dv), default_value(dv), upcomming_keyframes(),
        apply_default_value_on_empty_transition_queue(default_on_empty),
        apply_default_value_on_clear_command(default_on_clear),
        i_method(interleavingMethod), channel_name(name) {}

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
            for (auto& [trans_id, queue] : this->upcomming_keyframes) {
                auto keyframe_start_time = queue.get_start_time();
                auto keyframe_start_value = queue.get_start_value();
                if (keyframe_start_time <= 0.1) { // It's unlikely that were in the year 1970
                    queue.set_start_time(current_time);
                }
                do {
                    if (queue.empty()) {
                        transitions_to_remove.push_back(trans_id);
                        break;
                    }
                    const auto& current_frame = queue.front();
                    if (current_time * time_scale >= keyframe_start_time + current_frame.get_duration()) {
                        queue.advance(current_time, this->current_value);
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
                ::spdlog::debug("Finished transition for event {} in channel {}.", i, this->channel_name);
            }
        }

        bool transition_active(size_t transition_id) {
            return this->upcomming_keyframes.contains(transition_id);
        }

        bool insert_keyframes(const std::vector<keyframe<T>>* frames, const size_t transition_id, bool reset_allowed) {
            if (this->transition_active(transition_id)) {
                if(!reset_allowed) {
                    return false;
                } else {
                    ::spdlog::debug("Aborting execution of transition {} in channel {}.", transition_id, this->channel_name);
                    this->upcomming_keyframes.erase(transition_id);
                }
            }
            ::spdlog::debug("Starting execution of transition {} in channel {}.", transition_id, this->channel_name);
            return this->upcomming_keyframes.try_emplace(transition_id, frames, 0, this->current_value).second;
        }

        COMPILER_SUPRESS("-Weffc++")
        [[nodiscard]] inline T* get_channel_pointer() {
            return &(this->current_value);
        }
        COMPILER_RESTORE("-Weffc++")

        inline void clear() {
            this->upcomming_keyframes.clear();
            if (this->apply_default_value_on_clear_command) {
                this->current_value = this->default_value;
            }
        }

        [[nodiscard]] inline std::string get_name() const {
            return this->channel_name;
        }
    private:
        void perform_update_arbiting(std::vector<T>& values) {
            if (values.empty()) [[unlikely]] {
                return;
            }
            switch(this->i_method) {
                default:
                case interleaving_method::AVERAGE:
                    {
                        if constexpr (std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value) {
                            this->current_value = (T) (std::accumulate(values.begin(), values.end(), 0) / values.size());
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
                            this->current_value.setRed((uint16_t) ((r/vlen)*65535.0*i));
                            this->current_value.setGreen((uint16_t) ((g/vlen)*65535.0*i));
                            this->current_value.setBlue((uint16_t) ((b/vlen)*65535.0*i));
                        }
                    }
                    break;
                case interleaving_method::MAX:
                    {
                        if constexpr (std::is_same<T, dmxfish::dmx::pixel>::value) {
                            uint16_t max_r = 0, max_g = 0, max_b = 0;
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
                            uint16_t min_r = 65535, min_g = 65535, min_b = 65535;
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
