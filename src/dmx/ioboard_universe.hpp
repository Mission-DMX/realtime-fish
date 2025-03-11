//
// Created by leondietrich on 1/21/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//
#pragma once

#include <array>
#include <cstdint>

#include "dmx/universe.hpp"
#include "io/ioboard/types.h"

namespace dmxfish::dmx {

    class ioboard_universe : public universe {
    private:
        size_t port;
        std::array<uint8_t, DMX_UNIVERSE_SIZE> raw_data;
    private:
        friend class dmxfish::io::ioboard;
    public:
        ioboard_universe(dmxfish::io::ioboard_port_id_t port, int id);

        virtual channel_8bit_t& operator[](size_t p);

        virtual universe_iterator begin();

        virtual universe_iterator end();

        /**
         * The port this universe is mapped to.
         * @return the board port (0-indexed).
         */
        [[nodiscard]] inline size_t get_port() const {
            return this->port;
        }
    };

}