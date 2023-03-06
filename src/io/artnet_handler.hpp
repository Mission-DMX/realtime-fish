#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "dmx/artnet_universe.hpp"
#include "dmx/universe.hpp"
#include "lib/logging.hpp"
#include "net/socketaddress.hpp"
#include "net/udp_client.hpp"

namespace dmxfish::io {

	class artnet_handler {
		private:
			struct node_registry{
				int internal_universe_id;
				uint16_t device_universe_id;
				uint8_t sequence_number;
				rmrf::net::socketaddr node_address;
				std::shared_ptr<::dmxfish::dmx::artnet_universe> ptr;

				node_registry(int _id, uint16_t _device_universe_id, rmrf::net::socketaddr _node_address, std::shared_ptr<::dmxfish::dmx::artnet_universe> _ptr)
					: internal_universe_id(_id), device_universe_id(_device_universe_id),
					  sequence_number(1), node_address(_node_address), ptr(_ptr) { }
			};

			std::map<int, node_registry> nodes;
			rmrf::net::udp_client<dmxfish::dmx::artnet_pkg_size> client;

			void udp_incomming_cb(const rmrf::net::udp_packet<dmxfish::dmx::artnet_pkg_size>& data, rmrf::net::socketaddr& source) {
				// Don't do anything with data and source yet, Later implement RDM, POLL_REPLY and SYNC
				::spdlog::warn("Got incomming Art-Net packet of size {0} from {1}.", data.length(), source.str());
			}
		public:
			artnet_handler(const std::string& binding_interface_description)
				: nodes{}, client{binding_interface_description, 6454, std::bind(&artnet_handler::udp_incomming_cb, this, std::placeholders::_1, std::placeholders::_2)} {
			}

			void push_universe(::dmxfish::dmx::artnet_universe& u) {
				auto record = this->nodes.find(u.getID()); // this is slow, perhaps find a better way
				if(record == this->nodes.end()) {
					throw std::invalid_argument("This universe was not registered with this artnet_handler.");
				}
				u.update_sequence_number(++record->second.sequence_number == 0 ? ++record->second.sequence_number : record->second.sequence_number);
				this->client.send_packet(record->second.node_address, u.prep_and_get_packet());
			}

			std::shared_ptr<::dmxfish::dmx::artnet_universe> get_or_create_universe(const int id, const rmrf::net::socketaddr& addr, const uint16_t device_universe_id) {
				auto record = this->nodes.find(id);
				if (record == this->nodes.end()) {
					auto u = std::make_shared<::dmxfish::dmx::artnet_universe>(id, device_universe_id);
					node_registry r{id, device_universe_id, addr, u};
					this->nodes.insert({id, r});
					return u;
				}
				if (record != this->nodes.end() && (record->second.node_address != addr || record->second.device_universe_id != device_universe_id)) {
					const auto seq = record->second.sequence_number;
					record->second = node_registry(id, device_universe_id, addr, record->second.ptr);
					record->second.sequence_number = seq;
				}
				return record->second.ptr;
			}
	};

}

// TODO add support for ArtRdm, ArtSync and ArtPoll
