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
        std::atomic_bool running = false;
        int high_cutoff_frequency = 96;
        int low_cutoff_frequency = 9;
        double trigger_magnitude = 30.0;

        unsigned short channel_count = 1;
        unsigned short sampler_rate = 48000;
        unsigned short record_block_duration_ms = 40;

        std::string sound_dev_file = "";
        std::optional<std::thread> thread = std::nullopt;
    public:
        audioinput_event_source();
        ~audioinput_event_source();
        [[nodiscard]] missiondmx::fish::ipcmessages::event_sender encode_proto_message() const override;
        [[nodiscard]] bool update_conf_from_message(const missiondmx::fish::ipcmessages::event_sender& msg) override;
    private:
        void update_task();
    };

}