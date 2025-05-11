//
// Created by Doralitze on 11.05.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "audioinput_event_source.hpp"

#include "ALSA/ALSA.H"

#include "lib/logging.hpp"

namespace dmxfish::audio {

    audioinput_event_source::audioinput_event_source() {

    }

    missiondmx::fish::ipcmessages::event_sender audioinput_event_source::encode_proto_message() const {
        auto msg = event_source::encode_proto_message();
        msg.set_type("fish.builtin.audioextract");
        auto conf = (*mutable_configuration) msg.configuration();
        conf["dev"] = this->sound_dev_file;
        // TODO transmit cuttoff settings
        return msg;
    }

    bool audioinput_event_source::update_conf_from_message(const missiondmx::fish::ipcmessages::event_sender& msg) {
        // TODO update cutoff settings
        if (msg.configuration()["dev"] == this->sound_dev_file) {
            return true;
        }
        this->running = false;
        if (this->thread.has_value()) {
            this->thread->join();
        }
        this->thread = std::thread(&audioinput_event_source::update_task, this);
        return true;
    }

    void audioinput_event_source::update_task() {
        using namespace ALSA;
        Capture capture_dev(this->sound_dev_file);
        if (capute_dev.prepared()) {
            ::spdlog::error("Failed to open sound input device {}", this->sound_dev_file);
            return;
        } else {
            ::spdlog::info("Opened sound device {}.", capture_dev.getDeviceName());
        }
        capture_dev.resetParams();
        snd_pcm_format_t format=SND_PCM_FORMAT_S32_LE;
        if (const auto res = capture_dev.setFormat(format); res < 0) {
            ::spdlog::error("Failed to set ALSA record format. Error: {}", ALSADebug().evaluateError(res));
            return;
        }
        if (const auto res = capture_dev.setAccess(SND_PCM_ACCESS_RW_INTERLEAVED); res < 0) {
            ::spdlog::error("Failed to set sound card access mode. Error: {}", ALSADebug().evaluateError(res));
            return;
        }
        if (const auto res = capture.setChannels(this->channel_count); res < 0) {
            ::spdlog::error("Failed to set recording channel count to {}. Error: {}", this->channel_count, ALSADebug().evaluateError(res));
            return;
        }
        while(this->running) {
            // TODO analyze input
        }
    }
}