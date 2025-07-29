//
// Created by Leon Dietrich on 13.02.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "filters/sequencer/filter_sequencer.hpp"

#include <functional>
#include <ranges>
#include <sstream>
#include <string>

#include "events/event.hpp"
#include "events/sender_parsing.hpp"
#include "utils.hpp"
#include "lib/logging.hpp"
#include "main.hpp"

#include "proto_src/MessageTypes.pb.h"
#include "proto_src/FilterMode.pb.h"

namespace dmxfish {
    namespace filters {
        filter_sequencer::filter_sequencer() : channels_8bit(), channels_16bit(), channels_float(), channels_color(), transitions() {

        }

        filter_sequencer::~filter_sequencer() {

        }

        void filter_sequencer::decode_input_channels(const channel_mapping& input_channels, const std::string& own_id) {
            if (!input_channels.float_channels.contains("time")) {
                throw filter_config_exception("Unable to link input of sequencer filter: channel mapping does not "
                                              "contain channel 'time' of type 'double'. This input should come from "
                                              "the scenes global time node.", filter_type::filter_sequencer, own_id);
            }
            this->input_time = input_channels.float_channels.at("time");

            if (!input_channels.float_channels.contains("time_scale")) {
                throw filter_config_exception("Unable to link input of sequencer filter: channel mapping does not "
                                              "contain channel 'time_scale' of type 'double'. If you dont want to "
                                              "change the speed you can link a constant with value 1.0.",
                                              filter_type::filter_sequencer, own_id);
            }
            this->time_scale = input_channels.float_channels.at("time_scale");
        }

        void filter_sequencer::ensure_uniqueness(std::map<std::string, size_t>& m, const std::string& new_name, const std::string& own_id, size_t channel_id) {
            if(m.contains(new_name)) {
                throw filter_config_exception("Channel name '" + new_name + "' is not unique.", filter_type::filter_sequencer, own_id);
            }
            m[new_name] = channel_id;
        }

        void filter_sequencer::construct_channels(const std::map<std::string, std::string>& configuration, const std::string& own_id, name_maps& nm) {
            if(!configuration.contains("channels")) {
                throw filter_config_exception("Expected the configuration to contain a channel definition.", filter_type::filter_sequencer, own_id);
            }
            const auto& channel_map_str = configuration.at("channels");
            if(channel_map_str.empty()) [[unlikely]] {
                ::spdlog::warn("Found empty channel definition while constructing filter '{}'.", own_id);
            }

            std::map<std::string, size_t> global_chan_map;
            size_t current_start = 0, next_end = channel_map_str.find(";", 0);
            bool final_round = false;
            do {
                final_round = (next_end == std::string::npos);
                auto param_list = utils::split(channel_map_str.substr(current_start, next_end - current_start), ':');
                if (param_list.size() < 6) {
                    throw filter_config_exception("Channel definition contains underspecified channels.", filter_type::filter_sequencer, own_id);
                }
                const auto name = param_list.front();
                param_list.pop_front();
                const auto data_type = utils::toupper(param_list.front());
                param_list.pop_front();
                const auto default_value = param_list.front();
                param_list.pop_front();
                const auto default_on_empty = utils::stob(param_list.front());
                param_list.pop_front();
                const auto default_on_clear = utils::stob(param_list.front());
                param_list.pop_front();
                const auto interleaving_method = sequencer::interleaving_method_from_string(param_list.front());
                param_list.pop_front();

                if (data_type == "8BIT") {
                    const auto channel_id = this->channels_8bit.size();
                    this->ensure_uniqueness(global_chan_map, name, own_id, channel_id);
                    nm.name_to_id_8bit[name] = channel_id;
                    this->channels_8bit.emplace_back(name, std::stoi(default_value), default_on_empty, default_on_clear, interleaving_method);
                } else if (data_type == "16BIT") {
                    const auto channel_id = this->channels_16bit.size();
                    this->ensure_uniqueness(global_chan_map, name, own_id, channel_id);
                    nm.name_to_id_16bit[name] = channel_id;
                    this->channels_16bit.emplace_back(name, std::stoi(default_value), default_on_empty, default_on_clear, interleaving_method);
                } else if (data_type == "FLOAT") {
                    const auto channel_id = this->channels_float.size();
                    this->ensure_uniqueness(global_chan_map, name, own_id, channel_id);
                    nm.name_to_id_float[name] = channel_id;
                    this->channels_float.emplace_back(name, std::stod(default_value), default_on_empty, default_on_clear, interleaving_method);
                } else if (data_type == "COLOR") {
                    const auto channel_id = this->channels_color.size();
                    this->ensure_uniqueness(global_chan_map, name, own_id, channel_id);
                    nm.name_to_id_color[name] = channel_id;
                    this->channels_color.emplace_back(name, dmxfish::dmx::stopixel(default_value), default_on_empty, default_on_clear, interleaving_method);
                } else {
                    throw filter_config_exception(std::string("Unknown channel data type: '") + data_type
                        + std::string("'."), filter_type::filter_sequencer, own_id);
                }

                current_start = ++next_end;
                next_end = channel_map_str.find(";", next_end);
            } while (!final_round);
        }

