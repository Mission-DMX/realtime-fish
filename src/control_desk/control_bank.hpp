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
        black,
	red,
	green,
	yellow,
	blue,
	magenta,
	cyan,
	white,
	black_up_inverted,
	red_up_inverted,
	green_up_inverted,
	yellow_up_inverted,
	blue_up_inverted,
	magenta_up_inverted,
	cyan_up_inverted,
	white_up_inverted,
	black_down_inverted,
	red_down_inverted,
	green_down_inverted,
	yellow_down_inverted,
	blue_down_inverted,
	magenta_down_inverted,
	cyan_down_inverted,
	white_down_inverted,
	black_both_inverted,
	red_both_inverted,
	green_both_inverted,
	yellow_both_inverted,
	blue_both_inverted,
	magenta_both_inverted,
	cyan_both_inverted,
	white_both_inverted
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
	std::vector<std::string> display_text_up;
	std::vector<std::string> display_text_down;
	std::optional<dmxfish::dmx::pixel> color;
	std::optional<raw_column_configuration> raw_configuration;
	bool active_on_device = false;
	unsigned int display_scroll_position_up = 0;
	unsigned int display_scroll_position_down = 0;
	const bank_mode current_bank_mode = bank_mode::HSI_COLOR_MODE;
    public:
        bank_column(std::shared_ptr<device_handle> device_connection, bank_mode mode);
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
