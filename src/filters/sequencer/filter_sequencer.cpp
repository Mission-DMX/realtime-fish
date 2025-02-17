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

        void filter_sequencer::enqueue_transition(const sequencer::transition& t) {
            // TODO This check is expensive. We need to see if it's too expensive and we need to live with partial resets
            //  in case of transitions of uneven length.
            if(!t.is_reset_allowed()) {
                for (const auto cid : t.affected_channel_ids) {
                    if (this->channels_8bit[cid].transition_active(t.get_transition_id())
                    || this->channels_16bit[cid].transition_active(t.get_transition_id())
                    || this->channels_float[cid].transition_active(t.get_transition_id())
                    || this->channels_color[cid].transition_active(t.get_transition_id())) {
                        return;
                    }
                }
            }
            for (const auto& [channel_id, frames]: t.frames_8bit) {
                this->channels_8bit[channel_id].insert_keyframes(frames, t.get_transition_id(), t.is_reset_allowed());
            }
            for (const auto& [channel_id, frames]: t.frames_16bit) {
                this->channels_16bit[channel_id].insert_keyframes(frames, t.get_transition_id(), t.is_reset_allowed());
            }
            for (const auto& [channel_id, frames]: t.frames_float) {
                this->channels_float[channel_id].insert_keyframes(frames, t.get_transition_id(), t.is_reset_allowed());
            }
            for (const auto& [channel_id, frames]: t.frames_color) {
                this->channels_color[channel_id].insert_keyframes(frames, t.get_transition_id(), t.is_reset_allowed());
            }
        }

        void filter_sequencer::update() {
            for (const auto& event : get_event_storage_instance()->get_storage()) {
                for(auto [iter, range_end] = this->transitions.equal_range(event.get_event_sender().encoded_sender_id);
                        iter != range_end; iter++) {
                    this->enqueue_transition(iter->second);
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
            for (auto& c : this->channels_8bit) {
                c.clear();
            }
            for (auto& c : this->channels_16bit) {
                c.clear();
            }
            for (auto& c : this->channels_float) {
                c.clear();
            }
            for (auto& c : this->channels_color) {
                c.clear();
            }
        }
    } // filters
} // dmxfish
