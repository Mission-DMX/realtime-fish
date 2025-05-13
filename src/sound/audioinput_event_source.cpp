//
// Created by Doralitze on 11.05.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "audioinput_event_source.hpp"

#include <array>
#include <Eigen/Dense>

#include "event/event_storage.hpp"
#include "sound/ALSA/ALSA.H"
#include "sound/ALSA/Capture.H"

#include "lib/logging.hpp"
#include "main.hpp"

namespace dmxfish::audio {

    constexpr auto fft_size = 1024;

    audioinput_event_source::audioinput_event_source() {

    }

    audioinput_event_source::~audioinput_event_source() {
        this->running = false;
        if (this->thread.has_value()) {
            this->thread->join();
        }
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
        // TODO update cutoff and magnitude settings
        auto conf = msg.configuration();
        if (conf["dev"] == this->sound_dev_file
                && channel_count == std::stoi(conf["channel_count"])
                && sampler_rate == std::stoi(conf["sampler_rate"])
                && record_block_duration_ms == std::stoi(conf["sample_duration"])) {
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

        if(!this->running) {
            return;
        }

        if(this->channel_count < 1) {
            ::spdlog::error("Beat analysis requires at least 1 input channel. {} were configured.", this->channel_count);
            return;
        }

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

        ::spdlog::info("Successfully initialized device {} for capturing audio with format {}, {} channels, period size {} and a sampler rate of {}.",
                       this->sound_dev_file, capture_dev.getFormatName(format), capture_dev.getChannels(), pSize, this->sampler_rate);
        std::array<double, fft_size> fft_buffer;
        size_t initial_fft_buffer_pos = 0;

        auto event_storage = get_event_storage_instance();

        while(this->running) {
            capture_dev >> buffer;
            if (buffer.rows() == 0) {
                continue;
            }
            // A better approach than this is http://werner.yellowcouch.org/Papers/bpm04/
            // However implementing this on roalling data needs some math which I don't have
            // the time for at the moment. Therefore FFT it is.

            remaining_elements_in_buffer = buffer.rows();
            while (remaining_elements_in_buffer > fft_buffer.size()) {
                // load buffer content
                for (auto i = initial_fft_buffer_pos; i < fft_buffer.size(); i++) {
                    double avg = 0;
                    for (auto j = 0; j < buffer.cols(); j++) {
                        avg += buffer[buffer.rows() - remaining_elements_in_buffer][j];
                    }
                    fft_buffer[i] = avg / buffer.cols();
                    remaining_elements_in_buffer--;
                }
                initial_fft_buffer_pos = 0;

                // analyze buffer
                std::array<double, fft_size> real, imag;
                fft(fft_buffer, real, imag);

                for (auto i=0; i < real.size(); i++) {
                    real[i] = real[i] * real[i];
                }
                for (auto i=0; i < imag.size(); i++) {
                    imag[i] = imag[i] * imag[i];
                }

                double bassIntensity = 0;
                for (i=this->low_cutoff_frequency; i < this->high_cutoff_frequency; i++){
                    bassIntensity += real[i] + imag[i];
                }
                if (bassIntensity > this->trigger_magnitude) {
                    dmxfish::events::event e(this->get_sender_id(), 0);
                    event_storage->insert_event(e);
                }
            }

            // preload buffer for next iteration and update initial_fft_buffer_pos
            while(remaining_elements_in_buffer > 0) {
                double avg = 0;
                for (auto j = 0; j < buffer.cols(); j++) {
                    avg += buffer[buffer.rows() - remaining_elements_in_buffer][j];
                    remaining_elements_in_buffer--;
                }
                fft_buffer[initial_fft_buffer_pos++] = avg / buffer.cols();
            }
        }
    }

    void fft(const std::array<double, fft_size>& in_buffer, std::array<double, fft_size>& real, std::array<double, fft_size>& imag) {
        // TODO
    }
}