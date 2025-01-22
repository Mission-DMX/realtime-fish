//
// Created by leondietrich on 1/22/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "ioboard.hpp"

namespace dmxfish::io {
    ioboard::ioboard(const int usb_device_id, const int usb_vendor_id, const int usb_product_id,
                     const std::string &usb_name, const std::string &usb_serial_channel) :
            async(), io_sync(), usb_device_fd(rmrf::net::nullfd), priority_queue(), background_queue(), transmitting_background_msg(false),
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
}