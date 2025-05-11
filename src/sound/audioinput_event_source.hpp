//
// Created by Doralitze on 11.05.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#pragma once

#include <atomic>
#include <optional>
#include <string>
#include <thread>

#include "events/event_source.hpp"

namespace dmxfish::audio {

    class audioinput_event_source : public dmxfish::events::event_source {
    private:
        // TODO implement atomic cutoff settings
        std::atomic_bool running = false;
        std::atomic_short channel_count = 1;
        std::atomic_int sampler_rate = 48000;
        std::atomic_int record_block_duration_ms = 2100;
        std::string sound_dev_file = "";
        std::optional<std::thread> thread;
    public:
        audioinput_event_source();
        [[nodiscard]] missiondmx::fish::ipcmessages::event_sender encode_proto_message() const override;
        [[nodiscard]] bool update_conf_from_message(const missiondmx::fish::ipcmessages::event_sender& msg) override;
    private:
        void update_task();
    };

}