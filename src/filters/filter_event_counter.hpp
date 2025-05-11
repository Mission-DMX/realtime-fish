//
// Created by Doralitze on 11.05.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <cstdint>

#include "events/event.hpp"

namespace dmxfish::filters {

    class filter_event_counter: public filter {
    private:
        double time_til_next_count = 1000.0;
        COMPILER_SUPRESS("-Weffc++")
        double* time;
        COMPILER_RESTORE("-Weffc++")
        dmxfish::events::event_sender_t trigger_event_sender;
        unsigned long counted_events = 0;
        uint16_t freq = 0;
        uint16_t bpm = 0;
    public:
        filter_event_counter();
        virtual ~filter_event_counter();

        virtual void setup_filter(
                const std::map<std::string, std::string>& configuration,
                const std::map<std::string, std::string>& initial_parameters,
                const channel_mapping& input_channels,
                const std::string& own_id) override;

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override;
        virtual void get_output_channels(channel_mapping& map, const std::string& name) override;
        virtual void update() override;
        virtual void scene_activated() override;
    };
}
