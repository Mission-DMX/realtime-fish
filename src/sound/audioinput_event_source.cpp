//
// Created by Doralitze on 11.05.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "audioinput_event_source.hpp"

namespace dmxfish::audio {

    audioinput_event_source::audioinput_event_source() {

    }

    missiondmx::fish::ipcmessages::event_sender audioinput_event_source::encode_proto_message() const {
        auto msg = event_source::encode_proto_message();
        msg.set_type("fish.builtin.audioextract");
        auto conf = (*mutable_configuration) msg.configuration();
        conf["dev"] = this->sound_dev_file;
        // TODO transmit cuttoff settings
        return msg;
    }

    bool audioinput_event_source::update_conf_from_message(const missiondmx::fish::ipcmessages::event_sender& msg) {
        // TODO start/stop threads
        // TODO open new sound dev if changed
    }
}