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
#include "layer_mask_mod.hpp"
#include "layer_mask_shift.hpp"
#include "layer_color_shift.hpp"
#include "layer_trig.hpp"

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
            } else if (param_list.front() == "maskmod__add") {
                required_mem_size += sizeof(chaserlayers::mask_mod<chaserlayers::mod_operation_type::ADD>);
            } else if (param_list.front() == "maskmod__sub") {
                required_mem_size += sizeof(chaserlayers::mask_mod<chaserlayers::mod_operation_type::SUB>);
            } else if (param_list.front() == "maskmod__mul") {
                required_mem_size += sizeof(chaserlayers::mask_mod<chaserlayers::mod_operation_type::MUL>);
            } else if (param_list.front() == "maskmod__div") {
                required_mem_size += sizeof(chaserlayers::mask_mod<chaserlayers::mod_operation_type::DIV>);
            } else if (param_list.front() == "mask_shift") {
                required_mem_size += sizeof(chaserlayers::mask_shift);
            } else if (param_list.front() == "color_shift") {
                required_mem_size += sizeof(chaserlayers::color_shift);
            } else if (param_list.front() == "trig__sin") {
                required_mem_size += sizeof(chaserlayers::trig<chaserlayers::trig_operations::SIN>);
            } else if (param_list.front() == "trig__cos") {
                required_mem_size += sizeof(chaserlayers::trig<chaserlayers::trig_operations::COS>);
            } else if (param_list.front() == "trig__tan") {
                required_mem_size += sizeof(chaserlayers::trig<chaserlayers::trig_operations::TAN>);
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
            } else if (param_list.front() == "maskmod__add") {
                layers.push_back(make_inst(chaserlayers::mask_mod<chaserlayers::mod_operation_type::ADD>)(param_list, target.number_parameter_inputs));
            } else if (param_list.front() == "maskmod__sub") {
                layers.push_back(make_inst(chaserlayers::mask_mod<chaserlayers::mod_operation_type::SUB>)(param_list, target.number_parameter_inputs));
            } else if (param_list.front() == "maskmod__mul") {
                layers.push_back(make_inst(chaserlayers::mask_mod<chaserlayers::mod_operation_type::MUL>)(param_list, target.number_parameter_inputs));
            } else if (param_list.front() == "maskmod__div") {
                layers.push_back(make_inst(chaserlayers::mask_mod<chaserlayers::mod_operation_type::DIV>)(param_list, target.number_parameter_inputs));
            } else if (param_list.front() == "mask_shift") {
                layers.push_back(make_inst(chaserlayers::mask_shift)(param_list, target.number_parameter_inputs));
            } else if (param_list.front() == "color_shift") {
                layers.push_back(make_inst(chaserlayers::color_shift)(param_list, target.number_parameter_inputs));
            } else if (param_list.front() == "trig__sin") {
                layers.push_back(make_inst(chaserlayers::trig<chaserlayers::trig_operations::SIN>)(param_list, target.number_parameter_inputs));
            } else if (param_list.front() == "trig__cos") {
                layers.push_back(make_inst(chaserlayers::trig<chaserlayers::trig_operations::COS>)(param_list, target.number_parameter_inputs));
            } else if (param_list.front() == "trig__tan") {
                layers.push_back(make_inst(chaserlayers::trig<chaserlayers::trig_operations::TAN>)(param_list, target.number_parameter_inputs));
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
