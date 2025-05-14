//
// Created by doralitze on 3/31/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "event_source_factory.hpp"

#include "events/event.hpp"
#include "events/event_source.hpp"
#include "events/event_storage.hpp"
#include "midi/midirtp_event_source.hpp"
#include "sound/audioinput_event_source.hpp"

#include "lib/logging.hpp"
#include "main.hpp"

COMPILER_SUPRESS("-Wuseless-cast")
#include "proto_src/MessageTypes.pb.h"
COMPILER_RESTORE("-Wuseless-cast")

namespace dmxfish::events {
    bool construct_or_update_event_source_from_message(const missiondmx::fish::ipcmessages::event_sender& msg) {
        auto storage_ptr = get_event_storage_instance();
        auto s_ptr = storage_ptr->find_source_by_name(msg.name());
        bool success = false;
        if(s_ptr == nullptr) {
            const auto& type = msg.type();
            if (type == "fish.builtin.plain") {
                s_ptr = event_source::create<event_source>(storage_ptr, msg.name());
            } else if(type == "fish.builtin.midirtp") {
                s_ptr = event_source::create<dmxfish::midi::midirtp_event_source>(storage_ptr, msg.name());
            } else if (type == "fish.builtin.audioextract") {
                s_ptr = event_source::create<dmxfish::audio::audioinput_event_source>(storage_ptr, msg.name());
            } else {
                ::spdlog::error("Event sender type '{}' not yet implemented or unknown.", type);
                return false;
            }
            // TODO develop event_source fish.builtin.midi
            // TODO develop event_source fish.builtin.gpio
            // TODO develop event_source fish.builtin.macrokeypad
        }
        success = s_ptr->update_conf_from_message(msg);
        const auto update_msg = s_ptr->encode_proto_message();
        get_iomanager_instance()->push_msg_to_all_gui(update_msg, ::missiondmx::fish::ipcmessages::MSGT_EVENT_SENDER_UPDATE);
        if (success) {
            ::spdlog::info("Successfully created / updated event source '{}' of type {}.", msg.name(), msg.type());
        }
        return success;
    }

    bool insert_event_from_message(const missiondmx::fish::ipcmessages::event& msg) {
        auto storage_ptr = get_event_storage_instance();
        event_sender_t es;
        es.decoded_representation.sender = msg.sender_id();
        es.decoded_representation.sender_function = msg.sender_function();
        event e{(event_type) ((unsigned int) msg.type()), es};
        size_t i = 0;
        for (const auto c : msg.arguments()) {
            if (i > 7) {
                break;
            }
            e.set_arg_data(i, (uint8_t) c);
            i++;
        }
        return storage_ptr->insert_event(e);
    }
}
