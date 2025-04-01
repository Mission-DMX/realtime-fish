//
// Created by doralitze on 4/1/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "xtouch_gpio_event_sender.hpp"

#include <stdexcept>
#include <string>

#include "lib/logging.hpp"

namespace dmxfish {
    namespace control_desk {
        xtouch_gpio_event_sender::xtouch_gpio_event_sender() : event_source{} {

        }

        missiondmx::fish::ipcmessages::event_sender xtouch_gpio_event_sender::encode_proto_message() const {
            auto msg = event_source::encode_proto_message();
            auto conf = msg.configuration();
            conf["expression_pedal_threshold"] = std::to_string(this->expression_pedal_threshold);
            msg.set_type("fish.builtin.gpio");
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
    } // dmxfish
} // control_desk