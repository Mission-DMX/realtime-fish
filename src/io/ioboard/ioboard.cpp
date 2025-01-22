//
// Created by leondietrich on 1/21/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "ioboard.hpp"

#include <stdexcept>

namespace dmxfish::io {
    ioboard::ioboard(const int usb_device_id, const int usb_vendor_id, const int usb_product_id,
                     const std::string &usb_name, const std::string &usb_serial_channel) :
            async(), io_sync(), priority_queue(), background_queue(), transmitting_background_msg(false),
            linked_universes() {
        // TODO open usb device
        // TODO register it with libev
        //  this might be helping:
        // TODO construct libftdi context
        // TODO setup device
        // TODO issue hello message, thus resetting the io board
        // TODO initialize linked_universes vector based on received port count
    }

    ioboard::~ioboard() {
        // TODO clean up IO bindings
    }

    [[nodiscard]] std::weak_ptr<dmxfish::dmx::ioboard_universe> ioboard::get_universe(ioboard_port_id_t port) {
        if (const auto universe_vector_size = this->linked_universes.size(); port >= universe_vector_size) [[unlikely]] {
            throw std::out_of_range("Requested ioboard DMX port #" + std::to_string(port) +
                " (0-indexed) but there are only " + std::to_string(universe_vector_size) +
                " ports on the board.");
        }

        if (auto candidate = this->linked_universes[port]; candidate.has_value()) {
            return candidate.value();
        } else {
            auto univ = std::make_shared<dmxfish::dmx::ioboard_universe>(port);
            this->linked_universes[port] = univ;
            return univ;
        }
    }
}