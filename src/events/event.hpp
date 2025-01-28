//
// Created by doralitze on 11/14/23.
//

#pragma once

#include <array>

#include "event_type.hpp"

namespace dmxfish::events {

    using event_id_t = unsigned long;

    union event_sender_t {
        uint64_t encoded_sender_id;

        struct decoded_sender_t {
            uint32_t sender;
            uint32_t sender_function;
            decoded_sender_t(uint32_t s, uint32_t f) : sender(s), sender_function(f) {}
        };
        decoded_sender_t decoded_representation;
    public:
        event_sender_t() : encoded_sender_id(0) {};

        event_sender_t(uint32_t sender, uint32_t function) : decoded_representation(sender, function) {}

        inline bool operator<(const event_sender_t& other) const {
            return this->encoded_sender_id < other.encoded_sender_id;
        }

        inline bool operator==(const event_sender_t& other) const {
            return this->encoded_sender_id == other.encoded_sender_id;
        }

        inline bool is_same_sender(const event_sender_t& other) const {
            return this->decoded_representation.sender == other.decoded_representation.sender;
        }
    };

    class event {
    private:
        event_type type;
        std::array<uint8_t, 7> event_arguments;
        event_id_t event_id;
        event_sender_t sender_id;
    public:
        event();
        event(event_type _type, event_sender_t sender_id);
        event(const event& other);
        event(event_type _type, const event& other);

        [[nodiscard]] inline event_id_t get_event_id() const {
            return this->event_id;
        }

        [[nodiscard]] inline event_sender_t get_event_sender() const {
            return this->sender_id;
        }

        [[nodiscard]] inline event_type get_type() const {
            return this->type;
        }

        [[nodiscard]] inline bool is_valid() const {
            return !(this->type == event_type::INVALID || this->event_id == 0);
        }

        [[nodiscard]] inline const std::array<uint8_t, 7> get_args() const {
            return this->event_arguments;
        }

        inline void set_arg_data(size_t pos, uint8_t d) {
            this->event_arguments[pos] = d;
        }
    };

    std::ostream& operator<<(std::ostream& os, const event& ev);

}

