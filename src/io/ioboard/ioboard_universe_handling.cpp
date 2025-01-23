//
// Created by leondietrich on 1/21/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "ioboard.hpp"

#include <stdexcept>

#include "io/ioboard/types.h"

namespace dmxfish::io {

    [[nodiscard]] std::weak_ptr<dmxfish::dmx::ioboard_universe> ioboard::get_or_create_universe(ioboard_port_id_t port, int id) {
        if (const auto universe_vector_size = this->linked_universes.size(); port >= universe_vector_size) [[unlikely]] {
            throw std::out_of_range("Requested ioboard DMX port #" + std::to_string(port) +
                " (0-indexed) but there are only " + std::to_string(universe_vector_size) +
                " ports on the board.");
        }

        if (auto candidate = this->linked_universes[port]; candidate.has_value()) {
            return candidate.value();
        } else {
            auto univ = std::make_shared<dmxfish::dmx::ioboard_universe>(port, id);
            this->linked_universes[port] = univ;
            return univ;
        }
    }

    void ioboard::unregister_universe(ioboard_port_id_t port) {
        if (const auto universe_vector_size = this->linked_universes.size(); port >= universe_vector_size) [[unlikely]] {
            throw std::out_of_range("Requested to invalidate port #" + std::to_string(port) +
                                    " (0-indexed) but there are only " + std::to_string(universe_vector_size) +
                                    " ports on the board.");
        }
        this->linked_universes[port] = std::nullopt;
    }

    bool ioboard::unregister_universe_by_id(int id) {
        // linear search is fine as we're expecting 4 ports.
        for (size_t i = 0; i < this->linked_universes.size(); i++) {
            if (auto univ = this->linked_universes[i]; univ.has_value()) {
                if (univ->get()->getID() == id) {
                    this->unregister_universe(i);
                    return true;
                }
            }
        }
        return false;
    }

    void ioboard::transmit_universe(ioboard_port_id_t port) {
        if (const auto universe_vector_size = this->linked_universes.size(); port >= universe_vector_size) [[unlikely]] {
            throw std::out_of_range("Requested push universe data at port #" + std::to_string(port) +
                                    " (0-indexed), which does not exist.");
        }
        if (const auto univ_opt = this->linked_universes[port]; univ_opt.has_value()) [[likely]] {
            const auto univ = *(univ_opt->get());
            rmrf::net::iorecord r;
            r.reserve_space(4+586); // 586 = 512*8/7
            auto arr = r.ptr();
            arr[0] = (uint8_t) ioboard_message_type_t::SEND_DMX_DATA;
            arr[1] = port & 0b01111111;
            arr[2] = 0b100; // size, bits 9:7 of 586
            arr[3] = 0b01001010; // size, bits 6:0 of 586

            int i = 4, bit_offset = 0, last_data = 0;
            for (const auto channel : univ) {
                last_data = (last_data << 8) | channel;
                bit_offset += 8;
                while(bit_offset > 7) {
                    arr[i++] = (last_data >> (bit_offset - 7)) & 0b01111111;
                    bit_offset -= 7;
                }
            }
            if (bit_offset > 0) {
                arr[i++] = last_data >> (7 - bit_offset) & 0b01111111;
            }

            priority_queue.push_back(r);
            this->set_io_flags();

        } else {
            throw std::out_of_range("Requested push universe data at port #" + std::to_string(port) +
                                    " (0-indexed), which is not active.");
        }
    }

    [[nodiscard]] ioboard_port_id_t ioboard::find_universe(int id) const {
        for (size_t i = 0; i < this->linked_universes.size(); i++) {
            if (auto univ = this->linked_universes[i]; univ.has_value()) {
                if (univ->get()->getID() == id) {
                    return i;
                }
            }
        }
        return -1;
    }
}