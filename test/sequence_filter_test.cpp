//
// Created by leondietrich on 2/15/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE FISH_TESTS
#include <boost/test/included/unit_test.hpp>

#include "events/event.hpp"
#include "events/event_source.hpp"
#include "filters/sequencer/filter_sequencer.hpp"

#include <chrono>
#include <iostream>
#include <sstream>

#include "../test/iomanager_test_fixture.hpp"

using namespace dmxfish::filters;
using namespace dmxfish::events;

BOOST_AUTO_TEST_CASE(test_initialization_and_update) {
    // Construct input channels

    double current_time = 0.5;
    double time_scale = 1.0;
    const auto t_e_sender = dmxfish::events::event_source::create<dmxfish::events::event_source>(get_event_storage_instance());
    channel_mapping input_cm, output_cm;
    input_cm.float_channels["time"] = &current_time;
    input_cm.float_channels["time_scale"] = &time_scale;

    std::map<std::string, std::string> configuration;
    std::map<std::string, std::string> init_params;
    static const std::string filter_id = "test_sequencer_filter";

    // Construct one out channel per data type
    configuration["channels"] = "channel_8b:8bit:0:true:true:average;channel_16b:16bit:1024:true:true:max;"
                                "channel_f:float:0.5:false:true:average;channel_c:color:120.0,0.5,0.5:true:true:min";

    // Construct one Transition using all channels
    constexpr auto event_id_tall = 0;
    std::stringstream transition_builder;
    transition_builder << event_id_tall << ":0#T1";
    for(auto i = 0; i < 3; i++) {
        transition_builder << '#';
        transition_builder << "channel_8b:" << (60*i) << ":lin:120.0" << '#';
        transition_builder << "channel_16b:" << (1024*i) << ":lin:120.0" << '#';
        transition_builder << "channel_f:" << (0.25*i) << ":lin:120.0" << '#';
        transition_builder << "channel_c:" << (60.0*i) << ",1.0," << (0.3*(i+1)) << ":lin:120.0";
    }

    // Construct one transition using only two channels
    constexpr auto event_id_tsingle = 1;
    transition_builder << ';' << event_id_tsingle << ":0#T2";
    for(auto i = 0; i < 3; i++) {
        transition_builder << '#';
        transition_builder << "channel_8b:" << (60*i) << ":edg:100.0" << '#';
        transition_builder << "channel_c:" << (60.0*i) << ",1.0," << (0.3*i) << ":e_o:100.0";
    }

    configuration["transitions"] = transition_builder.str();
    filter_sequencer fs;
    fs.pre_setup(configuration, init_params, filter_id);
    fs.get_output_channels(output_cm, filter_id);
    auto* channel_8b = output_cm.eight_bit_channels[filter_id + ":channel_8b"];
    auto* channel_16b = output_cm.sixteen_bit_channels[filter_id + ":channel_16b"];
    auto* channel_f = output_cm.float_channels[filter_id + ":channel_f"];
    auto* channel_c = output_cm.color_channels[filter_id + ":channel_c"];
    fs.setup_filter(configuration, init_params, input_cm, filter_id);
    const auto start_time = std::chrono::high_resolution_clock::now();
    fs.scene_activated();

    get_event_storage_instance()->swap_buffers();
    fs.update();
    BOOST_CHECK_EQUAL(*channel_8b, 0);
    BOOST_CHECK_EQUAL(*channel_16b, 1024);
    BOOST_CHECK(*channel_f > 0.45 && *channel_f < 0.55);
    BOOST_CHECK(channel_c->getHue() >= 120.0 - 0.5 && channel_c->getHue() <= 120.0 + 0.5);

    // test full channel transition
    get_event_storage_instance()->insert_event(event(event_type::SINGLE_TRIGGER, event_sender_t(t_e_sender->get_sender_id(), event_id_tall)));
    get_event_storage_instance()->swap_buffers();
    current_time += 40.0;
    fs.update();
    get_event_storage_instance()->swap_buffers();
    current_time += 5.0;
    fs.update();
    current_time += 40;
    fs.update();
    std::cout << "1 - 16b: " << ((int) *channel_16b) << " f: " << *channel_f << " c: " << std::to_string(*channel_c) << std::endl;
    BOOST_CHECK_EQUAL(*channel_8b, 0);
    BOOST_CHECK(*channel_16b > 600 && *channel_16b < 690);
    BOOST_CHECK(*channel_f > 0.3 && *channel_f < 0.6);
    BOOST_CHECK(channel_c->getHue() >= 45.0- 0.5 && channel_c->getHue() <= 45.0 + 0.5);

    current_time += 40.1;
    get_event_storage_instance()->swap_buffers();
    fs.update();
    std::cout << "2 - 16b: " << ((int) *channel_16b) << " f: " << *channel_f << " c: " << std::to_string(*channel_c) << std::endl;
    BOOST_CHECK_EQUAL(*channel_8b, 0);
    BOOST_CHECK(*channel_16b > 200 && *channel_16b < 350);
    BOOST_CHECK(*channel_f > 0.1 && *channel_f < 0.2);
    BOOST_CHECK(channel_c->getHue() >= 80.0 - 0.5 && channel_c->getHue() <= 90.0 + 0.5);

    current_time += 40.0;
    get_event_storage_instance()->swap_buffers();
    fs.update();
    std::cout << "3 - 16b: " << ((int) *channel_16b) << " f: " << *channel_f << " c: " << std::to_string(*channel_c) << std::endl;
    BOOST_CHECK_EQUAL(*channel_8b, 0);
    BOOST_CHECK(*channel_16b > 250 && *channel_16b < 400);
    BOOST_CHECK(*channel_f < 0.25);
    BOOST_CHECK(channel_c->getHue() >= 55.0 && channel_c->getHue() <= 65.0);

    //Next frame from transition
    current_time += 40.0;
    get_event_storage_instance()->swap_buffers();
    fs.update();
    std::cout << "4 - 16b: " << ((int) *channel_16b) << " f: " << *channel_f << " c: " << std::to_string(*channel_c) << std::endl;
    BOOST_CHECK_EQUAL(*channel_8b, 20);
    BOOST_CHECK(*channel_16b > 500 && *channel_16b < 650);
    BOOST_CHECK(*channel_f < 0.2);
    BOOST_CHECK(channel_c->getHue() >= 70.0 - 5.0 && channel_c->getHue() <= 70.0 + 5.0);

    current_time += 40.1;
    get_event_storage_instance()->swap_buffers();
    auto last_value_f = *channel_f;
    fs.update();
    std::cout << "5 - 16b: " << ((int) *channel_16b) << " f: " << *channel_f << " c: " << std::to_string(*channel_c) << std::endl;
    BOOST_CHECK_EQUAL(*channel_8b, 40);
    BOOST_CHECK(*channel_16b > 700 && *channel_16b < 900);
    BOOST_CHECK(*channel_f > last_value_f);
    BOOST_CHECK(channel_c->getHue() >= 75.0 - 0.5 && channel_c->getHue() <= 80.0 + 0.5);

    current_time += 40.000;
    get_event_storage_instance()->swap_buffers();
    const auto last_16b_value = *channel_16b;
    last_value_f = *channel_f;
    fs.update();
    std::cout << "6 - 16b: " << ((int) *channel_16b) << " f: " << *channel_f << " c: " << std::to_string(*channel_c) << std::endl;
    BOOST_CHECK_EQUAL(*channel_8b, 40);
    BOOST_CHECK_EQUAL(*channel_16b, last_16b_value);
    BOOST_CHECK_EQUAL(*channel_f, last_value_f);
    BOOST_CHECK(channel_c->getHue() >= 120.0 - 0.5 && channel_c->getHue() <= 120.0 + 0.5);

    // TODO test smaller transition with different time scale

    // TODO run updates with both running in parallel.
    const auto end_time = std::chrono::high_resolution_clock::now();
    ::spdlog::info("Running base test took {} milliseconds.", std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count());
}

