#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE FISH_TESTS
#include <boost/test/included/unit_test.hpp>
#include "filters/filter_conversion.hpp"
#include "lib/logging.hpp"

#include <sstream>

using namespace dmxfish::filters;

BOOST_AUTO_TEST_CASE(test_8bit_to_float) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_8bit_to_float fil8 = filter_8bit_to_float();

    double test = 10.0;
    uint8_t in_channel = 10;

    channel_mapping input_channels = channel_mapping ();
    input_channels.eight_bit_channels["value_in"] = &in_channel;

    std::map < std::string, std::string > configuration;

    std::map < std::string, std::string > initial_parameters;

    fil8.setup_filter(configuration, initial_parameters, input_channels);

    channel_mapping map = channel_mapping();
    const std::string name = "t";
    fil8.get_output_channels(map, name);

    for (int i = 10; i < 255; i += 150){
        in_channel = i;
        test = i;
        fil8.update();

        BOOST_TEST(*map.float_channels["t:value"] == test, "value in filter 8bit_to_float should be " + std::to_string(test) + " but is " + std::to_string(*map.float_channels["t:value"]));

    }
}

BOOST_AUTO_TEST_CASE(test_16bit_to_float) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_16bit_to_float fil16 = filter_16bit_to_float();

    double test = 10.0;
    uint16_t in_channel = 10;

    channel_mapping input_channels = channel_mapping();
    input_channels.sixteen_bit_channels["value_in"] = &in_channel;

    std::map < std::string, std::string > configuration;

    std::map < std::string, std::string > initial_parameters;

    fil16.setup_filter(configuration, initial_parameters, input_channels);

    channel_mapping map = channel_mapping();
    const std::string name = "t";
    fil16.get_output_channels(map, name);
    for (int i = 10; i < 65000; i += 21250){
        in_channel = i;
        test = i;
        fil16.update();
        BOOST_TEST(*map.float_channels["t:value"] == test, "value in filter 8bit_to_float should be " + std::to_string(test) + " but is " + std::to_string(*map.float_channels["t:value"]));
    }
}

BOOST_AUTO_TEST_CASE(test_color_to_floats) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_16bit_to_float fil = filter_16bit_to_float();

    double testh = 355.0;
    double tests = 0.8;
    double testi = 0.2;
    dmxfish::dmx::pixel in_channel = dmxfish::dmx::pixel(355, 0.8, 0.2);

    channel_mapping input_channels = channel_mapping();
    input_channels.color_channels["input"] = &in_channel;
    std::map < std::string, std::string > configuration;
    std::map < std::string, std::string > initial_parameters;
    fil.setup_filter(configuration, initial_parameters, input_channels);

    channel_mapping map = channel_mapping();
    const std::string name = "test";
    fil.get_output_channels(map, name);
    fil.update();
    BOOST_TEST(*map.float_channels["t:hue"] == testh, "value in filter color_to_floats should be " + std::to_string(testh) + " but is " + std::to_string(*map.float_channels["t:hue"]));
    BOOST_TEST(*map.float_channels["t:saturation"] == tests, "value in filter color_to_floats should be " + std::to_string(tests) + " but is " + std::to_string(*map.float_channels["t:saturation"]));
    BOOST_TEST(*map.float_channels["t:iluminance"] == testi, "value in filter color_to_floats should be " + std::to_string(testi) + " but is " + std::to_string(*map.float_channels["t:iluminance"]));
}