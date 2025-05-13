//
// Created by Doralitze on 11.05.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "audioinput_event_source.hpp"

#include <Eigen/Dense>

#include "sound/ALSA/ALSA.H"
#include "sound/ALSA/Capture.H"


#include "lib/logging.hpp"

namespace dmxfish::audio {

    audioinput_event_source::audioinput_event_source() {

    }

    missiondmx::fish::ipcmessages::event_sender audioinput_event_source::encode_proto_message() const {
        auto msg = event_source::encode_proto_message();
        msg.set_type("fish.builtin.audioextract");
        auto conf = msg.configuration();
        conf["dev"] = this->sound_dev_file;
        // TODO transmit cuttoff settings
        return msg;
    }

    bool audioinput_event_source::update_conf_from_message(const missiondmx::fish::ipcmessages::event_sender& msg) {
        // TODO update cutoff settings
        auto conf = msg.configuration();
        if (conf["dev"] == this->sound_dev_file) {
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
        Capture capture_dev(this->sound_dev_file.c_str());
        if (capture_dev.prepared()) {
            ::spdlog::error("Failed to open sound input device {}", this->sound_dev_file);
            return;
        } else {
            ::spdlog::info("Opened sound device {}.", capture_dev.getDeviceName());
        }
        capture_dev.resetParams();
        snd_pcm_format_t format=SND_PCM_FORMAT_S32_LE;
        if (const auto res = capture_dev.setFormat(format); res < 0) {
            ::spdlog::error("Failed to set ALSA record format. Error: {}", ALSADebug().evaluateErrorS(res));
            return;
        }
        if (const auto res = capture_dev.setAccess(SND_PCM_ACCESS_RW_INTERLEAVED); res < 0) {
            ::spdlog::error("Failed to set sound card access mode. Error: {}", ALSADebug().evaluateErrorS(res));
            return;
        }
        if (const auto res = capture_dev.setChannels(this->channel_count); res < 0) {
            ::spdlog::error("Failed to set recording channel count to {}. Error: {}", this->channel_count, ALSADebug().evaluateErrorS(res));
            return;
        }

        const int latency = (this->sampler_rate / 1000) * this->record_block_duration_ms;
        Eigen::Array<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> buffer((int) latency, (int) this->channel_count);

        if (const auto res = capture_dev.start(); res < 0) {
            ::spdlog::error("Error while starting audio capture: {}.", ALSADebug().evaluateErrorS(res));
            return;
        }

        snd_pcm_uframes_t pSize;
        capture_dev.getPeriodSize(&pSize);

        ::spdlog::info("Successfully opened device {} for capturing audio with format {}, {} channels, period size {} and a sampler rate of {}.",
                       this->sound_dev_file, capture_dev.getFormatName(format), capture_dev.getChannels(), pSize, this->sampler_rate);

        while(this->running) {
            capture_dev >> buffer;
            // TODO analyze input
        }
    }
}