#include "executioners/project_configuration.hpp"

#include "executioners/scene_factory.hpp"
#include "io/universe_sender.hpp"

namespace dmxfish::execution {

[[nodiscard]] inline bool is_universe_setup_correct() {
	return (uptr->type == universe_type::PHYSICAL && universe.physical_location().present()) // TODO implement complete comparison once physical universes are implemented
		|| (uptr->type == universe_type::ARTNET && universe.artnet_location.present()); // TODO check for more than type data
}

project_configuration::project_configuration(const MissionDMX::ShowFile::BordConfiguration& show_file_dom) : scenes{}, universes{}, name{} {
	if(const auto optional_name = show_file_dom.show_name(); optional_name.present()) {
		this->name = optional_name.get();
	} else {
		this->name = "No Name";
	}

	for(const auto& universe : show_file_dom.universe()) {
		using namespace dmxfish::io;
		using namespace dmxfish::dmx;
		auto uptr = get_universe(universe.id());
		if(uptr == nullptr) {
			uptr = register_universe_from_xml(universe);
		} else if (!is_universe_setup_correct(uptr)) {
			unregister_universe(universe.id());
			// FIXME If the loading fails later on there is no roll back possible. We should add a recovery list for this purpose.
			uptr = register_universe_from_xml(universe);
		}
		this->universes.push_back(uptr);
	}

	auto scene_loading_result = populate_scene_vector(this->scenes, show_file_dom.scene());
	// FIXME input and output structures are not linked in scenes yet.

	if(const auto as = show_file_dom.default_active_scene(); as.present()) {
		this->default_active_scene = as.get();
	}
	this->current_active_scene = this->default_active_scene;
	this->scenes[this->get_active_scene()].on_start();
}

void project_configuration::set_active_scene(unsigned int new_scene) {
	this->scenes[this->get_active_scene()].on_stop();
	this->current_active_scene = new_scene;
	this->scenes[this->get_active_scene()].on_start();
}

void project_configuration::run_cycle_update() {
	this->scenes[this->get_active_scene()].invoke_filters();
}

}
