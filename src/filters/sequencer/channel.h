//
// Created by Leon Dietrich on 13.02.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <deque>
#include <unordered_map>
#include <tuple>

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

        void apply_update(sequencer_time_t current_time) {
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
                do {
                    if (keyframe_queue.empty()) {
                        transitions_to_remove.push_back(trans_id);
                        break;
                    }
                    auto &current_frame = keyframe_queue.front();
                    if (current_time >= keyframe_start_time + current_frame.duration) {
                        keyframe_start_time = current_time;
                        keyframe_start_value = current_frame.value;
                        keyframe_queue.pop_front();
                        continue;
                    }
                    requested_values.push_back(current_frame.calculate_update(current_time - keyframe_start_time, keyframe_start_value));
                    break;
                } while (true);
            }
            this->perform_update_arbiting(requested_values);
            for (auto i : transitions_to_remove) {
                // TODO test if this is really removing the key and not the position
                this->upcomming_keyframes.erase(i);
            }
        }
    private:
        void perform_update_arbiting(const std::vector<T>& values) {
            switch(this->i_method) {
                default:
                case interleaving_method::AVERAGE:
                    // TODO implement and update current_value
                    break;
                case interleaving_method::MAX:
                {
                    // TODO implement and update current_value
                }
                    break;
                case interleaving_method::MIN:
                    // TODO implement min and update current_value
                    break;
            }
        }
    };
}
