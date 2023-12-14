//
// Created by doralitze on 11/15/23.
//

#pragma once

#include <cstddef>
#include <vector>
#include <map>
#include <memory>
#include <mutex>

#include "event.hpp"

namespace dmxfish::events {

    class event_source;

    class event_storage {
    private:
        std::vector<event> storage_a, storage_b;
        std::vector<std::shared_ptr<event_source>> senders;
        bool current_read_storage_is_a = false;
        std::mutex storage_swap_mutex;
        std::map<event_sender_t, event> ongoing_events;
    public:
        friend class event_source;

        event_storage();
        ~event_storage();

        /**
         * This method tries to stores an event inside the queue to be processed on the next cycle. The return value
         * of this method needs to be honored in order to check if the operation was successful. THis method will not
         * block.
         *
         * @param e The event to store
         * @return True if the insert was successful at this time or false if this action needs to be repeated at a
         * later time.
         */
        [[nodiscard]] bool insert_event(const event& e);

        /**
         * This method should be called prior to executing the next cycle as it will swap the event queues. This method
         * will block if required.
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