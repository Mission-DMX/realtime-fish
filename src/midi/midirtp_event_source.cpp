//
// Created by doralitze on 5/6/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "midi/midirtp_event_source.hpp"

namespace dmxfish::midi {
    midirtp_event_source::midirtp_event_source() : event_source{} {
        // TODO setup io resources
    }

    missiondmx::fish::ipcmessages::event_sender midirtp_event_source::encode_proto_message() const {
        auto msg = event_source::encode_proto_message();
        msg.set_type("fish.builtin.midirtp");
        auto conf = msg.configuration();
        // TODO transmit connection settings
        return msg;
    }

    bool midirtp_event_source::update_conf_from_message(const missiondmx::fish::ipcmessages::event_sender& msg) {
        // TODO implement
        return false;
    }

    // TODO implement io callbacks
}