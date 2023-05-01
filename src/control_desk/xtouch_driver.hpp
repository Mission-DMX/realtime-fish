#pragma once

#include <array>
#include <cassert>

#include "control_desk/device_handle.hpp"
#include "control_desk/enum_iterator.hpp"

#define VENDOR_BEHRINGER {0, 20, 32}

#define CMD_7SEG 37
#define XTOUCH_FADER_INDEX_OFFSET 70

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

    /**
     * This enum defines the button IDs. The first part of the specific name defines the original meaning, the second one our labeling. For example BTN_CH1_MUTE_BLACK
     * defines the button on channel 1 that once was labeled MUTE but is now labeled BLACK.
     */
    enum class button : uint8_t { // TODO fill in correct values as soon as X-Touch arrived
        BTN_CH1_ENCODER_ROTARYMODE, // Switches rotary encoder between Hue, Saturation and (if applicable) UV and (if applicable) Amber, Second row of LCD displays mode and setting
        BTN_CH2_ENCODER_ROTARYMODE,
        BTN_CH3_ENCODER_ROTARYMODE,
        BTN_CH4_ENCODER_ROTARYMODE,
        BTN_CH5_ENCODER_ROTARYMODE,
        BTN_CH6_ENCODER_ROTARYMODE,
        BTN_CH7_ENCODER_ROTARYMODE,
        BTN_CH8_ENCODER_ROTARYMODE,
        BTN_CH1_REC_READY, // If ready is enabled (blinking red), the entered changes of the channel are not applied until the COMMITRDY button is pressed.
        BTN_CH2_REC_READY,
        BTN_CH3_REC_READY,
        BTN_CH4_REC_READY,
        BTN_CH5_REC_READY,
        BTN_CH6_REC_READY,
        BTN_CH7_REC_READY,
        BTN_CH8_REC_READY,
        BTN_CH1_SOLO_FIND, // When in quick console mode: let the ficture strobe in order to find the linked lamp
        BTN_CH2_SOLO_FIND, // When in show mode with displayed show UI: open an advanced color picker on the touch screen
        BTN_CH3_SOLO_FIND, // When show editor is displayed: jump to and highlight input filter
        BTN_CH4_SOLO_FIND,
        BTN_CH5_SOLO_FIND,
        BTN_CH6_SOLO_FIND,
        BTN_CH7_SOLO_FIND,
        BTN_CH8_SOLO_FIND,
        BTN_CH1_MUTE_BLACK, // Make the brightness output of this lamp zero; button led should be blinking if enabled
        BTN_CH2_MUTE_BLACK,
        BTN_CH3_MUTE_BLACK,
        BTN_CH4_MUTE_BLACK,
        BTN_CH5_MUTE_BLACK,
        BTN_CH6_MUTE_BLACK,
        BTN_CH7_MUTE_BLACK,
        BTN_CH8_MUTE_BLACK,
        BTN_CH1_SELECT_SELECT, // Select this column for linked linked advanced input, led is activated if selected
        BTN_CH2_SELECT_SELECT,
        BTN_CH3_SELECT_SELECT,
        BTN_CH4_SELECT_SELECT,
        BTN_CH5_SELECT_SELECT,
        BTN_CH6_SELECT_SELECT,
        BTN_CH7_SELECT_SELECT,
        BTN_CH8_SELECT_SELECT,
        BTN_TRACK_EDITSHOW, // Open the show file editor
        BTN_PAN_COMMITSHOW, // Apply the changes of the edited show
        BTN_EQ_COMMITRDY, // Apply the chanes of all columns in READY-Mode, is blinking as long as some colums are in Ready/Wait mode
        BTN_SEND_OOPS, // Undo the commit of show file or column updates (whichever was last)
        BTN_PLUGIN_PATCH, // Open the patching menu and review current transmitted values
        BTN_INST_UNIVERSES, // Edit the connected universes

        BTN_NAMEVALUE,
        BTN_BEATS,
        BTN_GLOBALVIEW,
        BTN_MIDITRACKS,
        BTN_INPUTS,
        BTN_AUDIOTRACKS,
        BTN_AUDIOINST,
        BTN_AUX,
        BTN_BUSSES,
        BTN_OUTPUTS,
        BTN_USER,

        BTN_FLIP_MAINDARK, // If enabled: makes the brightness of all attached fixtures 0; button led should be blinking if enabled

        BTN_F1_F1, // Assignable macro keys
        BTN_F2_F2,
        BTN_F3_F3,
        BTN_F4_F4,
        BTN_F5_F5,
        BTN_F6_F6,
        BTN_F7_F7,
        BTN_F8_F8,

        BTN_SHIFT,
        BTN_OPTION,
        BTN_CONTROL,
        BTN_ALT,

        BTN_READOFF,
        BTN_WRITE,
        BTN_TOUCH,
        BTN_LATCH,
        BTN_TRIM,
        BTN_GROUP,

        BTN_SAVE_SAVE, // Save show file, when in editor, save universe values when in quick console mode
        BTN_UNDO_UNDO, // Undo the changes made in the editor or quick console
        BTN_CANCLE_CANCLE, // Cancle the setting of a filter configuration
        BTN_ENTER_ENTER, // Submit the change of a filter configuration

        BTN_MARKER,
        BTN_NUDGE,
        BTN_CYCLE,
        BTN_DROP,
        BTN_REPLACE,
        BTN_CLICK,
        BTN_SOLO,

        // These buttons are useful for editing / playing Cues ans well as MIDI sequencing, button leds shall be lit, when the functionality is avaiable
        BTN_REV_LASTCUE, // (⏪) Go to previous Cue
        BTN_FF_NEXTCUE, // (⏩) Go to next Cue
        BTN_STOP_STOPCUE, // (⏹) Stop (pause and goto start) execution of current Cue
        BTN_PLAY_RUNCUE, // (⏵) Play/Pause execution of current Cue (blinking if paused)
        BTN_REC_RECFRAME, // (⏺) Insert keyframe at current cursor position with current scene settings

        BTN_FADERBANKPREV_FADERBANKPREV, // Go to the previous fader bank (when in quick console mode: touchscreen should also scroll with faders)
        BTN_FADERBANKNEXT_FADERBANKNEXT, // Go to the next fader bank
        BTN_CHPREV_UNIVERSEPREV, // When in quick console mode: jump to previous universe, when in Show (edit) mode: jump to previous UI page on primary display
        BTN_CHNEXT_UNIVERSENEXT, // When in quick console mode: jump to next universe, when in Show (edit) mode: jump to next UI page on primary display

        BTN_SCRUB,
        BTN_CROSSENTER,
        BTN_JOGWHEEL,
        BTN_UP_UP, // Key up, or when MH advanced edit selected: Zoom+
        BTN_DOWN_DOWN, // Key down, or when MH advanced edit selected: Zoom-
        BTN_RIGHT_RIGHT, // Key right, or when MH advanced edit selected: Focus+
        BTN_LEFT_LEFT, // Key left, or when MH advanced edit selected: Focus-

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

    void xtouch_set_seg_display(device_handle& d, const std::array<char, 12>& content);

}
