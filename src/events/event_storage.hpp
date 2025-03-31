//
// Created by doralitze on 11/15/23.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <cstddef>
#include <vector>
#include <string>
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
        std::map<std::string, std::weak_ptr<event_source>> index_source_by_name;
    public:
        friend class event_source;

        event_storage();
        ~event_storage();

        /**
         * This method tries to stores an event inside the queue to be processed on the next cycle. The return value
         * of this method needs to be honored in order to check if the operation was successful. This method will not
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

        inline std::vector<std::shared_ptr<event_source>> get_registered_senders() {
            return this->senders;
        }

        /**
         * This method searches for the event sender specified by its name.
         * @param name The name to look for
         * @return A shared pointer to the event sender or nullptr if it could not be found.
         */
        std::shared_ptr<event_source> find_source_by_name(const std::string& name);
    private:
        event_sender_t  register_event_source(const std::shared_ptr<event_source>& self);
    };


}