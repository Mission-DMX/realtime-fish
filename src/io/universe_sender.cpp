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
	// TODO build parser that always assignes a free universe
	return _artnet_handler.get_or_create_universe(-1, rmrf::net::get_first_general_socketaddr(output_description, 6454), 1);
}

std::shared_ptr<dmxfish::dmx::universe> register_universe_from_message(const missiondmx::fish::ipcmessages::Universe& u_dev) {
	std::shared_ptr<dmxfish::dmx::universe> u_ptr_candidate = nullptr;
	if(u_dev.has_remote_location()) {
		// ArtNet
		const auto& artnet_definition = u_dev.remote_location();
		const auto address = rmrf::net::get_first_general_socketaddr(artnet_definition.ip_address(), artnet_definition.port());
		u_ptr_candidate = _artnet_handler.get_or_create_universe(u_dev.id(), address, artnet_definition.universe_on_device());
	} else {
		// local universes are not yet implemented
		return nullptr;
	}
	bool insert_required = true;
	for (auto& u_ptr : active_universes) {
		if(u_ptr.use_count() > 0) {
			auto u_ptr_l = u_ptr.lock();
			if (u_ptr_l->getID() == u_ptr_candidate->getID()) {
				insert_required = false;
				u_ptr = u_ptr_candidate;
				break;
			}
		}
	}
	if(insert_required) {
		active_universes.push_back(u_ptr_candidate);
	}
	return u_ptr_candidate;
}

void unregister_universe(const int id) {
	_artnet_handler.unlink_universe(id);
	std::erase_if(active_universes, [id](std::weak_ptr<dmxfish::dmx::universe>& u_ptr) {
				return u_ptr.use_count() == 0 || u_ptr.lock()->getID() == id;
			});
}

std::forward_list<std::weak_ptr<dmxfish::dmx::universe>> get_universe_list() {
	std::forward_list<std::weak_ptr<dmxfish::dmx::universe>> l;
	for(auto& u_ptr : active_universes) {
		l.push_front(u_ptr);
	}
	return l;
}

std::shared_ptr<dmxfish::dmx::universe> get_universe(const int id) {
	return _artnet_handler.get_universe(id);
}

}
