//
// Created by Leon Dietrich on 12.02.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//
#include "filters/lua/lua_event_api.hpp"

#include "events/event.hpp"
#include "events/event_storage.hpp"
#include "main.hpp"

namespace dmxfish::filters::lua {

    std::vector get_events() {
        auto evs_ptr = get_event_storage_instance();
        return evs_ptr->get_storage();
    }

    bool has_event(dmxfish::events::event_sender_t source) {
        using namespace dmxfish::events;
        for(const auto& ev : get_event_storage_instance()->get_storage()) {
            if (ev.get_event_sender() == source && (ev.get_type() == event_type::START || ev.get_type() == event_type::ONGOING_EVENT || ev.get_type() == event_type::SINGLE_TRIGGER)) {
                return true;
            }
        }
        return false;
    }

    void insert_event(event_sender_t sender_id, event_type t, std::string args) {
        using namespace dmxfish::events;
        event e(t, sender_id);
        e.set_args_as_string(args);
        get_event_storage_instance()->insert_event(e);
    }

    void init_lua_event_api(sol::state& lua) {
        // TODO ensure lua_event_sender exists

        lua.new_usertype<dmxfish::events::event>(
                "Event",
                "type", sol::property(&dmxfish::events::event::get_type),
                "valid", sol::property(&dmxfish::events::event::is_valid),
                "sender", sol::property(&dmxfish::events::event::get_event_sender),
                "id", sol::property(&dmxfish::events::event::get_event_id),
                "args", sol::property(&dmxfish::events::event::get_args_as_str,
                                      &dmxfish::events::event::set_args_as_string));

        lua.set_function("get_events", dmxfish::filters::lua::get_events);
        lua.set_function("has_event", dmxfish::filters::lua::has_event);
        // TODO add function to query senders
        // TODO add overloaded methods not requiring type or sender id (using default optional sender)
        lua.set_function("insert_event", dmxfish::filters::lua::insert_event);
    }
}