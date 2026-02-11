#pragma once

#include <stdexcept>
#include <string>
#include <sstream>
#include <map>
#include <memory>
#include <vector>

#include "dmx/universe.hpp"
#include "executioners/scene.hpp"

#include "xml/show_files.hpp"

namespace dmxfish::execution {

class project_config_exception : public std::exception {
private:
    std::string cause;

public:
    project_config_exception(const std::string cause_) : cause(cause_) {}
    [[nodiscard]] inline virtual const char* what() const throw () {
        return this->cause.c_str();
    }
};

class project_configuration {
private:
	std::vector<scene> scenes;
	std::vector<std::shared_ptr<dmxfish::dmx::universe>> universes;
	std::map<int32_t, size_t> scene_id_mapping;
	std::string name;
	size_t default_active_scene = 0;
	size_t current_active_scene = 0;
	unsigned int current_scene_id = 0;
public:
	project_configuration(std::unique_ptr<MissionDMX::ShowFile::BordConfiguration> show_file_dom, std::stringstream& logging_target);

	/**
	 * Get the index of the current active scene.
	 */
	[[nodiscard]] inline size_t get_active_scene_index() const {
		return this->current_active_scene;
	}

	/**
	 * Get the index of the default active scene.
	 */
	[[nodiscard]] inline size_t get_default_scene() const {
		return this->default_active_scene;
	}

	/**
	 * Get the ID of the current active scene.
	 */
	[[nodiscard]] inline unsigned int get_current_scene_id() const {
		return this->current_scene_id;
	}

	[[nodiscard]] inline std::string get_name() const {
		return this->name;
	}

	[[nodiscard]] inline bool update_filter_parameter(int32_t show_id, const std::string& filter_id, const std::string& key, const std::string& value) {
		if(!this->scene_id_mapping.contains(show_id)) {
			throw std::invalid_argument("Failed to update filter parameter of non existant show " + std::to_string(show_id) + ".");
		}
		return this->scenes[this->scene_id_mapping.at(show_id)].update_filter_parameter(filter_id, key, value);
	}

	/**
	 * Set the current active scene to a scene with the specified ID.
	 *
	 * The ID does not need to be the scene index.
	 * @param new_scene_id The ID of the scene as specified in the show file.
	 */
	bool set_active_scene(unsigned int new_scene_id);

	void run_cycle_update();
};

}
