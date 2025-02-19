//
// Created by leondietrich on 2/15/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE FISH_TESTS
#include <boost/test/included/unit_test.hpp>

#include "filters/sequencer/filter_sequencer.hpp"

#include <sstream>

#include "../test/iomanager_test_fixture.hpp"

using namespace dmxfish::filters;

BOOST_AUTO_TEST_CASE(test_initialization_and_update) {
    // Construct input channels

    double current_time = 0.0;
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
        transition_builder << "channel_16b:" << (1024*i) << ":edg:120.0" << '#';
        transition_builder << "channel_f:" << (0.25*i) << ":e_i:120.0" << '#';
        transition_builder << "channel_c:" << (60.0*i) << ",1.0," << (0.3*i) << ":e_o:120.0";
    }

    // Construct one transition using only two channels
    constexpr auto event_id_tsingle = 1;
    transition_builder << ';' << event_id_tsingle;
    for(auto i = 0; i < 3; i++) {
        transition_builder << '#';
        transition_builder << "channel_8b:" << (60*i) << ":lin:100.0" << '#';
        transition_builder << "channel_c:" << (60.0*i) << ",1.0," << (0.3*i) << ":e_o:100.0";
    }

    configuration["transitions"] = transition_builder.str();
    filter_sequencer fs;
    fs.pre_setup(configuration, init_params, filter_id);
    fs.get_output_channels(output_cm, filter_id);
    fs.setup_filter(configuration, init_params, input_cm, filter_id);
    fs.scene_activated();

    // TODO run updates with individual transition executions (using event insertion), one of them with different time_scale
    // TODO run updates with both running in parallel.
}