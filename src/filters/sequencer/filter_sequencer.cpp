//
// Created by Leon Dietrich on 13.02.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "filters/sequencer/filter_sequencer.hpp"

namespace dmxfish {
    namespace filters {
        filter_sequencer::filter_sequencer() : channels_8bit(), channels_16bit(), channels_float(), channels_color(), transitions() {

        }

        filter_sequencer::~filter_sequencer() {

        }

        void filter_sequencer::setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) {

        }

        bool filter_sequencer::receive_update_from_gui(const std::string& key, const std::string& _value) {

        }

        void filter_sequencer::get_output_channels(channel_mapping& map, const std::string& name) {

        }

        void filter_sequencer::update() {
            // TODO queue transitions based on current events
            // TODO update all channels, using time input and time scale
        }

        void filter_sequencer::scene_activated() {
            // TODO clear transition queues of channels
            // TODO reset channels to their default value if required
        }
    } // filters
} // dmxfish