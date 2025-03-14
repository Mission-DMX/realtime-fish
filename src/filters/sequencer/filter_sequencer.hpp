//
// Created by Leon Dietrich on 13.02.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <map>
#include <cstdint>
#include <memory>
#include <vector>

#include "dmx/pixel.hpp"
#include "filters/filter.hpp"
#include "filters/sequencer/channel.hpp"
#include "filters/sequencer/keyframe.hpp"
#include "filters/sequencer/name_maps.h"
#include "filters/sequencer/transition.hpp"
#include "filters/sequencer/time.hpp"

#include "lib/macros.hpp"

namespace dmxfish {
    namespace filters {

        class filter_sequencer : public filter {
        private:
            COMPILER_SUPRESS("-Weffc++")
            double* input_time = nullptr;
            double* time_scale = nullptr;
            COMPILER_RESTORE("-Weffc++")
            std::vector<sequencer::channel<uint8_t>> channels_8bit;
            std::vector<sequencer::channel<uint16_t>> channels_16bit;
            std::vector<sequencer::channel<double>> channels_float;
            std::vector<sequencer::channel<dmxfish::dmx::pixel>> channels_color;
            std::multimap<uint64_t, sequencer::transition> transitions;
            std::unique_ptr<name_maps> tmp_name_maps = nullptr;
        public:
            filter_sequencer();
            virtual ~filter_sequencer();

            virtual void pre_setup(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const std::string& own_id) override;
            virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override;
            virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override;
            virtual void get_output_channels(channel_mapping& map, const std::string& name) override;
            virtual void update() override;
            virtual void scene_activated() override;
        private:
            void ensure_uniqueness(std::map<std::string, size_t>& m, const std::string& new_name, const std::string& own_id, size_t channel_id);
            void decode_input_channels(const channel_mapping& input_channels, const std::string& own_id);
            void construct_channels(const std::map<std::string, std::string>& configuration, const std::string& own_id, name_maps& nm);
            void construct_transitions(const std::map<std::string, std::string>& configuration, const std::string& own_id, name_maps& nm);
            void enqueue_transition(const sequencer::transition& t);
        };

    } // filters
} // dmxfish

