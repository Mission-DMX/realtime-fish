#pragma once

#include "dmx/ftdi_universe.hpp"

#include <chrono>
#include <thread>
#include <unistd.h>
#include <sstream>

#include "lib/logging.hpp"

#define START_MSG 0x7E
#define MSG_TYPE_SEND_DMX 0x06
#define MSG_TYPE_RECV_IN_DMX 0x05
#define MSG_TYPE_SEND_DMX_PORT_2 0xA9
#define END_MSG 0xE7

namespace dmxfish::dmx {

unsigned int bcd_lulz(unsigned char const* nybbles, size_t length)
{
    unsigned int result(0);
    while (length--) {
        result = result * 100 + (*nybbles >> 4) * 10 + (*nybbles & 15);
        ++nybbles;
    }
    return result;
}

    // TODO implement function to query avaiable devices using libusb_get_device_list(ftdi.usb_ctx, ...)

    ftdi_universe::ftdi_universe(const int _id, const int vendor_id, const int product_id, const std::string& name, const std::string& serial) : universe(_id, universe_type::FTDI), data{}, device_handle{} {
	for(auto& x : data) {
		x = 0;
	}
        data[0] = START_MSG;
        data[1] = MSG_TYPE_SEND_DMX;
        data[2] = 1; //(uint8_t) ((512) & 0xff); // LSB of 16bit 512
        data[3] = 2; // (uint8_t) (((512) >> 8) & 0xff); // MSB of 16bit 512
        data[4] = 0x00; // Start of DMX payload
        data[512+1+4] = END_MSG;

        this->device_handle = device_ptr_t(ftdi_new());
        if(this->device_handle == nullptr) {
            throw std::invalid_argument("Memory allocation error while using ftdi_new.");
        }

        // connect
        if (ftdi_usb_open_desc(device_handle.get(), vendor_id, product_id, name.length() > 0 ? name.c_str() : nullptr, serial.length() > 0 ? serial.c_str() : nullptr) < 0) {
            throw ftdi_exception("Failed to open USB device.", std::move(device_handle));
        }

        // reset device
        if (ftdi_usb_reset(device_handle.get()) < 0) {
            throw ftdi_exception("Failed to reset USB device.", std::move(device_handle));
        }

        // set line properties
        if (ftdi_set_line_property(device_handle.get(), BITS_8, STOP_BIT_2, NONE) < 0) {
            throw ftdi_exception("Failed to set FTDI device line mode.", std::move(device_handle));
        }

        // set flow control
        if (ftdi_setflowctrl(device_handle.get(), SIO_DISABLE_FLOW_CTRL) < 0) {
            throw ftdi_exception("Failed to set FTDI device flow control.", std::move(device_handle));
        }

        if (ftdi_set_baudrate(device_handle.get(), 250000) < 0) {
            throw ftdi_exception("Failed to set FTDI device baud rate.", std::move(device_handle));
        }

        // clear RTS
        if(ftdi_setrts(device_handle.get(), 0) < 0) {
            throw ftdi_exception("Failed to clear FTDI device RTS flag.", std::move(device_handle));
        }

        // purge buffers
#if defined(LIBFTDI1_5)
        if (ftdi_tcioflush(device_handle.get()) < 0) {
            throw ftdi_exception("Failed to perform an TCIO flush on FTDI device.", std::move(device_handle));
        }
#else
        if (ftdi_usb_purge_buffers(device_handle.get()) < 0) {
            throw ftdi_exception("Failed to reset FTDI device buffers.", std::move(device_handle));
        }
#endif


        // Enable device
        std::array<unsigned char, 9> enable_device_message = {
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
        std::array<unsigned char, 7> set_dmx_message = {
            START_MSG,
            0xCB, // Port configuration message
            0x02, // length of message LSB
            0x00, // length of message MSB
            0x01, // configure mode of port 1 to DMX
            0x01, // configure mode of port 2 to DMX
            END_MSG
        };
        if(ftdi_write_data(device_handle.get(), set_dmx_message.data(), set_dmx_message.size()) < 0) {
            throw ftdi_exception("Failed to configure port mode on Enttec USB DMX Pro.", std::move(device_handle));
        }

        if(unsigned char latency = 0; ftdi_get_latency_timer(device_handle.get(), &latency) >= 0) {
            ::spdlog::info("FTDI Device latency: {}", latency);
        }

        std::array<uint8_t, 5> serial_read_msg = {START_MSG, 0x0A, 0x00, 0x00, END_MSG};
        if(ftdi_write_data(device_handle.get(), serial_read_msg.data(), serial_read_msg.size()) < 0) {
            throw ftdi_exception("Failed to send serial number read request to Enttec USB DMX Pro.", std::move(device_handle));
        }

        // TODO read back and decode serial number
	
        // query device configuration
	set_dmx_message[1] = 0x03;
	set_dmx_message[5] = 0x00;
	if(ftdi_write_data(device_handle.get(), set_dmx_message.data(), set_dmx_message.size() < 0)) {
		throw ftdi_exception("Failed to transmit configuration request", std::move(device_handle));
	}
        device_successfully_opened = true;
    }

    ftdi_universe::~ftdi_universe() {}

    int ftdi_universe::decode_reply(const std::array<unsigned char, 80>& buffer, int start) {
	    auto eof_expected = 0;
	    switch(buffer[start + 1]) {
		    case 10: // Serial number reply
			    serial_number = bcd_lulz(buffer.data() + start + 4, 4);
			    eof_expected = start + 8;
			    ::spdlog::debug("FTDI dongle Reported serial number: {}.", this->serial_number);
			    break;
		default:
			    ::spdlog::error("Unknown message label on FTDI dongle reply: {}", buffer[start + 1]);
	    }
	    if(buffer[eof_expected] != END_MSG) {
		    ::spdlog::error("FTDI dongle replied with invalid length");
	    }
	    return eof_expected + 1;
    }

    bool ftdi_universe::send_data() {
        std::array<unsigned char, 80> read_buf;
        if(auto read = ftdi_read_data(device_handle.get(), read_buf.data(), read_buf.size()); read > 0) {
	    auto start = 0;
            while (read_buf[start] == START_MSG) {
                start = decode_reply(read_buf, start);
            }
            std::stringstream ss;
            for(auto i = start; i < read; i++){
                ss << (int) read_buf[i] << " ";
            }
	    if(auto s = ss.str(); s.length() > 0)
                ::spdlog::debug("FTDI device reported: {}", s);
        }
        return !(ftdi_write_data(device_handle.get(), this->data.data(), (int) this->data.size()) < 0);
    }
}
