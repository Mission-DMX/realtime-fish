#include "chaser_setup.hpp"

#include "utils.hpp"

#include "color_chaser.hpp"

#include "layer_plain_color.hpp"
#include "layer_rainbow.hpp"

namespace dmxfish::filters {

	chaser_setup::chaser_setup(const std::string& configuration, filter_color_chaser& target) : layers(), last_update_time(-1) {
		const auto layer_descriptions = utils::split(configuration, ';');
		layers.reserve(layer_descriptions.size());
        // TODO use a linear allocator and collect sizes beforehand
		for (const auto& entry : layer_descriptions) {
            auto param_list = utils::split(entry, '|');
            if (param_list.front() == "plain_color") {
                layers.push_back(std::move(std::make_unique<chaserlayers::plain_color>(param_list, target.color_parameter_inputs, target.number_parameter_inputs)));
            } if (param_list.front() == "rainbow") {
                layers.push_back(std::move(std::make_unique<chaserlayers::rainbow>(param_list, target.color_parameter_inputs, target.number_parameter_inputs)));
            }
            // TODO
		}
	}

    void chaser_setup::execute(filter_color_chaser& target) {
        auto scaled_time = *(target.time_input) * *(target.timescale_input);
        if(scaled_time < 0.0) {
                scaled_time = 0.0;
        }
        const auto elapsed_time = this->last_update_time - scaled_time;
        this->last_update_time = scaled_time;
        for(auto& layer : this->layers) {
            layer->apply(elapsed_time, target.pixels, target.mask);
        }
    }

    void chaser_setup::reset(filter_color_chaser& target) {
        auto scaled_time = *(target.time_input) * *(target.timescale_input);
        if(scaled_time < 0.0) {
                scaled_time = 0.0;
        }
        this->last_update_time = scaled_time;
        for (auto& layer : this->layers) {
            layer->reset();
        }
    }

}
