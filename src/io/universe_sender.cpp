#include "universe_sender.hpp"

#include <map>
#include <optional>
#include <vector>

#include "dmx/ftdi_universe.hpp"
#include "dmx/ioboard_universe.hpp"
#include "io/artnet_handler.hpp"
#include "io/ioboard/ioboard.hpp"
#include "lib/logging.hpp"
#include "net/sock_address_factory.hpp"

namespace dmxfish::io {

static artnet_handler _artnet_handler{"0.0.0.0"}; // TODO get external interface from configuration
static std::optional<ioboard> ioboard_handler_opt;
static std::map<int, std::shared_ptr<dmxfish::dmx::ftdi_universe>> dongle_map{};
static std::vector<std::weak_ptr<dmxfish::dmx::universe>> active_universes;

bool publish_universe_update(std::shared_ptr<dmxfish::dmx::universe> universe) {
	switch (universe->getUniverseType()) {
		case dmxfish::dmx::universe_type::ARTNET:
			_artnet_handler.push_universe(*(static_cast<dmxfish::dmx::artnet_universe*>(universe.get())));
			return true;
		case dmxfish::dmx::universe_type::PHYSICAL:
            if (!ioboard_handler_opt.has_value()) {
                return false;
            }
            ioboard_handler_opt->transmit_universe(static_cast<dmxfish::dmx::ioboard_universe*>(universe.get())->get_port());
            return true;
		case dmxfish::dmx::universe_type::sACN:
			return false;
		case dmxfish::dmx::universe_type::FTDI:
			return static_cast<dmxfish::dmx::ftdi_universe*>(universe.get())->send_data();
		default:
			::spdlog::error("Unable to send unknown universe type: {}.", (int) universe->getUniverseType());
			return false;
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

void check_update_required_from_registration(std::shared_ptr<dmxfish::dmx::universe> u_ptr_candidate) {
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
}

std::shared_ptr<dmxfish::dmx::universe> register_universe_from_message(const missiondmx::fish::ipcmessages::Universe& u_dev) {
	std::shared_ptr<dmxfish::dmx::universe> u_ptr_candidate = nullptr;
	if(u_dev.has_remote_location()) {
		// ArtNet
		const auto& artnet_definition = u_dev.remote_location();
		const auto address = rmrf::net::get_first_general_socketaddr(artnet_definition.ip_address(), artnet_definition.port());
		u_ptr_candidate = _artnet_handler.get_or_create_universe(u_dev.id(), address, artnet_definition.universe_on_device());
		if(dongle_map.contains(u_dev.id())) {
			dongle_map.erase(u_dev.id());
		}
	} else if(u_dev.has_ftdi_dongle()) {
		const auto& usb_definition = u_dev.ftdi_dongle();
		if(dongle_map.contains(u_dev.id())) {
			return dongle_map.at(u_dev.id());
		}
		const auto u = std::make_shared<dmxfish::dmx::ftdi_universe>(u_dev.id(), usb_definition.vendor_id(), usb_definition.product_id(), usb_definition.device_name(), usb_definition.serial());
		dongle_map[u_dev.id()] = u;
		u_ptr_candidate = u;
		::spdlog::debug("Created FTDI universe from protobuf.");
		_artnet_handler.unlink_universe(u_dev.id());
	} else {
		// TODO local universes are not yet implemented
		return nullptr;
	}
	check_update_required_from_registration(u_ptr_candidate);
	return u_ptr_candidate;
}

std::shared_ptr<dmxfish::dmx::universe> register_universe_from_xml(const MissionDMX::ShowFile::Universe& universe) {
	std::shared_ptr<dmxfish::dmx::universe> u_ptr_candidate = nullptr;
	if(universe.artnet_location().present()) {
		const auto& artnet_definition = universe.artnet_location().get();
		const auto address = rmrf::net::get_first_general_socketaddr(artnet_definition.ip_address(), artnet_definition.udp_port());
		u_ptr_candidate = _artnet_handler.get_or_create_universe(universe.id(), address, artnet_definition.device_universe_id());
		if(dongle_map.contains(universe.id())) {
			dongle_map.erase(universe.id());
		}
	} else if(universe.ftdi_location().present()) {
		const auto& fdev = universe.ftdi_location().get();
		if(dongle_map.contains(universe.id())) {
			return dongle_map.at(universe.id());
		}
		const auto c = std::make_shared<dmxfish::dmx::ftdi_universe>(universe.id(), fdev.vendor_id(), fdev.product_id(), fdev.device_name(), fdev.serial_identifier().present() ? fdev.serial_identifier().get() : "");
		dongle_map[universe.id()] = c;
		u_ptr_candidate = c;
		::spdlog::debug("Created FTDI universe from xml.");
		_artnet_handler.unlink_universe(universe.id());
	} else {
		// TODO other universe types are not yet implemented
		return nullptr;
	}
	check_update_required_from_registration(u_ptr_candidate);
	return u_ptr_candidate;
}

void unregister_universe(const int id) {
	if(!_artnet_handler.unlink_universe(id)) {
		if(dongle_map.contains(id)) {
			dongle_map.erase(id);
		} else if(ioboard_handler_opt.has_value() && ioboard_handler_opt->unregister_universe_by_id(id)) {
            // we erased it and do not need to look any further
        }
	}
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
	if (dongle_map.contains(id)) {
		return dongle_map.at(id);
	}
	return _artnet_handler.get_universe(id);
}

}
