#pragma once

#include <string>
#include <vector>

#include "dmx/universe.hpp"
#include "executioners/scene.hpp"

#include "xml/show_files.hpp"

namespace dmxfish::execution {

class project_configuration {
private:
	std::vector<scene> scenes;
	std::vector<std::shared_ptr<dmxfish::dmx::universe>> universes;
	std::string name;
	unsigned int default_active_scene = 0;
	unsigned int current_active_scene = 0;
public:
	project_configuration(const MissionDMX::ShowFile::BordConfiguration& show_file_dom);

	[[nodiscard]] inline unsigned int get_active_scene() {
		return this->current_active_scene;
	}

	[[nodiscard]] inline unsigned int get_default_scene() {
		return this->default_active_scene;
	}

	void set_active_scene(unsigned int new_scene);

	void run_cycle_update();
};

}
