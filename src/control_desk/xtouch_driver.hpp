#pragma once

#include <array>
#include <cassert>

#include "control_desk/device_handle.hpp"
#include "control_desk/enum_iterator.hpp"

#define VENDOR_BEHRINGER {0x00, 0x20, 0x32}

#define CMD_7SEG 0x37
#define CMD_LCD 0x4C

#define XTOUCH_COLUMN_COUNT 8

#define XTOUCH_FADER_INDEX_OFFSET 70
#define XTOUCH_ENCODER_INDEX_OFFSET 80
#define XTOUCH_DISPLAY_INDEX_OFFFSET 0

namespace dmxfish::control_desk {

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
        black_up_inverted = 0 | (0b00010000),
        red_up_inverted = 1 | (0b00010000),
        green_up_inverted = 2 | (0b00010000),
        yellow_up_inverted = 3 | (0b00010000),
        blue_up_inverted = 4 | (0b00010000),
        magenta_up_inverted = 5 | (0b00010000),
        cyan_up_inverted = 6 | (0b00010000),
        white_up_inverted = 7 | (0b00010000),
        black_down_inverted = 0 | (0b00100000),
        red_down_inverted = 1 | (0b00100000),
        green_down_inverted = 2 | (0b00100000),
        yellow_down_inverted = 3 | (0b00100000),
        blue_down_inverted = 4 | (0b00100000),
        magenta_down_inverted = 5 | (0b00100000),
        cyan_down_inverted = 6 | (0b00100000),
        white_down_inverted = 7 | (0b00100000),
        black_both_inverted = 0 | (0b00110000),
        red_both_inverted = 1 | (0b00110000),
        green_both_inverted = 2 | (0b00110000),
        yellow_both_inverted = 3 | (0b00110000),
        blue_both_inverted = 4 | (0b00110000),
        magenta_both_inverted = 5 | (0b00110000),
        cyan_both_inverted = 6 | (0b00110000),
        white_both_inverted = 7 | (0b00110000),
    };

    enum class button_change : uint8_t {
        PRESS = 127,
        RELEASE = 0,
    };

    /**
     * This enum defines the button IDs. The first part of the specific name defines the original meaning, the second one our labeling. For example BTN_CH1_MUTE_BLACK
     * defines the button on channel 1 that once was labeled MUTE but is now labeled BLACK.
     */
    enum class button : uint8_t { // TODO fill in correct values as soon as X-Touch arrived
        BTN_CH1_ENCODER_ROTARYMODE = 0, // Switches rotary encoder between Hue, Saturation and (if applicable) UV and (if applicable) Amber, Second row of LCD displays mode and setting
        BTN_CH2_ENCODER_ROTARYMODE = 1,
        BTN_CH3_ENCODER_ROTARYMODE = 2,
        BTN_CH4_ENCODER_ROTARYMODE = 3,
        BTN_CH5_ENCODER_ROTARYMODE = 4,
        BTN_CH6_ENCODER_ROTARYMODE = 5,
        BTN_CH7_ENCODER_ROTARYMODE = 6,
        BTN_CH8_ENCODER_ROTARYMODE = 7,
        BTN_CH1_REC_READY = 8, // If ready is enabled (blinking red), the entered changes of the channel are not applied until the COMMITRDY button is pressed.
        BTN_CH2_REC_READY = 9,
        BTN_CH3_REC_READY = 10,
        BTN_CH4_REC_READY = 11,
        BTN_CH5_REC_READY = 12,
        BTN_CH6_REC_READY = 13,
        BTN_CH7_REC_READY = 14,
        BTN_CH8_REC_READY = 15,
        BTN_CH1_SOLO_FIND = 16, // When in quick console mode: let the ficture strobe in order to find the linked lamp
        BTN_CH2_SOLO_FIND = 17, // When in show mode with displayed show UI: open an advanced color picker on the touch screen
        BTN_CH3_SOLO_FIND = 18, // When show editor is displayed: jump to and highlight input filter
        BTN_CH4_SOLO_FIND = 19,
        BTN_CH5_SOLO_FIND = 20,
        BTN_CH6_SOLO_FIND = 21,
        BTN_CH7_SOLO_FIND = 22,
        BTN_CH8_SOLO_FIND = 23,
        BTN_CH1_MUTE_BLACK = 24, // Make the brightness output of this lamp zero; button led should be blinking if enabled
        BTN_CH2_MUTE_BLACK = 25,
        BTN_CH3_MUTE_BLACK = 26,
        BTN_CH4_MUTE_BLACK = 27,
        BTN_CH5_MUTE_BLACK = 28,
        BTN_CH6_MUTE_BLACK = 29,
        BTN_CH7_MUTE_BLACK = 30,
        BTN_CH8_MUTE_BLACK = 31,
        BTN_CH1_SELECT_SELECT = 32, // Select this column for linked linked advanced input, led is activated if selected
        BTN_CH2_SELECT_SELECT = 33,
        BTN_CH3_SELECT_SELECT = 34,
        BTN_CH4_SELECT_SELECT = 35,
        BTN_CH5_SELECT_SELECT = 36,
        BTN_CH6_SELECT_SELECT = 37,
        BTN_CH7_SELECT_SELECT = 38,
        BTN_CH8_SELECT_SELECT = 39,
        BTN_TRACK_EDITSHOW = 40, // Open the show file editor
        BTN_PAN_COMMITSHOW = 41, // Apply the changes of the edited show
        BTN_EQ_COMMITRDY = 42, // Apply the chanes of all columns in READY-Mode, is blinking as long as some colums are in Ready/Wait mode
        BTN_SEND_OOPS = 43, // Undo the commit of show file or column updates (whichever was last)
        BTN_PLUGIN_PATCH = 44, // Open the patching menu and review current transmitted values
        BTN_INST_UNIVERSES = 45, // Edit the connected universes

        BTN_NAMEVALUE = 46,
        BTN_BEATS = 47,
        BTN_GLOBALVIEW = 48,
        BTN_MIDITRACKS = 49,
        BTN_INPUTS = 50,
        BTN_AUDIOTRACKS = 51,
        BTN_AUDIOINST = 52,
        BTN_AUX = 53,
        BTN_BUSSES = 54,
        BTN_OUTPUTS = 55,
        BTN_USER = 56,

        BTN_FLIP_MAINDARK = 57, // If enabled: makes the brightness of all attached fixtures 0; button led should be blinking if enabled

        BTN_F1_F1 = 58, // Assignable macro keys
        BTN_F2_F2 = 59,
        BTN_F3_F3 = 60,
        BTN_F4_F4 = 61,
        BTN_F5_F5 = 62,
        BTN_F6_F6 = 63,
        BTN_F7_F7 = 64,
        BTN_F8_F8 = 65,

        BTN_SHIFT = 66,
        BTN_OPTION = 67,
        BTN_CONTROL = 73,
        BTN_ALT = 74,

        BTN_READOFF = 68,
        BTN_WRITE = 69,
        BTN_TOUCH = 75,
        BTN_LATCH = 76,
        BTN_TRIM = 70,
        BTN_GROUP = 77,

        BTN_SAVE_SAVE = 71, // Save show file, when in editor, save universe values when in quick console mode
        BTN_UNDO_UNDO = 72, // Undo the changes made in the editor or quick console
        BTN_CANCEL_CANCEL = 78, // Cancle the setting of a filter configuration
        BTN_ENTER_ENTER = 79, // Submit the change of a filter configuration

        BTN_MARKER_GOBO = 80, // When channel selected edit gobo of channel, when no channel selected edit default/global gobo; when continiously pressed: use jogwheel to change gobo
        BTN_NUDGE_STROBO = 81, // When channel selected edit strobe of connected fixture, when no channel selected edit global strobe frequency; when continiously pressed: use jogwheel to change strobe settings
        BTN_CYCLE_SHUTTER = 82, // When channel selected edit shutter of connected fixture, when no channel selected edit global shutter value; continiously pressed: use jogwheel
        BTN_DROP_COLOR = 83, // Use color picker to edit color of fixture (if input selected) or global color. Jogwheel might be used for precise editing if button is continiously pressed
        BTN_REPLACE_TEMPERATURE = 84, // Edit color temperature of single or global color, jogwheel might be used as well
        BTN_CLICK_IMAGE = 85, // Select image send to shader or pixel mapper of selected column
        BTN_SOLO_SPEED = 86, // Select speed pattern of corresponding fixture

        // These buttons are useful for editing / playing Cues ans well as MIDI sequencing, button leds shall be lit, when the functionality is avaiable
        BTN_REV_LASTCUE = 87, // (⏪) Go to previous Cue
        BTN_FF_NEXTCUE = 88, // (⏩) Go to next Cue
        BTN_STOP_STOPCUE = 89, // (⏹) Stop (pause and goto start) execution of current Cue
        BTN_PLAY_RUNCUE = 90, // (⏵) Play/Pause execution of current Cue (blinking if paused)
        BTN_REC_RECFRAME = 91, // (⏺) Insert keyframe at current cursor position with current scene settings

        BTN_FADERBANKPREV_FADERBANKPREV = 92, // Go to the previous fader bank (when in quick console mode: touchscreen should also scroll with faders)
        BTN_FADERBANKNEXT_FADERBANKNEXT = 93, // Go to the next fader bank
        BTN_CHPREV_UNIVERSEPREV = 94, // When in quick console mode: jump to previous universe, when in Show (edit) mode: jump to previous UI page on primary display
        BTN_CHNEXT_UNIVERSENEXT = 95, // When in quick console mode: jump to next universe, when in Show (edit) mode: jump to next UI page on primary display

        BTN_SCRUB = 101,
        BTN_CROSSENTER = 100,
        BTN_UP_UP = 96, // Key up, or when MH advanced edit selected: Zoom+
        BTN_DOWN_DOWN = 97, // Key down, or when MH advanced edit selected: Zoom-
        BTN_RIGHT_RIGHT = 99, // Key right, or when MH advanced edit selected: Focus+
        BTN_LEFT_LEFT = 98, // Key left, or when MH advanced edit selected: Focus-

        FADERTOUCH_CH1 = 110, // These cannot be used as led ids but can be used for event decoding
        FADERTOUCH_CH2 = 111,
        FADERTOUCH_CH3 = 112,
        FADERTOUCH_CH4 = 113,
        FADERTOUCH_CH5 = 114,
        FADERTOUCH_CH6 = 115,
        FADERTOUCH_CH7 = 116,
        FADERTOUCH_CH8 = 117,
        FADERTOUCH_MAIN = 118,
    };

    typedef Iterator<button, button{0}, button{103}> xtouch_buttons;
    typedef Iterator<button, button::BTN_CH1_ENCODER_ROTARYMODE, button::BTN_CH8_SELECT_SELECT> xtouch_extender_buttons;
    static constexpr uint8_t xtouch_biggest_led_index = (uint8_t) button{103};

    enum class fader : uint8_t {
        FADER_CH1 = 70,
        FADER_CH2 = 71,
        FADER_CH3 = 72,
        FADER_CH4 = 73,
        FADER_CH5 = 74,
        FADER_CH6 = 75,
        FADER_CH7 = 76,
        FADER_CH8 = 77,
        FADER_MAIN = 78,
    };

    typedef Iterator<fader, fader::FADER_CH1, fader::FADER_MAIN> xtouch_faders;

    enum class encoder_change : uint8_t {
        RE_CW_INCREASE = 65,
        RE_CCW_DECREASE = 1,
    };

    inline void xtouch_set_button_led(device_handle& d, button b, button_led_state s) {
        assert(((uint8_t) b) < 104); // Make sure we don't try to set leds for fader touch keys
        d.send_command(midi_command{midi_status::NOTE_ON, 0, (uint8_t) b, (uint8_t) s});
    }

    inline void xtouch_set_fader_position(device_handle& d, fader f, uint8_t position) {
        d.send_command(midi_command{midi_status::CONTROL_CHANGE, 0, (uint8_t) f, position});
    }

    inline bool xtouch_is_column_button(button b) {
        return ((uint8_t) b) <= (uint8_t) button::BTN_CH8_SELECT_SELECT;
    }

    inline bool xtouch_is_fader_touch(button b) {
        auto num = (uint8_t) b;
        return num > ((uint8_t) button::BTN_LEFT_LEFT) && num <= (uint8_t) button::FADERTOUCH_MAIN;
    }

    [[nodiscard]] inline bool xtouch_is_column_fader(uint8_t c) {
        return (c >= XTOUCH_FADER_INDEX_OFFSET) && (c < XTOUCH_FADER_INDEX_OFFSET + 8);
    }

    [[nodiscard]] inline bool xtouch_is_column_encoder(uint8_t c) {
        return (c >= XTOUCH_ENCODER_INDEX_OFFSET) && (c < XTOUCH_ENCODER_INDEX_OFFSET + 8);
    }

    void xtouch_set_seg_display(device_handle& d, const std::array<char, 12>& content);
    void xtouch_set_lcd_display(device_handle& d, uint8_t display_index, lcd_color color, const std::array<char, 14> content);

    enum class encoder : uint8_t {
        ENC_CH1 = 80,
	ENC_CH2 = 81,
	ENC_CH3 = 82,
	ENC_CH4 = 83,
	ENC_CH5 = 84,
	ENC_CH6 = 85,
	ENC_CH7 = 86,
	ENC_CH8 = 87,
    };

    inline void xtouch_set_ring_led(device_handle& d, encoder enc, uint8_t value) {
	d.send_command(midi_command{midi_status::CONTROL_CHANGE, 0, (uint8_t) enc, value});
    }

    // TODO implement level leds as indicator for third channel value

}