        void filter_sequencer::construct_transitions(const std::map<std::string, std::string>& configuration, const std::string& own_id, name_maps& nm) {
            if(const auto trans_iter = configuration.find("transitions"); trans_iter == configuration.end()) [[unlikely]] {
                throw filter_config_exception("Configuration map does not contain 'transitions'.", filter_type::filter_sequencer, own_id);
            } else {
                try {
                    for (const auto& transition_str: utils::split(trans_iter->second, ';')) {
                        auto glob_param = utils::split(transition_str, '#');
                        const auto trigger_event_id = dmxfish::events::parse_sender_representation(glob_param.front()).encoded_sender_id;
                        glob_param.pop_front();
                        const auto name = glob_param.front();
                        glob_param.pop_front();
                        this->transitions.insert({trigger_event_id, sequencer::transition(name, glob_param, nm)});
                    }
                } catch (const std::invalid_argument& e) {
                    throw filter_config_exception(std::string("Unable to decode transitions: ") + e.what(), filter_type::filter_sequencer, own_id);
                }
            }
        }

        void filter_sequencer::pre_setup(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const std::string& own_id) {
            MARK_UNUSED(initial_parameters);
            this->tmp_name_maps = std::make_unique<name_maps>();
            this->own_filter_id = own_id;
            name_maps& nm = *(this->tmp_name_maps);
            this->construct_channels(configuration, own_id, nm);
        }

        void filter_sequencer::setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) {
            MARK_UNUSED(initial_parameters);
            name_maps& nm = *(this->tmp_name_maps);
            this->decode_input_channels(input_channels, own_id);
            this->construct_transitions(configuration, own_id, nm);
            this->tmp_name_maps = nullptr;
        }

        bool filter_sequencer::receive_update_from_gui(const std::string& key, const std::string& _value) {
            // Do nothing for now
            MARK_UNUSED(key);
            MARK_UNUSED(_value);
            return false;
        }

        void filter_sequencer::get_output_channels(channel_mapping& map, const std::string& name) {
            for (auto& c : this->channels_8bit) {
                map.eight_bit_channels[name + ":" + c.get_name()] = c.get_channel_pointer();
            }
            for (auto& c : this->channels_16bit) {
                map.sixteen_bit_channels[name + ":" + c.get_name()] = c.get_channel_pointer();
            }
            for (auto& c : this->channels_float) {
                map.float_channels[name + ":" + c.get_name()] = c.get_channel_pointer();
            }
            for (auto& c : this->channels_color) {
                map.color_channels[name + ":" + c.get_name()] = c.get_channel_pointer();
            }
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
                this->channels_8bit[channel_id].insert_keyframes(&frames, t.get_transition_id(), t.is_reset_allowed());
            }
            for (const auto& [channel_id, frames]: t.frames_16bit) {
                this->channels_16bit[channel_id].insert_keyframes(&frames, t.get_transition_id(), t.is_reset_allowed());
            }
            for (const auto& [channel_id, frames]: t.frames_float) {
                this->channels_float[channel_id].insert_keyframes(&frames, t.get_transition_id(), t.is_reset_allowed());
            }
            for (const auto& [channel_id, frames]: t.frames_color) {
                this->channels_color[channel_id].insert_keyframes(&frames, t.get_transition_id(), t.is_reset_allowed());
            }
        }

        void filter_sequencer::update() {
            bool transitions_changed = false;
            for (const auto& event : get_event_storage_instance()->get_storage()) {
                for(auto [iter, range_end] = this->transitions.equal_range(event.get_event_sender().encoded_sender_id);
                        iter != range_end; iter++) {
                    this->enqueue_transition(iter->second);
                    transitions_changed = true;
                }
            }

            const auto current_time = *(this->input_time);
            const auto ts = *(this->time_scale);
            for (auto& c : this->channels_8bit) {
                transitions_changed |= c.apply_update(current_time, ts);
            }
            for (auto& c : this->channels_16bit) {
                transitions_changed |= c.apply_update(current_time, ts);
            }
            for (auto& c : this->channels_float) {
                transitions_changed |= c.apply_update(current_time, ts);
            }
            for (auto& c : this->channels_color) {
                transitions_changed |= c.apply_update(current_time, ts);
            }
            if (transitions_changed) {
                this->send_transition_update_to_gui();
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
            this->send_transition_update_to_gui();
        }

        void filter_sequencer::send_transition_update_to_gui() const {
            auto iomanager = get_iomanager_instance();
            if (iomanager == nullptr) {
                return;
            }
            const auto active_show = iomanager->get_active_show();
            if (active_show == nullptr) {
                return;
            }
            std::stringstream ss;
            // TODO get active transitions, insert them into ss, separated by ';'
            auto update_message = missiondmx::fish::ipcmessages::update_parameter();
            update_message.set_filter_id(this->own_filter_id);
            update_message.set_parameter_key("active_transition_list");
            update_message.set_scene_id(active_show->get_active_scene());
            update_message.set_parameter_value(ss.str());
            iomanager->push_msg_to_all_gui(update_message, ::missiondmx::fish::ipcmessages::MSGT_UPDATE_PARAMETER);
        }
    } // filters
} // dmxfish
