#include "control_desk/bank_column.hpp"

#include <chrono>
#include <sstream>
#include <string>

#include "control_desk/desk.hpp"
#include "control_desk/xtouch_driver.hpp"

COMPILER_SUPRESS("-Wuseless-cast")
#include "proto_src/MessageTypes.pb.h"
#include "proto_src/Console.pb.h"
COMPILER_RESTORE("-Wuseless-cast")

#include "lib/logging.hpp"
#include "main.hpp"

namespace dmxfish::control_desk {

	bank_column::bank_column(
            std::weak_ptr<device_handle> device_connection,
            std::function<void(std::string const&, bool)> _desk_ready_update,
            std::function<void(std::string const&, bool)> _select_state_handler,
            bank_mode mode, std::string _id, uint8_t column_index
            ) :
		connection(device_connection),
        desk_ready_update(_desk_ready_update),
        select_state_handler(_select_state_handler),
        id(_id), display_text_up{}, display_text_down{},
        color{}, readymode_color{}, raw_configuration{},
		readymode_raw_configuration{},
        current_bank_mode(mode),
        fader_index(column_index) {
			display_text_up.emplace_back("");
			display_text_down.emplace_back("");
		}

	void bank_column::set_active(bool new_value) {
		this->active_on_device = new_value;
		if(new_value) {
			// absolute mode for amber and uv will be emulated
			update_display_text();
			update_physical_fader_position(true);
			update_encoder_leds();
			update_button_leds();
			update_side_leds();
		}
	}

	void bank_column::reset_column() {
		if(connection.expired()) {
			return;
		}
		auto d_ptr = connection.lock();
		const std::array<char, 14> empty_lcd_data{' '};
		xtouch_set_lcd_display(*d_ptr, this->fader_index, lcd_color::black, empty_lcd_data);
		xtouch_set_fader_position(*d_ptr, fader{(uint8_t) fader::FADER_CH1 + this->fader_index}, 0);
		xtouch_set_button_led(*d_ptr, button{(uint8_t) button::BTN_CH1_REC_READY + this->fader_index}, button_led_state::off);
		xtouch_set_button_led(*d_ptr, button{(uint8_t) button::BTN_CH1_SOLO_FLASH + this->fader_index}, button_led_state::off);
		xtouch_set_button_led(*d_ptr, button{(uint8_t) button::BTN_CH1_MUTE_BLACK + this->fader_index}, button_led_state::off);
		xtouch_set_button_led(*d_ptr, button{(uint8_t) button::BTN_CH1_SELECT_SELECT + this->fader_index}, button_led_state::off);
		xtouch_set_ring_led(*d_ptr, encoder{(uint8_t) encoder::ENC_CH1 + this->fader_index}, 128);
		xtouch_set_meter_leds(*d_ptr, led_bar{(uint8_t) led_bar::BAR_CH1 + this->fader_index}, 0);
	}

	void bank_column::update() {
		namespace sc = std::chrono;
		const auto now = sc::duration_cast<sc::milliseconds>(sc::system_clock::now().time_since_epoch()).count();
		bool display_update_required = false;

        if(last_encoder_turn + 750 < last_update_timestamp) {
            accumulated_change = 0;
        }

		if(last_update_timestamp != 0 && now >= display_hold_until) {
			display_update_required = true;
		}
		if(last_display_scroll + 1000 < last_update_timestamp) {
			if(display_text_index_up < display_text_up.size()) {
				const auto text_length = display_text_up[display_text_index_up].length();
				if(text_length > 7 && display_scroll_position_up < text_length - 7) {
					display_scroll_position_up++;
				} else {
					display_scroll_position_up = 0;
					if (++display_text_index_up == display_text_up.size()) {
						display_text_index_up = 0;
					}
				}
			} else {
				display_text_index_up = 0;
			}
			if(display_text_index_down < display_text_down.size()) {
				const auto text_length = display_text_down[display_text_index_down].length();
				if(text_length > 7 && display_scroll_position_down < text_length - 7) {
					display_scroll_position_down++;
				} else {
					display_scroll_position_down = 0;
					if (++display_text_index_down == display_text_down.size()) {
						display_text_index_down = 0;
					}
				}
			} else {
				display_text_index_down = 0;
			}
			last_display_scroll = now;
			display_update_required = true;
		}
		this->last_update_timestamp = now;
		if(display_update_required) {
			update_display_text();
		}
	}

