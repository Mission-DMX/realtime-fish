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

    fil8.setup_filter(configuration, initial_parameters, input_channels, "");

    channel_mapping map = channel_mapping();
    const std::string name = "t";
    fil8.get_output_channels(map, name);

    for (int i = 10; i < 255; i += 150){
        in_channel = (uint8_t) i;
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

    fil16.setup_filter(configuration, initial_parameters, input_channels, "");

    channel_mapping map = channel_mapping();
    const std::string name = "t";
    fil16.get_output_channels(map, name);
    for (int i = 10; i < 65000; i += 21250){
        in_channel = (uint16_t) i;
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
    fil.setup_filter(configuration, initial_parameters, input_channels, "");

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
    fil.setup_filter(configuration, initial_parameters, input_channels, "");

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
    fil.setup_filter(configuration, initial_parameters, input_channels, "");

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



BOOST_AUTO_TEST_CASE(test_float_map_range_16) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_float_map_range_16bit fil = filter_float_map_range_16bit();
    dmxfish::filters::filter_float_map_range_16bit fil_limit = filter_float_map_range_16bit();

    double in_channel = -20.0;
    uint16_t test = 10;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["value_in"] = &in_channel;

    std::map < std::string, std::string > configuration;
    std::map < std::string, std::string > initial_parameters;
    initial_parameters["lower_bound_in"] = "-20";
    initial_parameters["upper_bound_in"] = "80";

    fil.setup_filter(configuration, initial_parameters, input_channels, "");
    initial_parameters["limit_range"] = "1";
    fil_limit.setup_filter(configuration, initial_parameters, input_channels, "");

    channel_mapping map = channel_mapping();
    const std::string name = "t";
    const std::string name2 = "t2";
    fil.get_output_channels(map, name);
    fil_limit.get_output_channels(map, name2);
    for (double i = -20; i < 81; i += 10){
        in_channel = i;
        test = (uint16_t) std::round(((i + 20) / 100) * 65535);
        fil.update();
        fil_limit.update();
        BOOST_TEST(*map.sixteen_bit_channels["t:value"] == test, "value in filter float_map_range_16bit should be " + std::to_string(test) + " but is " + std::to_string(*map.sixteen_bit_channels["t:value"]));
        BOOST_TEST(*map.sixteen_bit_channels["t2:value"] == test, "value in filter float_map_range_16bit limited should be " + std::to_string(test) + " but is " + std::to_string(*map.sixteen_bit_channels["t2:value"]));
    }
}



BOOST_AUTO_TEST_CASE(test_float_map_range_8) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_float_map_range_8bit fil = filter_float_map_range_8bit();
    dmxfish::filters::filter_float_map_range_8bit fil_limit = filter_float_map_range_8bit();

    double in_channel = -20.0;
    uint8_t test = 10;
    uint8_t test_limit = 10;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["value_in"] = &in_channel;

    std::map < std::string, std::string > configuration;
    std::map < std::string, std::string > initial_parameters;
    initial_parameters["lower_bound_in"] = "-50";
    initial_parameters["upper_bound_in"] = "1450";
    initial_parameters["lower_bound_out"] = "20";
    initial_parameters["upper_bound_out"] = "170";

    fil.setup_filter(configuration, initial_parameters, input_channels, "");
    initial_parameters["limit_range"] = "1";
    fil_limit.setup_filter(configuration, initial_parameters, input_channels, "");

    channel_mapping map = channel_mapping();
    const std::string name1 = "t1";
    const std::string name2 = "t2";
    fil.get_output_channels(map, name1);
    fil_limit.get_output_channels(map, name2);
    for (double i = -300; i < 2400 ; i += 10){
        in_channel = i;
        if (i < -250) {
            test = 0;
            test_limit = 20;
        } else if (i > 2300) {
            test = 255;
            test_limit = 170;
        } else {
            double test_pre = std::round(((i + 50) / 1500) * 150 + 20);
            test = (uint8_t) test_pre;
            test_limit = (uint8_t) (std::max(std::min(test_pre, 170.0), 20.0));
        }
        fil.update();
        fil_limit.update();
        BOOST_TEST(*map.eight_bit_channels["t1:value"] == test, "value in filter float_map_range_8bit should be " + std::to_string(test) + " but is " + std::to_string(*map.eight_bit_channels["t1:value"]));
        BOOST_TEST(*map.eight_bit_channels["t2:value"] == test_limit, "value in filter float_map_range_8bit limited should be " + std::to_string(test_limit) + " but is " + std::to_string(*map.eight_bit_channels["t2:value"]));
    }
}


