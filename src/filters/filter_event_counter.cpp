//
// Created by Doralitze on 11.05.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "filters/filter_event_counter.hpp"

#include "events/sender_parsing.hpp"
#include "filters/types.hpp"
#include "lib/macros.hpp"

#include "main.hpp"

namespace dmxfish::filters {
    filter_event_counter::filter_event_counter() : filter(), bpm_window{} {
        this->trigger_event_sender.encoded_sender_id = 0;
    }

    filter_event_counter::~filter_event_counter() {}

    void filter_event_counter::setup_filter(
            const std::map<std::string, std::string>& configuration,
            const std::map<std::string, std::string>& initial_parameters,
            const channel_mapping& input_channels,
            const std::string& own_id) {
        MARK_UNUSED(initial_parameters);
        if (!configuration.contains("event")){
            throw filter_config_exception("Unable to setup bpm counter filter: configuration does not contain a value for"
                                          " 'event'", filter_type::filter_event_counter, own_id);
        }
        this->trigger_event_sender = dmxfish::events::parse_sender_representation(configuration.at("event"));
        this->time = input_channels.float_channels.at("time");
    }

    bool filter_event_counter::receive_update_from_gui(const std::string& key, const std::string& _value) {
        MARK_UNUSED(key);
        MARK_UNUSED(_value);
        return false;
    }

    void filter_event_counter::get_output_channels(channel_mapping& map, const std::string& name) {
        map.sixteen_bit_channels[name + ":bpm"] = &bpm;
        map.sixteen_bit_channels[name + ":freq"] = &freq;
    }

    void filter_event_counter::scene_activated() {
        this->freq = 0;
        this->bpm = 0;
        this->counted_events = 0;
        this->time_til_next_count = 1000.0;
	for (auto& t : this->bpm_window) {
	    t = 0;
	}
    }

    void filter_event_counter::update() {
        this->time_til_next_count -= (*time - this->last_update);
	this->last_update = *time;
        if (this->time_til_next_count <= 0.0) {
            this->freq = this->counted_events;
	    uint16_t window_counter = 0;
	    for (int i = 1; i < this->bpm_window.size(); i++) {
		bpm_window[i - 1] = bpm_window[i];
		window_counter += bpm_window[i];
	    }
	    bpm_window[bpm_window.size() - 1] = this->counted_events;
            this->bpm = window_counter * 15;
            this->time_til_next_count = 1000.0;
            this->counted_events = 0;
        }
        for(const auto& event : get_event_storage_instance()->get_storage()) {
            if (event.get_event_sender().is_same_sender(this->trigger_event_sender)) {
                this->counted_events++;
            }
        }
    }
}
