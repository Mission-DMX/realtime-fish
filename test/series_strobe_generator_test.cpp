#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE FISH_TESTS
#include <boost/test/included/unit_test.hpp>
#include "filters/filter_series_strobe_generator.hpp"
#include "lib/logging.hpp"

using namespace dmxfish::filters;

namespace {

template <typename FilterT>
void configure(FilterT& fil,
               uint8_t* brightness,
               uint8_t* frequency,
               uint8_t* duty_cycle,
               const std::string& slowest_period_ms) {
    channel_mapping input_channels = channel_mapping();
    input_channels.eight_bit_channels["brightness"] = brightness;
    input_channels.eight_bit_channels["frequency"] = frequency;
    input_channels.eight_bit_channels["duty_cycle"] = duty_cycle;

    std::map<std::string, std::string> configuration;
    configuration["slowest_period_ms"] = slowest_period_ms;
    std::map<std::string, std::string> initial_parameters;

    fil.setup_filter(configuration, initial_parameters, input_channels, "t");
    fil.scene_activated();
}

uint8_t read_output(filter_series_strobe_generator_8bit& fil) {
    channel_mapping map = channel_mapping();
    fil.get_output_channels(map, "t");
    return *map.eight_bit_channels.at("t:value");
}

}

BOOST_AUTO_TEST_CASE(passthrough_when_frequency_zero) {
    spdlog::set_level(spdlog::level::debug);
    auto fil = filter_series_strobe_generator_8bit();

    uint8_t brightness = 123;
    uint8_t frequency = 0;
    uint8_t duty_cycle = 128;
    configure(fil, &brightness, &frequency, &duty_cycle, "1000");

    for (int i = 0; i < 10; i++) {
        fil.update();
        BOOST_TEST(read_output(fil) == 123);
    }

    brightness = 42;
    for (int i = 0; i < 5; i++) {
        fil.update();
        BOOST_TEST(read_output(fil) == 42);
    }
}

BOOST_AUTO_TEST_CASE(max_frequency_toggles_each_tick) {
    auto fil = filter_series_strobe_generator_8bit();

    uint8_t brightness = 200;
    uint8_t frequency = 255;
    uint8_t duty_cycle = 128;
    configure(fil, &brightness, &frequency, &duty_cycle, "1000");

    for (int i = 0; i < 10; i++) {
        fil.update();
        const uint8_t expected = (i % 2 == 0) ? 200 : 0;
        BOOST_TEST(read_output(fil) == expected);
    }
}

BOOST_AUTO_TEST_CASE(slowest_frequency_duty_half) {
    // slowest_period_ms = 1000 => 40 ticks per cycle at 40 Hz updates.
    // duty = 128 => on_ticks = 1 + round(128/255 * 38) = 20.
    auto fil = filter_series_strobe_generator_8bit();

    uint8_t brightness = 100;
    uint8_t frequency = 1;
    uint8_t duty_cycle = 128;
    configure(fil, &brightness, &frequency, &duty_cycle, "1000");

    int on_count = 0;
    int off_count = 0;
    for (int i = 0; i < 40; i++) {
        fil.update();
        const uint8_t out = read_output(fil);
        if (out == 100) on_count++;
        else if (out == 0) off_count++;
        else BOOST_FAIL("unexpected output value");
    }
    BOOST_TEST(on_count == 20);
    BOOST_TEST(off_count == 20);
}

BOOST_AUTO_TEST_CASE(duty_zero_gives_single_on_tick) {
    // frequency = 50 => current_hz = 1 + (49/254) * 19 ≈ 4.665 Hz
    //                 => period_ticks = round(40 / 4.665) = 9
    // duty = 0 => on_ticks = 1.
    auto fil = filter_series_strobe_generator_8bit();

    uint8_t brightness = 100;
    uint8_t frequency = 50;
    uint8_t duty_cycle = 0;
    configure(fil, &brightness, &frequency, &duty_cycle, "1000");

    int on_count = 0;
    for (int i = 0; i < 9; i++) {
        fil.update();
        if (read_output(fil) == 100) on_count++;
    }
    BOOST_TEST(on_count == 1);
}

