#pragma once

#include <ev++.h>
#include <optional>

#include "control_desk/command.hpp"
#include "net/async_fd.hpp"
#include "net/ioqueue.hpp"

namespace dmxfish::control_desk {
    class device_handle {
    private:
        ::ev::async async;
        ::ev::io file_sync;
        rmrf::net::auto_fd driver_fd;
        rmrf::net::ioqueue<rmrf::net::iorecord> event_queue, sysex_queue;
        std::deque<midi_command> incomming_queue;
        std::vector<uint8_t> event_construction, sysex_construction;
        std::deque<uint8_t> in_construction;
        int max_sysex_queue_length = 3;
        const midi_device_id device_id;
    public:
        device_handle(const std::string& driver_file_path, const midi_device_id& mdi);
        device_handle(const device_handle&) = delete;
        device_handle(device_handle&&) = delete;
        ~device_handle();
        void send_command(const midi_command& c);
        void send_sysex_command(const sysex_command& sc);
        void schedule_transmission();

        inline void set_max_sysex_queue_length(int new_length) {
            this->max_sysex_queue_length = new_length;
        }

        [[nodiscard]] inline int get_max_sysex_queue_length() const {
            return this->max_sysex_queue_length;
        }

        [[nodiscard]] inline midi_device_id get_device_id() const {
            return this->device_id;
        }

        [[nodiscard]] inline std::optional<midi_command> get_next_command_from_desk() {
            if (incomming_queue.empty()) {
                return {};
            }
            auto c = incomming_queue.front();
            incomming_queue.pop_front();
            return c;
        }
    private:
        bool decode_incomming_event();
        void cb_async_schedule(::ev::async& w, int events);
        void cb_io_handler(::ev::io& w, int events);

        void set_io_flags() {
            auto flags = 0;
            flags |= ::ev::READ;
            if(!event_construction.empty()) {
                flags |= ::ev::WRITE;
            }
            if(!sysex_construction.empty()) {
                flags |= ::ev::WRITE;
            }
            this->file_sync.set(flags);
        }
    };
}