	void bank_column::process_fader_change_message(unsigned int position_request) {
		if(position_request > 65535) {
			position_request = 65535;
		}
		if(readymode_active) {
            if (raw_working_on_primary) {
                readymode_raw_configuration.primary_position = (uint16_t) position_request;
            } else {
                readymode_raw_configuration.secondary_position = (uint16_t) position_request;
            }
		    if(current_bank_mode != bank_mode::DIRECT_INPUT_MODE) {
		        this->readymode_color.setIluminance((uint16_t) position_request / 65535.0);
		    }
		} else {
            if (raw_working_on_primary) {
                raw_configuration.primary_position = (uint16_t) position_request;
            } else {
                raw_configuration.secondary_position = (uint16_t) position_request;
            }
		    if(current_bank_mode != bank_mode::DIRECT_INPUT_MODE) {
		        this->color.setIluminance(((uint16_t) position_request) / 65535.0);
		    }
		}
		update_physical_fader_position();
        if (current_bank_mode == bank_mode::DIRECT_INPUT_MODE) {
            update_display_text();
        }
		send_col_update_to_fish();
	}

	void bank_column::process_encoder_change_message(int change_request) {
        if (this->accumulated_change < 1024) {
            this->accumulated_change++;
        }
        this->last_encoder_turn = this->last_update_timestamp;
        change_request *= this->accumulated_change;
        if (readymode_active) {
            if (raw_working_on_primary) {
                readymode_raw_configuration.primary_position += (uint16_t) change_request;
            } else {
                readymode_raw_configuration.secondary_position += (uint16_t) change_request;
            }
        } else {
            if (raw_working_on_primary) {
                raw_configuration.primary_position += (uint16_t) change_request;
            } else {
                raw_configuration.secondary_position += (uint16_t) change_request;
            }
        }
		auto& selected_color = readymode_active ? readymode_color : color;
		auto& selected_amber = readymode_active ? readymode_amber : amber;
		auto& selected_uv = readymode_active ? readymode_uv : uv;
		if(current_bank_mode != bank_mode::DIRECT_INPUT_MODE) {
			switch(current_re_assignment) {
				case rotary_encoder_assignment::HUE:
					selected_color.setHue(selected_color.getHue() + 15 * (double) change_request);
                    if(selected_color.getHue() > 360.0) {
                        selected_color.setHue(0.0 + (selected_color.getHue() - 360.0));
                    } else if(selected_color.getHue() < 0.0) {
                        selected_color.setHue(360.0 + selected_color.getHue());
                    }
					break;
				case rotary_encoder_assignment::SATURATION:
					selected_color.setSaturation(selected_color.getSaturation() + (1.0/128.0) * (double) change_request);
					if(selected_color.getSaturation() > 1) {
						selected_color.setSaturation(1.0);
					} else if(selected_color.getSaturation() < 0) {
						selected_color.setSaturation(0.0);
					}
					break;
				case rotary_encoder_assignment::AMBER:
					if(selected_amber + change_request > 255) {
						selected_amber = 255;
					} else if(selected_amber + change_request < 0) {
						selected_amber = 0;
					} else {
						selected_amber += (uint8_t) change_request;
					}
					break;
				case rotary_encoder_assignment::UV:
					if(selected_uv + change_request > 255) {
						selected_uv = 255;
					} else if(selected_uv + change_request < 0) {
						selected_uv = 0;
					} else {
						selected_uv += (uint8_t) change_request;
					}
					break;
				default:
					::spdlog::error("Unsupported rotary encoder mode: {}.", (uint8_t) current_re_assignment);
					break;
			}
		}
		display_hold_until = last_update_timestamp + 2500;
		update_display_text();
		update_encoder_leds();
		update_side_leds();
        if(current_bank_mode == bank_mode::DIRECT_INPUT_MODE) {
            this->update_physical_fader_position();
        }
		send_col_update_to_fish();
	}

