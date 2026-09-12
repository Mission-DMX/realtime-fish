//
// Created by leondietrich on 9/12/26.
//

#include "event_scheduler.hpp"

#include <string>

#include "main.hpp"
#include "lib/macros.hpp"
#include "filters/types.hpp"
#include "utils.hpp"
#include "../main.hpp"

#include "proto_src/MessageTypes.pb.h"
#include "proto_src/FilterMode.pb.h"

namespace dmxfish {
    namespace filters {

        event_scheduler::event_scheduler() : filter(),
        synchronization_target{}, current_step{0}, max_step{0}, sequence_steps{}, event_templates{}, own_filter_id{} {}

        event_scheduler::~event_scheduler() {}

        void event_scheduler::setup_filter(
                const std::map<std::string, std::string>& configuration,
                const std::map<std::string, std::string>& initial_parameters,
                const channel_mapping& input_channels,
                const std::string& own_id) {
            this->own_filter_id = own_id;
            if (!initial_parameters.contains("length")) [[unlikely]] {
                throw filter_config_exception("Initial parameters do not contain length.", filter_type::filter_event_scheduler, own_id);
            }
            if (!configuration.contains("event_data")) [[unlikely]] {
                throw filter_config_exception("Configuration does not contain event_data.", filter_type::filter_event_scheduler, own_id);
            }
            this->event_templates.clear();
            for (auto event_template_str : utils::split(configuration.at("event_data"), ';')) {
                auto event_template_parts = utils::split(event_template_str, ',');
                const auto sender_id = std::stol(event_template_parts.front());
                event_template_parts.pop_front();
                const auto sender_function = std::stol(event_template_parts.front());
                event_template_parts.pop_front();
                const dmxfish::events::event_type evtype{std::stoi(event_template_parts.front())};
                event_template_parts.pop_front();
                event_template templ;
                for (auto i = 0; i < templ.arguments.size() && !event_template_parts.empty(); i++) {
                    templ.arguments[i] = (uint8_t) std::stoi(event_template_parts.front());
                    event_template_parts.pop_front();
                }
                templ.type = evtype;
                templ.sender = {sender_id, sender_function};
                this->event_templates.emplace_back(templ);
            }
            this->receive_update_from_gui("length", initial_parameters.at("length"));
            for (auto entry : this->sequence_steps) {
                entry = false;
            }
            if (initial_parameters.contains("update_triggers")) {
                this->receive_update_from_gui("update_triggers", initial_parameters.at("update_triggers"));
            }
            if (initial_parameters.contains("step")) {
                this->receive_update_from_gui("step", initial_parameters.at("step"));
            }
            if (initial_parameters.contains("synchronization_target")) {
                this->receive_update_from_gui("synchronization_target", initial_parameters.at("synchronization_target"));
            }
        }

        bool event_scheduler::receive_update_from_gui(
                const std::string& key, const std::string& _value) {
            if (key == "length") {
                const auto new_size = std::stol(_value);
                if (new_size < 0) [[unlikely]] {
                    return false;
                }
                this->sequence_steps.resize(new_size * this->event_templates.size(), false);
                this->max_step = new_size;
                return true;
            }
            if (key == "step") {
                const auto new_step = std::stol(_value) % this->max_step;
                return true;
            }
            if (key == "update_triggers") {
                for (auto& date : utils::split(_value, ';')) {
                    auto parts = utils::split(date, ',');
                    const auto step = parts.front();
                    parts.pop_front();
                    const auto event = parts.front();
                    parts.pop_front();
                    const auto value = parts.front();
                    if (const auto step_l = std::stol(step); step_l >= 0 && step_l < this->max_step) {
                        if (const auto event_l = std::stol(event); event_l >= 0 && event_l < this->event_templates.size()) {
                            this->sequence_steps[step_l * event_l] = utils::toupper(value) == "TRUE";
                        } else {
                            return false;
                        }
                    } else {
                        return false;
                    }
                }
                return true;
            }
            if (key == "synchronization_target") {
                auto parts = utils::split(_value, ',');
                const auto sender_str = parts.front();
                parts.pop_front();
                const auto function_str = parts.front();
                this->synchronization_target = dmxfish::events::event_sender_t{std::stol(sender_str), std::stol(function_str)};
                return true;
            }
            return false;
        }

        void event_scheduler::update() {
            bool found = false;
            for(const auto& event : get_event_storage_instance()->get_storage()) {
                if (event.get_event_sender() == this->synchronization_target) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return;
            }
            this->current_step++;
            if (this->current_step >= this->max_step) {
                this->current_step = 0;
            }
            auto es = get_event_storage_instance();
            auto iom = get_iomanager_instance();
            auto update_message = missiondmx::fish::ipcmessages::update_parameter();
            update_message.set_filter_id(this->own_filter_id);
            update_message.set_parameter_key("step");
            update_message.set_scene_id(iom->get_active_show()->get_current_scene_id());
            update_message.set_parameter_value(std::to_string(this->current_step));
            iom->push_msg_to_all_gui(update_message, ::missiondmx::fish::ipcmessages::MSGT_UPDATE_PARAMETER);

            const auto size = this->event_templates.size();
            for(auto i = 0; i < size; i++) {
                if (this->sequence_steps[(this->current_step * size) + i]) {
                    const auto& t = this->event_templates[i];
                    dmxfish::events::event e{t.type, t.sender};
                    e.set_args(t.arguments);
                    es->insert_event(e);
                }
            }
        }

        void event_scheduler::scene_activated() {
            this->current_step = 0;
        }

        void event_scheduler::get_output_channels(channel_mapping& map, const std::string& name) {
            MARK_UNUSED(map);
            MARK_UNUSED(name);
            return;
        }
    } // filters
} // dmxfish