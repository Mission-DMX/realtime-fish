//
// Created by leondietrich on 1/21/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//
#pragma once

#include "dmx/universe.hpp"

namespace dmxfish::io {
    class ioboard;
}

namespace dmxfish::dmx {

    class ioboard_universe : public universe {
    private:
        size_t port;
    private:
        friend class dmxfish::io::ioboard;
        ioboard_universe(size_t port);
    public:
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