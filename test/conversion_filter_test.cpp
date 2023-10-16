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
    dmxfish::filters::filter_pixel_to_floats fil = filter_pixel_to_floats();

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
    BOOST_TEST(*map.float_channels["test:hue"] == testh, "value h in filter color_to_floats should be " + std::to_string(testh) + " but is " + std::to_string(*map.float_channels["test:hue"]));
    BOOST_TEST(*map.float_channels["test:saturation"] == tests, "value s in filter color_to_floats should be " + std::to_string(tests) + " but is " + std::to_string(*map.float_channels["test:saturation"]));
    BOOST_TEST(*map.float_channels["test:iluminance"] == testi, "value i in filter color_to_floats should be " + std::to_string(testi) + " but is " + std::to_string(*map.float_channels["test:iluminance"]));
}

BOOST_AUTO_TEST_CASE(test_color_to_rgb) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_pixel_to_rgb_channels fil = filter_pixel_to_rgb_channels();

    uint8_t testr = 0;
    uint8_t testg = 127;
    uint8_t testb = 128;
    dmxfish::dmx::pixel in_channel = dmxfish::dmx::pixel(180, 1, 1);

    channel_mapping input_channels = channel_mapping();
    input_channels.color_channels["value"] = &in_channel;
    std::map < std::string, std::string > configuration;
    std::map < std::string, std::string > initial_parameters;
    fil.setup_filter(configuration, initial_parameters, input_channels);

    channel_mapping map = channel_mapping();
    const std::string name = "test";
    fil.get_output_channels(map, name);
    fil.update();
    BOOST_TEST(*map.eight_bit_channels["test:r"] == testr, "value r in filter color_to_rgb should be " + std::to_string(testr) + " but is " + std::to_string(*map.eight_bit_channels["test:r"]));
    BOOST_TEST(*map.eight_bit_channels["test:g"] == testg, "value g in filter color_to_rgb should be " + std::to_string(testg) + " but is " + std::to_string(*map.eight_bit_channels["test:g"]));
    BOOST_TEST(*map.eight_bit_channels["test:b"] == testb, "value b in filter color_to_rgb should be " + std::to_string(testb) + " but is " + std::to_string(*map.eight_bit_channels["test:b"]));

    in_channel.saturation = 0.5;
    testr = 43;
    testg = 106;
    testb = 106;
    fil.update();
    BOOST_TEST(*map.eight_bit_channels["test:r"] == testr, "value r in filter color_to_rgb should be " + std::to_string(testr) + " but is " + std::to_string(*map.eight_bit_channels["test:r"]));
    BOOST_TEST(*map.eight_bit_channels["test:g"] == testg, "value g in filter color_to_rgb should be " + std::to_string(testg) + " but is " + std::to_string(*map.eight_bit_channels["test:g"]));
    BOOST_TEST(*map.eight_bit_channels["test:b"] == testb, "value b in filter color_to_rgb should be " + std::to_string(testb) + " but is " + std::to_string(*map.eight_bit_channels["test:b"]));

    in_channel.hue = 240;
    testr = 43;
    testg = 42;
    testb = 170;
    fil.update();
    BOOST_TEST(*map.eight_bit_channels["test:r"] == testr, "value r in filter color_to_rgb should be " + std::to_string(testr) + " but is " + std::to_string(*map.eight_bit_channels["test:r"]));
    BOOST_TEST(*map.eight_bit_channels["test:g"] == testg, "value g in filter color_to_rgb should be " + std::to_string(testg) + " but is " + std::to_string(*map.eight_bit_channels["test:g"]));
    BOOST_TEST(*map.eight_bit_channels["test:b"] == testb, "value b in filter color_to_rgb should be " + std::to_string(testb) + " but is " + std::to_string(*map.eight_bit_channels["test:b"]));

    in_channel.iluminance = 0.2;
    testr = 9;
    testg = 8;
    testb = 34;
    fil.update();
    BOOST_TEST(*map.eight_bit_channels["test:r"] == testr, "value r in filter color_to_rgb should be " + std::to_string(testr) + " but is " + std::to_string(*map.eight_bit_channels["test:r"]));
    BOOST_TEST(*map.eight_bit_channels["test:g"] == testg, "value g in filter color_to_rgb should be " + std::to_string(testg) + " but is " + std::to_string(*map.eight_bit_channels["test:g"]));
    BOOST_TEST(*map.eight_bit_channels["test:b"] == testb, "value b in filter color_to_rgb should be " + std::to_string(testb) + " but is " + std::to_string(*map.eight_bit_channels["test:b"]));
}

