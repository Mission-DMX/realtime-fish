#pragma once

#include "dmx/ftdi_universe.hpp"

#include <unistd.h>
#include <sstream>

#define START_MSG 0x7E
#define MSG_TYPE_SEND_DMX 0x06
#define END_MSG 0xE7

namespace dmxfish::dmx {

    // TODO implement function to query avaiable devices using libusb_get_device_list(ftdi.usb_ctx, ...)

    ftdi_universe::ftdi_universe(const int _id, const int vendor_id, const int product_id, const std::string& name, const std::string& serial) : universe(_id, universe_type::FTDI), data{}, device_handle{} {
        data[0] = START_MSG;
        data[1] = MSG_TYPE_SEND_DMX;
        data[2] = 0x04; // LSB of 16bit 512
        data[3] = 0x00; // MSB of 16bit 512
        data[4] = 0x00; // Start of DMX payload
        data[512+1] = END_MSG;

	this->device_handle = std::make_unique<ftdi_context, ftdi_deleter>(ftdi_new());
	if(this->device_handle == nullptr) {
		throw std::exception("Memory allocation error while using ftdi_new.");
	}

        // connect
        if (ftdi_usb_open_desc(device_handle.get(), vendor_id, product_id, name.c_str(), serial.length() > 0 ? serial.c_str() : nullptr) < 0) {
            throw ftdi_exception("Failed to open USB device.", std::move(device_handle));
        }
        // TODO do we need to query the latency timer?

        // reset device
        if (ftdi_usb_reset(device_handle.get()) < 0) {
            throw ftdi_exception("Failed to reset USB device.", std::move(device_handle));
            close_device();
        }

        // set line properties
        if (ftdi_set_line_property(device_handle.get(), BITS_8, STOP_BIT_2, NONE) < 0) {
            throw ftdi_exception("Failed to set FTDI device line mode.", std::move(device_handle));
            close_device();
        }

        // set flow control
        if (ftdi_setflowctrl(device_handle.get(), SIO_DISABLE_FLOW_CTRL) < 0) {
            throw ftdi_exception("Failed to set FTDI device flow control.", std::move(device_handle));
            close_device();
        }

        if (ftdi_set_baudrate(device_handle.get(), 250000) < 0) {
            throw ftdi_exception("Failed to set FTDI device baud rate.", std::move(device_handle));
            close_device();
        }

        // clear RTS
        if(ftdi_setrts(device_handle.get(), 0) < 0) {
            throw ftdi_exception("Failed to clear FTDI device RTS flag.", std::move(device_handle));
            close_device();
        }

        // purge buffers
#if defined(LIBFTDI1_5)
        if (ftdi_tcioflush(device_handle.get()) < 0) {
            throw ftdi_exception("Failed to perform an TCIO flush on FTDI device.", std::move(device_handle));
            close_device();
        }
#else
        if (ftdi_usb_purge_buffers(device_handle.get()) < 0) {
            throw ftdi_exception("Failed to reset FTDI device buffers.", std::move(device_handle));
            close_device();
        }
#endif


        // Enable device
        std::array<uint8_t, 9> enable_device_message = {
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
        if(ftdi_write_data(device_handle.get(), enable_device_message.data(), enable_device_message.size()) < 0) {
            throw ftdi_exception("Failed to enable write mode on Enttec USB DMX Pro.", std::move(device_handle));
        }

        // Configure DMX mode. (chip also supports MIDI but it is disabled in enttec custom firmware in order to sell a separate device)
        std::array<uint8_t, 7> set_dmx_message = {
            START_MSG,
            0xCB, // Port configuration message
            0x02, // length of message LSB
            0x00, // length of message MSB
            0x01, // configure mode of port 1 to DMX
            0x01, // configure mode of port 2 to DMX
            END_MSG
        };
        if(ftdi_write_data(device_handle.get(), set_dmx_message.data(), set_dmx_message.size()) < 0) {
            throw ftdi_exception("Failed to configure port mode on Enttec USB DMX Pro.");
        }

        device_successfully_opened = true;
    }

    ftdi_universe::~ftdi_universe() {}

    bool ftdi_universe::send_data() {
        return !(ftdi_write_data(device_handle.get(), this->data.data(), this->data.size()) < 0);
    }
}