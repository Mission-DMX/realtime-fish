//
// Created by Doralitze on 11.05.25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include "audioinput_event_source.hpp"

#include <array>
#include <Eigen/Dense>
#include <limits>

#include "events/event.hpp"
#include "events/event_storage.hpp"
#include "sound/ALSA/ALSA.H"
#include "sound/ALSA/Capture.H"
#include "sound/BTrack.hpp"
#include "sound/fft.hpp"
#include "sound/pulse.hpp"

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
	conf->operator[]("use_alsa_directly") = this->use_alsa_directly ? "true" : "false";
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
	const bool new_alsa_use = conf.contains("use_alsa_directly") ? (conf["use_alsa_directly"] == "true") : false;
        if (conf["dev"] == this->sound_dev_file
                && this->channel_count == new_channel_count
                && this->sampler_rate == new_sampler_rate
		&& this->use_alsa_directly == new_alsa_use
                && this->record_block_duration_ms == new_duration) {
            return true;
        }
        this->running = false;
        if (this->thread.has_value()) {
            this->thread->join();
        }
	this->use_alsa_directly = new_alsa_use;
        this->sound_dev_file = conf.contains("dev") ? conf["dev"] : "default";
        this->channel_count = new_channel_count;
        this->sampler_rate = new_sampler_rate;
        this->record_block_duration_ms = new_duration;
        if(this->sound_dev_file == "") {
            ::spdlog::info("Stopping audio analysis.");
            this->thread = std::nullopt;
            return true;
        }
        this->running = true;
        if (this->use_alsa_directly) {
            this->thread = std::thread(&audioinput_event_source::update_task_alsa, this);
        } else {
            this->thread = std::thread(&audioinput_event_source::update_task_pulse, this);
        }
        return true;
    }

    bool audioinput_event_source::common_init() {
	if(!this->running) {
            ::spdlog::info("Leaving audio extraction thread early.");
            return false;
        } else {
            ::spdlog::debug("Starting new audio extraction thread");
        }

        if(this->channel_count < 1) {
            ::spdlog::error("Beat analysis requires at least 1 input channel. {} were configured.", this->channel_count);
            return false;
        }
	return true;
    }

    struct detection_parameters {
	    double low_cutoff_frequency;
	    double high_cutoff_frequency;
	    double trigger_magnitude;
	    uint32_t sender_id;
    };

    void process(Eigen::Array<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>& buffer,
                 fft_context& ctx,
                 size_t& initial_fft_buffer_pos,
                 std::array<double, fft_size>& post_buffer,
                 const detection_parameters& params) {
	    auto event_storage = get_event_storage_instance();
            if (buffer.rows() == 0) {
                return;
            }
            // A better approach than this is http://werner.yellowcouch.org/Papers/bpm04/
            // However implementing this on roalling data needs some math which I don't have
            // the time for at the moment. Therefore FFT it is.

            auto remaining_elements_in_buffer = buffer.rows();

            // TODO use windows instead of blocks
            while (remaining_elements_in_buffer + initial_fft_buffer_pos > ctx.fft_buffer.size()) {
                // load buffer content
                for (auto i = initial_fft_buffer_pos; i < fft_size; i++) {
                    double avg = 0;
                    for (auto j = 0; j < buffer.cols(); j++) {
                        avg += ((double) buffer(buffer.rows() - remaining_elements_in_buffer, j)) / ((double) std::numeric_limits<int>::max() / 10);
                    }
                    ctx.fft_buffer[i*2] = avg / buffer.cols();
                    remaining_elements_in_buffer--;
                }
                initial_fft_buffer_pos = 0;

                // analyze buffer
                fft(ctx);

                for (size_t i=0; i < post_buffer.size(); i++) {
                    const auto pos_real = 2*i;
                    const auto pos_imag = 2*i+1;
                    post_buffer[i] = ctx.out_buffer[pos_real] * ctx.out_buffer[pos_real] + ctx.out_buffer[pos_imag] * ctx.out_buffer[pos_imag];
                }

                double bassIntensity = 0;
                for (auto i = params.low_cutoff_frequency; i < params.high_cutoff_frequency; i++){
                    bassIntensity += post_buffer[i];
                }
                if (bassIntensity >= params.trigger_magnitude) {
                    dmxfish::events::event e(dmxfish::events::event_type::SINGLE_TRIGGER,
                                             dmxfish::events::event_sender_t{params.sender_id, 0});
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

    void audioinput_event_source::update_task_alsa() {
        using namespace ALSA;

        if(!this->common_init()) { return; }

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
	    std::array<double, fft_size> post_buffer;
        fft_context ctx; 

        while(this->running) {
            capture_dev >> buffer;
	    detection_parameters params;
	    params.high_cutoff_frequency = this->high_cutoff_frequency;
	    params.low_cutoff_frequency = this->low_cutoff_frequency;
	    params.trigger_magnitude = this->trigger_magnitude;
	    params.sender_id = this->get_sender_id();
            process(buffer, ctx, initial_fft_buffer_pos, post_buffer, params);
        }
        ::spdlog::debug("Leaving ALSA audio extraction thread.");
    }

    void audioinput_event_source::update_task_pulse() {
	if(!this->common_init()) { return; }
	using namespace YukiWorkshop;

	SimplePA::Recorder r;
	r.set_name("fish.builtin.audioextract");
	r.set_device(this->sound_dev_file);
	r.set_sample_spec({PA_SAMPLE_S32LE, this->sampler_rate, this->channel_count});
	try {
	    r.open();
	    ::spdlog::info("Opened sound device for analysis.");
	    
	    const auto latency = (this->sampler_rate * this->record_block_duration_ms) / 1000;
        const auto channel_count = this->channel_count;
	    std::vector<int32_t> in_buf;
        std::vector<double> out_buf;
        in_buf.reserve(channel_count * latency);
        out_buf.reserve(latency);
        for (auto i = 0; i < channel_count * latency; i++) {
            in_buf.push_back(0);
        }
        for(auto i = 0; i < latency; i++) {
            out_buf.push_back(0.0);
        }
        BTrack b(512, out_buf.size());
        auto event_storage = get_event_storage_instance();
	    
	    while(this->running) {
            r.record(in_buf.data(), in_buf.size());
            for (int i = 0; i < latency; i++) {
                double avg = 0.0;
                for (int j = 0; j < channel_count; j++) {
                    avg += (in_buf[(channel_count * i) + j] / (std::numeric_limits<int32_t>::max() / 1000));
                }
                out_buf[i] = avg / channel_count;
            }
            b.processAudioFrame(out_buf.data());
            if (b.beatDueInCurrentFrame()) {
                dmxfish::events::event e(dmxfish::events::event_type::SINGLE_TRIGGER,
                                         dmxfish::events::event_sender_t{this->get_sender_id(), 0});
                event_storage->insert_event(e);
            }
	    }
	} catch (std::runtime_error& e) {
	    ::spdlog::error("Failed to use pulse stream: {}", e.what());
	}
	::spdlog::info("Leaving Pulse audio extraction thread.");
    }
}
