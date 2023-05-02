#include "control_desk/xtouch_driver.hpp"

namespace dmxfish::control_desk {

    inline uint8_t xtouch_translate_char_to_seg_data(const char c) {
        // 0b0gfedcba
        switch(c) {
            case 'a':
            case 'A':
                return 0b01110111;
            case 'b':
            case 'B':
                return 0b01111100;
            case 'c':
                return 0b01011000;
            case 'C':
                return 0b00111001;
            case 'd':
            case 'D':
                return 0b01011110;
            case 'e':
            case 'E':
                return 0b00111001;
            case 'f':
            case 'F':
                return 0b00110001;
            case 'o':
                return 0b01011100;
            case 'O':
            case '0':
                return 0b00111111;
            case '1':
                return 0b00000110;
            case '2':
                return 0b01011011;
            case '3':
                return 0b01001111;
            case '4':
                return 0b01100110;
            case '5':
                return 0b01101101;
            case '6':
                return 0b01111100;
            case '7':
                return 0b00000111;
            case '8':
                return 0b01111111;
            case '9':
                return 0b01101111;
            case '-':
                return 0b01000000;
            case '_':
                return 0b00001000;
            case ' ':
            default:
                return 0b0;
        }
    }

    void xtouch_set_seg_display(device_handle& d, const std::array<char, 12>& content) {
        sysex_command cmd;
        cmd.vendor = VENDOR_BEHRINGER;
        cmd.sysex_data.push_back((uint8_t) d.get_device_id());
        cmd.sysex_data.push_back(CMD_7SEG);
        for(auto c : content) {
            cmd.sysex_data.push_back(xtouch_translate_char_to_seg_data(c));
        }
        // disable dots
        cmd.sysex_data.push_back(0); // Dots 1-7
        cmd.sysex_data.push_back(0); // Dots 8-12
        d.send_sysex_command(cmd);
    }

    void xtouch_set_lcd_display(device_handle& d, uint8_t display_index, lcd_color color, const std::array<char, 14> content) {
        sysex_command cmd;
        cmd.vendor = VENDOR_BEHRINGER;
        cmd.sysex_data.reserve(4 + 14);
        cmd.sysex_data.push_back((uint8_t) d.get_device_id());
        cmd.sysex_data.push_back(CMD_LCD);
        cmd.sysex_data.push_back(display_index);
        cmd.sysex_data.push_back((uint8_t) color);
        for(const auto c : content) {
            cmd.sysex_data.push_back(c);
        }
	d.send_sysex_command(cmd);
    }

}
