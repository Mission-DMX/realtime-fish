#include "universe_sender.hpp"

#include "io/artnet_handler.hpp"
#include "net/sock_address_factory.hpp"

namespace dmxfish::io {

static artnet_handler _artnet_handler{"0.0.0.0"}; // TODO get external interface from configuration

bool publish_universe_update(std::shared_ptr<dmxfish::dmx::universe> universe) {
	if (universe->getUniverseType() == dmxfish::dmx::universe_type::ARTNET) {
		_artnet_handler.push_universe(*(static_cast<dmxfish::dmx::artnet_universe*>(universe.get())));
		return true;
	}
	return false;
}

std::shared_ptr<dmxfish::dmx::universe> get_temporary_universe(const std::string& output_description) {
	// TODO build parser that assignes a free universe
	return _artnet_handler.get_or_create_universe(1, rmrf::net::get_first_general_socketaddr(output_description, 6454), 1);
}

std::shared_ptr<dmxfish::dmx::universe> get_universe(const int id) {
	return _artnet_handler.get_universe(id);
}

}
