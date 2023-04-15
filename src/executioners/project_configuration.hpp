#pragma once

#include <string>
#include <sstream>
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

	// TODO write update_parameter method

	void set_active_scene(unsigned int new_scene);

	void run_cycle_update();
};

}
