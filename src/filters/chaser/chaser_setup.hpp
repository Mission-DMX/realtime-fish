#pragma once

#include <list>
#include <map>
#include <string>
#include <vector>

#include "allocators/LinearAllocator.h"

#include "cle_parameters.hpp"

namespace dmxfish::filters {

	class filter_color_chaser;

        /**
         * Implements the behavior of a layer.
         */
	class chaser_layer_executor {
	public:
                /**
                 * This method gets called if the layer should be reset due to scene switch.
                 */
		virtual void reset() = 0;

                /**
                 * Implements the updating behavior.
                 *
                 * This method needs to update the provided data sets in place.
                 * Each layer updating the pixel data must use the mask to apply the new color in the correct intensity.
                 *
                 * @param elapsed_time: How much time has passed since the last invocation?
                 * Use this to synchronize the effect.
                 * In case of step based synchronization, 0 will be passed.
                 *
                 * @param pixels: The color data to be modified.
                 * @param mask: The mask data to be used or modified.
                 */
		virtual void apply(
				const double elapsed_time,
				std::vector<dmxfish::dmx::pixel>& pixels,
				std::vector<uint16_t>& mask
		) = 0;

                /**
                 * In case of step based synchronization, this method will be called on every occuring event.
                 * Implement your synchronization logic here.
                 */
                virtual void step() = 0;
	};

        /**
         * Provides a coherent configuration for the chaser filter.
         *
         * Only one configuration may be active at any given time.
         */
	class chaser_setup {
	private:
        double last_update_time;
		std::vector<chaser_layer_executor*> layers;
        LinearAllocator alloc;
	public:
                /**
                 * Parse the provided description and build a configuration for the provided chaser instance.
                 *
                 * @param configuration: The configuration to parse.
                 * @param target: The target chaser filter.
                 */
		chaser_setup(const std::string& configuration, filter_color_chaser& target);

                /**
                 * Frees all allocated resources and destroys the allocator.
                 */
                ~chaser_setup();

                /**
                 * Perform the layer inference logic for the current filter iteration.
                 *
                 * @param target: The calling chaser filter.
                 */
                void execute(filter_color_chaser& target);

                /**
                 * Synchronize based on event.
                 */
                void step();

                /**
                 * Reset configuration after scene switch.
                 *
                 * @param target: The parent chaser filter.
                 */
                void reset(filter_color_chaser& target);
    private:
        void ensure_arg_count(const std::list<std::string>& arg_l, size_t required, const filter_color_chaser& target);
	};
}
