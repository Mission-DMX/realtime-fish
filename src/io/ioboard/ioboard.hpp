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
#include "io/ioboard/types.h"

namespace dmxfish::io {

    class ioboard {
    private:
        ::ev::async async;
        ::ev::io io_sync;
        rmrf::net::auto_fd usb_device_fd;
        rmrf::net::ioqueue<rmrf::net::iorecord> priority_queue, background_queue;
        bool transmitting_background_msg;
        bool uses_file_driver;

        friend class dmxfish::dmx::ioboard_universe; // Required to notify this about insertion of new data
        std::vector<std::optional<std::shared_ptr<dmxfish::dmx::ioboard_universe>>> linked_universes;
    public:
        ioboard(const int usb_device_id, const int usb_vendor_id, const int usb_product_id, const std::string& usb_name, const std::string& usb_serial_channel);
        ioboard(const std::string driver_file_path);
        ioboard(const ioboard&) = delete;
        ioboard(ioboard&&) = delete;
        ~ioboard();

        /**
         * This method gets or creates the universe associated with that port.
         *
         * Note: The universe returned might have a different id than was specified. If the same id is a requirement,
         * this needs to be checked by the callee. In that case the callee should call unregister_universe and call
         * this method again.
         *
         * @param port The physical port the universe is located on.
         * @param id The ID a newly created universe would have.
         * @return The universe actually associated with that port.
         */
        [[nodiscard]] std::weak_ptr<dmxfish::dmx::ioboard_universe> get_or_create_universe(ioboard_port_id_t port, int id);

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

        /**
         * Use this method to potentially unregister a universe associated with this io board.
         * @param id The ID of the universe to look for
         * @return True if the id was indeed associated with this io board and the universe was unregistered
         */
        [[nodiscard]] bool unregister_universe_by_id(int id);

        /**
         * Use this method in order to unregister the universe associated with the given port
         * @param port The port to unregister the universe from
         */
        void unregister_universe(ioboard_port_id_t port);
    private:
        // io management
        void set_io_flags();
        void cb_async_schedule(::ev::async& w, int events);
        void cb_io_handler(::ev::io& w, int events);
    };

}