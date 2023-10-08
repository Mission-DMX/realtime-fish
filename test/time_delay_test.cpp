#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE FISH_TESTS
#include <boost/test/included/unit_test.hpp>
#include "filters/filter_time.hpp"
#include "lib/logging.hpp"

#include <sstream>

using namespace dmxfish::filters;

BOOST_AUTO_TEST_CASE(test_switch_on_delay_8bit) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::delay_switch_on_8bit fil = delay_switch_on_8bit();

    uint8_t in_channel = 200;
    double time_s = 2000;
    double time = time_s;
    uint8_t test = 0;

    channel_mapping input_channels = channel_mapping ();
    input_channels.eight_bit_channels["value_in"] = &in_channel;
    input_channels.float_channels["time"] = &time;

    std::map < std::string, std::string > configuration;
    configuration["delay"] = "10";
    std::map < std::string, std::string > initial_parameters;
    fil.setup_filter(configuration, initial_parameters, input_channels);

    channel_mapping map = channel_mapping();
    const std::string name = "t";
    fil.get_output_channels(map, name);

    fil.scene_activated();

    for (int i = 0; i <= 20; i += 1){
        time = time_s + 1000 * i;
        if (i<10) {
            test = 0;
        } else if (i< 19){
            test = 200;
        } else {
            in_channel = 222;
            test = 222;
        }

        fil.update();

        BOOST_TEST(*map.eight_bit_channels["t:value"] == test, "value in filter switch_on_delay should be " + std::to_string(test) + " but is " + std::to_string(*map.eight_bit_channels["t:value"]));

    }
    time_s = 28000;
    time = time_s;
    in_channel = 105;
    fil.scene_activated();

    for (int i = 0; i < 20; i += 1){
        time = time_s + 1000 * i;
        if (i<10) {
            test = 0;
        } else {
            test = 105;
        }

        fil.update();

        BOOST_TEST(*map.eight_bit_channels["t:value"] == test, " at time " + std::to_string(time) + " value in filter switch_on_delay should be " + std::to_string(test) + " but is " + std::to_string(*map.eight_bit_channels["t:value"]) + " in the second round");

    }
}

BOOST_AUTO_TEST_CASE(test_switch_off_delay_8bit) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::delay_switch_off_8bit fil = delay_switch_off_8bit();

    uint8_t in_channel = 200;
    double time_s = 2000;
    double time = time_s;
    uint8_t test = 0;

    channel_mapping input_channels = channel_mapping ();
    input_channels.eight_bit_channels["value_in"] = &in_channel;
    input_channels.float_channels["time"] = &time;

    std::map < std::string, std::string > configuration;
    configuration["delay"] = "10";
    std::map < std::string, std::string > initial_parameters;
    fil.setup_filter(configuration, initial_parameters, input_channels);

    channel_mapping map = channel_mapping();
    const std::string name = "t";
    fil.get_output_channels(map, name);

    fil.scene_activated();

    for (int i = 0; i <= 30; i += 1){
        time = time_s + 1000 * i;
        if (i<=5) {
            test = 200;
        } else if (i< 15){
            in_channel = 0;
            test = 200;
        } else if (i< 20){
            in_channel = 0;
            test = 0;
        } else {
            in_channel = 150;
            test = 150;
        }

        fil.update();

        BOOST_TEST(*map.eight_bit_channels["t:value"] == test, "value in filter switch_off_delay should be " + std::to_string(test) + " but is " + std::to_string(*map.eight_bit_channels["t:value"]));

    }
    time_s = 40000;
    time = time_s;
    in_channel = 105;
    fil.scene_activated();

    for (int i = 0; i < 20; i += 1){
        time = time_s + 1000 * i;
        if (i<=5) {
            test = 105;
        } else if (i< 15){
            in_channel = 0;
            test = 105;
        } else {
            in_channel = 0;
            test = 0;
        }

        fil.update();

        BOOST_TEST(*map.eight_bit_channels["t:value"] == test, " at time " + std::to_string(time) + " value in filter switch_off_delay should be " + std::to_string(test) + " but is " + std::to_string(*map.eight_bit_channels["t:value"]) + " in the second round");

    }
}