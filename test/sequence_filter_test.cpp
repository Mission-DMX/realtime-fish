//
// Created by leondietrich on 2/15/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE FISH_TESTS
#include <boost/test/included/unit_test.hpp>

#include "events/event.hpp"
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
    transition_builder << event_id_tall;
    for(auto i = 0; i < 3; i++) {
        transition_builder << '#';
        transition_builder << "channel_8b:" << (60*i) << ":lin:120.0" << '#';
        transition_builder << "channel_16b:" << (1024*i) << ":lin:120.0" << '#';
        transition_builder << "channel_f:" << (0.25*i) << ":lin:120.0" << '#';
        transition_builder << "channel_c:" << (60.0*i) << ",1.0," << (0.3*i) << ":lin:120.0";
    }

    // Construct one transition using only two channels
    constexpr auto event_id_tsingle = 1;
    transition_builder << ';' << event_id_tsingle;
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
    get_event_storage_instance()->insert_event(event(event_type::SINGLE_TRIGGER, event_sender_t(0, event_id_tall)));
    get_event_storage_instance()->swap_buffers();
    current_time += 40.0;
    fs.update();
    current_time += 5.0;
    fs.update();
    BOOST_CHECK_EQUAL(*channel_8b, 0);
    BOOST_CHECK(*channel_16b > 680 && *channel_16b < 690);
    BOOST_CHECK(*channel_f > 0.3 && *channel_f < 0.4);
    BOOST_CHECK(channel_c->getHue() <= 0.0 + 0.5);

    current_time += 40.1;
    get_event_storage_instance()->swap_buffers();
    fs.update();
    BOOST_CHECK_EQUAL(*channel_8b, 0);
    BOOST_CHECK(*channel_16b > 200 && *channel_16b < 350);
    BOOST_CHECK(*channel_f > 0.1 && *channel_f < 0.2);
    BOOST_CHECK(channel_c->getHue() >= 40.0 - 0.5 && channel_c->getHue() <= 40.0 + 0.5);

    current_time += 40.0;
    get_event_storage_instance()->swap_buffers();
    fs.update();
    BOOST_CHECK_EQUAL(*channel_8b, 0);
    BOOST_CHECK(*channel_16b < 10);
    BOOST_CHECK(*channel_f < 0.125);
    BOOST_CHECK(channel_c->getHue() <= 0.5);

    //Next frame from transition
    current_time += 40.0;
    get_event_storage_instance()->swap_buffers();
    fs.update();
    BOOST_CHECK_EQUAL(*channel_8b, 20);
    BOOST_CHECK(*channel_16b > 335 && *channel_16b < 350);
    BOOST_CHECK(*channel_f < 0.2);
    BOOST_CHECK(channel_c->getHue() >= 20.0 - 0.5 && channel_c->getHue() <= 20.0 + 0.5);

    current_time += 40.1;
    get_event_storage_instance()->swap_buffers();
    fs.update();
    BOOST_CHECK_EQUAL(*channel_8b, 40);
    BOOST_CHECK(*channel_16b > 680 && *channel_16b < 690);
    BOOST_CHECK(*channel_f > 0.1 && *channel_f < 0.2);
    BOOST_CHECK(channel_c->getHue() >= 40.0 - 0.5 && channel_c->getHue() <= 40.0 + 0.5);

    current_time += 40.000;
    get_event_storage_instance()->swap_buffers();
    fs.update();
    BOOST_CHECK_EQUAL(*channel_8b, 60);
    BOOST_CHECK(*channel_16b > 1022 && *channel_16b < 1026);
    BOOST_CHECK(*channel_f > 0.24 && *channel_f < 0.26);
    BOOST_CHECK(channel_c->getHue() >= 60.0 - 0.5 && channel_c->getHue() <= 60.0 + 0.5);

    // TODO test smaller transition with different time scale

    // TODO run updates with both running in parallel.
    const auto end_time = std::chrono::high_resolution_clock::now();
    ::spdlog::info("Running base test took {} milliseconds.", std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count());
}

BOOST_AUTO_TEST_CASE(test_linear_update) {
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
    configuration["transitions"] = "0#channel_8b:255:lin:260";

    filter_sequencer fs;
    fs.pre_setup(configuration, init_params, filter_id);
    fs.get_output_channels(output_cm, filter_id);
    auto* channel_8b = output_cm.eight_bit_channels[filter_id + ":channel_8b"];
    fs.setup_filter(configuration, init_params, input_cm, filter_id);
    fs.scene_activated();
    fs.update();
    get_event_storage_instance()->insert_event(event(event_type::SINGLE_TRIGGER, event_sender_t(0, 0)));
    int last_value = 0;
    for (current_time = 10.0; current_time <= 275.0; current_time += 10.0) {
        get_event_storage_instance()->swap_buffers();
        fs.update();
        bool value_range_correct = ((*channel_8b > last_value && *channel_8b < (last_value + 15)) || (*channel_8b > 240));
        BOOST_CHECK(value_range_correct);
        if(!value_range_correct) {
            std::cout << ((int) (*channel_8b)) << " does not have correct size at " << current_time << std::endl;
        }
        last_value = *channel_8b;
    }
}