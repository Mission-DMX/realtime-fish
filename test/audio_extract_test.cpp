//
// Created by doralitze on 5/13/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE FISH_TESTS
#include <boost/test/included/unit_test.hpp>

#include <chrono>
#include <cstdlib>
#include <thread>

#include "events/event.hpp"
#include "sound/audioinput_event_source.hpp"
#include "sound/fft.hpp"

#include "main.hpp"

#include "../test/iomanager_test_fixture.hpp"

struct audio_gf {
    /*
     * For debugging purposes: pactl load-module module-loopback routes micorphone input to the speakers
     */
    audio_gf() {
        dmxfish::audio::train_fft(true);
        ::spdlog::info("Creating virtual microphone");
        system("pactl load-module module-pipe-source source_name=virtmic file=/tmp/virtmic_test format=s16le rate=44100 channels=2");
    }
    ~audio_gf() {
        ::spdlog::info("Destroying virtual microphone");
        system("pactl unload-module module-pipe-source");
    }
};

BOOST_TEST_GLOBAL_FIXTURE(audio_gf);

BOOST_AUTO_TEST_CASE(event_count_test) {
    using namespace dmxfish::events;
    auto storage_ptr = get_event_storage_instance();
    auto s_ptr = event_source::create<dmxfish::audio::audioinput_event_source>(storage_ptr, "test-audio-extract-source");
    auto msg = s_ptr->encode_proto_message();
    std::cout << "loading conf" << std::endl;
    auto conf = msg.mutable_configuration();
    conf->operator[]("dev") = "virtmic";
    std::cout << "Received configuration:" << std::endl;
    for (auto& [k, v] : *conf) {
        std::cout << k << "=" << v << std::endl;
    }
    std::cout << "Writing conf" << std::endl;
    s_ptr->update_conf_from_message(msg);
    std::cout << "Playing test file" << std::endl;
    std::thread t([]() {system("timeout 30 sh -c 'cat submodules/resources/testdata/test_extract_music_features.wav > /tmp/virtmic_test'");});
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(20000ms);
    conf->operator[]("dev") = "";
    s_ptr->update_conf_from_message(msg);
    size_t count = 0;
    for (const auto& e : storage_ptr->get_storage()) {
        if (e.get_event_sender().decoded_representation.sender == s_ptr->get_sender_id()) {
            count++;
        }
    }
    std::cout << "\nCollected " << count << " beats." << std::endl;
    s_ptr = nullptr;
    t.join();
}

// TODO write test case for event count filter
