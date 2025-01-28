//
// Created by leondietrich on 1/22/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "ioboard.hpp"

#include <array>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "lib/logging.hpp"
#include "io/ioboard/ioboard_midi_event_source.hpp"
#include "main.hpp"

namespace dmxfish::io {
    ioboard::ioboard(const std::string& driver_file_path) :
            async(), io_sync(), usb_device_fd(rmrf::net::nullfd), priority_queue(), background_queue(), transmitting_background_msg(false),
            uses_file_driver(true), linked_universes(), in_message_construction(), evs(nullptr) {
        this->in_message_construction.reserve(1024);

        // TODO ensure that ft60x.ko (from https://github.com/lambdaconcept/ft60x_driver/tree/master) is loaded

        // open usb device
        this->usb_device_fd = rmrf::net::auto_fd{open(driver_file_path.c_str(), O_RDWR)};
        if (!this->usb_device_fd) {
            std::array<char, 64> buffer;
#if defined(__linux__) && defined(_GNU_SOURCE)
            auto ret_str = strerror_r(errno, buffer.data(), buffer.size());
            throw rmrf::net::netio_exception(
                    "Failed to connect fo ft60x kernel driver. Errno: " + std::to_string(errno)
                    + " (" + std::string(ret_str) + ") Driver path: " + driver_file_path);
#else
            int ret = strerror_r(errno, buffer.data(), buffer.size());
            throw rmrf::net::netio_exception(
                    "Failed to connect fo ft60x kernel driver. Errno: " + std::to_string(errno)
                    + " (" + ((RET == 0) ? std::string(buffer.data()) : "UNKNOWN") + ") Driver path: "
                    + driver_file_path);
#endif
        }

        // issue hello message, thus resetting the io board
        size_t written = 0;
        std::array<uint8_t, 5> setup_buffer;
        setup_buffer[0] = (uint8_t) ioboard_message_type_t::PROTOCOL_HANDSHAKE;
        setup_buffer[1] = 0; // Target
        setup_buffer[2] = 0;
        setup_buffer[3] = 1; // 1 Byte size
        setup_buffer[4] = 0; // Protocol version 0

        while(written < 5) {
            written += write(usb_device_fd.get() + written, setup_buffer.data(), 5 - written);
        }
        ::spdlog::debug("Opened and greeted io board.");

        written = 0;
        while (true) {
            written = read(usb_device_fd.get(), setup_buffer.data(), 1);
            if (written == 1) {
                if (setup_buffer[0] == (uint8_t) ioboard_message_type_t::PROTOCOL_HANDSHAKE) {
                    break;
                }
            }
        }
        written = 0;
        while (written < 5) {
            written += read(usb_device_fd.get() + written, setup_buffer.data(), 5 - written);
        }
        if (setup_buffer[0] != 0 || setup_buffer[1] != 0 || setup_buffer[2] != 2) [[unlikely]] {
            ::spdlog::error("Expected io board handshake target to be zero and packet length to be 2.");
        }
        if (setup_buffer[3] != 0) [[unlikely]] {
            ::spdlog::warn("Expected Protocol version 0. Instead version {} was presented.", (unsigned int) setup_buffer[3]);
        }
        this->linked_universes.reserve(setup_buffer[4]);
        for(written = 0; written < setup_buffer[4]; written++) {
            this->linked_universes.push_back(std::nullopt);
        }
        ::spdlog::info("Found {} dmx ports on io board.", this->linked_universes.size());

        // make fd nonblocking, register it with libev
        written = fcntl(this->usb_device_fd.get(), F_GETFL, 0);
        if (fcntl(this->usb_device_fd.get(), F_SETFL, written | O_NONBLOCK) < 0) {
            throw rmrf::net::netio_exception("Failed to enable non blocking option with ft60x driver. errno: " + std::to_string(errno));
        }
        this->async.set<ioboard, &ioboard::cb_async_schedule>(this);
        this->async.start();
        this->io_sync.set<ioboard, &ioboard::cb_io_handler>(this);
        this->io_sync.start(this->usb_device_fd.get(), 0);

        this->evs = dmxfish::events::event_source::create<ioboard_midi_event_source>(get_event_storage_instance());
    }

    ioboard::~ioboard() {
        async.stop();
        io_sync.stop();
    }
}
