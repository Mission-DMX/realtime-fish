#include "chaser_setup.hpp"

#include "utils.hpp"

#include "color_chaser.hpp"

#include "layer_plain_color.hpp"
#include "layer_rainbow.hpp"
#include "layer_sprinkles.hpp"
#include "layer_dots.hpp"
#include "layer_scale.hpp"
#include "layer_scale_inv.hpp"
#include "layer_flat_mask.hpp"

namespace dmxfish::filters {

	chaser_setup::chaser_setup(const std::string& configuration, filter_color_chaser& target) : layers(), last_update_time(-1), alloc(0) {
		const auto layer_descriptions = utils::split(configuration, ';');
		layers.reserve(layer_descriptions.size());
        size_t required_mem_size = 0;
        for (const auto& entry : layer_descriptions) {
            auto param_list = utils::split(entry, '|');
            if (param_list.front() == "plain_color") {
                required_mem_size += sizeof(chaserlayers::plain_color);
            } else if (param_list.front() == "rainbow") {
                required_mem_size += sizeof(chaserlayers::rainbow);
            } else if (param_list.front() == "sprinkles") {
                required_mem_size += sizeof(chaserlayers::sprinkles);
            } else if (param_list.front() == "dots") {
                required_mem_size += sizeof(chaserlayers::dots);
            } else if (param_list.front() == "scale") {
                required_mem_size += sizeof(chaserlayers::scale);
            } else if (param_list.front() == "scale_inv") {
                required_mem_size += sizeof(chaserlayers::scale_inv);
            } else if (param_list.front() == "flat_mask") {
                required_mem_size += sizeof(chaserlayers::flat_mask);
            }
            // TODO continue
        }
        this->alloc = LinearAllocator(required_mem_size);
        this->alloc.Init();
		for (const auto& entry : layer_descriptions) {
            auto param_list = utils::split(entry, '|');
#define make_inst(cls) new (this->alloc.Allocate(sizeof(cls))) cls
            if (param_list.front() == "plain_color") {
                layers.push_back(make_inst(chaserlayers::plain_color)(param_list, target.color_parameter_inputs, target.number_parameter_inputs));
            } else if (param_list.front() == "rainbow") {
                layers.push_back(make_inst(chaserlayers::rainbow)(param_list, target.color_parameter_inputs, target.number_parameter_inputs));
            } else if (param_list.front() == "sprinkles") {
                layers.push_back(make_inst(chaserlayers::sprinkles)(param_list, target.number_parameter_inputs));
            } else if (param_list.front() == "dots") {
                layers.push_back(make_inst(chaserlayers::dots)(param_list, target.number_parameter_inputs));
            } else if (param_list.front() == "scale") {
                layers.push_back(make_inst(chaserlayers::scale)(param_list, target.number_parameter_inputs));
            } else if (param_list.front() == "scale_inv") {
                layers.push_back(make_inst(chaserlayers::scale_inv)(param_list, target.number_parameter_inputs));
            } else if (param_list.front() == "flat_mask") {
                layers.push_back(make_inst(chaserlayers::flat_mask)(param_list, target.number_parameter_inputs));
            }
            // TODO continue
#undef make_inst
		}
	}

    chaser_setup::~chaser_setup() {
        // We do not need to do things here as we delete both the vector as well as the allocator, thus freeing the
        // memory
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
