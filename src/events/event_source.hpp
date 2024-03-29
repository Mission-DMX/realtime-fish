//
// Created by doralitze on 11/15/23.
//

#pragma once

#include <memory>

#include "event_storage.hpp"

namespace dmxfish::events {

    /**
     * Event Source base class. Please implement a singleton for every system that should input events. Please register
     * with the primary event storage singleton unless there is a very good reason to use a separate one.
     */
    class event_source : public std::enable_shared_from_this<event_source> {
        friend class event_storage;

        event_sender_t sender_id;
    private:
        event_source();
    public:
        event_source(const event_source& other) = default;
        explicit event_source(event_source* other) : sender_id(other->sender_id) {}

        /**
         * Obtain the sender id.
         * @return The sender base ID.
         */
        [[nodiscard]] inline uint32_t get_sender_id() const {
            return this->sender_id.decoded_representation.sender;
        }

        /**
         * This method provides the factory for event senders. Please use it for all deriving classes.
         * @tparam T The class of the object that should be created. T needs to be a class that inherits event_source.
         * @param storage_to_register_with The event storage that should be used with this event class.
         * @throws std::numeric_limits If there are already more event sources created than can be registered.
         * @return A shared pointer to the created event source.
         */
        template<class T>
        static typename std::enable_if<std::is_base_of<event_source, T>::value, std::shared_ptr<T>>::type
        create(std::shared_ptr<event_storage> storage_to_register_with) {
            auto ptr = std::make_shared<T>(new T());
            ptr->sender_id = storage_to_register_with->register_event_source(ptr->shared_from_this());
            return ptr;
        }
    protected:

        /**
         * This method needs to be called by the event storage if it is being taken down.
         */
        virtual void deregister();
    };

} // dmxfish

