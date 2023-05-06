#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "control_desk/device_handle.hpp"
#include "control_desk/xtouch_driver.hpp"
#include "dmx/pixel.hpp"

namespace dmxfish::control_desk {

    // TODO implement image and text with font support once we control X-Touch firmware
    
    enum class bank_mode : uint8_t {
        HSI_COLOR_MODE = 0,
        HSI_WITH_AMBER_MODE = 2,
        HSI_WITH_UV_MODE = 3,
        HSI_WITH_AMBER_AND_UV_MODE = 4,
        DIRECT_INPUT_MODE = 1,
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

    class bank;

    class bank_column {
    private:
        enum class rotary_encoder_assignment : uint8_t {
            HUE,
            SATURATION,
            AMBER,
            UV
        };
    private:
        const bank_mode current_bank_mode = bank_mode::HSI_COLOR_MODE;
        rotary_encoder_assignment current_re_assignment = rotary_encoder_assignment::HUE;
        lcd_color display_color = lcd_color::green;
        const uint8_t fader_index;
        uint8_t amber = 0;
        uint8_t readymode_amber = 0;
        uint8_t uv = 0;
        uint8_t readymode_uv = 0;
        bool active_on_device = false;
        bool select_active = false;
        bool readymode_active = false;
        bool black_active = false;
        dmxfish::dmx::pixel color;
        dmxfish::dmx::pixel readymode_color;

        raw_column_configuration raw_configuration;
        raw_column_configuration readymode_raw_configuration;

        const std::weak_ptr<device_handle> connection;
        const std::function<void(std::string const&, bool)> desk_ready_update;
        const std::string id;
        std::vector<std::string> display_text_up;
        std::vector<std::string> display_text_down;
        unsigned int display_scroll_position_up = 0;
        unsigned int display_scroll_position_down = 0;
        unsigned int display_text_index_up = 0;
        unsigned int display_text_index_down = 0;
        // TODO we should find a nice way to link what happens, when the select button was pressed (for example on may link an MH control (Joystick = Pan/Tilt, Arrows = Zoom/Focus))
        // TODO should we introduce a message that sends the column id for it?
    public:
        bank_column(std::weak_ptr<device_handle> device_connection, std::function<void(std::string const&, bool)> _desk_ready_update, bank_mode mode, std::string id, uint8_t column_index);

        /**
         * Notify the column that it is now / no longer displayed on the control desk. In case of disabling: it will not call reset_column(). This means that if the column
         * won't be utilized after deactivation one needs to manually call reset_column. Furthermore this method does not invoke schedule_transmission on the device handle.
         */
        void set_active(bool new_value);

        /**
         * Clear the content of the column on the control desk. This method does not invoke schedule_transmission on the device handle.
         */
        void reset_column();

        [[nodiscard]] inline bank_mode get_mode() const {
            return this->current_bank_mode;
        }

        [[nodiscard]] inline dmxfish::dmx::pixel get_color() const {
            if(black_active) {
                return dmxfish::dmx::pixel{this->color.hue, this->color.saturation, 0.0};
            } else {
                return this->color;
            }
        }

        inline void set_color(const dmxfish::dmx::pixel& p) {
            if(this->current_bank_mode != bank_mode::HSI_COLOR_MODE) {
                return;
            }
            this->color = p;
            update_physical_fader_position();
            update_encoder_leds();
            update_side_leds();
        }

        [[nodiscard]] inline raw_column_configuration get_raw_configuration() const {
                return raw_configuration;
        }

        inline void set_raw_configuration(raw_column_configuration c) {
            this->raw_configuration = c;
            update_physical_fader_position();
            update_encoder_leds();
        }

        inline void set_amber_value(uint8_t new_value) {
			this->amber = new_value;
		}

		[[nodiscard]] inline uint8_t get_amber_value() const {
            if(black_active) {
                return 0;
            } else {
                return this->amber;
            }
		}

		inline void set_uv_value(uint8_t new_value) {
			this->uv = new_value;
		}

        [[nodiscard]] inline uint8_t get_uv_value() const {
            if(black_active) {
                return 0;
            } else {
                return this->uv;
            }
        }

		[[nodiscard]] inline std::string get_id() const {
            return this->id;
        }

        [[nodiscard]] inline bool is_column_blacked_out() const {
            return this->black_active;
        }

        inline void set_display_color(lcd_color c) {
            display_color = c;
        }

		void process_fader_change_message(unsigned int position_request);
        void process_encoder_change_message(int change_request);
        void process_button_press_message(button b, button_change c);
        void commit_from_readymode();
        void set_display_text(const std::string& text, bool up);
    private:
        void update_display_text();
        void update_physical_fader_position();
        void update_encoder_leds();
        void update_button_leds();
        void update_side_leds();
        void notify_bank_about_ready_mode();
    };

}
