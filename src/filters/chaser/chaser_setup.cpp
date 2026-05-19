#include "chaser_setup.hpp"

#include "utils.hpp"

#include "filters/filter.hpp"
#include "filters/types.hpp"

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
#include "layer_chancalc.hpp"
#include "layer_gaussian_blur.hpp"
#include "layer_gaussian_curve.hpp"
#include "layer_invert_mask.hpp"
#include "layer_invert_color.hpp"
#include "layer_mask_close_to_center.hpp"
#include "layer_mask_wave.hpp"

namespace dmxfish::filters {

        using chancalc_type_r_add = chaserlayers::chancalc<chaserlayers::color_channel_target::R, chaserlayers::mod_operation_type::ADD>;
        using chancalc_type_r_sub = chaserlayers::chancalc<chaserlayers::color_channel_target::R, chaserlayers::mod_operation_type::SUB>;
        using chancalc_type_r_mul = chaserlayers::chancalc<chaserlayers::color_channel_target::R, chaserlayers::mod_operation_type::MUL>;
        using chancalc_type_r_div = chaserlayers::chancalc<chaserlayers::color_channel_target::R, chaserlayers::mod_operation_type::DIV>;
        using chancalc_type_g_add = chaserlayers::chancalc<chaserlayers::color_channel_target::G, chaserlayers::mod_operation_type::ADD>;
        using chancalc_type_g_sub = chaserlayers::chancalc<chaserlayers::color_channel_target::G, chaserlayers::mod_operation_type::SUB>;
        using chancalc_type_g_mul = chaserlayers::chancalc<chaserlayers::color_channel_target::G, chaserlayers::mod_operation_type::MUL>;
        using chancalc_type_g_div = chaserlayers::chancalc<chaserlayers::color_channel_target::G, chaserlayers::mod_operation_type::DIV>;
        using chancalc_type_b_add = chaserlayers::chancalc<chaserlayers::color_channel_target::B, chaserlayers::mod_operation_type::ADD>;
        using chancalc_type_b_sub = chaserlayers::chancalc<chaserlayers::color_channel_target::B, chaserlayers::mod_operation_type::SUB>;
        using chancalc_type_b_mul = chaserlayers::chancalc<chaserlayers::color_channel_target::B, chaserlayers::mod_operation_type::MUL>;
        using chancalc_type_b_div = chaserlayers::chancalc<chaserlayers::color_channel_target::B, chaserlayers::mod_operation_type::DIV>;
        using chancalc_type_h_add = chaserlayers::chancalc<chaserlayers::color_channel_target::H, chaserlayers::mod_operation_type::ADD>;
        using chancalc_type_h_sub = chaserlayers::chancalc<chaserlayers::color_channel_target::H, chaserlayers::mod_operation_type::SUB>;
        using chancalc_type_h_mul = chaserlayers::chancalc<chaserlayers::color_channel_target::H, chaserlayers::mod_operation_type::MUL>;
        using chancalc_type_h_div = chaserlayers::chancalc<chaserlayers::color_channel_target::H, chaserlayers::mod_operation_type::DIV>;
        using chancalc_type_s_add = chaserlayers::chancalc<chaserlayers::color_channel_target::S, chaserlayers::mod_operation_type::ADD>;
        using chancalc_type_s_sub = chaserlayers::chancalc<chaserlayers::color_channel_target::S, chaserlayers::mod_operation_type::SUB>;
        using chancalc_type_s_mul = chaserlayers::chancalc<chaserlayers::color_channel_target::S, chaserlayers::mod_operation_type::MUL>;
        using chancalc_type_s_div = chaserlayers::chancalc<chaserlayers::color_channel_target::S, chaserlayers::mod_operation_type::DIV>;
        using chancalc_type_i_add = chaserlayers::chancalc<chaserlayers::color_channel_target::I, chaserlayers::mod_operation_type::ADD>;
        using chancalc_type_i_sub = chaserlayers::chancalc<chaserlayers::color_channel_target::I, chaserlayers::mod_operation_type::SUB>;
        using chancalc_type_i_mul = chaserlayers::chancalc<chaserlayers::color_channel_target::I, chaserlayers::mod_operation_type::MUL>;
        using chancalc_type_i_div = chaserlayers::chancalc<chaserlayers::color_channel_target::I, chaserlayers::mod_operation_type::DIV>;

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
            } else if (param_list.front() == "color_chanmod__r") {
                required_mem_size += sizeof(chaserlayers::chanmod<chaserlayers::color_channel_target::R>);
            } else if (param_list.front() == "color_chanmod__g") {
                required_mem_size += sizeof(chaserlayers::chanmod<chaserlayers::color_channel_target::G>);
            } else if (param_list.front() == "color_chanmod__b") {
                required_mem_size += sizeof(chaserlayers::chanmod<chaserlayers::color_channel_target::B>);
            } else if (param_list.front() == "color_chanmod__h") {
                required_mem_size += sizeof(chaserlayers::chanmod<chaserlayers::color_channel_target::H>);
            } else if (param_list.front() == "color_chanmod__s") {
                required_mem_size += sizeof(chaserlayers::chanmod<chaserlayers::color_channel_target::S>);
            } else if (param_list.front() == "color_chanmod__i") {
                required_mem_size += sizeof(chaserlayers::chanmod<chaserlayers::color_channel_target::I>);
            } else if (param_list.front() == "color_chancalc__r__add") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::R, chaserlayers::mod_operation_type::ADD>);
            } else if (param_list.front() == "color_chancalc__r__sub") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::R, chaserlayers::mod_operation_type::SUB>);
            } else if (param_list.front() == "color_chancalc__r__mul") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::R, chaserlayers::mod_operation_type::MUL>);
            } else if (param_list.front() == "color_chancalc__r__div") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::R, chaserlayers::mod_operation_type::DIV>);
            } else if (param_list.front() == "color_chancalc__g__add") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::G, chaserlayers::mod_operation_type::ADD>);
            } else if (param_list.front() == "color_chancalc__g__sub") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::G, chaserlayers::mod_operation_type::SUB>);
            } else if (param_list.front() == "color_chancalc__g__mul") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::G, chaserlayers::mod_operation_type::MUL>);
            } else if (param_list.front() == "color_chancalc__g__div") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::G, chaserlayers::mod_operation_type::DIV>);
            } else if (param_list.front() == "color_chancalc__b__add") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::B, chaserlayers::mod_operation_type::ADD>);
            } else if (param_list.front() == "color_chancalc__b__sub") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::B, chaserlayers::mod_operation_type::SUB>);
            } else if (param_list.front() == "color_chancalc__b__mul") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::B, chaserlayers::mod_operation_type::MUL>);
            } else if (param_list.front() == "color_chancalc__b__div") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::B, chaserlayers::mod_operation_type::DIV>);
            } else if (param_list.front() == "color_chancalc__h__add") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::H, chaserlayers::mod_operation_type::ADD>);
            } else if (param_list.front() == "color_chancalc__h__sub") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::H, chaserlayers::mod_operation_type::SUB>);
            } else if (param_list.front() == "color_chancalc__h__mul") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::H, chaserlayers::mod_operation_type::MUL>);
            } else if (param_list.front() == "color_chancalc__h__div") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::H, chaserlayers::mod_operation_type::DIV>);
            } else if (param_list.front() == "color_chancalc__s__add") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::S, chaserlayers::mod_operation_type::ADD>);
            } else if (param_list.front() == "color_chancalc__s__sub") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::S, chaserlayers::mod_operation_type::SUB>);
            } else if (param_list.front() == "color_chancalc__s__mul") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::S, chaserlayers::mod_operation_type::MUL>);
            } else if (param_list.front() == "color_chancalc__s__div") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::S, chaserlayers::mod_operation_type::DIV>);
            } else if (param_list.front() == "color_chancalc__i__add") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::I, chaserlayers::mod_operation_type::ADD>);
            } else if (param_list.front() == "color_chancalc__i__sub") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::I, chaserlayers::mod_operation_type::SUB>);
            } else if (param_list.front() == "color_chancalc__i__mul") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::I, chaserlayers::mod_operation_type::MUL>);
            } else if (param_list.front() == "color_chancalc__i__div") {
                required_mem_size += sizeof(chaserlayers::chancalc<chaserlayers::color_channel_target::I, chaserlayers::mod_operation_type::DIV>);
            } else if (param_list.front() == "gaussian_blur") {
                required_mem_size += sizeof(chaserlayers::gaussian_blur);
            } else if (param_list.front() == "gaussian_curve_on_mask") {
                required_mem_size += sizeof(chaserlayers::gaussian_curve);
            } else if (param_list.front() == "invert_mask") {
                required_mem_size += sizeof(chaserlayers::invert_mask);
            } else if (param_list.front() == "invert_color") {
                required_mem_size += sizeof(chaserlayers::invert_color);
            } else if (param_list.front() == "close_to_center") {
                required_mem_size += sizeof(chaserlayers::close_to_center<false>);
            } else if (param_list.front() == "open_from_center") {
                required_mem_size += sizeof(chaserlayers::close_to_center<true>);
            } else if (param_list.front() == "segwave__fwd") {
                required_mem_size += sizeof(chaserlayers::segwave<true>);
            } else if (param_list.front() == "segwave__rev") {
                required_mem_size += sizeof(chaserlayers::segwave<false>);
            } else if (param_list.front() == "wave__fwd") {
                required_mem_size += sizeof(chaserlayers::wave<true>);
            } else if (param_list.front() == "wave__rev") {
                required_mem_size += sizeof(chaserlayers::wave<false>);
            } else {
		throw filter_config_exception("Unsupported chaser layer: " + param_list.front(), filter_type::filter_color_chaser, target.own_id);
	    }
        }
        this->alloc = LinearAllocator(required_mem_size);
        this->alloc.Init();
		for (const auto& entry : layer_descriptions) {
            auto param_list = utils::split(entry, '|');
            if (param_list.empty()) [[unlikely]] {
                throw filter_config_exception("Unable to parse layer '" + entry + "'.", filter_type::filter_color_chaser, target.own_id);
            }
            try {
#define make_inst(cls) new (this->alloc.Allocate(sizeof(cls))) cls
                if (param_list.front() == "plain_color") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chaserlayers::plain_color)(param_list, target.color_parameter_inputs,
                                                                          target.number_parameter_inputs));
                } else if (param_list.front() == "rainbow") {
                    ensure_arg_count(param_list, 2, target);
                    layers.push_back(make_inst(chaserlayers::rainbow)(param_list, target.color_parameter_inputs,
                                                                      target.number_parameter_inputs));
                } else if (param_list.front() == "sprinkles") {
                    ensure_arg_count(param_list, 5, target);
                    layers.push_back(make_inst(chaserlayers::sprinkles)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "dots") {
                    ensure_arg_count(param_list, 5, target);
                    layers.push_back(make_inst(chaserlayers::dots)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "scale") {
                    ensure_arg_count(param_list, 4, target);
                    layers.push_back(make_inst(chaserlayers::scale)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "scale_inv") {
                    ensure_arg_count(param_list, 4, target);
                    layers.push_back(make_inst(chaserlayers::scale_inv)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "flat_mask") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chaserlayers::flat_mask)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "maskmod__add") {
                    ensure_arg_count(param_list, 3, target);
                    layers.push_back(
                            make_inst(chaserlayers::mask_mod<chaserlayers::mod_operation_type::ADD>)(param_list,
                                                                                                     target.number_parameter_inputs));
                } else if (param_list.front() == "maskmod__sub") {
                    ensure_arg_count(param_list, 3, target);
                    layers.push_back(
                            make_inst(chaserlayers::mask_mod<chaserlayers::mod_operation_type::SUB>)(param_list,
                                                                                                     target.number_parameter_inputs));
                } else if (param_list.front() == "maskmod__mul") {
                    ensure_arg_count(param_list, 3, target);
                    layers.push_back(
                            make_inst(chaserlayers::mask_mod<chaserlayers::mod_operation_type::MUL>)(param_list,
                                                                                                     target.number_parameter_inputs));
                } else if (param_list.front() == "maskmod__div") {
                    ensure_arg_count(param_list, 3, target);
                    layers.push_back(
                            make_inst(chaserlayers::mask_mod<chaserlayers::mod_operation_type::DIV>)(param_list,
                                                                                                     target.number_parameter_inputs));
                } else if (param_list.front() == "mask_shift") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chaserlayers::mask_shift)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_shift") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chaserlayers::color_shift)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "trig__sin") {
                    ensure_arg_count(param_list, 4, target);
                    layers.push_back(make_inst(chaserlayers::trig<chaserlayers::trig_operations::SIN>)(param_list,
                                                                                                       target.number_parameter_inputs));
                } else if (param_list.front() == "trig__cos") {
                    ensure_arg_count(param_list, 4, target);
                    layers.push_back(make_inst(chaserlayers::trig<chaserlayers::trig_operations::COS>)(param_list,
                                                                                                       target.number_parameter_inputs));
                } else if (param_list.front() == "trig__tan") {
                    ensure_arg_count(param_list, 4, target);
                    layers.push_back(make_inst(chaserlayers::trig<chaserlayers::trig_operations::TAN>)(param_list,
                                                                                                       target.number_parameter_inputs));
                } else if (param_list.front() == "strobe") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chaserlayers::mask_strobe)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "johnson__fwd") {
                    ensure_arg_count(param_list, 2, target);
                    layers.push_back(make_inst(chaserlayers::johnson<chaserlayers::direction::FWD>)(param_list,
                                                                                                    target.number_parameter_inputs));
                } else if (param_list.front() == "johnson__rev") {
                    ensure_arg_count(param_list, 2, target);
                    layers.push_back(make_inst(chaserlayers::johnson<chaserlayers::direction::REV>)(param_list,
                                                                                                    target.number_parameter_inputs));
                } else if (param_list.front() == "colormix") {
                    ensure_arg_count(param_list, 2, target);
                    layers.push_back(make_inst(chaserlayers::colormix)(param_list, target.color_parameter_inputs));
                } else if (param_list.front() == "random_color") {
                    ensure_arg_count(param_list, 2, target);
                    layers.push_back(make_inst(chaserlayers::randomcolor)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chanmod_r") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chaserlayers::chanmod<chaserlayers::color_channel_target::R>)(param_list,
                                                                                                             target.number_parameter_inputs));
                } else if (param_list.front() == "color_chanmod_g") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chaserlayers::chanmod<chaserlayers::color_channel_target::G>)(param_list,
                                                                                                             target.number_parameter_inputs));
                } else if (param_list.front() == "color_chanmod_b") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chaserlayers::chanmod<chaserlayers::color_channel_target::B>)(param_list,
                                                                                                             target.number_parameter_inputs));
                } else if (param_list.front() == "color_chanmod_h") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chaserlayers::chanmod<chaserlayers::color_channel_target::H>)(param_list,
                                                                                                             target.number_parameter_inputs));
                } else if (param_list.front() == "color_chanmod_s") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chaserlayers::chanmod<chaserlayers::color_channel_target::S>)(param_list,
                                                                                                             target.number_parameter_inputs));
                } else if (param_list.front() == "color_chanmod_i") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chaserlayers::chanmod<chaserlayers::color_channel_target::I>)(param_list,
                                                                                                             target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_r_add") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_r_add)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_r_sub") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_r_sub)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_r_mul") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_r_mul)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_r_div") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_r_div)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_g_add") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_g_add)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_g_sub") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_g_sub)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_g_mul") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_g_mul)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_g_div") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_g_div)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_b_add") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_b_add)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_b_sub") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_b_sub)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_b_mul") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_b_mul)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_b_div") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_b_div)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_h_add") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_h_add)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_h_sub") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_h_sub)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_h_mul") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_h_mul)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_h_div") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_h_div)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_s_add") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_s_add)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_s_sub") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_s_sub)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_s_mul") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_s_mul)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_s_div") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_s_div)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_i_add") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_i_add)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_i_sub") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_i_sub)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_i_mul") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_i_mul)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "color_chancalc_i_div") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(make_inst(chancalc_type_i_div)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "gaussian_blur") {
                    ensure_arg_count(param_list, 1, target);
                    layers.push_back(
                            make_inst(chaserlayers::gaussian_blur)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "gaussian_curve_on_mask") {
                    ensure_arg_count(param_list, 3, target);
                    layers.push_back(
                            make_inst(chaserlayers::gaussian_curve)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "invert_mask") {
                    ensure_arg_count(param_list, 0, target);
                    layers.push_back(make_inst(chaserlayers::invert_mask)());
                } else if (param_list.front() == "invert_color") {
                    ensure_arg_count(param_list, 0, target);
                    layers.push_back(make_inst(chaserlayers::invert_color)());
                } else if (param_list.front() == "close_to_center") {
                    ensure_arg_count(param_list, 3, target);
                    layers.push_back(make_inst(chaserlayers::close_to_center<false>)(param_list,
                                                                                     target.number_parameter_inputs));
                } else if (param_list.front() == "open_from_center") {
                    ensure_arg_count(param_list, 3, target);
                    layers.push_back(
                            make_inst(chaserlayers::close_to_center<true>)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "segwave__fwd") {
                    ensure_arg_count(param_list, 4, target);
                    layers.push_back(
                            make_inst(chaserlayers::segwave<true>)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "segwave__rev") {
                    ensure_arg_count(param_list, 4, target);
                    layers.push_back(
                            make_inst(chaserlayers::segwave<false>)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "wave__fwd") {
                    ensure_arg_count(param_list, 4, target);
                    layers.push_back(make_inst(chaserlayers::wave<true>)(param_list, target.number_parameter_inputs));
                } else if (param_list.front() == "wave__rev") {
                    ensure_arg_count(param_list, 4, target);
                    layers.push_back(make_inst(chaserlayers::wave<false>)(param_list, target.number_parameter_inputs));
                }
#undef make_inst
            } catch (const std::invalid_argument& e) {
                throw filter_config_exception("Unable to parse parameter (std::invalid_argument) in layer description: "
                                              + entry + ". Cause: " + e.what(), filter_type::filter_color_chaser, target.own_id);
            } catch (const std::out_of_range& e) {
                throw filter_config_exception("Unable to parse parameter (std::out_of_range) in layer description: "
                                              + entry + ". Cause: " + e.what(), filter_type::filter_color_chaser, target.own_id);
            }
		}
	}

    chaser_setup::~chaser_setup() {
        // We do not need to do things here as we delete both the vector as well as the allocator, thus freeing the
        // memory
    }

    void chaser_setup::ensure_arg_count(const std::list<std::string>& arg_l, size_t required,
                                        const filter_color_chaser& target) {
        if (arg_l.size() < required + 1) {
            throw filter_config_exception("Not enough parameters to instantiate layer " + arg_l.front() + ".",
                                          filter_type::filter_color_chaser, target.own_id);
        }
    }

    void chaser_setup::execute(filter_color_chaser& target) {
        auto scaled_time = *(target.time_input) * *(target.timescale_input);
        if(scaled_time < 0.0) {
                scaled_time = 0.0;
        }
        const auto elapsed_time = target.uses_steps ? 0.0 : scaled_time - this->last_update_time;
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
