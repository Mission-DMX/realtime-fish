//
// Created by leondietrich on 2/4/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "filter_color_mixer.hpp"

#include <cmath>

#include "filters/types.hpp"

namespace dmxfish {
    namespace filters {
        filter_color_mixer_norm_rgb::filter_color_mixer_norm_rgb() : filter(), inputs(), output() {}

        void filter_color_mixer_norm_rgb::get_output_channels(channel_mapping& map, const std::string& name) {
            COMPILER_SUPRESS("-Weffc++")
            map.color_channels[name + ":value"] = &this->output;
            COMPILER_RESTORE("-Weffc++")
        }

        void filter_color_mixer_norm_rgb::setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) {
            MARK_UNUSED(initial_parameters);
            if(!configuration.contains("input_count")) {
                return;
            }
            const auto input_count = std::stoi(configuration.at("input_count"));
            this->inputs.reserve(input_count);
            for (auto i = 0; i < input_count; i++) {
                const auto i_s = std::to_string(i);
                if(!input_channels.color_channels.contains(i_s)) {
                    throw filter_config_exception("Unable to link input channel number " + i_s +
                    ". Hint: input_count is configured to " + std::to_string(input_count),
                    filter_type::filter_color_mixer_rgb_normative, own_id);
                }
                this->inputs.push_back(input_channels.color_channels.at(i_s));
            }
        }

        void filter_color_mixer_norm_rgb::update() {
            double r = 0.0;
            double g = 0.0;
            double b = 0.0;
            for (const auto in_ptr : this->inputs) {
                const auto r2 = in_ptr->getRed();
                const auto g2 = in_ptr->getGreen();
                const auto b2 = in_ptr->getBlue();
                r += r2*r2;
                g += g2*g2;
                b += b2*b2;
            }
            r = std::sqrt(r);
            g = std::sqrt(g);
            b = std::sqrt(b);
            const auto vlen = std::sqrt(r*r+g*g+b*b);
            this->output.setRed((r/vlen)*255);
            this->output.setGreen((g/vlen)*255);
            this->output.setBlue((b/vlen)*255);
        }

        void filter_color_mixer_norm_rgb::scene_activated() {
            // Do nothing
        }
    } // dmxfish
} // filters
