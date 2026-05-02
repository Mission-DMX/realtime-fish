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
#include "layer_strobe.hpp"
#include "layer_johnson.hpp"
#include "layer_colormix.hpp"
#include "layer_randomcolor.hpp"
#include "layer_chanmod.hpp"

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
            } else if (param_list.front() == "strobe") {
                required_mem_size += sizeof(chaserlayers::mask_strobe);
            } else if (param_list.front() == "johnson__fwd") {
                required_mem_size += sizeof(chaserlayers::johnson<chaserlayers::direction::FWD>);
            } else if (param_list.front() == "johnson__rev") {
                required_mem_size += sizeof(chaserlayers::johnson<chaserlayers::direction::REV>);
            } else if (param_list.front() == "colormix") {
                required_mem_size += sizeof(chaserlayers::colormix);
            } else if (param_list.front() == "random_color") {
                required_mem_size += sizeof(chaserlayers::randomcolor);
            } else if (param_list.front() == "color_chanmod_r") {
                required_mem_size += sizeof(chaserlayers::chanmod<chaserlayers::color_channel_target::R>);
            } else if (param_list.front() == "color_chanmod_g") {
                required_mem_size += sizeof(chaserlayers::chanmod<chaserlayers::color_channel_target::G>);
            } else if (param_list.front() == "color_chanmod_b") {
                required_mem_size += sizeof(chaserlayers::chanmod<chaserlayers::color_channel_target::B>);
            } else if (param_list.front() == "color_chanmod_h") {
                required_mem_size += sizeof(chaserlayers::chanmod<chaserlayers::color_channel_target::H>);
            } else if (param_list.front() == "color_chanmod_s") {
                required_mem_size += sizeof(chaserlayers::chanmod<chaserlayers::color_channel_target::S>);
            } else if (param_list.front() == "color_chanmod_i") {
                required_mem_size += sizeof(chaserlayers::chanmod<chaserlayers::color_channel_target::I>);
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
            } else if (param_list.front() == "strobe") {
                layers.push_back(make_inst(chaserlayers::mask_strobe)(param_list, target.number_parameter_inputs));
            } else if (param_list.front() == "johnson__fwd") {
                layers.push_back(make_inst(chaserlayers::johnson<chaserlayers::direction::FWD>)(param_list, target.number_parameter_inputs));
            } else if (param_list.front() == "johnson__rev") {
                layers.push_back(make_inst(chaserlayers::johnson<chaserlayers::direction::REV>)(param_list, target.number_parameter_inputs));
            } else if (param_list.front() == "colormix") {
                layers.push_back(make_inst(chaserlayers::colormix)(param_list, target.color_parameter_inputs));
            } else if (param_list.front() == "random_color") {
                layers.push_back(make_inst(chaserlayers::randomcolor)(param_list, target.number_parameter_inputs));
            } else if (param_list.front() == "color_chanmod_r") {
		layers.push_back(make_inst(chaserlayers::chanmod<chaserlayers::color_channel_target::R>)(param_list, target.number_parameter_inputs));
	    } else if (param_list.front() == "color_chanmod_g") {
		layers.push_back(make_inst(chaserlayers::chanmod<chaserlayers::color_channel_target::G>)(param_list, target.number_parameter_inputs));
	    } else if (param_list.front() == "color_chanmod_b") {
		layers.push_back(make_inst(chaserlayers::chanmod<chaserlayers::color_channel_target::B>)(param_list, target.number_parameter_inputs));
	    } else if (param_list.front() == "color_chanmod_h") {
		layers.push_back(make_inst(chaserlayers::chanmod<chaserlayers::color_channel_target::H>)(param_list, target.number_parameter_inputs));
	    } else if (param_list.front() == "color_chanmod_s") {
		layers.push_back(make_inst(chaserlayers::chanmod<chaserlayers::color_channel_target::S>)(param_list, target.number_parameter_inputs));
	    } else if (param_list.front() == "color_chanmod_i") {
		layers.push_back(make_inst(chaserlayers::chanmod<chaserlayers::color_channel_target::I>)(param_list, target.number_parameter_inputs));
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
        const auto elapsed_time = target.uses_steps ? 0.0 : this->last_update_time - scaled_time;
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

    void chaser_setup::step() {
        for(auto& layer : this->layers) {
            layer->step();
        }
    }

}
