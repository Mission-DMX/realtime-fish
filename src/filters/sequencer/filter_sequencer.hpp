//
// Created by Leon Dietrich on 13.02.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <vector>

#include "dmx/pixel.hpp"
#include "filters/filter.hpp"
#include "filters/sequencer/channel.h"
#include "filters/sequencer/keyframe.hpp"

namespace dmxfish {
    namespace filters {

        class filter_sequencer : public filter {
        private:
            std::vector<sequencer_channel<uint8_t>> channels_8bit;
            std::vector<sequencer_channel<uint16_t>> channels_16bit;
            std::vector<sequencer_channel<double>> channels_float;
            std::vector<sequencer_channel<dmxfish::dmx::pixel>> channels_color;
        public:
            filter_sequencer();
            virtual ~filter_sequencer();

            virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override;
            virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override;
            virtual void get_output_channels(channel_mapping& map, const std::string& name) override;
            virtual void update() override;
            virtual void scene_activated() override;
        };

    } // filters
} // dmxfish

