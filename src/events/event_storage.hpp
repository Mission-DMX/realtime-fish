//
// Created by doralitze on 11/15/23.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <mutex>

#include "event.hpp"

namespace dmxfish::events {

    class event_source;
    class event_storage;

    /**
     * Cycle-scoped, non-owning reference to an event inside the current read
     * buffer. The referenced event becomes invalid at the next call to
     * event_storage::swap_buffers(); callers MUST NOT retain an event_ref past
     * the end of their update cycle.
     *
     * The type is intentionally not assignable and cannot be heap-allocated,
     * so typical misuse (caching it in a member field, "new event_ref(...)")
     * is rejected at compile time. Copy construction stays enabled so the
     * reference can be passed into helper functions by value.
     *
     * Zero runtime overhead: sizeof(event_ref) == sizeof(const event*).
     */
    class [[nodiscard]] event_ref {
        const event* ptr;
        friend class event_storage;
        explicit event_ref(const event* p) noexcept : ptr(p) {}
    public:
        event_ref() = delete;
        event_ref(const event_ref&) = default;
        event_ref(event_ref&&) = default;
        event_ref& operator=(const event_ref&) = delete;
        event_ref& operator=(event_ref&&) = delete;
        void* operator new(std::size_t) = delete;
        void* operator new[](std::size_t) = delete;

        [[nodiscard]] explicit operator bool() const noexcept { return ptr != nullptr; }
        [[nodiscard]] const event& operator*()  const noexcept { return *ptr; }
        [[nodiscard]] const event* operator->() const noexcept { return ptr; }
    };

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

        /**
         * Resolve a registered event source's name to its numeric sender id.
         * This is a convenience wrapper around find_source_by_name used by the
         * sender-parsing code path.
         * @param name The name the event source was created with.
         * @return The sender id if a source with that name is registered and
         *         still alive, std::nullopt otherwise.
         */
        [[nodiscard]] std::optional<uint32_t> find_sender_id_by_name(const std::string& name);

        /**
         * Find all events whose sender id matches full exactly (sender AND
         * sender_function). Uses the sorted order established by swap_buffers()
         * to answer in O(log n).
         *
         * The returned span and any references obtained from it are
         * invalidated at the next swap_buffers() call. Do not retain them.
         *
         * @param full The full sender id (sender + sender_function) to look up.
         * @return A span of matching events inside the current read buffer.
         *         The span is empty if there is no match.
         */
        [[nodiscard]] std::span<const event> find(event_sender_t full) const noexcept;

        /**
         * Find all events whose sender matches sender, regardless of
         * sender_function. Uses the sorted order established by swap_buffers()
         * to answer in O(log n).
         *
         * The returned span and any references obtained from it are
         * invalidated at the next swap_buffers() call. Do not retain them.
         *
         * @param sender The sender id (ignoring the sender_function sub-field).
         * @return A contiguous span over the matching events in the current
         *         read buffer. Empty if there is no match.
         */
        [[nodiscard]] std::span<const event> find_by_sender(uint32_t sender) const noexcept;

        /**
         * Return a reference to the first event in the current read buffer
         * whose sender id matches full exactly. Convenience wrapper around
         * find() for callers that only need an existence check or the first
         * match.
         *
         * The returned event_ref is invalidated at the next swap_buffers()
         * call. Do not retain it.
         *
         * @param full The full sender id (sender + sender_function) to look up.
         * @return An event_ref wrapping the first match, or an event_ref that
         *         evaluates to false if there is no match.
         */
        [[nodiscard]] event_ref find_first(event_sender_t full) const noexcept;
    private:
        event_sender_t  register_event_source(const std::shared_ptr<event_source>& self);
        void notify_guis_about_event(const event& e);
    };


}