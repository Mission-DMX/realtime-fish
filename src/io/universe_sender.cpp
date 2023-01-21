#include "universe_sender.hpp"

#include "io/artnet_handler.hpp"

namespace dmxfish::io {

static artnet_handler _artnet_handler{"0.0.0.0"}; // TODO get external interface from configuration

bool publish_universe_update(std::shared_ptr<dmxfish::dmx::universe> universe) {
	if (universe->getUniverseType() == dmxfish::dmx::universe_type::ARTNET) {
		_artnet_handler.push_universe(*(static_cast<dmxfish::dmx::artnet_universe*>(universe.get())));
		return true;
	}
	return false;
}

}
