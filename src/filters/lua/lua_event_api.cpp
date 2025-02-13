//
// Created by Leon Dietrich on 12.02.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//
#include "filters/lua/lua_event_api.hpp"

#include <memory>
#include <stdexcept>
#include <vector>

#include "events/event.hpp"
#include "events/event_storage.hpp"
#include "events/event_source.hpp"
#include "main.hpp"

namespace dmxfish::filters::lua {

    static std::shared_ptr<dmxfish::events::event_source> lua_event_sender;

    std::vector<dmxfish::events::event> get_events() {
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

    void insert_event(dmxfish::events::event_sender_t sender_id, dmxfish::events::event_type t, std::string args) {
        using namespace dmxfish::events;
        event e(t, sender_id);
        e.set_args_as_string(args);
        get_event_storage_instance()->insert_event(e);
    }

    void insert_event_sa(dmxfish::events::event_sender_t sender_id, std::string args) {
        insert_event(sender_id, dmxfish::events::event_type::SINGLE_TRIGGER, args);
    }

    void insert_event_s(dmxfish::events::event_sender_t sender_id) {
        insert_event(sender_id, dmxfish::events::event_type::SINGLE_TRIGGER, "");
    }

    uint64_t get_event_sender_id(uint32_t function) {
        if (lua_event_sender == nullptr) [[unlikely]] {
            return 0;
        }
        const auto id = lua_event_sender->get_sender_id();
        dmxfish::events::event_sender_t es;
        es.decoded_representation.sender = id;
        es.decoded_representation.sender_function = function;
        return es.encoded_sender_id;
    }

    uint64_t get_event_sender_id_auto() {
        return get_event_sender_id(0);
    }

    std::vector<uint64_t> get_all_event_senders() {
        std::vector<uint64_t> v;
        auto& sv = get_event_storage_instance()->get_registered_senders();
        v.reserve(sv.size());
        for (auto& es_ptr : sv) {
            dmxfish::events::event_sender_t es;
            es.decoded_representation.sender = es_ptr->get_sender_id();
            es.decoded_representation.sender_function = 0;
            v.push_back(es.encoded_sender_id);
        }
        return v;
    }

    void insert_event_n() {
        insert_event(get_event_sender_id_auto(), dmxfish::events::event_type::SINGLE_TRIGGER, "");
    }

    void init_lua_event_api(sol::state& lua) {
        if(lua_event_sender == nullptr) {
            try {
                lua_event_sender = dmxfish::events::event_source::create<dmxfish::events::event_source>(
                        get_event_storage_instance());
            } catch (const std::invalid_argument& e) {
                throw std::runtime_error(std::string("Global Event Storage not yet initialized. Base exception: ") + e.what());
            }
        }

        lua.new_usertype<dmxfish::events::event>(
                "Event",
                "type", sol::property(&dmxfish::events::event::get_type),
                "valid", sol::property(&dmxfish::events::event::is_valid),
                "sender", sol::property(&dmxfish::events::event::get_event_sender),
                "id", sol::property(&dmxfish::events::event::get_event_id),
                "args", sol::property(&dmxfish::events::event::get_args_as_str,
                                      &dmxfish::events::event::set_args_as_string));

        lua["event_type"] = lua.create_table_with(
                "SINGLE_TRIGGER", dmxfish::events::event_type::SINGLE_TRIGGER,
                "START", dmxfish::events::event_type::START,
                "RELEASE", dmxfish::events::event_type::RELEASE,
                "ONGOING_EVENT", dmxfish::events::event_type::ONGOING_EVENT,
                "INVALID", dmxfish::events::event_type::INVALID
                );

        lua.set_function("get_events", dmxfish::filters::lua::get_events);
        lua.set_function("has_event", dmxfish::filters::lua::has_event);
        lua.set_function("get_all_event_senders", dmxfish::filters::lua::get_all_event_senders);

        lua.set_function("insert_event", sol::overload(
                dmxfish::filters::lua::insert_event,
                dmxfish::filters::lua::insert_event_sa,
                dmxfish::filters::lua::insert_event_s,
                dmxfish::filters::lua::insert_event_n
                ));
        lua.set_function("get_event_sender", sol::overload(
                dmxfish::filters::lua::get_event_sender_id,
                dmxfish::filters::lua::get_event_sender_id_auto));
    }
}