//
// Created by doralitze on 11/15/23.
//

#pragma once

#include <memory>

#include "event_storage.hpp"

namespace dmxfish::events {

    class event_source {
        friend class event_storage;
    public:
        explicit event_source(std::shared_ptr<event_storage> storage_to_register_with);

    private:

        /**
         * This method needs to be called by the event storage if it is being taken down.
         */
        void deregister();
    };

} // dmxfish

