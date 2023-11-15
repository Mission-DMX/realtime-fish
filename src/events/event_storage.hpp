//
// Created by doralitze on 11/15/23.
//

#pragma once

#include <cstddef>
#include <vector>
#include <memory>

#include "event.hpp"

namespace dmxfish::events {

    class event_source;

    class event_storage {
    private:
        std::vector<event> storage_a, storage_b;
        std::vector<std::shared_ptr<event_source>> senders;
        // TODO think about good active events data structure
        bool current_read_storage_is_a = false;
    public:
        friend class event_source;

        event_storage();
        ~event_storage();

        /**
         * This method stores an event inside the queue to be processed on the next cycle.
         * @param e The event to store
         */
        void insert_event(const event& e);

        /**
         * This method should be called prior to executing the next cycle as it will swap the event queues.
         */
        void swap_buffers();

        /**
         * Use this method in order to access the stored events during a cycle
         * @return
         */
        [[nodiscard]] const std::vector<event>& get_storage() const {
            return this->current_read_storage_is_a ? this->storage_a : this->storage_b;
            // TODO change this to return a view on the active event storage
        }
    private:
        event_sender_t  register_event_source(const std::shared_ptr<event_source>& self);
    };


}