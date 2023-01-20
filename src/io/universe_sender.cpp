#include "universe_sender.hpp"

#include "io/artnet_handler.hpp"

namespace dmxfish::io {

static artnet_handler _artnet_handler{"0.0.0.0"}; // TODO get external interface from configuration

bool publish_universe_update(dmxfish::dmx::universe& universe) {
	if (universe.getUniverseType() == universe_type::ARTNET) {
		_artnet_handler.push_universe((dmxfish::dmx::artnet_universe) universe);
		return true;
	}
	return false;
}

}