BOOST_AUTO_TEST_CASE(test_linear_update) {
    const auto t_e_sender = dmxfish::events::event_source::create<dmxfish::events::event_source>(get_event_storage_instance());
    // Construct input channels

    double current_time = 0.5;
    double time_scale = 1.0;
    channel_mapping input_cm, output_cm;
    input_cm.float_channels["time"] = &current_time;
    input_cm.float_channels["time_scale"] = &time_scale;

    std::map<std::string, std::string> configuration;
    std::map<std::string, std::string> init_params;
    static const std::string filter_id = "test_sequencer_filter";

    // Construct one out channel per data type
    configuration["channels"] = "channel_8b:8bit:0:true:true:max";
    configuration["transitions"] = std::to_string(t_e_sender->get_sender_id()) + ":0:1:0##channel_8b:255:lin:260";

    filter_sequencer fs;
    fs.pre_setup(configuration, init_params, filter_id);
    fs.get_output_channels(output_cm, filter_id);
    auto* channel_8b = output_cm.eight_bit_channels[filter_id + ":channel_8b"];
    fs.setup_filter(configuration, init_params, input_cm, filter_id);
    fs.scene_activated();
    fs.update();
    get_event_storage_instance()->insert_event(event(event_type::SINGLE_TRIGGER, event_sender_t(t_e_sender->get_sender_id(), 0)));
    int last_value = 0;
    for (current_time = 10.0; current_time <= 275.0; current_time += 10.0) {
        get_event_storage_instance()->swap_buffers();
        fs.update();
        bool value_range_correct = ((*channel_8b > last_value && *channel_8b < (last_value + 15)) || (*channel_8b > 240) || (*channel_8b == 0 && current_time < 15.0));
        BOOST_CHECK(value_range_correct);
        if(!value_range_correct) {
            std::cout << ((int) (*channel_8b)) << " does not have correct size at " << current_time << std::endl;
        }
        last_value = *channel_8b;
    }
}

