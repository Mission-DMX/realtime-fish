//
// Created by leondietrich on 2/4/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <string>
#include <vector>

#include "filters/filter.hpp"
#include "lib/macros.hpp"

namespace dmxfish {
    namespace filters {

        class filter_color_mixer : public filter {
        private:
            COMPILER_SUPRESS("-Weffc++")
            std::vector<dmxfish::dmx::pixel*> inputs;
            COMPILER_RESTORE("-Weffc++")
            dmxfish::dmx::pixel output;
        public:
            filter_color_mixer();
            virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override;
            virtual void get_output_channels(channel_mapping& map, const std::string& name) override;
            virtual void update() override;
            virtual void scene_activated() override;
        };

    } // dmxfish
} // filters

