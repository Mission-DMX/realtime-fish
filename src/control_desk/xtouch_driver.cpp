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
            default:
                return 0b0;
            // TODO complete me
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

}
