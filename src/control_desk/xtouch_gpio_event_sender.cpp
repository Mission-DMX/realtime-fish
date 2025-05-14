//
// Created by doralitze on 4/1/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "xtouch_gpio_event_sender.hpp"

#include <stdexcept>
#include <string>

#include "events/event.hpp"

#include "lib/logging.hpp"
#include "main.hpp"

namespace dmxfish {
    namespace control_desk {
        xtouch_gpio_event_sender::xtouch_gpio_event_sender() : event_source{} {

        }

        missiondmx::fish::ipcmessages::event_sender xtouch_gpio_event_sender::encode_proto_message() const {
            auto msg = event_source::encode_proto_message();
            auto conf = msg.mutable_configuration();
            conf->operator[]("expression_pedal_threshold") = std::to_string(this->expression_pedal_threshold);
            msg.set_type("fish.builtin.xtouchgpio");
            return msg;
        }

        bool xtouch_gpio_event_sender::update_conf_from_message(const missiondmx::fish::ipcmessages::event_sender& msg) {
            auto res = event_source::update_conf_from_message(msg);
            try {
                const auto& conf = msg.configuration();
                if(conf.contains("expression_pedal_threshold")) {
                    this->expression_pedal_threshold = std::stoi(conf.at("expression_pedal_threshold"));
                }
            } catch (const std::invalid_argument& e) {
                ::spdlog::error("Error while updating xtouch event sender config (invalid argument): {}", e.what());
                res = false;
            } catch (const std::out_of_range& e) {
                ::spdlog::error("Error while updating xtouch event sender config (out of range): {}", e.what());
                res = false;
            }
            return res;
        }

        bool xtouch_gpio_event_sender::send_message(unsigned int port, unsigned int new_state) {
            using namespace dmxfish::events;
            const event_sender_t evt{this->get_sender_id(), port};
            event ev{port == 2 ? event_type::SINGLE_TRIGGER : (new_state == 0 ? event_type::RELEASE : event_type::START), evt};
            ev.set_arg_data(0, new_state);
            return get_event_storage_instance()->insert_event(ev);
        }

    } // dmxfish
} // control_desk