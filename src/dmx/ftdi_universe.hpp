#pragma once

#include <array>
#include <ftdi.h>
#include <string>

#include "dmx/universe.hpp"

namespace dmxfish::dmx {
    class ftdi_universe : public universe {
    private:
        ftdi_context device_handle;
        std::array<uint8_t, 512 + 6> data;
        bool device_successfully_opened = false;
    public:
        ftdi_universe(const int _id, int vendor_id, int product_id, const std::string& name, const std::string& serial);

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

		void send_data();
    private:
        bool close_device();
    }
}
