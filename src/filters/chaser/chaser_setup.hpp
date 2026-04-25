#pragma once

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cle_parameters.hpp"

namespace dmxfish::filters {

	class filter_color_chaser;

	class chaser_layer_executor {
	public:
		virtual void reset() = 0;
		virtual void apply(
				const double elapsed_time,
				std::vector<dmxfish::dmx::pixel>& pixels,
				std::vector<uint16_t>& mask
		) = 0;
	};

	class chaser_setup {
	private:
		std::vector<std::unique_ptr<chaser_layer_executor>> layers;
		double last_update_time;
	public:
		chaser_setup(const std::string& configuration, filter_color_chaser& target);
		void execute(filter_color_chaser& target);
		void reset(filter_color_chaser& target);
	};
}
