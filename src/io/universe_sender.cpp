#include "universe_sender.hpp"

#include <vector>

#include "io/artnet_handler.hpp"
#include "net/sock_address_factory.hpp"

namespace dmxfish::io {

static artnet_handler _artnet_handler{"0.0.0.0"}; // TODO get external interface from configuration
static std::vector<std::weak_ptr<dmxfish::dmx::universe>> active_universes;

bool publish_universe_update(std::shared_ptr<dmxfish::dmx::universe> universe) {
	if (universe->getUniverseType() == dmxfish::dmx::universe_type::ARTNET) {
		_artnet_handler.push_universe(*(static_cast<dmxfish::dmx::artnet_universe*>(universe.get())));
		return true;
	}
	return false;
}

bool push_all_registered_universes() {
	bool house_keeping_required = false;
	for(auto u_ptr : active_universes) {
		if(u_ptr.use_count() > 0) {
			publish_universe_update(u_ptr.lock());
		} else {
			house_keeping_required = true;
		}
	}
	// Now that we've scheduled the transmission, we've got plenty of time to do house keeping
	if(house_keeping_required) {
		std::erase_if(active_universes, [](std::weak_ptr<dmxfish::dmx::universe>& u_ptr) {
				return u_ptr.use_count() == 0;
		});
	}
	return !house_keeping_required;
}

std::shared_ptr<dmxfish::dmx::universe> get_temporary_universe(const std::string& output_description) {
	// TODO build parser that assignes a free universe
	return _artnet_handler.get_or_create_universe(1, rmrf::net::get_first_general_socketaddr(output_description, 6454), 1);
}

void unregister_universe(const int id) {
	_artnet_handler.unlink_universe(id);
	std::erase_if(active_universes, [id](std::weak_ptr<dmxfish::dmx::universe>& u_ptr) {
				return u_ptr.use_count() == 0 || u_ptr.lock()->getID() == id;
			});
}

}