BOOST_AUTO_TEST_CASE(duty_max_gives_single_off_tick) {
    // Same period as above (9), duty = 255 => on_ticks = 1 + round(1 * 7) = 8.
    auto fil = filter_series_strobe_generator_8bit();

    uint8_t brightness = 100;
    uint8_t frequency = 50;
    uint8_t duty_cycle = 255;
    configure(fil, &brightness, &frequency, &duty_cycle, "1000");

    int off_count = 0;
    for (int i = 0; i < 9; i++) {
        fil.update();
        if (read_output(fil) == 0) off_count++;
    }
    BOOST_TEST(off_count == 1);
}

BOOST_AUTO_TEST_CASE(brightness_change_follows_during_on_phase) {
    // Period 40, duty 255 => 39 on ticks then 1 off tick.
    auto fil = filter_series_strobe_generator_8bit();

    uint8_t brightness = 100;
    uint8_t frequency = 1;
    uint8_t duty_cycle = 255;
    configure(fil, &brightness, &frequency, &duty_cycle, "1000");

    for (int i = 0; i < 10; i++) {
        fil.update();
        BOOST_TEST(read_output(fil) == 100);
    }
    brightness = 220;
    for (int i = 10; i < 20; i++) {
        fil.update();
        BOOST_TEST(read_output(fil) == 220);
    }
}

BOOST_AUTO_TEST_CASE(sixteen_bit_variant_passthrough_and_toggle) {
    auto fil = filter_series_strobe_generator_16bit();

    uint16_t brightness = 50000;
    uint8_t frequency = 0;
    uint8_t duty_cycle = 128;

    channel_mapping input_channels = channel_mapping();
    input_channels.sixteen_bit_channels["brightness"] = &brightness;
    input_channels.eight_bit_channels["frequency"] = &frequency;
    input_channels.eight_bit_channels["duty_cycle"] = &duty_cycle;

    std::map<std::string, std::string> configuration;
    configuration["slowest_period_ms"] = "1000";
    std::map<std::string, std::string> initial_parameters;

    fil.setup_filter(configuration, initial_parameters, input_channels, "t");
    fil.scene_activated();

    channel_mapping map = channel_mapping();
    fil.update();
    fil.get_output_channels(map, "t");
    BOOST_TEST(*map.sixteen_bit_channels.at("t:value") == 50000);

    frequency = 255;
    for (int i = 0; i < 6; i++) {
        fil.update();
        channel_mapping map2 = channel_mapping();
        fil.get_output_channels(map2, "t");
        const uint16_t expected = (i % 2 == 0) ? 50000 : 0;
        BOOST_TEST(*map2.sixteen_bit_channels.at("t:value") == expected);
    }
}

BOOST_AUTO_TEST_CASE(float_variant_passthrough_and_toggle) {
    auto fil = filter_series_strobe_generator_float();

    double brightness = 0.75;
    uint8_t frequency = 0;
    uint8_t duty_cycle = 128;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["brightness"] = &brightness;
    input_channels.eight_bit_channels["frequency"] = &frequency;
    input_channels.eight_bit_channels["duty_cycle"] = &duty_cycle;

    std::map<std::string, std::string> configuration;
    configuration["slowest_period_ms"] = "1000";
    std::map<std::string, std::string> initial_parameters;

    fil.setup_filter(configuration, initial_parameters, input_channels, "t");
    fil.scene_activated();

    channel_mapping map = channel_mapping();
    fil.update();
    fil.get_output_channels(map, "t");
    BOOST_TEST(*map.float_channels.at("t:value") == 0.75);

    frequency = 255;
    for (int i = 0; i < 4; i++) {
        fil.update();
        channel_mapping map2 = channel_mapping();
        fil.get_output_channels(map2, "t");
        const double expected = (i % 2 == 0) ? 0.75 : 0.0;
        BOOST_TEST(*map2.float_channels.at("t:value") == expected);
    }
}

BOOST_AUTO_TEST_CASE(missing_slowest_period_throws) {
    auto fil = filter_series_strobe_generator_8bit();

    uint8_t brightness = 100;
    uint8_t frequency = 0;
    uint8_t duty_cycle = 128;

    channel_mapping input_channels = channel_mapping();
    input_channels.eight_bit_channels["brightness"] = &brightness;
    input_channels.eight_bit_channels["frequency"] = &frequency;
    input_channels.eight_bit_channels["duty_cycle"] = &duty_cycle;

    std::map<std::string, std::string> configuration;
    std::map<std::string, std::string> initial_parameters;

    BOOST_CHECK_THROW(fil.setup_filter(configuration, initial_parameters, input_channels, "t"),
                      filter_config_exception);
}
