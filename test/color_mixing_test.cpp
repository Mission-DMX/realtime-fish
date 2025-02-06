//
// Created by leondietrich on 2/4/25.
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include <cmath>

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE FISH_TESTS

#include <boost/test/included/unit_test.hpp>

#include "filters/filter_color_mixer.hpp"
#include "lib/logging.hpp"

using namespace dmxfish::filters;

void test_two_input_filter(filter& cmf, double accuracy, int method) {
    dmxfish::dmx::pixel p1, p2;

    channel_mapping input_channels, output_channels;
    input_channels.color_channels["0"] = &p1;
    input_channels.color_channels["1"] = &p2;
    std::map<std::string, std::string> configuration;
    configuration["input_count"] = "2";
    std::map<std::string, std::string> initial_parameters;
    ::spdlog::debug("1");

    cmf.setup_filter(configuration, initial_parameters, input_channels, "test_filter");
    cmf.get_output_channels(output_channels, "test_filter");
    cmf.scene_activated();
    ::spdlog::debug("2");

    p1.setHue(60);
    p2.setHue(30);
    p1.setSaturation(1.0);
    p2.setSaturation(1.0);
    p1.setIluminance(1.0);
    p2.setIluminance(1.0);
    ::spdlog::debug("3");
    cmf.update();
    ::spdlog::debug("4");
    auto h = output_channels.color_channels["test_filter:value"]->getHue();
    ::spdlog::debug("4.1");
    double target = (method == 1) ? 60.0 : 45.0;
    BOOST_TEST(h > (target - accuracy), "Expected hue to be approximately " + std::to_string(target) + " deg. Actual: " + std::to_string(h));
    BOOST_TEST(h < (target + accuracy), "Expected hue to be approximately " + std::to_string(target) + " deg. Actual: " + std::to_string(h));
    auto s = output_channels.color_channels["test_filter:value"]->getSaturation();
    BOOST_TEST(s > 1 - accuracy, "Expected saturation to be approximately 1. Actual: " + std::to_string(s));
    BOOST_TEST(s < 1 + accuracy, "Expected saturation to be approximately 1. Actual: " + std::to_string(s));
    auto i = output_channels.color_channels["test_filter:value"]->getIluminance();
    BOOST_TEST(i > 1 - accuracy, "Expected intensity to be approximately 1. Actual: " + std::to_string(i));
    BOOST_TEST(i < 1 + accuracy, "Expected intensity to be approximately 1. Actual: " + std::to_string(i));
    ::spdlog::debug("4.2");

    p1.setHue(330.0);
    p2.setHue(30.0);
    p1.setSaturation(1.0);
    p2.setSaturation(0.0);
    p1.setIluminance(1.0);
    p2.setIluminance(1.0);
    ::spdlog::debug("5");
    cmf.update();
    ::spdlog::debug("6");
    h = output_channels.color_channels["test_filter:value"]->getHue();
    if (method != 2) {
        const bool hue_close_to_mod_zero = (h > 360-accuracy && h < 360+accuracy) || (h >= 0 && h < 0+accuracy);
        BOOST_TEST(hue_close_to_mod_zero, "Expected hue to be approximately 0 deg. Actual: " + std::to_string(h));
    } else {
	const bool hue_close_to_45 = (h > 45 - accuracy) && (h < 45 + accuracy);
        BOOST_TEST(hue_close_to_45, "Expecting normative rgb adding to yield 45. Got: " + std::to_string(h));
    }
    s = output_channels.color_channels["test_filter:value"]->getSaturation();
    BOOST_TEST(s > 0.5 - accuracy, "Expected saturation to be approximately 0.5. Actual: " + std::to_string(s));
    BOOST_TEST(s < 0.5 + accuracy, "Expected saturation to be approximately 0.5. Actual: " + std::to_string(s));
    i = output_channels.color_channels["test_filter:value"]->getIluminance();
    BOOST_TEST(i > 1 - accuracy, "Expected intensity to be approximately 1. Actual: " + std::to_string(i));
    BOOST_TEST(i < 1 + accuracy, "Expected intensity to be approximately 1. Actual: " + std::to_string(i));

    p1.setHue(320.0);
    p2.setHue(30.0);
    p1.setSaturation(1.0);
    p2.setSaturation(0.0);
    p1.setIluminance(1.0);
    p2.setIluminance(1.0);
    ::spdlog::debug("7");
    cmf.update();
    ::spdlog::debug("8");
    h = output_channels.color_channels["test_filter:value"]->getHue();
    target = (method == 2) ? 45 : 355;
    const bool hue_about_355 = ((h > target - accuracy) && (h < target + accuracy)) || (h >= 0 && h < std::fmod(target + accuracy, 360.0));
    BOOST_TEST(hue_about_355, "Expected hue to be approximately " + std::to_string(target) + " deg. Actual: " + std::to_string(h));
    s = output_channels.color_channels["test_filter:value"]->getSaturation();
    BOOST_TEST(s > 0.5 - accuracy, "Expected saturation to be approximately 0.5. Actual: " + std::to_string(s));
    BOOST_TEST(s < 0.5 + accuracy, "Expected saturation to be approximately 0.5. Actual: " + std::to_string(s));
    i = output_channels.color_channels["test_filter:value"]->getIluminance();
    BOOST_TEST(i > 1 - accuracy, "Expected intensity to be approximately 1. Actual: " + std::to_string(i));
    BOOST_TEST(i < 1 + accuracy, "Expected intensity to be approximately 1. Actual: " + std::to_string(i));

    p1.setHue(0.0);
    p2.setHue(180.0);
    p1.setSaturation(1.0);
    p2.setSaturation(0.0);
    p1.setIluminance(1.0);
    p2.setIluminance(1.0);
    ::spdlog::debug("7");
    cmf.update();
    ::spdlog::debug("8");
    h = output_channels.color_channels["test_filter:value"]->getHue();
    target = (method == 0) ? 90 : 0;
    const bool hue_about_90 = (h > target - accuracy) && (h < target + accuracy);
    BOOST_TEST(hue_about_90, "Mixing yellow and blue, Expected hue to be approximately 90 deg (green). Actual: " + std::to_string(h));
    s = output_channels.color_channels["test_filter:value"]->getSaturation();
    BOOST_TEST(s > 0.5 - accuracy, "Expected saturation to be approximately 0.5. Actual: " + std::to_string(s));
    BOOST_TEST(s < 0.5 + accuracy, "Expected saturation to be approximately 0.5. Actual: " + std::to_string(s));
    i = output_channels.color_channels["test_filter:value"]->getIluminance();
    BOOST_TEST(i > 1 - accuracy, "Expected intensity to be approximately 1. Actual: " + std::to_string(i));
    BOOST_TEST(i < 1 + accuracy, "Expected intensity to be approximately 1. Actual: " + std::to_string(i));
}

