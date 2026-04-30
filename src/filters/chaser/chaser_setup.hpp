#pragma once

#include <list>
#include <map>
#include <string>
#include <vector>

#include "allocators/LinearAllocator.h"

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
                virtual void step() = 0;
	};

	class chaser_setup {
	private:
        double last_update_time;
		std::vector<chaser_layer_executor*> layers;
        LinearAllocator alloc;
	public:
		chaser_setup(const std::string& configuration, filter_color_chaser& target);
        ~chaser_setup();
		void execute(filter_color_chaser& target);
		void step();
		void reset(filter_color_chaser& target);
	};
}
