#pragma once

#include "dmx/ftdi_universe.hpp"

#include <unistd.h>
#include <libusb.h>

#define START_MSG 0x7E
#define MSG_TYPE_SEND_DMX 0x06
#define END_MSG 0xE7

namespace dmxfish::dmx {

    // TODO implement function to query avaiable devices using libusb_get_device_list(ftdi.usb_ctx, ...)

    ftdi_universe::ftdi_universe(const int _id, int vendor_id, int product_id, const std::string& name, const std::string& serial) : universe(_id, universe_type::FTDI), data{} {
        data[0] = START_MSG;
        data[1] = MSG_TYPE_SEND_DMX;
        data[2] = 0x04; // LSB of 16bit 512
        data[3] = 0x00; // MSB of 16bit 512
        data[4] = 0x00; // Start of DMX payload
        data[512+1] = END_MSG;

        // open device
        bzero(&device_handle, sizeof(ftdi_context));
        if(ftdi_init(&device_handle) < 0) {
            // TODO do something with error
        }

        // connect
        if (ftdi_usb_open_desc(&device_handle, vendor_id, product_id, name().c_str(), serial.length() > 0 ? serial.c_str() : nullptr) < 0) {
            // TODO
        }
        // TODO do we need to query the latency timer?

        // reset device
        if (ftdi_usb_reset(&device_handle) < 0) {
            // TODO
        }

        // set line properties
        if (ftdi_set_line_property(&device_handle, BITS_8, STOP_BIT_2, NONE) < 0) {
            // TODO
        }

        // set flow control
        if (ftdi_setflowctrl(&device_handle, SIO_DISABLE_FLOW_CTRL) < 0) {
            // TODO
        }

        if (ftdi_set_baudrate(&device_handle, 250000) < 0) {
            // TODO
        }

        // clear RTS
        if(ftdi_setrts(&device_handle, 0) < 0) {
            // TODO
        }

        // purge buffers
#if defined(LIBFTDI1_5)
        if (ftdi_tcioflush(&device_handle) < 0) {
            // TODO
        }
#else
        if (ftdi_usb_purge_buffers(&device_handle) < 0) {
            // TODO
        }
#endif


        // Enable device
        const std::array<uint8_t, 9> enable_device_message = {
            START_MSG,
            0x0D, // Enable Write Mode
            0x04, // max universe length LSB
            0x00, // max universe length MSB
            0xAD, // documented as "WTF ?!?"
            0x88, // documented as "WTF ?!?"
            0xD0, // documented as "WTF ?!?"
            0xC8, // documented as "WTF ?!?"
            END_MSG
        };
        if(ftdi_write_data(&device_handle, (uchar*) enable_device_message.data(), enable_device_message.size()) < 0) {
            // TODO
        }

        // Configure DMX mode. (chip also supports MIDI but it is disabled in enttec custom firmware in order to sell a separate device)
        const std::array<uint8_t, 7> set_dmx_message = {
            START_MSG,
            0xCB, // Port configuration message
            0x02, // length of message LSB
            0x00, // length of message MSB
            0x01, // configure mode of port 1 to DMX
            0x01, // configure mode of port 2 to DMX
            END_MSG
        };
        if(ftdi_write_data(&device_handle, (uchar*) set_dmx_message.data(), set_dmx_message.size()) < 0) {
            // TODO
        }

        device_successfully_opened = true;
    }

    ftdi_universe::~ftdi_universe() {
        if(device_successfully_opened) {
            close_device();
        }
        ftdi_deinit(&device_handle);
    }

    void ftdi_universe::send_data() {
        if(ftdi_write_data(&device_handle, (uchar*) this->data.data(), this->data.size()) < 0) {
            // TODO
        }
    }

    bool ftdi_universe::close_device() {
        if(ftdi_usb_close(&device_handle) < 0) {
            // TODO log error using ftdi_get_error_string(&device_handle)
            return false;
        }
    }
}
