#include "filters/project_configuration.hpp"

#include "filters/scene_factory.hpp"

namespace dmxfish::filters {

project_configuration::project_configuration(const MissionDMX::ShowFile::BordConfiguration& show_file_dom) : scenes{}, universes{}, name{} {
	if(const auto optional_name = show_file_dom.show_name(); optional_name.present()) {
		this->name = optional_name.get();
	} else {
		this->name = "No Name";
	}
	populate_scene_vector(this->scenes, show_file_dom.scene());

	// TODO populate universes by calling get_or_create from universe_sender
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
