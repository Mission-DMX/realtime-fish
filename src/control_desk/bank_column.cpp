#include "control_desk/bank_column.hpp"

#include "control_desk/desk.hpp"
#include "control_desk/xtouch_driver.hpp"

#include "lib/logging.hpp"

namespace dmxfish::control_desk {

	bank_column::bank_column(std::weak_ptr<device_handle> device_connection, std::function<void(std::string const&, bool)> _desk_ready_update, bank_mode mode, std::string _id, uint8_t column_index) :
		connection(device_connection), desk_ready_update(_desk_ready_update), id(_id), display_text_up{}, display_text_down{}, color{}, readymode_color{}, raw_configuration{},
		readymode_raw_configuration{}, current_bank_mode(mode), fader_index(column_index + XTOUCH_FADER_INDEX_OFFSET) {}

	void bank_column::set_active(bool new_value) {
		this->active_on_device = new_value;
		if(new_value) {
			// TODO implement activation
			// TODO set encoder in relative mode
			// absolute mode for amber and uv will be emulated
			update_display_text();
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
		raw_configuration.rotary_position += (int16_t) change_request;
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
						this->amber += (uint8_t) change_request;
					}
					break;
				case rotary_encoder_assignment::UV:
					if(this->uv + change_request > 255) {
						this->uv = 255;
					} else if(this->uv + change_request < 0) {
						this->uv = 0;
					} else {
						this->uv += (uint8_t) change_request;
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
		const auto b_base = button{((uint8_t) b) / XTOUCH_COLUMN_COUNT};
		switch(c) {
			case button_change::PRESS:
				if(b_base == button::BTN_CH1_ENCODER_ROTARYMODE) {
					if(current_bank_mode == bank_mode::DIRECT_INPUT_MODE) {
						return;
					}
					switch(current_re_assignment) {
						case rotary_encoder_assignment::HUE:
							current_re_assignment = rotary_encoder_assignment::SATURATION;
							break;
						case rotary_encoder_assignment::SATURATION:
							current_re_assignment = (current_bank_mode == bank_mode::HSI_WITH_AMBER_MODE || current_bank_mode == bank_mode::HSI_WITH_AMBER_AND_UV_MODE) ? rotary_encoder_assignment::AMBER : (current_bank_mode == bank_mode::HSI_WITH_UV_MODE) ? rotary_encoder_assignment::UV : rotary_encoder_assignment::HUE;
							break;
						case rotary_encoder_assignment::AMBER:
							current_re_assignment = current_bank_mode == bank_mode::HSI_WITH_AMBER_AND_UV_MODE ? rotary_encoder_assignment::UV : rotary_encoder_assignment::HUE;
							break;
						case rotary_encoder_assignment::UV:
							current_re_assignment = rotary_encoder_assignment::HUE;
							break;
					}
					update_display_text();
				} else if(b_base == button::BTN_CH1_REC_READY) {
					this->readymode_active = !this->readymode_active;
					if(this->readymode_active) {
						this->readymode_color = this->color;
						this->readymode_raw_configuration = this->raw_configuration;
						this->readymode_amber = this->amber;
						this->readymode_uv = this->uv;
						update_button_leds();
						notify_bank_about_ready_mode();
					} else {
						update_button_leds();
						update_physical_fader_position();
						notify_bank_about_ready_mode();
					}
				} else if(b_base == button::BTN_CH1_SOLO_FIND) {
					// TODO implement
				} else if(b_base == button::BTN_CH1_MUTE_BLACK) {
					this->black_active = !this->black_active;
				} else if(b_base == button::BTN_CH1_SELECT_SELECT) {
					// TODO implement
				} else {
					::spdlog::error("Handling PRESS of button {} not yet implemented in column handler.", (uint8_t) b);
				}
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
		if(connection.expired()) {
			return;
		}
		std::array<char, 14> content;
		auto text_index = 0;
		const auto& up_text = display_text_up[text_index];
		if(text_index < display_text_up.size()) {
			for(auto i = 0; i < 7; i++){
				const auto tpos = i + display_scroll_position_up;
				if(tpos >= up_text.length()) {
					content[i] = up_text.at(tpos);
				} else {
					content[i] = ' ';
				}
			}
		} else {
			std::array<char, 7> no_data = {'N', 'o', ' ', 'D', 'a', 't', 'a'};
			for(auto i = 0; i < 7; i++){
				content[i] = no_data[i];
			}
		}
		// TODO set lower display line if not in direct input mode
		for(auto i = 7; i < 14; i++) {
			content[i] = '-';
		}
		xtouch_set_lcd_display(*(connection.lock()), fader_index + XTOUCH_DISPLAY_INDEX_OFFFSET, lcd_color::green, content);
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
		// TODO replace with driver method xtouch_set_fader_position
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
		if(connection.expired()) {
			return;
		}
		auto dev_ptr = connection.lock();
		auto offset = fader_index * XTOUCH_COLUMN_COUNT;
		// TODO implement select
		// TODO implement find
		xtouch_set_button_led(*dev_ptr, button{offset + (uint8_t) button::BTN_CH1_MUTE_BLACK}, black_active ? button_led_state::flash : button_led_state::off);
		xtouch_set_button_led(*dev_ptr,
							  button{offset + (uint8_t) button::BTN_CH1_REC_READY},
							  this->readymode_active ? button_led_state::on : button_led_state::off);
	}

	void bank_column::commit_from_readymode() {
		this->color = this->readymode_color;
		this->raw_configuration = this->readymode_raw_configuration;
		this->readymode_active = false;
		this->amber = this->readymode_amber;
		this->uv = this->readymode_uv;
		update_button_leds();
		notify_bank_about_ready_mode();
		// TODO notify GUI about new values
	}

	void bank_column::notify_bank_about_ready_mode() {
		this->desk_ready_update(this->get_id(), this->readymode_active);
	}

}
