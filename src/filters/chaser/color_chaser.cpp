#include "color_chaser.hpp"

#include <string>

#include "lib/logging.hpp"

#include "events/sender_parsing.hpp"
#include "filters/types.hpp"
#include "utils.hpp"
#include "main.hpp"

#include "chaser_setup.hpp"

namespace dmxfish::filters {

        filter_color_chaser::filter_color_chaser() :
                color_parameter_inputs(),
                number_parameter_inputs(),
                pixels(),
                mask(),
                event_sender(),
                event_arguments() {
                for (auto& entry : this->event_arguments) {
                        entry = 0;
                }
        }

        void filter_color_chaser::pre_setup(const std::map<std::string, std::string>& configuration,
                        const std::map<std::string, std::string>& initial_parameters, const std::string& own_id) {
                MARK_UNUSED(initial_parameters);
                this->own_id = own_id;
                constexpr filter_type own_type = filter_type::filter_color_chaser;
                if (configuration.contains("number_of_pixels")) [[likely]] {
                        const auto num_pix = std::stol(configuration.at("number_of_pixels"));
                        this->pixels.reserve(num_pix);
                        this->mask.reserve(num_pix);
                        for (auto i = 0; i < num_pix; i++) {
                                this->pixels.emplace_back(0.0, 0.0, 1.0);
                        }
                        for (auto i = 0; i < num_pix; i++) {
                                this->mask.emplace_back(65535);
                        }
                } else {
                        throw filter_config_exception("Filter configuration needs to contain number_of_pixels parameter.", own_type, own_id);
                }
                this->uses_steps = configuration.contains("trigger_event") && configuration.at("trigger_event") != "";
                if (this->uses_steps) {
                        auto ev_params = utils::split(configuration.at("trigger_event"), ':');
                        if (ev_params.size() < 2) [[unlikely]] {
                                throw filter_config_exception("Invalid event format. Got: " + configuration.at("trigger_event"), own_type, own_id);
                        }
                        const auto sender = std::stoi(ev_params.front());
                        ev_params.pop_front();
                        this->event_sender = dmxfish::events::event_sender_t(sender, std::stoi(ev_params.front()));
                        ev_params.pop_front();
                        for (auto i = 0; i < this->event_arguments.size(); i++) {
                                if (ev_params.size() == 0) {
                                        break;
                                }
                                this->event_arguments[0] = (uint8_t) std::stoi(ev_params.front());
                                ev_params.pop_front();
                                this->uses_args = true;
                        }
                }
        }


        void filter_color_chaser::setup_filter(
                const std::map<std::string, std::string>& configuration,
                const std::map<std::string, std::string>& initial_parameters,
                const channel_mapping& input_channels,
                const std::string& own_id) {
                constexpr filter_type own_type = filter_type::filter_color_chaser;

                if (!this->uses_steps) {
                        if (!input_channels.float_channels.contains("time")) {
                                throw filter_config_exception("This filter has not been configured to operate in event mode but the time input is missing.", own_type, own_id);
                        }
                        if (!input_channels.float_channels.contains("time_scale")) {
                                throw filter_config_exception("This filter has not been configured to operate in event mode but the time_scale input is missing.", own_type, own_id);
                        }
                        this->time_input = input_channels.float_channels.at("time");
                        this->timescale_input = input_channels.float_channels.at("time_scale");
                }

                if (configuration.contains("color_parameters")) [[likely]] {
                        const auto entries = utils::split(configuration.at("color_parameters"), ':');
                        for(const auto& entry : entries) {
                                if (!input_channels.color_channels.contains(entry)) {
                                        throw filter_config_exception("Expected color input channel named '" + entry + ".", own_type, own_id);
                                }
                                this->color_parameter_inputs[entry] = input_channels.color_channels.at(entry);
                        }
                } else {
                        throw filter_config_exception("Filter configuration must contain 'color_parameters'.", own_type, own_id);
                }
                if (configuration.contains("number_parameters")) [[likely]] {
                        const auto entries = utils::split(configuration.at("number_parameters"), ':');
                        for(const auto& entry : entries) {
                                if (!input_channels.sixteen_bit_channels.contains(entry)) {
                                        throw filter_config_exception("Expected 16bit input channel named '" + entry + ".", own_type, own_id);
                                }
                                this->number_parameter_inputs[entry] = input_channels.sixteen_bit_channels.at(entry);
                        }
                } else {
                        throw filter_config_exception("Filter configuration must contain 'number_parameters'.", own_type, own_id);
                }

		for(const auto& [k, v] : initial_parameters) {
			this->receive_update_from_gui(k, v);
		}
        }

        void filter_color_chaser::get_output_channels(channel_mapping& map, const std::string& name) {
                for (auto i = 0; i < this->pixels.size(); i++) {
                        map.color_channels[name + ":" + std::to_string(i)] = &(this->pixels[i]);
                }
        }

        void filter_color_chaser::update() {
                if (this->setup == nullptr) [[unlikely]] {
                        return;
                }
                if (this->uses_steps) {
                        for (const auto& event : get_event_storage_instance()->get_storage()) {
                                if (event.get_event_sender() == this->event_sender && (!this->uses_args || (this->uses_args && this->event_arguments == event.get_args()))) {
                                this->setup->step();
                                }
                        }
                }
                for(auto& mask_entry : this->mask) {
                        mask_entry = 65535; // init to 100%
                }
                for(auto& pixel : this->pixels) {
                        pixel = dmxfish::dmx::pixel(0.0, 0.0, 1.0); // init to white
                }
                if (this->setup != nullptr) [[likely]] {
                        this->setup->execute(*this);
                } else {
                        ::spdlog::error("Color chaser '{}' does not have an active configuration!", this->own_id);
                }
        }

        void filter_color_chaser::scene_activated() {
                if (this->setup == nullptr) [[unlikely]] {
                        return;
                }
                this->setup->reset(*this);
        }

        bool filter_color_chaser::receive_update_from_gui(const std::string& key, const std::string& _value) {
                if (key != "config") {
                        return false;
                }
                try {
                        this->setup = std::make_unique<chaser_setup>(_value, *this);
                } catch (const filter_config_exception& e) {
                        ::spdlog::error("Failed to construct chaser configuration in filter '{}': {}", this->own_id, e.what());
                        return false;
                }
                return true;
        }

}
