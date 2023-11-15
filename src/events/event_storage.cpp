//
// Created by doralitze on 11/15/23.
//

#include "event_storage.hpp"

#include "event_source.hpp"

namespace dmxfish::events {

    event_storage::event_storage() : storage_a(), storage_b() {

    }

    event_storage::~event_storage() {
        for(auto& registered_source : this->senders) {
            registered_source->deregister();
        }
    }

    void event_storage::insert_event(const event& e) {
        // TODO aquire a lock for the write storage
        auto& storage = !this->current_read_storage_is_a ? this->storage_a : this->storage_b;
        storage.emplace_back(e);
        // TODO insert or release lasting events is the event type is start or release
        // TODO release the write storage lock
    }

    void event_storage::swap_buffers() {
        auto& storage = this->current_read_storage_is_a ? this->storage_a : this->storage_b;
        storage.clear();
        // TODO aquire a lock on the write storage
        this->current_read_storage_is_a = !this->current_read_storage_is_a;
        // TODO release lock from write storage
        // TODO generate active events storage
    }

    event_sender_t event_storage::register_event_source(const std::shared_ptr<event_source>& self) {
        const unsigned long next_id = this->senders.size();
        this->senders.emplace_back(self);
        return (event_sender_t) next_id;
    }
}