//
// Created by leondietrich on 2/17/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <functional>
#include <type_traits>

#include "filters/sequencer/keyframe.hpp"
#include "filters/sequencer/time.hpp"

namespace dmxfish {
    namespace filters {
        namespace sequencer {

            template <typename T>
            class frame_queue {
                const std::vector<keyframe<T>>* storage_wrapper;
                size_t position = 0;
                sequencer_time_t start_time;
                T start_value;
                static_assert(!std::is_reference<T>::value && !std::is_pointer<T>::value, "Enclosed type must not be reference or pointer");
            public:
                //frame_queue() = delete;
                //frame_queue(frame_queue& other) = default;
                frame_queue(const std::vector<keyframe<T>>* ref_to_storage, sequencer_time_t current_time, T current_value) :
                    storage_wrapper(ref_to_storage), start_time(current_time), start_value(current_value) {}

                [[nodiscard]] inline bool empty() const {
                    return position >= storage_wrapper->size();
                }

                [[nodiscard]] inline const keyframe<T>& front() const {
                    return (*this->storage_wrapper)[position];
                }

                inline void pop_front() {
                    this->position++;
                }

                [[nodiscard]] inline sequencer_time_t get_start_time() const {
                    return this->start_time;
                }

                inline void set_start_time(sequencer_time_t current_time) {
                    this->start_time = current_time;
                }

                [[nodiscard]] inline T get_start_value() const {
                    return this->start_value;
                }

                inline void set_start_value(const T& current_value) {
                    this->start_value = current_value;
                }

                inline void advance(sequencer_time_t current_time, const T& current_value) {
                    this->set_start_time(current_time);
                    this->set_start_value(current_value);
                    this->pop_front();
                }
            };

        } // dmxfish
    } // filters
} // sequencer

