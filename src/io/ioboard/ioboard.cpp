//
// Created by leondietrich on 1/21/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "ioboard.hpp"

namespace dmxfish::io {
    ioboard::ioboard(const int usb_device_id, const int usb_vendor_id, const int usb_product_id,
                     const std::string &usb_name, const std::string &usb_serial_channel) {
        // TODO open usb device
        // TODO register it with libev
        //  this might be helping:
        // TODO construct libftdi context
        // TODO setup device
        // TODO issue hello message, thus resetting the io board
    }
}