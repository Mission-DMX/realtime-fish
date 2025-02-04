//
// Created by leondietrich on 2/4/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "filter_color_mixer.hpp"

#include <cmath>

#include "filters/types.hpp"

namespace dmxfish {
    namespace filters {
        filter_color_mixer::filter_color_mixer() : filter(), inputs(), output() {}

        void filter_color_mixer::get_output_channels(channel_mapping& map, const std::string& name) {
            COMPILER_SUPRESS("-Weffc++")
            map.color_channels[name + ":value"] = &this->output;
            COMPILER_RESTORE("-Weffc++")
        }

        void filter_color_mixer::setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) {
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
                    filter_type::filter_color_mixer, own_id);
                }
                this->inputs.push_back(input_channels.color_channels.at(i_s));
            }
        }

        void filter_color_mixer::update() {
            if (this->inputs.size() == 0) {
                return;
            }
            this->output = *(this->inputs[0]);
            for(const auto input_ptr : this->inputs) {
                const auto h1 = this->output.getHue();
                const auto h2 = input_ptr->getHue();
                this->output.setHue(std::fmod(h1 + h2, 360.0) / 2);
                this->output.setSaturation(this->output.getSaturation() + input_ptr->getSaturation() / 2);
                this->output.setIluminance(this->output.getIluminance() + input_ptr->getIluminance() / 2);
            }
        }

        void filter_color_mixer::scene_activated() {
            // Do nothing
        }
    } // dmxfish
} // filters