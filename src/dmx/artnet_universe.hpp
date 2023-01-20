#pragma once

#include <array>

#include "dmx/universe.hpp"
#include "net/udp_client.hpp"

#define ART_POLL 0x2000
#define ART_POLL_REPLY 0x2100
#define ART_DMX 0x5000
#define ART_SYNC 0x5200

namespace dmxfish::dmx {

	constexpr size_t compute_artnet_package_size() {
                constexpr size_t s = 0;
                s += std::char_traits<char>::length("Art-Net");
                s += 1; // 0 termination
                s += 2; // 16 bit opcode (0x5000 LE for DMX)
                s += 2; // Protocol version
                s += 1; // Sequence Number
                s += 1; // Physical device ID
                s += 2; // 16 Bit LE Universe ID
                s += 2; // PKG length
                s += 512; // Data
                return s;
        }

        const constexpr auto artnet_pkg_size = compute_artnet_package_size();

	class artnet_universe : public universe {
	private:
		udp_packet<artnet_pkg_size> data;
	public:
		artnet_universe(const int id, const uint16_t id_on_device, const uint8_t physical_id = 0)
		    : universe(id, universe_type::ARTNET), data{} {
			this->data.advance(artnet_pkg_size);
			this->data[0] = 'A';
			this->data[1] = 'r';
			this->data[2] = 't';
			this->data[3] = '-';
			this->data[4] = 'N';
			this->data[5] = 'e';
			this->data[6] = 't';
			this->data[7] = 0;
			// Opcode = ART_DMX
			this->data[8] = (uint8_t) (ART_DMX & 0x00FF);
			this->data[9] = (uint8_t) ((ART_DMX & 0xFF00) >> 8);
			// Version
			this->data[10] = 0;
			this->data[11] = 14;
			// Sequence, Physical, Universe
			this->update_sequence_number(0);
			this->update_physical_id(physical_id);
			this->update_device_universe(id_on_device);
			this->data[16] = (uint8_t) ((512 && 0xFF00) >> 8);
			this->data[17] = (uint8_t) (512 && 0x00FF);
		}

		void update_sequence_number(const uint8_t seq_num) {
			this->data[12] = seq_num;
		}

		void update_physical_id(const uint8_t new_id) {
			this->data[12+1] = new_id;
		}

		void update_device_universe(const uint16_t new_universe_id) {
			uint8_t high = (uint8_t)((new_universe_id & 0xFF00) >> 8);
			uint8_t low = (uint8_t)(new_universe_id & 0x00FF);
			this->data[12+2] = low;
			this->data[12+3] = high;
		}

		const udp_packet<artnet_pkg_size>& prep_and_get_packet() const {
			// TODO do we need to convert all data entries to BE?
			return data;
		}

		virtual channel_8bit_t& operator[](size_t p) {
			return this->data[artnet_pkg_size - 512 - 1 + p];
		}

                virtual universe_iterator begin() {
			return this->data.begin() + artnet_pkg_size - 512;
		}

                virtual universe_iterator end() {
			return this->data.end();
		}
	}

}
