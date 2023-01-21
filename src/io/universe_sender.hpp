#pragma once

#include <memory>

#include "dmx/universe.hpp"

namespace dmxfish::io {

	// TODO write method that creates/updates all universes from show file
	bool publish_universe_update(std::shared_ptr<dmxfish::dmx::universe> universe);
}
