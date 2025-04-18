//
// Created by leondietrich on 1/21/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "ioboard_universe.hpp"

namespace dmxfish::dmx {

    ioboard_universe::ioboard_universe(dmxfish::io::ioboard_port_id_t _port, int _id) :
			universe(_id, universe_type::PHYSICAL), port(_port), raw_data() {
        // No Op
    }

    channel_8bit_t& ioboard_universe::operator[](std::size_t p) {
        return this->raw_data.operator[](p);
    }

    universe_iterator ioboard_universe::begin() {
        return this->raw_data.begin();
    }

    universe_iterator ioboard_universe::end() {
        return this->raw_data.end();
    }

}
