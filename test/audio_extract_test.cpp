//
// Created by doralitze on 5/13/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE FISH_TESTS
#include <boost/test/included/unit_test.hpp>

#include "events/event.hpp"
#include "sound/audioinput_event_source.hpp"

BOOST_AUTO_TEST_CASE(event_count_test) {
    // TODO setup main event storage
    auto s_ptr = event_source::create<dmxfish::audio::audioinput_event_source>(storage_ptr, msg.name());
    // TODO create fake input
    // TODO generate config message
    // TODO run input
    // TODO count events
}

// TODO write test case for event count filter
