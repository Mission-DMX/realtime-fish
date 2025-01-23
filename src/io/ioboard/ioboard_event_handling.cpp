//
// Created by leondietrich on 1/22/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "io/ioboard/ioboard.hpp"

#include <array>
#include <fcntl.h>
#include <unistd.h>

#include <ev++.h>

#include "net/netio_exception.hpp"

namespace dmxfish::io {
    void ioboard::set_io_flags() {
        auto flags = ::ev::READ;
        if (!this->priority_queue.empty() || !this->background_queue.empty()) {
            flags |= ::ev::WRITE;
        }
        this->io_sync.set(flags);
        if (flags & ::ev::WRITE) {
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
}