//
// Created by doralitze on 3/31/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "event_source_factory.hpp"

#include "event_source.hpp"
#include "event_storage.hpp"

#include "main.hpp"

namespace dmxfish::events {
    bool construct_or_update_event_source_from_message(const missiondmx::fish::ipcmessages::event_sender& msg) {
        auto storage_ptr = get_event_storage_instance();
        if(const auto s_ptr = storage_ptr->find_source_by_name(msg.name()); s_ptr != nullptr) {
            return s_ptr->update_conf_from_message(msg);
        } else {
            // TODO construct sender
        }
    }
}
