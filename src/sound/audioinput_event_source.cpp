//
// Created by Doralitze on 11.05.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "audioinput_event_source.hpp"

#include <array>
#include <Eigen/Dense>

#include "events/event.hpp"
#include "events/event_storage.hpp"
#include "sound/ALSA/ALSA.H"
#include "sound/ALSA/Capture.H"
#include "sound/fft.hpp"

#include "lib/logging.hpp"
#include "main.hpp"

namespace dmxfish::audio {

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
        auto conf = msg.mutable_configuration();
        conf->operator[]("dev") = this->sound_dev_file;
        conf->operator[]("high_cut") = std::to_string(this->high_cutoff_frequency);
        conf->operator[]("low_cut") = std::to_string(this->low_cutoff_frequency);
        conf->operator[]("magnitude") = std::to_string(this->trigger_magnitude);
        conf->operator[]("fft_window_size") = "1024";
        conf->operator[]("channel_count") = std::to_string(this->channel_count);
        conf->operator[]("sampler_rate") = std::to_string(this->sampler_rate);
        conf->operator[]("sample_duration") = std::to_string(this->record_block_duration_ms);
        return msg;
    }

    bool audioinput_event_source::update_conf_from_message(const missiondmx::fish::ipcmessages::event_sender& msg) {
        auto conf = msg.configuration();
	if (conf.contains("high_cut")) {
            this->high_cutoff_frequency = std::stoi(conf["high_cut"]);
        }
	if (conf.contains("low_cut")) {
            this->low_cutoff_frequency = std::stoi(conf["low_cut"]);
	}
	if (conf.contains("magnitude")) {
            this->trigger_magnitude = std::stod(conf["magnitude"]);
	}
        const auto new_channel_count = conf.contains("channel_count") ? std::stoi(conf["channel_count"]) : this->channel_count;
        const auto new_sampler_rate = conf.contains("sampler_rate") ? std::stoi(conf["sampler_rate"]) : this->sampler_rate;
        const auto new_duration = conf.contains("sample_duration") ? std::stoi(conf["sample_duration"]) : this->record_block_duration_ms;
        if (conf["dev"] == this->sound_dev_file
                && this->channel_count == new_channel_count
                && this->sampler_rate == new_sampler_rate
                && this->record_block_duration_ms == new_duration) {
            return true;
        }
        this->running = false;
        if (this->thread.has_value()) {
            this->thread->join();
        }
        this->running = true;
        this->sound_dev_file = conf.contains("dev") ? conf["dev"] : "default";
        this->channel_count = new_channel_count;
        this->sampler_rate = new_sampler_rate;
        this->record_block_duration_ms = new_duration;
        this->thread = std::thread(&audioinput_event_source::update_task, this);
        return true;
    }

    void audioinput_event_source::update_task() {
        using namespace ALSA;

        if(!this->running) {
            ::spdlog::info("Leaving audio extraction thread early.");
            return;
        } else {
            ::spdlog::debug("Starting new audio extraction thread");
        }

        if(this->channel_count < 1) {
            ::spdlog::error("Beat analysis requires at least 1 input channel. {} were configured.", this->channel_count);
            return;
        }

        Capture capture_dev(this->sound_dev_file.c_str());
        if (capture_dev.prepared()) {
            ::spdlog::error("Failed to open sound input device {}.", this->sound_dev_file);
            return;
        } else {
            ::spdlog::info("Opened sound device '{}' from '{}'.", capture_dev.getDeviceName(), this->sound_dev_file);
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

        size_t initial_fft_buffer_pos = 0;

        auto event_storage = get_event_storage_instance();
        fft_context ctx;
        std::array<double, fft_size> post_buffer;

        while(this->running) {
            capture_dev >> buffer;
            if (buffer.rows() == 0) {
                continue;
            }
            // A better approach than this is http://werner.yellowcouch.org/Papers/bpm04/
            // However implementing this on roalling data needs some math which I don't have
            // the time for at the moment. Therefore FFT it is.

            auto remaining_elements_in_buffer = buffer.rows();

            // TODO use windows instead of blocks
            while (remaining_elements_in_buffer > ctx.fft_buffer.size()) {
                // load buffer content
                for (auto i = initial_fft_buffer_pos; i < fft_size; i++) {
                    double avg = 0;
                    for (auto j = 0; j < buffer.cols(); j++) {
                        avg += buffer(buffer.rows() - remaining_elements_in_buffer, j);
                    }
                    ctx.fft_buffer[i*2] = avg / buffer.cols();
                    remaining_elements_in_buffer--;
                }
                initial_fft_buffer_pos = 0;

                // analyze buffer
                fft(ctx);

                for (size_t i=0; i < fft_size; i++) {
                    const auto pos_real = 2*i;
                    const auto pos_imag = 2*i+1;
                    post_buffer[i] = ctx.out_buffer[pos_real] * ctx.out_buffer[pos_real] + ctx.out_buffer[pos_imag] * ctx.out_buffer[pos_imag];
                }

                double bassIntensity = 0;
                for (auto i = this->low_cutoff_frequency; i < this->high_cutoff_frequency; i++){
                    bassIntensity += post_buffer[i];
                }
                if (bassIntensity > this->trigger_magnitude) {
                    dmxfish::events::event e(dmxfish::events::event_type::SINGLE_TRIGGER,
                                             dmxfish::events::event_sender_t{this->get_sender_id(), 0});
                    event_storage->insert_event(e);
                }
            }

            // preload buffer for next iteration and update initial_fft_buffer_pos
            while(remaining_elements_in_buffer > 0) {
                double avg = 0;
                for (auto j = 0; j < buffer.cols(); j++) {
                    avg += buffer(buffer.rows() - remaining_elements_in_buffer, j);
                    remaining_elements_in_buffer--;
                }
                ctx.fft_buffer[initial_fft_buffer_pos++] = avg / buffer.cols();
            }
        }
        ::spdlog::debug("Leaving audio extraction thread");
    }
}