BOOST_AUTO_TEST_CASE(test_color_to_rgbw) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_pixel_to_rgbw_channels fil = filter_pixel_to_rgbw_channels();

    uint8_t testr = 0;
    uint8_t testg = 127;
    uint8_t testb = 128;
    uint8_t testw = 0;
    dmxfish::dmx::pixel in_channel = dmxfish::dmx::pixel(180, 1, 1);

    channel_mapping input_channels = channel_mapping();
    input_channels.color_channels["value"] = &in_channel;
    std::map < std::string, std::string > configuration;
    std::map < std::string, std::string > initial_parameters;
    fil.setup_filter(configuration, initial_parameters, input_channels);

    channel_mapping map = channel_mapping();
    const std::string name = "test";
    fil.get_output_channels(map, name);
    fil.update();
    BOOST_TEST(*map.eight_bit_channels["test:r"] == testr, "value r in filter color_to_rgbw should be " + std::to_string(testr) + " but is " + std::to_string(*map.eight_bit_channels["test:r"]));
    BOOST_TEST(*map.eight_bit_channels["test:g"] == testg, "value g in filter color_to_rgbw should be " + std::to_string(testg) + " but is " + std::to_string(*map.eight_bit_channels["test:g"]));
    BOOST_TEST(*map.eight_bit_channels["test:b"] == testb, "value b in filter color_to_rgbw should be " + std::to_string(testb) + " but is " + std::to_string(*map.eight_bit_channels["test:b"]));
    BOOST_TEST(*map.eight_bit_channels["test:w"] == testw, "value w in filter color_to_rgbw should be " + std::to_string(testw) + " but is " + std::to_string(*map.eight_bit_channels["test:w"]));

    in_channel.saturation = 0.5;
    testr = 0;
    testg = 64;
    testb = 64;
    testw = 128;
    fil.update();
    BOOST_TEST(*map.eight_bit_channels["test:r"] == testr, "value r in filter color_to_rgbw should be " + std::to_string(testr) + " but is " + std::to_string(*map.eight_bit_channels["test:r"]));
    BOOST_TEST(*map.eight_bit_channels["test:g"] == testg, "value g in filter color_to_rgbw should be " + std::to_string(testg) + " but is " + std::to_string(*map.eight_bit_channels["test:g"]));
    BOOST_TEST(*map.eight_bit_channels["test:b"] == testb, "value b in filter color_to_rgbw should be " + std::to_string(testb) + " but is " + std::to_string(*map.eight_bit_channels["test:b"]));
    BOOST_TEST(*map.eight_bit_channels["test:w"] == testw, "value w in filter color_to_rgbw should be " + std::to_string(testw) + " but is " + std::to_string(*map.eight_bit_channels["test:w"]));

    in_channel.hue = 240;
    testr = 0;
    testg = 0;
    testb = 128;
    testw= 128;
    fil.update();
    BOOST_TEST(*map.eight_bit_channels["test:r"] == testr, "value r in filter color_to_rgbw should be " + std::to_string(testr) + " but is " + std::to_string(*map.eight_bit_channels["test:r"]));
    BOOST_TEST(*map.eight_bit_channels["test:g"] == testg, "value g in filter color_to_rgbw should be " + std::to_string(testg) + " but is " + std::to_string(*map.eight_bit_channels["test:g"]));
    BOOST_TEST(*map.eight_bit_channels["test:b"] == testb, "value b in filter color_to_rgbw should be " + std::to_string(testb) + " but is " + std::to_string(*map.eight_bit_channels["test:b"]));
    BOOST_TEST(*map.eight_bit_channels["test:w"] == testw, "value w in filter color_to_rgbw should be " + std::to_string(testw) + " but is " + std::to_string(*map.eight_bit_channels["test:w"]));

    in_channel.iluminance = 0.2;
    testr = 0;
    testg = 0;
    testb = 26;
    testw = 26;
    fil.update();
    BOOST_TEST(*map.eight_bit_channels["test:r"] == testr, "value r in filter color_to_rgbw should be " + std::to_string(testr) + " but is " + std::to_string(*map.eight_bit_channels["test:r"]));
    BOOST_TEST(*map.eight_bit_channels["test:g"] == testg, "value g in filter color_to_rgbw should be " + std::to_string(testg) + " but is " + std::to_string(*map.eight_bit_channels["test:g"]));
    BOOST_TEST(*map.eight_bit_channels["test:b"] == testb, "value b in filter color_to_rgbw should be " + std::to_string(testb) + " but is " + std::to_string(*map.eight_bit_channels["test:b"]));
    BOOST_TEST(*map.eight_bit_channels["test:w"] == testw, "value w in filter color_to_rgbw should be " + std::to_string(testw) + " but is " + std::to_string(*map.eight_bit_channels["test:w"]));
}