//
// Created by doralitze on 5/6/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include "events/event_source.hpp"

namespace dmxfish::midi {
    class midirtp_event_source : public dmxfish::events::event_source {
    public:
        midirtp_event_source();
        [[nodiscard]] missiondmx::fish::ipcmessages::event_sender encode_proto_message() const override;
        [[nodiscard]] bool update_conf_from_message(const missiondmx::fish::ipcmessages::event_sender& msg) override;
    };

}