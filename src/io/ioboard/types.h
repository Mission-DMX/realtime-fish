//
// Created by leondietrich on 1/22/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//
#pragma once

#include <cstdint>

namespace dmxfish::io {

    class ioboard;
    using ioboard_port_id_t = long;

    enum class ioboard_message_state_t : uint8_t {
        IDLE,
        TYPE_SEEN,
        TARGET_SEEN,
        CONTINUING_LENGTH,
        LENGTH_COMPLETE,
        DATA_COMPLETE
    };

    enum class ioboard_message_type_t : uint8_t {
        INVALID = 0,
        SEND_DMX_DATA = 0b10000001,
        CONFIGURE_DMX = 0b10000010,
        SEND_MIDI_DATA = 0b10000100,
        CONFIGURE_MIDI_DATA = 0b10000101,
        SEND_RS232_DATA = 0b10000111,
        CONFIGURE_RS232_DATA = 0b10001000,
        SEND_DISPLAY_DATA = 0b10001010,
        DMX_EVENT_DATA = 0b10001100,
        MIDI_EVENT_DATA = 0b10001101,
        RS232_EVENT_DATA = 0b10001110,
        KEYPAD_EVENT_DATA = 0b10001111,
        STATUS_DMX_PORT_VACANCY_REQUEST = 0b10010001,
        STATUS_DMX_PORT_VACANCY_REPLY = 0b10010010,
        STATUS_DMX_PORT_ERROR_COUNT_REQUEST = 0b10010011,
        STATUS_DMX_PORT_ERROR_COUNT_REPLY = 0b10010100,
        PROTOCOL_HANDSHAKE = 0b11111111
    };

}
