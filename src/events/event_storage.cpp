//
// Created by doralitze on 11/15/23.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "event_storage.hpp"

#include <limits>
#include <stdexcept>
#include "event_source.hpp"
#include "main.hpp"

COMPILER_SUPRESS("-Wuseless-cast")
#include "proto_src/MessageTypes.pb.h"
#include "proto_src/Events.pb.h"
COMPILER_RESTORE("-Wuseless-cast")

namespace dmxfish::events {

    event_storage::event_storage() : storage_a(), storage_b(), senders(), storage_swap_mutex(), ongoing_events() {
        // Experiments have shown that we grow to a size of approx. 50 very quick and usually stay there
        storage_a.reserve(50);
        storage_b.reserve(50);
    }

    event_storage::~event_storage() {
        for(auto& registered_source : this->senders) {
            registered_source->deregister();
        }
    }

    [[nodiscard]] bool event_storage::insert_event(const event& e) {
        if(!this->storage_swap_mutex.try_lock()) {
            return false;
        }
        auto& storage = !this->current_read_storage_is_a ? this->storage_a : this->storage_b;
        storage.emplace_back(e);
        this->storage_swap_mutex.unlock();
        const auto& sender = e.get_event_sender().decoded_representation.sender;
        if(this->senders[sender]->remote_debug_enabled) {
            missiondmx::fish::ipcmessages::event msg;
            msg.set_type((::missiondmx::fish::ipcmessages::event_type) ((uint8_t) e.get_type()));
            msg.set_sender_id(sender);
            msg.set_sender_function(e.get_event_sender().decoded_representation.sender_function);
            msg.set_event_id(e.get_event_id());
            for (const auto& arg : e.get_args()) {
                msg.add_arguments((unsigned int) arg);
            }
            get_iomanager_instance()->push_msg_to_all_gui(msg, ::missiondmx::fish::ipcmessages::MSGT_EVENT);
        }
        return true;
    }

    void event_storage::swap_buffers() {
        auto& old_read_storage = this->current_read_storage_is_a ? this->storage_a : this->storage_b;
        old_read_storage.clear();
        this->storage_swap_mutex.lock();
        this->current_read_storage_is_a = !this->current_read_storage_is_a;
        this->storage_swap_mutex.unlock();

        auto& new_read_storage = this->current_read_storage_is_a ? this->storage_a : this->storage_b;
        for(auto& ev : new_read_storage) {
            switch (ev.get_type()) {
                case event_type::SINGLE_TRIGGER:
                case event_type::ONGOING_EVENT:
                case event_type::INVALID:
                default:
                    break;
                case event_type::START:
                    this->ongoing_events[ev.get_event_sender()] = event{event_type::ONGOING_EVENT, ev};
                    break;
                case event_type::RELEASE:
                    this->ongoing_events.erase(ev.get_event_sender());
                    break;
            }
        }

        for (auto& lasting_ev : this->ongoing_events) {
            new_read_storage.push_back(lasting_ev.second);
        }
    }

    event_sender_t event_storage::register_event_source(const std::shared_ptr<event_source>& self) {
        const unsigned long next_slot = this->senders.size();
        if(next_slot > std::numeric_limits<uint32_t>::max()) [[unlikely]] {
            throw std::range_error("The amount of event senders reached the limit that can be stored. "
                                   "This seams to be a bug.");
        }
        event_sender_t next_id{};
        next_id.decoded_representation.sender = next_slot;
        this->senders.emplace_back(self);
        return (event_sender_t) next_id;
    }

    std::shared_ptr<event_source> event_storage::find_source_by_name(const std::string& name) {
        if (name.empty()) [[unlikely]] {
            return nullptr;
        }
        auto& m = this->index_source_by_name;
        if(m.count(name) == 0) {
            return nullptr;
        }
        auto wptr = this->index_source_by_name.at(name);
        if (wptr.expired()) [[unlikely]] {
            m.erase(name);
            return nullptr;
        }
        return wptr.lock();
    }
}