BOOST_AUTO_TEST_CASE(test_time_scale_behavior) {
    // Regression test for sequencer time_scale handling.
    //
    // Previously, inputs other than 1.0 made the filter appear "stuck" because
    // the raw (wall-clock) time was multiplied by time_scale on each tick and
    // then compared/subtracted against keyframe start-times that lived in a
    // different unit system. The fix accumulates a per-filter virtual_time from
    // (delta_wall_time * max(0, time_scale)); this test exercises the key
    // invariants that follow from that design:
    //
    //   * time_scale > 1 actually speeds up transitions (not freezes them).
    //   * Changing time_scale mid-flight is continuous (no jumps).
    //   * time_scale == 0 pauses.
    //   * time_scale < 0 is clamped to 0 (also pauses).
    //   * scene_activated resets the clock so a large wall time doesn't blow
    //     up virtual_time on the first tick.
    const auto t_e_sender = dmxfish::events::event_source::create<dmxfish::events::event_source>(get_event_storage_instance());

    // Start wall time at a non-trivial value so a regression that does
    // `virtual_time = current_time * time_scale` (rather than accumulating
    // scaled deltas) would overshoot by hundreds of ms on tick one.
    double current_time = 1000.0;
    double time_scale = 1.0;
    channel_mapping input_cm, output_cm;
    input_cm.float_channels["time"] = &current_time;
    input_cm.float_channels["time_scale"] = &time_scale;

    std::map<std::string, std::string> configuration;
    std::map<std::string, std::string> init_params;
    static const std::string filter_id = "test_time_scale_filter";

    // default_on_empty = false so the output latches at the last transition
    // value once the queue drains, instead of snapping back to 0.
    configuration["channels"] = "channel_8b:8bit:0:false:true:max";
    // Single linear transition: 0 -> 200 over 400ms.
    configuration["transitions"] = std::to_string(t_e_sender->get_sender_id()) + ":0:1:0##channel_8b:200:lin:400";

    filter_sequencer fs;
    fs.pre_setup(configuration, init_params, filter_id);
    fs.get_output_channels(output_cm, filter_id);
    auto* channel_8b = output_cm.eight_bit_channels[filter_id + ":channel_8b"];
    fs.setup_filter(configuration, init_params, input_cm, filter_id);
    time_scale = 2.0;
    fs.scene_activated();

    // Helper: swap_buffers must be called every tick so that the trigger event
    // is only visible on the first update after insertion. Otherwise the event
    // would stay in the readable buffer and keep re-enqueueing (and resetting)
    // the transition each tick, pinning elapsed at 0.
    const auto tick = [&](double dt_ms) {
        current_time += dt_ms;
        get_event_storage_instance()->swap_buffers();
        fs.update();
    };

    // ---- Phase 1: time_scale = 2.0 progresses the transition at double rate ----
    get_event_storage_instance()->insert_event(event(event_type::SINGLE_TRIGGER, event_sender_t(t_e_sender->get_sender_id(), 0)));
    tick(20.0);  // event becomes visible, transition enqueued; elapsed = 0 on this tick
    BOOST_CHECK_EQUAL(*channel_8b, 0);

    // 5 further 20ms real-time ticks => 100ms real => 200ms virtual
    // => interleave ~= 200/400 = 0.5 => value ~= 100.
    for (int i = 0; i < 5; i++) {
        tick(20.0);
    }
    // Lower bound is the real teeth of this test: before the fix, the value
    // was stuck at 0 for time_scale > 1 (interleave clamped to 0 by the broken
    // elapsed computation).
    BOOST_CHECK(*channel_8b > 70);
    BOOST_CHECK(*channel_8b < 130);

    // ---- Phase 2: time_scale change mid-flight is continuous, not a jump ----
    const auto value_before_change = *channel_8b;
    time_scale = 0.5;
    tick(20.0);  // 20ms real at 0.5x => +10ms virtual
    // A naive "scale raw wall time by time_scale each tick" implementation
    // would discontinuously jump the virtual timeline here; the accumulator
    // only scales the *new* delta.
    BOOST_CHECK(*channel_8b >= value_before_change);
    BOOST_CHECK(*channel_8b < value_before_change + 20);

    // ---- Phase 3: time_scale == 0 pauses interpolation ----
    time_scale = 0.0;
    const auto frozen_value = *channel_8b;
    for (int i = 0; i < 10; i++) {
        tick(50.0);
    }
    BOOST_CHECK_EQUAL(*channel_8b, frozen_value);

    // ---- Phase 4: negative time_scale is clamped to 0 (still paused) ----
    time_scale = -3.0;
    for (int i = 0; i < 10; i++) {
        tick(50.0);
    }
    BOOST_CHECK_EQUAL(*channel_8b, frozen_value);

    // ---- Phase 5: resume at 1.0; transition completes near the target ----
    time_scale = 1.0;
    for (int i = 0; i < 50; i++) {
        tick(10.0);
    }
    BOOST_CHECK(*channel_8b > frozen_value + 50);
    BOOST_CHECK(*channel_8b <= 200);
}
