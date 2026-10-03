//
// Created by leondietrich on 9/12/26.
//

#pragma once

#include <vector>

#include "filter.hpp"
#include "events/event.hpp"

namespace dmxfish {
    namespace filters {

        struct event_template {
            ::dmxfish::events::event_type type;
            std::array<uint8_t, 8> arguments;
            ::dmxfish::events::event_sender_t sender;
        };

        /**
         * Event Scheduler filter.
         *
         * The event scheduler advances the current step on every received synchronization event.
         * Usually, the synchronization event is originating from a beat tracker, show UI widget or TAP button.
         * Once the step advanced, the templates for each active event in the step are inserted.
         */
        class event_scheduler : public filter {
        private:
            dmxfish::events::event_sender_t synchronization_target;
            size_t current_step;
            size_t max_step;
            std::vector<bool> sequence_steps;
            std::vector<event_template> event_templates;
            std::string own_filter_id;
        public:
            event_scheduler();
            virtual ~event_scheduler();
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

    } // filters
} // dmxfish
