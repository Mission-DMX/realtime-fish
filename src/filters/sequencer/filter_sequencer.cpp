//
// Created by Leon Dietrich on 13.02.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "filters/sequencer/filter_sequencer.hpp"

#include <ranges>

#include "main.hpp"

namespace dmxfish {
    namespace filters {
        filter_sequencer::filter_sequencer() : channels_8bit(), channels_16bit(), channels_float(), channels_color(), transitions() {

        }

        filter_sequencer::~filter_sequencer() {

        }

        void filter_sequencer::setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) {
            // TODO construct transitions and make sure that their ids are unique
        }

        bool filter_sequencer::receive_update_from_gui(const std::string& key, const std::string& _value) {

        }

        void filter_sequencer::get_output_channels(channel_mapping& map, const std::string& name) {

        }

        void filter_sequencer::enqueue_transition(const transition& t, uint64_t event) {
            if(!t.reset_allowed) {
                for (const auto cid : t.affected_channel_ids) {
                    if (this->channels_8bit[cid].transition_active(t.id) || this->channels_16bit[cid].transition_active(t.id)
                    || this->channels_float[cid].transition_active(t.id) || this->channels_color[cid].transition_active(t.id)) {
                        return;
                    }
                }
            }
            {
                std::unordered_map<size_t, std::deque<sequencer::keyframe<uint8_t>>> queues;
                for (const auto& frame: t.frames_8bit) {
                    if (!queues.contains(frame.first)) {
                        queues[frame.first] = {};
                    }
                    queues[frame.first].push_back(frame.second);
                }
                for (auto& [channel_id, queue] : queues) {
                    this->channels_8bit[channel_id].insert_keyframes(queue, t.id);
                }
            }
            {
                std::unordered_map<size_t, std::deque<sequencer::keyframe<uint16_t>>> queues;
                for (const auto& frame: t.frames_16bit) {
                    if (!queues.contains(frame.first)) {
                        queues[frame.first] = {};
                    }
                    queues[frame.first].push_back(frame.second);
                }
                for (auto& [channel_id, queue] : queues) {
                    this->channels_16bit[channel_id].insert_keyframes(queue, t.id);
                }
            }
            {
                std::unordered_map<size_t, std::deque<sequencer::keyframe<double>>> queues;
                for (const auto& frame: t.frames_float) {
                    if (!queues.contains(frame.first)) {
                        queues[frame.first] = {};
                    }
                    queues[frame.first].push_back(frame.second);
                }
                for (auto& [channel_id, queue] : queues) {
                    this->channels_float[channel_id].insert_keyframes(queue, t.id);
                }
            }
            {
                std::unordered_map<size_t, std::deque<sequencer::keyframe<dmxfish::dmx::pixel>>> queues;
                for (const auto& frame: t.frames_color) {
                    if (!queues.contains(frame.first)) {
                        queues[frame.first] = {};
                    }
                    queues[frame.first].push_back(frame.second);
                }
                for (auto& [channel_id, queue] : queues) {
                    this->channels_color[channel_id].insert_keyframes(queue, t.id);
                }
            }
        }

        void filter_sequencer::update() {
            for (const auto& event : get_event_storage_instance()->get_storage()) {
                const auto found_transitions = this->transitions | std::views::filter([](auto& v) {return v.first == event;});
                if(const auto trans : found_transitions) {
                    this->enqueue_transition(trans, event);
                }
            }

            auto current_time = *(this->input_time);
            for (auto& c : this->channels_8bit) {
                c.apply_update(current_time, *time_scale);
            }
            for (auto& c : this->channels_16bit) {
                c.apply_update(current_time, *time_scale);
            }
            for (auto& c : this->channels_float) {
                c.apply_update(current_time, *time_scale);
            }
            for (auto& c : this->channels_color) {
                c.apply_update(current_time, *time_scale);
            }
        }

        void filter_sequencer::scene_activated() {
            // TODO clear transition queues of channels
            // TODO reset channels to their default value if required
        }
    } // filters
} // dmxfish
