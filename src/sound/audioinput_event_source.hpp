//
// Created by Doralitze on 11.05.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <string>

#include "events/event_source.hpp"

namespace dmxfish::audio {

    class audioinput_event_source : public dmxfish::events::event_source {
    private:
        std::string sound_dev_file;
    public:
        audioinput_event_source();
        [[nodiscard]] missiondmx::fish::ipcmessages::event_sender encode_proto_message() const override;
        [[nodiscard]] bool update_conf_from_message(const missiondmx::fish::ipcmessages::event_sender& msg) override;
    };

}