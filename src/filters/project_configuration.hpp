#pragma once

#include <string>
#include <vector>

#include "dmx/universe.hpp"
#include "filters/scene.hpp"

#include "xml/show_files.hpp"

namespace dmxfish::filters {

class project_configuration {
private:
	std::vector<scene> scenes;
	std::vector<std::shared_ptr<dmxfish::dmx::universe>> universes;
	std::string name;
	unsigned int default_active_scene;
	unsigned int current_active_scene;
public:
	project_configuration(const MissionDMX::ShowFile::BordConfiguration& show_file_dom);

	[[nodiscard]] inline unsigned int get_active_scene() {
		return this->current_active_scene;
	}

	void set_active_scene(unsigned int new_scene);

	void run_cycle_update();
};

}