BOOST_AUTO_TEST_CASE(test_color_mix_filter_two_inputs) {
    spdlog::set_level(spdlog::level::info);
    //spdlog::set_level(spdlog::level::debug);

    spdlog::info("Testing CSV mixing with 2 inputs");
    filter_color_mixer_hsv cmf_hsv;
    test_two_input_filter(cmf_hsv, 0.5, 0);

    spdlog::info("Testing RGB adding with 2 inputs");
    filter_color_mixer_add_rgb cmf_argb;
    test_two_input_filter(cmf_argb, 7.5, 1);

    spdlog::info("Testing RGB normative mixing with 2 inputs");
    filter_color_mixer_norm_rgb cmf_nrgb;
    test_two_input_filter(cmf_nrgb, 2.5, 2);
}

void test_zero_input_filter(filter& cmf) {
    channel_mapping input_channels, output_channels;
    std::map<std::string, std::string> configuration, initial_parameters;

    cmf.setup_filter(configuration, initial_parameters, input_channels, "test_filter");
    cmf.get_output_channels(output_channels, "test_filter");
    cmf.scene_activated();

    cmf.update();
    BOOST_TEST(output_channels.color_channels["test_filter:value"]->getRed() < 0.001, "Expected Red to be 0.");
    BOOST_TEST(output_channels.color_channels["test_filter:value"]->getGreen() < 0.001, "Expected Green to be 0.");
    BOOST_TEST(output_channels.color_channels["test_filter:value"]->getBlue() < 0.001, "Expected Blue to be 0.");
}

BOOST_AUTO_TEST_CASE(test_color_mix_filter_zero_inputs) {
        spdlog::set_level(spdlog::level::info);
        //spdlog::set_level(spdlog::level::debug);

        spdlog::info("Testing CSV mixing with 0 inputs");
        filter_color_mixer_hsv cmf_hsv;
        test_zero_input_filter(cmf_hsv);

        spdlog::info("Testing RGB adding with 0 inputs");
        filter_color_mixer_add_rgb cmf_argb;
        test_zero_input_filter(cmf_argb);

        spdlog::info("Testing RGB normative mixing with 0 inputs");
        filter_color_mixer_norm_rgb cmf_nrgb;
        test_zero_input_filter(cmf_nrgb);
}

void test_single_input_filter(filter& cmf, double accuracy) {
    channel_mapping input_channels, output_channels;
    std::map<std::string, std::string> configuration, initial_parameters;
    dmxfish::dmx::pixel p;

    configuration["input_count"] = "1";
    input_channels.color_channels["0"] = &p;

    cmf.setup_filter(configuration, initial_parameters, input_channels, "test_filter");
    cmf.get_output_channels(output_channels, "test_filter");
    cmf.scene_activated();

    p.setHue(320.0);
    p.setSaturation(1.0);
    p.setIluminance(0.5);

    cmf.update();

    const auto h = output_channels.color_channels["test_filter:value"]->getHue();
    BOOST_TEST(h > 320 - accuracy, "Expected hue to be approximately 320 deg. Actual: " + std::to_string(h));
    BOOST_TEST(h < 320 + accuracy, "Expected hue to be approximately 320 deg. Actual: " + std::to_string(h));
    const auto s = output_channels.color_channels["test_filter:value"]->getSaturation();
    BOOST_TEST(s > 1.0 - accuracy, "Expected saturation to be approximately 1. Actual: " + std::to_string(s));
    BOOST_TEST(s < 1.0 + accuracy, "Expected saturation to be approximately 1. Actual: " + std::to_string(s));
    const auto i = output_channels.color_channels["test_filter:value"]->getIluminance();
    BOOST_TEST(i > 0.5 - accuracy, "Expected intensity to be approximately 1. Actual: " + std::to_string(i));
    BOOST_TEST(i < 0.5 + accuracy, "Expected intensity to be approximately 1. Actual: " + std::to_string(i));
}

BOOST_AUTO_TEST_CASE(test_color_mix_filter_single_input) {
        spdlog::set_level(spdlog::level::info);
        //spdlog::set_level(spdlog::level::debug);

        spdlog::info("Testing CSV mixing with 1 inputs");
        filter_color_mixer_hsv cmf_hsv;
        test_single_input_filter(cmf_hsv, 0.5);

        spdlog::info("Testing RGB adding with 1 inputs");
        filter_color_mixer_add_rgb cmf_argb;
        test_single_input_filter(cmf_argb, 25.0);

        spdlog::info("Testing RGB normative mixing with 1 inputs");
        filter_color_mixer_norm_rgb cmf_nrgb;
        test_single_input_filter(cmf_nrgb, 2.5);
}

// TODO also write test case for 3
