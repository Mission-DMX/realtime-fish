//
// Created by doralitze on 11/15/23.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <memory>
#include <stdexcept>
#include <string>

#include "event_storage.hpp"

#include "lib/macros.hpp"
COMPILER_SUPRESS("-Wuseless-cast")
#include "proto_src/Events.pb.h"
COMPILER_RESTORE("-Wuseless-cast")

namespace dmxfish::events {

    /**
     * Event Source base class. Please implement a singleton for every system that should input events. Please register
     * with the primary event storage singleton unless there is a very good reason to use a separate one.
     */
    class event_source : public std::enable_shared_from_this<event_source> {
        friend class event_storage;

        event_sender_t sender_id;
        std::string name;
        bool remote_debug_enabled = false;
    private:
        event_source();
    public:
        event_source(const event_source& other) = default;
        explicit event_source(event_source* other);
        virtual ~event_source();

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
        create(std::shared_ptr<event_storage> storage_to_register_with, const std::string& name = "") {
            if(storage_to_register_with == nullptr) {
                throw std::invalid_argument("The provided storage must not be null.");
            }
            if (!name.empty()) {
                if(const auto& m = storage_to_register_with->index_source_by_name; m.find(name) != m.end()) {
                    throw std::invalid_argument("Name of new event source must be unique.");
                }
            }
            auto ptr = std::shared_ptr<T>(new T());
            ptr->sender_id = storage_to_register_with->register_event_source(ptr->shared_from_this());
            if(!name.empty()) {
                storage_to_register_with->index_source_by_name[name] = ptr;
            }
            return ptr;
        }

        /**
         * This method gets called in order to get the protobuf representation of the sender.
         * Implementing classes need to override (and call the base of) this method in order to
         * implement special configuration handling and setting an appropriate type.
         * @return A protobuf message filled with the state of this sender.
         */
        virtual missiondmx::fish::ipcmessages::event_sender encode_proto_message() const;
    protected:

        /**
         * This method needs to be called by the event storage if it is being taken down.
         */
        virtual void deregister();
    };

} // dmxfish