	void bank_column::process_button_press_message(button b, button_change c) {
		const auto b_base = button{((uint8_t) (((uint8_t) b) / XTOUCH_COLUMN_COUNT)) * XTOUCH_COLUMN_COUNT};
		//::spdlog::debug("Column {} received button base {}.", this->fader_index, (uint8_t) b_base);
		switch(c) {
			case button_change::PRESS:
				if(b_base == button::BTN_CH1_ENCODER_ROTARYMODE) {
					if(current_bank_mode == bank_mode::DIRECT_INPUT_MODE) {
						this->raw_working_on_primary = !this->raw_working_on_primary;
						update_display_text();
						update_physical_fader_position();
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
						update_display_text();
						update_encoder_leds();
						update_side_leds();
					} else {
						update_button_leds();
						update_physical_fader_position();
						notify_bank_about_ready_mode();
						update_display_text();
						update_encoder_leds();
						update_side_leds();
					}
				} else if(b_base == button::BTN_CH1_SOLO_FLASH) {
					this->flash_active = true;
					update_button_leds();
				} else if(b_base == button::BTN_CH1_MUTE_BLACK) {
					this->black_active = !this->black_active;
					update_button_leds();
				} else if(b_base == button::BTN_CH1_SELECT_SELECT) {
					this->select_state_handler(this->id, !this->select_active);
				} else {
					::spdlog::error("Handling PRESS of button {} not yet implemented in column handler.", (uint8_t) b);
				}
				break;
			case button_change::RELEASE:
				if (b_base == button::BTN_CH1_SOLO_FLASH) {
					this->flash_active = false;
					update_button_leds();
				}
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
		{
			if(display_text_index_up < display_text_up.size()) {
				const auto& up_text = display_text_up[display_text_index_up];
				for(auto i = 0, last_printable_char = 7; i < last_printable_char; i++){
					const auto tpos = i + display_scroll_position_up;
					if(tpos < up_text.length()) {
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
		}
		if(display_hold_until > last_update_timestamp) {
			std::string value;
			if (current_bank_mode == bank_mode::DIRECT_INPUT_MODE) {
				value = std::to_string(
					readymode_active ? (raw_working_on_primary ? readymode_raw_configuration.primary_position : readymode_raw_configuration.secondary_position) :
                    (raw_working_on_primary ? raw_configuration.primary_position : raw_configuration.secondary_position)
				);	
			} else {
				switch(this->current_re_assignment) {
				case rotary_encoder_assignment::HUE: {
					const auto hue = readymode_active ? readymode_color.getHue() : color.getHue();
					if(hue < 30.0) {
						value = "red";
					} else if (hue < 50.0) {
						value = "orange";
					} else if (hue < 70.0) {
						value = "yellow";
					} else if (hue < 150.0) {
						value = "green";
					} else if (hue < 185.0) {
						value = "cyan";
					} else if (hue < 285.0) {
						value = "blue";
					} else if (hue < 330.0) {
						value = "purple";
					} else {
						value = "red";
					}
					} break;
				case rotary_encoder_assignment::SATURATION:
					value = std::to_string(readymode_active ? readymode_color.getSaturation() : color.getSaturation());
					break;
				case rotary_encoder_assignment::AMBER:
					value = std::to_string(readymode_active ? readymode_amber : amber);
					break;
				case rotary_encoder_assignment::UV:
					value = std::to_string(readymode_active ? readymode_uv : uv);
					break;
				default:
					value = "?";
					break;
				}
			}
			for (auto i = 0; i < 7; i++) {
				content[7 + i] = i < value.length() ? value.at(i) : ' ';
			}
		} else {
			switch(current_bank_mode) {
			case bank_mode::HSI_COLOR_MODE:
			case bank_mode::HSI_WITH_AMBER_MODE:
			case bank_mode::HSI_WITH_UV_MODE:
			case bank_mode::HSI_WITH_AMBER_AND_UV_MODE:
				switch(this->current_re_assignment) {
					case rotary_encoder_assignment::HUE:
						content[7] = 'H';
						content[8] = 'u';
						content[9] = 'e';
						content[10] = ' ';
						content[11] = ' ';
						content[12] = ' ';
						content[13] = ' ';
						break;
					case rotary_encoder_assignment::SATURATION:
						content[7] = 'S';
						content[8] = 'a';
						content[9] = 't';
						content[10] = 'u';
						content[11] = 'r';
						content[12] = 'e';
						content[13] = ' ';
						break;
					case rotary_encoder_assignment::AMBER:
						content[7] = 'A';
						content[8] = 'm';
						content[9] = 'b';
						content[10] = 'e';
						content[11] = 'r';
						content[12] = ' ';
						content[13] = ' ';
						break;
					case rotary_encoder_assignment::UV:
						content[7] = 'U';
						content[8] = 'V';
						content[9] = ' ';
						content[10] = ' ';
						content[11] = ' ';
						content[12] = ' ';
						content[13] = ' ';
						break;
					default:
						content[7] = 'U';
						content[8] = 'n';
						content[9] = 'k';
						content[10] = 'n';
						content[11] = 'o';
						content[12] = 'w';
						content[13] = 'n';
						break;
				};
				break;
			case bank_mode::DIRECT_INPUT_MODE:
				if(display_text_index_down < display_text_down.size()) {
					if(display_text_down[display_text_index_down].length() > 0) {
						const auto& down_text = display_text_down[display_text_index_down];
						for(auto i = 7; i < 14; i++) {
							const auto tpos = i + display_scroll_position_down;
							if(tpos < down_text.length()) {
								content[i] = down_text.at(tpos);
							} else {
								content[i] = ' ';
							}
						}
						break;
					} else {
						const auto text = this->raw_working_on_primary ? "primary" : "second.";
						for(auto i = 7; i < 14; i++) {
							content[i] = text[i - 7];
						}
						break;
					}
				}
				[[fallthrough]];
			default:
				for(auto i = 7; i < 14; i++) {
					content[i] = '-';
				}
			}
		}
		if(this->readymode_active) {
			content[13] = '*';
		}
		xtouch_set_lcd_display(*(connection.lock()), fader_index + XTOUCH_DISPLAY_INDEX_OFFFSET, display_color, content);
	}

	void bank_column::update_physical_fader_position(bool from_activate) {
		if(!active_on_device) {
			return;
		}
		if(readymode_active && !from_activate) {
			return;
		}
		if(connection.expired()) {
			return;
		}
        const auto raw_value = raw_working_on_primary ? (readymode_active ? readymode_raw_configuration.primary_position : raw_configuration.primary_position)
                : (readymode_active ? readymode_raw_configuration.secondary_position : raw_configuration.secondary_position);
		xtouch_set_fader_position(*connection.lock(), fader{fader_index + XTOUCH_FADER_INDEX_OFFSET}, (raw_value * 128) / 65536);
	}

	void bank_column::update_encoder_leds() {
		if(!active_on_device) {
			return;
		}
		if(connection.expired()) {
			return;
		}
		const encoder e{fader_index + XTOUCH_ENCODER_INDEX_OFFSET};
		if(current_bank_mode == bank_mode::DIRECT_INPUT_MODE) {
            if(readymode_active) {
                if (raw_working_on_primary) {
                    xtouch_set_ring_led(*connection.lock(), e, (raw_configuration.secondary_position * 128) / 65536);
                } else {
                    xtouch_set_ring_led(*connection.lock(), e, (raw_configuration.primary_position * 128) / 65536);
                }
            } else {
                if (raw_working_on_primary) {
                    xtouch_set_ring_led(*connection.lock(), e, (readymode_raw_configuration.secondary_position * 128) / 65536);
                } else {
                    xtouch_set_ring_led(*connection.lock(), e, (readymode_raw_configuration.secondary_position * 128) / 65536);
                }
            }
		} else {
			if(readymode_active)
			    xtouch_set_ring_led(*connection.lock(), e, (uint8_t) ((this->readymode_color.getHue() / 360.0) * 128));
			else
			    xtouch_set_ring_led(*connection.lock(), e, (uint8_t) ((this->color.getHue() / 360.0) * 128));
		}
	}

	void bank_column::update_button_leds() {
		if(!active_on_device) {
			return;
		}
		if(connection.expired()) {
			return;
		}
		auto dev_ptr = connection.lock();
		const auto offset = fader_index;
		xtouch_set_button_led(*dev_ptr, button{offset + (uint8_t) button::BTN_CH1_REC_READY}, readymode_active ? button_led_state::on : button_led_state::off);
		xtouch_set_button_led(*dev_ptr, button{offset + (uint8_t) button::BTN_CH1_SOLO_FLASH}, flash_active ? button_led_state::on : button_led_state::off);
		xtouch_set_button_led(*dev_ptr, button{offset + (uint8_t) button::BTN_CH1_MUTE_BLACK}, black_active ? button_led_state::flash : button_led_state::off);
		xtouch_set_button_led(*dev_ptr, button{offset + (uint8_t) button::BTN_CH1_SELECT_SELECT}, select_active ? button_led_state::on : button_led_state::off);
	}

	void bank_column::update_side_leds() {
		if(!active_on_device) {
			return;
		}
		if(connection.expired()) {
			return;
		}
		if(current_bank_mode == bank_mode::DIRECT_INPUT_MODE) {
			return;
		}
		xtouch_set_meter_leds(*connection.lock(), led_bar{(uint8_t) led_bar::BAR_CH1 + fader_index}, (uint8_t) ((readymode_active ? readymode_color.getSaturation() : color.getSaturation()) * 126 + 1));
	}

	void bank_column::commit_from_readymode() {
		this->color = this->readymode_color;
		this->raw_configuration = this->readymode_raw_configuration;
		this->readymode_active = false;
		this->amber = this->readymode_amber;
		this->uv = this->readymode_uv;
		update_button_leds();
		update_display_text();
		update_side_leds();
		send_col_update_to_fish();
	}

	void bank_column::notify_bank_about_ready_mode() {
		this->desk_ready_update(this->get_id(), this->readymode_active);
		// TODO notify GUI about new values
	}

	void bank_column::set_display_text(const std::string& text, bool up) {
		auto& selected_text_vector = up ? this->display_text_up : this->display_text_down;
		selected_text_vector.clear();
		std::stringstream ss;
		size_t length = 0;
		for(const char c : text) {
			if (c == '\n' || c == '\r') {
				if (length > 0) {
					selected_text_vector.emplace_back(ss.str());
					ss.clear();
					length = 0;
				}
			} else if(c == '\t') {
				ss << ' ';
				length++;
			} else {
				ss << c;
				length++;
			}
		}
		selected_text_vector.emplace_back(ss.str());
		if (up) {
			display_scroll_position_up = 0;
			display_text_index_up = selected_text_vector.size() - 1;
		} else {
			display_scroll_position_down = 0;
			display_text_index_down = selected_text_vector.size() - 1;
		}
		update_display_text();
	}

	std::string bank_column::get_display_text(bool up) {
		auto& tv = up ? this->display_text_up : this->display_text_down;
		if (tv.size() > 0) {
			return tv[0];
		} else {
			return "";
		}
	}

	inline void color_to_message(dmxfish::dmx::pixel& c, missiondmx::fish::ipcmessages::fader_column_hsi_color& cm) {
		cm.set_hue(c.getHue());
		cm.set_saturation(c.getSaturation());
		cm.set_intensity(c.getIluminance());
	}

	void bank_column::send_col_update_to_fish() {
		missiondmx::fish::ipcmessages::fader_column msg;
		msg.set_column_id(this->id);
		switch(this->current_bank_mode) {
			case bank_mode::HSI_COLOR_MODE: {
				color_to_message(this->readymode_active ? this->readymode_color : this->color, *msg.mutable_plain_color());
				break;
			} default:
			case bank_mode::DIRECT_INPUT_MODE:
			case bank_mode::HSI_WITH_AMBER_MODE: {
				auto rd = msg.mutable_raw_data();
				const raw_column_configuration& data = this->readymode_active ? this->readymode_raw_configuration : raw_configuration;
				rd->set_fader(data.primary_position);
				rd->set_rotary_position(data.secondary_position);
				rd->set_meter_leds(data.meter_leds);
				// TODO copy button states
				break;
			} case bank_mode::HSI_WITH_UV_MODE:
			case bank_mode::HSI_WITH_AMBER_AND_UV_MODE: {
				auto cu = msg.mutable_color_with_uv();
				cu->set_uv(this->readymode_active ? this->readymode_uv : this->uv);
				color_to_message(this->readymode_active ? this->readymode_color : this->color, *cu->mutable_base());
				break;
			}
		}
		get_iomanager_instance()->push_msg_to_all_gui(msg, ::missiondmx::fish::ipcmessages::MSGT_UPDATE_COLUMN);
	}
}
