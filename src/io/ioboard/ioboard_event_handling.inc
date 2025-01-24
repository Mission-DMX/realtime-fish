//
// Created by leondietrich on 1/22/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "io/ioboard/ioboard.hpp"

#include <array>
#include <fcntl.h>
#include <unistd.h>

#include <ev++.h>

#include "lib/logging.hpp"
#include "net/netio_exception.hpp"

namespace dmxfish::io {
    void ioboard::set_io_flags() {
        int flags = (int) ::ev::READ;
        if (!this->priority_queue.empty() || !this->background_queue.empty()) {
            flags |= (int) ::ev::WRITE;
        }
        this->io_sync.set(flags);
        if (flags & (int) ::ev::WRITE) {
            this->async.send();
        }
    }

    void ioboard::cb_async_schedule(::ev::async& w, int events) {
        MARK_UNUSED(w);
        MARK_UNUSED(events);
        this->set_io_flags();
    }

    void ioboard::cb_io_handler(::ev::io& w, int events) {
        if(events & ::ev::ERROR) [[unlikely]] {
            throw rmrf::net::netio_exception("IO board client error. libev: state=" + std::to_string(events));
        }

        if (events & ::ev::READ) {
            std::array<uint8_t, 256> buffer;
            const auto read_bytes = read(w.fd, buffer.data(), buffer.size());
            for(auto i = 0; i < read_bytes; i++) {
                const auto current_byte = buffer[i];
                this->in_message_construction.push_back(current_byte);
                if(this->check_if_message_is_complete(current_byte)) {
                    this->process_complete_message();
                    this->in_message_construction.clear();
                    this->incoming_message_state = ioboard_message_state_t::IDLE;
                }
            }
        }

        if (events & ::ev::WRITE) {
            while(!this->priority_queue.empty() || !this->background_queue.empty()) {
                auto& selected_queue = (this->transmitting_background_msg || this->priority_queue.empty()) ? this->background_queue : this->priority_queue;
                if (selected_queue.empty()) {
                    this->transmitting_background_msg = false;
                } else {
                    auto message = selected_queue.pop_front();
                    const auto written = write(w.fd, message.ptr(), message.size());
                    if (written >= 0) {
                        message.advance((size_t) written);
                    }
                    selected_queue.push_front(message);
                    if (written < 0) {
                        if (errno == EAGAIN) {
                            break;
                        } else [[unlikely]] {
                            throw rmrf::net::netio_exception("Could not write to io board FT60X driver. libev: state=" + std::to_string(events));
                        }
                    }
                }
            }
        }

        this->set_io_flags();
    }

    void ioboard::process_complete_message() {
        ioboard_message_state_t current_state = ioboard_message_state_t::IDLE;
        ioboard_message_type_t msg_type = ioboard_message_type_t::INVALID;
        unsigned int target;
        unsigned int length;

        for(const auto b : this->in_message_construction) {
            if ((b & 0b10000000) > 0) {
                current_state = ioboard_message_state_t::TYPE_SEEN;
                msg_type = (ioboard_message_type_t) b;
            }
            switch (current_state) {
                case ioboard_message_state_t::IDLE:
                    break;
                case ioboard_message_state_t::TYPE_SEEN:
                    target = b;
                    current_state = ioboard_message_state_t::TARGET_SEEN;
                    break;
                case ioboard_message_state_t::TARGET_SEEN:
                    length = b << 7;
                    current_state = ioboard_message_state_t::CONTINUING_LENGTH;
                    break;
                case ioboard_message_state_t::CONTINUING_LENGTH:
                    length |= b;
                    current_state = ioboard_message_state_t::LENGTH_COMPLETE;
                    // TODO message types with length = 0 need to be handled here already
                    //  As of now, this includes the handshake.
                case ioboard_message_state_t::LENGTH_COMPLETE:
                    switch(msg_type) {
			case ioboard_message_type_t::SEND_DMX_DATA:
                        case ioboard_message_type_t::DMX_EVENT_DATA:
                            if (target >= this->linked_universes.size()) [[unlikely]] {
                                ::spdlog::error("IO Board responded with DMX data for a port ({}) it reported to be non-existant.", target);
                                return;
                            }
                            if(!this->linked_universes[target].has_value()) [[unlikely]] {
                                ::spdlog::warn("Got DMX data for non-registered universe on port {} from io board.", target);
                                return;
                            }
                            this->linked_universes[target]->get()->operator[](DMX_UNIVERSE_SIZE - length) = b;
                            break;
                            // TODO implement remaining message types
                        default:
                            ::spdlog::error("Handling data of message type {} is not yet implemented.", (unsigned int) msg_type);
                            return;
                    }
                    length--;
                    if (length == 0) {
                        current_state = ioboard_message_state_t::DATA_COMPLETE;
                    }
                case ioboard_message_state_t::DATA_COMPLETE:
                    ::spdlog::error("Received further message bytes although message is already finished.");
                    return;
                default:
                    throw rmrf::net::netio_exception("Unknown message decoding state: " + std::to_string((unsigned int) current_state));
            }
        }
    }

    bool ioboard::check_if_message_is_complete(uint8_t latest_byte) {
        if ((latest_byte & 0b10000000) > 0) {
            this->incoming_message_state = ioboard_message_state_t::TYPE_SEEN;
            return false;
        }
        switch (this->incoming_message_state) {
            case ioboard_message_state_t::IDLE:
                return false;
            case ioboard_message_state_t::TYPE_SEEN:
                this->incoming_message_state = ioboard_message_state_t::TARGET_SEEN;
                return false;
            case ioboard_message_state_t::TARGET_SEEN:
                this->in_message_length = latest_byte << 7;
                this->incoming_message_state = ioboard_message_state_t::CONTINUING_LENGTH;
                return false;
            case ioboard_message_state_t::CONTINUING_LENGTH:
                this->in_message_length |= latest_byte;
                this->incoming_message_state = ioboard_message_state_t::LENGTH_COMPLETE;
                return this->in_message_length == 0;
            case ioboard_message_state_t::LENGTH_COMPLETE:
                this->in_message_length--;
                if (this->in_message_length < 1) {
                    this->incoming_message_state = ioboard_message_state_t::DATA_COMPLETE;
                    return true;
                }
                return false;
            case ioboard_message_state_t::DATA_COMPLETE:
                return true;
            default:
                throw rmrf::net::netio_exception("Unknown message decoding state: " + std::to_string((uint8_t) this->incoming_message_state));
        }
    }
}
