#pragma once

#include <ev++.h>

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
        std::vector<uint8_t> event_construction, sysex_construction;
        int max_sysex_queue_length = 3;
        const midi_device_id device_id;
    public:
        device_handle(const std::string& driver_file_path, const midi_device_id& mdi);
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

        [[nodiscard]] inline midi_device_id get_device_id() {
            return this->device_id;
        }
    private:
        void decode_incomming_event(const midi_command& c);
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
