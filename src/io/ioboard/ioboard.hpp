//
// Created by leondietrich on 1/21/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <memory>
#include <optional>

#include <ev++.h>

#include "net/async_fd.hpp"
#include "net/ioqueue.hpp"

#include "dmx/ioboard_universe.hpp"

namespace dmxfish::io {

    using ioboard_port_id_t = size_t;

    class ioboard {
    private:
        ::ev::async async;
        ::ev::io io_sync;
        rmrf::net::auto_fd usb_device_fd;
        rmrf::net::ioqueue<rmrf::net::iorecord> priority_queue, background_queue;
        bool transmitting_background_msg;

        friend class dmxfish::dmx::ioboard_universe; // Required to notify this about insertion of new data
        std::vector<std::optional<std::shared_ptr<dmxfish::dmx::ioboard_universe>>> linked_universes;
    public:
        ioboard(const int usb_device_id, const int usb_vendor_id, const int usb_product_id, const std::string& usb_name, const std::string& usb_serial_channel);
        ioboard(const ioboard&) = delete;
        ioboard(ioboard&&) = delete;
        ~ioboard();

        [[nodiscard]] std::weak_ptr<dmxfish::dmx::ioboard_universe> get_universe(ioboard_port_id_t port);

        // TODO implement methods for sending midi data
        // TODO implement midi event output
        // TODO implement methods for sending RS232 data
        // TODO implement RS232 event output
        // TODO implement methods for configuration of displays
        // TODO implement keypad event output

        /**
         * This method is should be called by the universe_sender. It will push all registered universes content to the
         * io board.
         */
        void transmit_universe(ioboard_port_id_t port);
    private:
        void unregister_universe(ioboard_port_id_t port);

        // io management
        void set_io_flags();
        void cb_async_schedule(::ev::async& w, int events);
        void cb_io_handler(::ev::io& w, int events);
    };

}