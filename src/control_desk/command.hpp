#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace dmxfish::control_desk {

    // TODO we should move this enum to a separate file
    enum class midi_status : uint8_t {
        INVALID = 0x00,
        NOTE_OFF = 0x80,
        NOTE_ON = 0x90,
        POLYPHONIC = 0xA0,
        CONTROL_CHANGE = 0xB0,
        PROG_CHANGE = 0xC0,
        MONOPHONIC = 0xD0,
        PITCH_BEND = 0xE0,
        SYS_EX = 0xF0
    };

    struct midi_command {
        midi_status status;
        uint8_t channel;
        uint8_t data_1;
        uint8_t data_2;

        midi_command() : status{midi_status::INVALID}, channel{0}, data_1{0}, data_2{0} {}
        midi_command(midi_status s, uint8_t c, uint8_t d1, uint8_t d2)
            : status{s}, channel{c}, data_1{d1}, data_2{d2} {}
        midi_command(uint8_t status_byte, uint8_t data_byte_1, uint8_t data_byte_2)
            : status{(uint8_t) (status_byte & 0xF0)}, channel{(uint8_t) (status_byte & 0x0F)}, data_1{data_byte_1}, data_2{data_byte_2} {}


        bool encode(std::vector<uint8_t>& dv) const {
            dv.emplace_back((uint8_t) status | (0x0F & channel));
            // Bit 7 of data bytes needs to always be 0.
            dv.emplace_back(data_1 & 0x7F);
            dv.emplace_back(data_2 & 0x7F);
            return true;
        }
    };

    enum midi_device_id {
        X_TOUCH = 0x14,
        X_TOUCH_EXTENSION = 0x15,
    };

    struct sysex_command {
        std::vector<uint8_t> sysex_data;
        std::array<uint8_t, 3> vendor;

        sysex_command() : sysex_data{}, vendor{} {}

        bool encode(std::vector<uint8_t>& dv, const midi_device_id& devid) const {
            dv.emplace_back(0xF0);
            dv.emplace_back(vendor[0]);
            if(vendor[0] == 0) {
                dv.emplace_back(vendor[1]);
                dv.emplace_back(vendor[2]);
            }
            dv.emplace_back((uint8_t) devid);
            for(const auto& d : sysex_data) {
                dv.emplace_back(d & 0x7F);
            }
            dv.emplace_back(0xF7);
            return true;
        };
    };
}
