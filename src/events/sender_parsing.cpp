//
// Created by Doralitze on 11.05.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "events/sender_parsing.hpp"

#include <stdexcept>

#include "events/event_storage.hpp"
#include "main.hpp"
#include "utils.hpp"

namespace dmxfish::events {

    dmxfish::events::event_sender_t parse_sender_representation(const std::string& s) {
        auto parts = utils::split(s, ':');
        if (parts.size() < 2) [[unlikely]] {
            throw std::invalid_argument("Sender representation '" + s + "' is missing the ':function' part.");
        }
        const auto sender_part = parts.front();
        parts.pop_front();
        const auto function_part = parts.front();

        dmxfish::events::event_sender_t es;
        if (utils::is_number(sender_part)) {
            es.decoded_representation.sender = std::stoul(sender_part);
        } else {
            auto storage = get_event_storage_instance();
            if (storage == nullptr) [[unlikely]] {
                throw std::invalid_argument("Cannot resolve sender name '" + sender_part +
                                            "': event storage is not initialized.");
            }
            const auto resolved = storage->find_sender_id_by_name(sender_part);
            if (!resolved.has_value()) {
                throw std::invalid_argument("Cannot resolve sender name '" + sender_part +
                                            "': no event source is registered under that name.");
            }
            es.decoded_representation.sender = *resolved;
        }
        es.decoded_representation.sender_function = std::stoul(function_part);
        return es;
    }
}