BOOST_AUTO_TEST_CASE(test_float_map_range_float) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_float_map_range_float fil = filter_float_map_range_float();
    dmxfish::filters::filter_float_map_range_float fil_limit = filter_float_map_range_float();

    double in_channel = -20.0;
    double test = 10;
    double test_limit = 10;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["value_in"] = &in_channel;

    std::map < std::string, std::string > configuration;
    std::map < std::string, std::string > initial_parameters;
    initial_parameters["lower_bound_in"] = "";
    initial_parameters["upper_bound_in"] = "";
    initial_parameters["lower_bound_out"] = "-100";
    initial_parameters["upper_bound_out"] = "300";

    fil.setup_filter(configuration, initial_parameters, input_channels, "");
    initial_parameters["limit_range"] = "1";
    fil_limit.setup_filter(configuration, initial_parameters, input_channels, "");

    channel_mapping map = channel_mapping();
    const std::string name1 = "t1";
    fil.get_output_channels(map, name1);
    const std::string name2 = "t2";
    fil_limit.get_output_channels(map, name2);
    for (double i = -1; i < 2 ; i += 0.125){
        in_channel = i;
        test = (i * 400 - 100);
        test_limit = std::max(std::min(test, 300.0), -100.0);
        fil.update();
        fil_limit.update();
        BOOST_TEST(std::abs(*map.float_channels["t1:value"] - test) <= std::abs(test * 0.00001), "value in filter float_map_range_float should be " + std::to_string(test) + " but is " + std::to_string(*map.float_channels["t1:value"]));
        BOOST_TEST(std::abs(*map.float_channels["t2:value"] - test_limit) <= std::abs(test_limit * 0.00001), "value in filter float_map_range_float limited should be " + std::to_string(test_limit) + " but is " + std::to_string(*map.float_channels["t2:value"]));
    }
}

BOOST_AUTO_TEST_CASE(test_split_and_merge_16bit) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_combine_bytes_to_16bit fil_merge = filter_combine_bytes_to_16bit();
    dmxfish::filters::filter_16bit_to_dual_byte fil_split = filter_16bit_to_dual_byte();

    uint8_t lower = 0;
    uint8_t upper = 0;
    uint16_t value = 0;

    channel_mapping input_channels_merge = channel_mapping();
    input_channels_merge.eight_bit_channels["lower"] = &lower;
    input_channels_merge.eight_bit_channels["upper"] = &upper;
    std::map < std::string, std::string > configuration;
    std::map < std::string, std::string > initial_parameters;
    fil_merge.setup_filter(configuration, initial_parameters, input_channels_merge, "");

    channel_mapping map_merge = channel_mapping();
    const std::string name_merge = "t1";
    fil_merge.get_output_channels(map_merge, name_merge);

    channel_mapping input_channels_split = channel_mapping();
    input_channels_split.sixteen_bit_channels["value"] = map_merge.sixteen_bit_channels["t1:value"];
    fil_split.setup_filter(configuration, initial_parameters, input_channels_split, "");

    channel_mapping map_split = channel_mapping();
    const std::string name_split = "t2";
    fil_split.get_output_channels(map_split, name_split);

    for (int i = 0; i < 65536 ; i++){
        value = (uint16_t) i;
        lower = (uint8_t) (value & (0x00FF));
        upper = (uint8_t) ((value & (0xFF00)) >> 8);

        fil_merge.update();
        fil_split.update();
        BOOST_TEST(*map_merge.sixteen_bit_channels["t1:value"] == value, "value in filter dual_byte_to_16bit should be " + std::to_string(value) + " but is " + std::to_string(*map_merge.sixteen_bit_channels["t1:value"]));
        BOOST_TEST(*map_split.eight_bit_channels["t2:value_lower"] == lower, "value in filter 16bit_to_dual_byte lower part should be " + std::to_string(lower) + " but is " + std::to_string(*map_split.eight_bit_channels["t2:value_lower"]));
        BOOST_TEST(*map_split.eight_bit_channels["t2:value_upper"] == upper, "value in filter 16bit_to_dual_byte upper part should be " + std::to_string(upper) + " but is " + std::to_string(*map_split.eight_bit_channels["t2:value_upper"]));
    }
}

BOOST_AUTO_TEST_CASE(test_one_8bit_to_16bit) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_map_8bit_to_16bit fil = filter_map_8bit_to_16bit();

    uint8_t input = 0;
    uint16_t value = 0;

    channel_mapping input_channels = channel_mapping();
    input_channels.eight_bit_channels["value_in"] = &input;
    std::map < std::string, std::string > configuration;
    std::map < std::string, std::string > initial_parameters;
    fil.setup_filter(configuration, initial_parameters, input_channels, "");

    channel_mapping map = channel_mapping();
    const std::string name = "t";
    fil.get_output_channels(map, name);

    for (int i = 0; i < 256 ; i++){
        value = (uint16_t) (i * 65535 / 255);
        input = (uint8_t) i;

        fil.update();
        BOOST_TEST(*map.sixteen_bit_channels["t:value"] == value, "value in filter one_byte_to_16bit should be " + std::to_string(value) + " but is " + std::to_string(*map.sixteen_bit_channels["t:value"]));
    }
}


BOOST_AUTO_TEST_CASE(test_zero_range_for_filter_map_float) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_float_map_range_float fil = filter_float_map_range_float();

    double input = 0;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["value_in"] = &input;
    std::map < std::string, std::string > configuration;
    std::map < std::string, std::string > initial_parameters;
    initial_parameters["lower_bound_in"] = "3";
    initial_parameters["upper_bound_in"] = "3";
    try {
        fil.setup_filter(configuration, initial_parameters, input_channels, "");
        BOOST_TEST(false, "filter should throw an error because input range has size 0");
    } catch (std::exception &e) {
        MARK_UNUSED(e);
    }
}