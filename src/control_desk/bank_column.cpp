#include "control_desk/bank_column.hpp"

#include "control_desk/xtouch_driver.hpp"

#include "lib/logging.hpp"

namespace dmxfish::control_desk {

	bank_column::bank_column(std::weak_ptr<device_handle> device_connection, bank_mode mode, std::string _id, uint8_t column_index) :
		connection(device_connection), id(_id), display_text_up{}, display_text_down{}, color{}, readymode_color{}, raw_configuration{}, readymode_raw_configuration{}, current_bank_mode(mode), fader_index(column_index + XTOUCH_FADER_INDEX_OFFSET) {}

	void bank_column::set_active(bool new_value) {
		this->active_on_device = new_value;
		if(new_value) {
			// TODO implement activation
			// TODO set encoder in relative mode
			// absolute mode for amber and uv will be emulated
		} else {
			// TODO reset faders to 0
            // TODO set side leds to 0
			// TODO disable rotary encoder leds
            // TODO clear lcd displays
		}
	}

	void bank_column::reset_column() {
		// TODO implement
	}

	void bank_column::process_fader_change_message(unsigned int position_request) {
		if(position_request > 65535) {
			position_request = 65535;
		}
		raw_configuration.fader_position = (uint16_t) position_request;
		if(current_bank_mode != bank_mode::DIRECT_INPUT_MODE) {
			this->color.iluminance = raw_configuration.fader_position / 65535.0;
		}
		update_physical_fader_position();
	}

	void bank_column::process_encoder_change_message(int change_request) {
		raw_configuration.rotary_position += change_request;
		if(current_bank_mode != bank_mode::DIRECT_INPUT_MODE) {
			switch(current_re_assignment) {
				case rotary_encoder_assignment::HUE:
					this->color.hue += (1.0/128.0) * (double) change_request;
					if(this->color.hue > 1.0) {
						this->color.hue = 0.0 + (this->color.hue - 1.0);
					} else if(this->color.hue < 0.0) {
						this->color.hue = 1.0 + this->color.hue;
					}
					break;
				case rotary_encoder_assignment::SATURATION:
					this->color.saturation += (1.0/128.0) * (double) change_request;
					if(this->color.saturation > 1) {
						this->color.saturation = 1.0;
					} else if(this->color.saturation < 0) {
						this->color.saturation = 0.0;
					}
					break;
				case rotary_encoder_assignment::AMBER:
					if(this->amber + change_request > 255) {
						this->amber = 255;
					} else if(this->amber + change_request < 0) {
						this->amber = 0;
					} else {
						this->amber += change_request;
					}
					break;
				case rotary_encoder_assignment::UV:
					if(this->uv + change_request > 255) {
						this->uv = 255;
					} else if(this->uv + change_request < 0) {
						this->uv = 0;
					} else {
						this->uv += change_request;
					}
					break;
				default:
					::spdlog::error("Unsupported rotary encoder mode: {}.", (uint8_t) current_re_assignment);
					break;
			}
		}
		update_display_text();
		update_encoder_leds();
	}

	void bank_column::process_button_press_message(button b, button_change c) {
		switch(c) {
			case button_change::PRESS:
				// TODO implement
				::spdlog::error("Handling PRESS of button {} not yet implemented in column handler.", (uint8_t) b);
				break;
			case button_change::RELEASE:
				// TODO implement
				::spdlog::error("Handling RELEASE of button {} not yet implemented in column handler.", (uint8_t) b);
				break;
			default:
				::spdlog::error("Unexpected button action state ({}) in column handler.", (uint8_t) c);
				break;
		}
	}

	void bank_column::update_display_text() {
		if(!active_on_device) {
			return;
		}
		// TODO set lower display line if not in direct input mode
	}

	void bank_column::update_physical_fader_position() {
		if(!active_on_device) {
			return;
		}
		if(readymode_active) {
			return;
		}
		if(connection.expired()) {
			return;
		}
		connection.lock()->send_command(midi_command{midi_status::CONTROL_CHANGE, 0, fader_index, (raw_configuration.fader_position * 128) / 65536});
	}

	void bank_column::update_encoder_leds() {
		if(!active_on_device) {
			return;
		}
		// TODO implement
	}

	void bank_column::update_button_leds() {
		if(!active_on_device) {
			return;
		}
		// TODO implement
	}

}
