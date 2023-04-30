#include "control_desk/device_handle.hpp"

#include <array>
#include <fcntl.h>
#include <unistd.h>

#include "net/netio_exception.hpp"
#include "lib/logging.hpp"
#include "lib/macros.hpp"

namespace dmxfish::control_desk {
    device_handle::device_handle(const std::string& driver_file_path, const midi_device_id& mdi) :
        async{}, file_sync{}, driver_fd{open(driver_file_path.c_str(), O_RDWR | O_NONBLOCK)},
        event_queue{}, sysex_queue{}, incomming_queue{}, event_construction{63}, sysex_construction{64},
        in_construction{}, device_id{mdi}
    {
        if(driver_fd) {
            async.set<device_handle, &device_handle::cb_async_schedule>(this);
            async.start();
            file_sync.set<device_handle, &device_handle::cb_io_handler>(this);
            file_sync.start(driver_fd.get(), 0);
        } else {
            throw rmrf::net::netio_exception("Failed to connect to MIDI driver.");
        }
    }

    device_handle::~device_handle() {
        async.stop();
        file_sync.stop();
    }

    void device_handle::send_command(const midi_command& c) {
        c.encode(event_construction);
    }

    void device_handle::send_sysex_command(const sysex_command& sc) {
        sc.encode(sysex_construction, this->get_device_id());
    }

    void device_handle::schedule_transmission() {
        if(!event_construction.empty()) {
            this->event_queue.push_back(rmrf::net::iorecord{event_construction.data(), event_construction.size()});
            event_construction.clear();
        }
        if(!sysex_construction.empty()) {
            // TODO erase first element from ioqueue if required
            this->sysex_queue.push_back(rmrf::net::iorecord{sysex_construction.data(), sysex_construction.size()});
            sysex_construction.clear();
        }
        this->async.send();
    }

    bool device_handle::decode_incomming_event() {
        midi_command c;
        uint8_t initial_byte;
        do {
            initial_byte = in_construction.front();
            in_construction.pop_front();
        } while ((initial_byte & 0x80) == 0);
        if(in_construction.size() < 2) {
            in_construction.push_front(initial_byte);
            return false;
        }
        c.status = (midi_status) (initial_byte & 0xF0);
        if(c.status == midi_status::SYS_EX) {
            ::spdlog::warn("Received sysex MIDI command from device. This should never happen.");
        } else {
            c.channel = initial_byte & 0x0F;
            c.data_1 = in_construction.front();
            in_construction.pop_front();
            c.data_2 = in_construction.front();
            in_construction.pop_front();
            incomming_queue.push_back(c);
        }
        return true;
    }

    void device_handle::cb_async_schedule(::ev::async& w, int events) {
        MARK_UNUSED(w);
        MARK_UNUSED(events);
        set_io_flags();
    }

    void device_handle::cb_io_handler(::ev::io& w, int events) {
        if(events & ::ev::READ) {
            throw rmrf::net::netio_exception("MIDI client error. libev: state=" + std::to_string(events));
        }

        if (events & ::ev::READ) {
            std::array<uint8_t, 48> buffer;
            const auto read_bytes = read(w.fd, buffer.data(), buffer.size());
            for(auto i = 0; i < read_bytes; i++) {
                in_construction.push_back(buffer[i]);
            }
            while(!in_construction.empty() && this->decode_incomming_event());
        }

        if (events & ::ev::WRITE) {
            bool driver_has_buffer_capacity = true;
            while(driver_has_buffer_capacity && !this->event_queue.empty()) {
                auto buffer = this->event_queue.pop_front();
                const auto written = write(w.fd, buffer.ptr(), buffer.size());
                if (written >= 0) {
                    buffer.advance((size_t)written);
                } else if (errno == EAGAIN) {
                    driver_has_buffer_capacity = false;
                } else {
                    throw rmrf::net::netio_exception("Could not write to MIDI driver. libev: state=" + std::to_string(events));
                }
                this->event_queue.push_front(buffer);
            }
            while(driver_has_buffer_capacity && !this->sysex_queue.empty()) {
                auto buffer = this->sysex_queue.pop_front();
                const auto written = write(w.fd, buffer.ptr(), buffer.size());
                if (written >= 0) {
                    buffer.advance((size_t)written);
                } else if (errno == EAGAIN) {
                    driver_has_buffer_capacity = false;
                } else {
                    throw rmrf::net::netio_exception("Could not write to MIDI driver. libev: state=" + std::to_string(events));
                }
                this->sysex_queue.push_front(buffer);
            }
        }
        set_io_flags();
    }
}
