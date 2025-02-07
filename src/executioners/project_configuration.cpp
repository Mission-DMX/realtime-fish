#include "executioners/project_configuration.hpp"

#include "executioners/scene_factory.hpp"
#include "io/universe_sender.hpp"

namespace dmxfish::execution {

project_configuration::project_configuration(std::unique_ptr<MissionDMX::ShowFile::BordConfiguration> show_file_dom, std::stringstream& logging_target) : scenes{}, universes{}, name{}, scene_id_mapping{} {
	if(const auto optional_name = show_file_dom->show_name(); optional_name.present()) {
		this->name = optional_name.get();
	} else {
		this->name = "No Name";
	}

	for(const auto& universe : show_file_dom->universe()) {
		this->universes.push_back(dmxfish::io::register_universe_from_xml(universe));
	}

	auto scene_loading_result = populate_scene_vector(this->scenes, show_file_dom->scene(), this->scene_id_mapping);
	logging_target << "Scene scheduling results: " << (scene_loading_result.second ? "Success." : "Failed.") << " Logs:" << std::endl << scene_loading_result.first << std::endl;
	if(!scene_loading_result.second) {
		throw project_config_exception("Scheduling failed. Please review log.");
	}

	if(const auto as = show_file_dom->default_active_scene(); as.present()) {
		this->default_active_scene = as.get();
	}

	logging_target << "Initializing default scene." << std::endl;
	this->current_active_scene = this->default_active_scene;
	if(const auto scene_index = this->get_active_scene(); scene_index < this->scenes.size()) {
		this->scenes[scene_index].on_start();
	}
	logging_target << "Show file '" << this->get_name() << "' successfully loaded." << std::endl;
}

bool project_configuration::set_active_scene(unsigned int new_scene) {
	if(!this->scene_id_mapping.contains(new_scene)) {
		return false;
	}
	const auto new_scene_index = this->scene_id_mapping.at(new_scene);
    if (new_scene_index == this->current_active_scene) {
        return false;
    }
	if (const auto current_scene = this->get_active_scene(); current_scene < this->scenes.size()) {
		this->scenes[current_scene].on_stop();
	}
	this->current_active_scene = new_scene_index;
	if (new_scene_index >= this->scenes.size()) {
		this->current_active_scene = this->scenes.size() - 1;
	}
	if (const auto scene_index = this->get_active_scene(); scene_index < this->scenes.size()) {
		this->scenes[scene_index].on_start();
	}
	return true;
}

void project_configuration::run_cycle_update() {
	this->scenes[this->get_active_scene()].invoke_filters();
}

}
