//
// Created by doralitze on 5/13/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE FISH_TESTS
#include <boost/test/included/unit_test.hpp>

#include <cstdlib>

#include "events/event.hpp"
#include "sound/audioinput_event_source.hpp"

#include "../test/iomanager_test_fixture.hpp"

BOOST_AUTO_TEST_CASE(event_count_test) {
    system("pactl load-module module-pipe-source source_name=virtmic file=/tmp/virtmic_test format=s16le rate=44100 channels=2");
    auto s_ptr = event_source::create<dmxfish::audio::audioinput_event_source>(storage_ptr, msg.name());
    // TODO create fake input
    // TODO generate config message
    // TODO run input using cat submodules/resources/testdata/test_extract_music_features.wav > /tmp/virtmic_test
    // TODO count events
    system("pactl unload-module module-pipe-source");
}

// TODO write test case for event count filter
