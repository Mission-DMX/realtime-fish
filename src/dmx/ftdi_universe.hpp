#pragma once

#include <array>
#include <exception>
#include <ftdi.h>
#include <string>
#include <sstream>

#include "dmx/universe.hpp"

namespace dmxfish::dmx {

    class ftdi_exception : public std::exception {
    private:
        std::string cause;
    public:
        ftdi_exception(const std::string& failed_operation, ftdi_context& c) {
            std::stringstream ss;
            ss << failed_operation << " Cause: " << ftdi_get_error_string(&c);
            cause = ss.str();
        }
        virtual const char* what() const throw () {return cause.c_str();}
    };

    class ftdi_universe : public universe {
    private:
        ftdi_context device_handle;
        std::array<uint8_t, 512 + 6> data;
        bool device_successfully_opened = false;
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
    private:
        bool close_device();
    };
}
