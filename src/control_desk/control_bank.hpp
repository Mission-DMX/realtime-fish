#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "control_desk/device_handle.hpp"
#include "dmx/pixel.hpp"

namespace dmxfish::control_desk {

    // TODO implement image and text with font support once we control X-Touch firmware
    
    enum class bank_mode : unsigned int {
        HSI_COLOR_MODE = 0,
	DIRECT_INPUT_MODE = 1,
    };

    enum class button_led_state : uint8_t {
        off = 0,
	flash = 64,
	on = 127,
    };

    enum class lcd_color : uint8_t {
        black = 0,
		red = 1,
		green = 2,
		yellow = 3,
		blue = 4,
		magenta = 5,
		cyan = 6,
		white = 7,
		black_up_inverted = 0 & (0b00010000),
		red_up_inverted = 1 & (0b00010000),
		green_up_inverted = 2 & (0b00010000),
		yellow_up_inverted = 3 & (0b00010000),
		blue_up_inverted = 4 & (0b00010000),
		magenta_up_inverted = 5 & (0b00010000),
		cyan_up_inverted = 6 & (0b00010000),
		white_up_inverted = 7 & (0b00010000),
		black_down_inverted = 0 & (0b00100000),
		red_down_inverted = 1 & (0b00100000),
		green_down_inverted = 2 & (0b00100000),
		yellow_down_inverted = 3 & (0b00100000),
		blue_down_inverted = 4 & (0b00100000),
		magenta_down_inverted = 5 & (0b00100000),
		cyan_down_inverted = 6 & (0b00100000),
		white_down_inverted = 7 & (0b00100000),
		black_both_inverted = 0 & (0b00110000),
		red_both_inverted = 1 & (0b00110000),
		green_both_inverted = 2 & (0b00110000),
		yellow_both_inverted = 3 & (0b00110000),
		blue_both_inverted = 4 & (0b00110000),
		magenta_both_inverted = 5 & (0b00110000),
		cyan_both_inverted = 6 & (0b00110000),
		white_both_inverted = 7 & (0b00110000),
    };

    struct raw_column_configuration {
        uint16_t fader_position = 0;
	uint16_t rotary_position = 0;
	uint8_t meter_leds = 0;
	button_led_state select_led = button_led_state::off;
	button_led_state button_1 = button_led_state::off;
	button_led_state button_2 = button_led_state::off;
	button_led_state button_3 = button_led_state::off;
	lcd_color display_color = lcd_color::blue;
    };

    class bank_column {
    private:
        std::shared_ptr<device_handle> connection;
		std::string id;
	std::vector<std::string> display_text_up;
	std::vector<std::string> display_text_down;
	std::optional<dmxfish::dmx::pixel> color;
	std::optional<raw_column_configuration> raw_configuration;
	bool active_on_device = false;
	unsigned int display_scroll_position_up = 0;
	unsigned int display_scroll_position_down = 0;
	const bank_mode current_bank_mode = bank_mode::HSI_COLOR_MODE;
    public:
        bank_column(std::shared_ptr<device_handle> device_connection, bank_mode mode, std::string id);
		void set_active(bool new_value);
		inline bank_mode get_mode() const {
				return this->current_bank_mode;
		}

		inline std::optional<dmxfish::dmx::pixel> get_color() const {
				return this->color;
		}

		inline void set_color(const dmxfish::dmx::pixel& p) {
				if(this->current_bank_mode != bank_mode::HSI_COLOR_MODE) {
					return;
			}
			this->color = p;
			// TODO set fader position, 2nd row display text and rotary encoder positions
		}

		inline std::optional<raw_column_configuration> get_raw_configuration() const {
				return raw_configuration;
		}
    };

}
