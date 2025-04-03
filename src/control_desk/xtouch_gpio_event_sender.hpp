//
// Created by doralitze on 4/1/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <array>

#include "events/event_source.hpp"

namespace dmxfish {
    namespace control_desk {

    class xtouch_gpio_event_sender : public dmxfish::events::event_source {
        int expression_pedal_threshold = 0;
    public:
        xtouch_gpio_event_sender();
        [[nodiscard]] missiondmx::fish::ipcmessages::event_sender encode_proto_message() const override;
        [[nodiscard]] bool update_conf_from_message(const missiondmx::fish::ipcmessages::event_sender& msg) override;

        bool send_message(unsigned int port, unsigned int new_state);
    };

    } // dmxfish
} // control_desk

