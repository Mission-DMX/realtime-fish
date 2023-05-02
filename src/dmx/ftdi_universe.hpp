#pragma once

#include <array>
#include <stdexcept>
#include <ftdi.h>
#include <memory>
#include <string>
#include <sstream>

#include "dmx/universe.hpp"

namespace dmxfish::dmx {

    struct ftdi_deleter {
	    void operator()(ftdi_context* p) {
		    if (p->usb_dev != nullptr) {
		    	ftdi_usb_close(p);
		    }
		    ftdi_free(p);
	    }
    };

    using device_ptr_t = std::unique_ptr<ftdi_context, ftdi_deleter>; 

    class ftdi_exception : public std::exception {
    private:
        std::string cause;
    public:
        ftdi_exception(const std::string& failed_operation, device_ptr_t c) : cause{} {
            std::stringstream ss;
            ss << failed_operation << " Cause: " << ftdi_get_error_string(c.get());
            cause = ss.str();
        }
        virtual const char* what() const throw () {return cause.c_str();}
    };

    class ftdi_universe : public universe {
    private:
	device_ptr_t device_handle;
        std::array<uint8_t, 512 + 6> data;
        bool device_successfully_opened = false;
	int serial_number = 0;
	int product_id;
	int vendor_id;
    public:
        ftdi_universe(const int _id, const int vendor_id, const int product_id, const std::string& name, const std::string& serial);

        ~ftdi_universe();

        virtual channel_8bit_t& operator[](size_t p) override {
			return this->data[p + 5];
		}

		virtual universe_iterator begin() override {
			return this->data.begin() + 5;
		}

		virtual universe_iterator end() override {
			return this->data.end() - 1;
		}

		bool send_data();
		unsigned int get_chip_id();
		int get_vendor_id();
		int get_product_id();
    private:
		int decode_reply(const std::array<unsigned char, 80>& buffer, int start);
    };
}
