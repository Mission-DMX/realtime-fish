//
// Created by doralitze on 11/15/23.
//

#include "event_source.hpp"
#include "event_storage.hpp"

namespace dmxfish::events {
    event_source::event_source() : sender_id{} {}

    event_source::event_source(event_source* other) : sender_id(other->sender_id) {}

    void event_source::deregister() {

    }

    event_source::~event_source() {
    }

    missiondmx::fish::ipcmessages::event_sender event_source::encode_proto_message() const {
        missiondmx::fish::ipcmessages::event_sender msg;
        msg.set_sender_id(this->sender_id.decoded_representation.sender);
        msg.set_type("fish.builtin.plain");
        msg.set_name(this->name);
        msg.set_gui_debug_enabled(this->remote_debug_enabled);
        return msg;
    }

    bool event_source::update_conf_from_message(const missiondmx::fish::ipcmessages::event_sender& msg) {
        this->remote_debug_enabled = msg.gui_debug_enabled();
        // we cannot update the name, type or sender id.
        return true;
    }
} // dmxfish