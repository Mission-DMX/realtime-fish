#pragma once

#include <map>
#include <memory>
#include <string>

#include "dmx/artnet_universe.hpp"
#include "dmx/universe.hpp"
#include "net/socketaddress.hpp"
#include "net/udp_client.hpp"

namespace dmxfish::io {

	class artnet_handler {
		private:
			typedef struct node_registry{
				// TODO sequence number, address universe id, etc.
				const int internal_universe_id;
				const uint16_t device_universe_id;
				uint8_t sequence_number;
				const rmrf::net::socketaddr node_address;
				const std::shared_ptr<artnet_universe> ptr;
				
				node_registry(int id, uint16_t device_universe_id, rmrf::net::socketaddr node_address, std::shared_ptr<artnet_universe> ptr)
					: internal_universe_id(id), device_universe_id(device_universe_id),
					  sequence_number(1), node_address(node_address), ptr(ptr) { }
			};

			std::map<int, node_registry> nodes;
			rmrf::net::udp_client<dmxfish::dmx::artnet_pkg_size> client;

			void udp_incomming_cb(const udp_packet<dmxfish::dmx::artnet_pkg_size>& _, socketaddr& _) {
				// Don't do anything with data and source yet, Later implement RDM, POLL_REPLY and SYNC
			}
		public:
			artnet_handler(const std::string& binding_interface_description)
				: nodes{}, client{binding_interface_description, 6454, udp_incomming_cb} {
			}

			void push_universe(artnet_universe& u) {
				const auto& record = this->nodes[u.getID()]; // this is slow, perhaps find a better way
				u.update_sequence_number(++record.sequence_number == 0 ? ++record.sequence_number : record.sequence_number);
				this->client.send_packet(record.node_address; u.prep_and_get_packet());
			}

			std::shared_ptr<artnet_universe> get_or_create_universe(const int id, const rmrf::net::socketaddr& addr, const uint16_t device_universe_id) {
				auto record = this->nodes.find(id);
				if (record == this->nodes.end()) {
					auto u = std::make_shared<artnet_universe>(id, device_universe_id);
					node_registry r{id, device_universe_id, addr, u};
					this->nodes[id] = r;
					return u;
				}
				if (record != this->nodes.end() && (record->node_address != addr || record->device_universe_id != device_universe_id)) {
					const auto seq = record->sequence_number;
					*record = node_registry{id, device_universe_id, addr, record->ptr};
					record->sequence_number = seq;
				}
				return record->ptr;
			}
	};

}

// TODO add support for ArtRdm, ArtSync and ArtPoll
