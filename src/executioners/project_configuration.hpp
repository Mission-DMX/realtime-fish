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
	unsigned int default_active_scene = 0;
	unsigned int current_active_scene = 0;
public:
	project_configuration(std::unique_ptr<MissionDMX::ShowFile::BordConfiguration> show_file_dom, std::stringstream& logging_target);

	[[nodiscard]] inline unsigned int get_active_scene() const {
		return this->current_active_scene;
	}

	[[nodiscard]] inline unsigned int get_default_scene() const {
		return this->default_active_scene;
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

	void set_active_scene(unsigned int new_scene);

	void run_cycle_update();
};

}
