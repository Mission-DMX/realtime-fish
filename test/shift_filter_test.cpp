#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE FISH_TESTS
#include <boost/test/included/unit_test.hpp>
#include "filters/filter_shift.hpp"
#include "lib/logging.hpp"

#include <sstream>

using namespace dmxfish::filters;

BOOST_AUTO_TEST_CASE(testshift) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_shift_8bit fil8 = filter_shift_8bit ();

    double time_s = 0;
    double switch_time = 10;
    uint8_t in_channel = 100;

    channel_mapping input_channels = channel_mapping ();
    input_channels.float_channels["time"] = &time_s;
    input_channels.float_channels["switch_time"] = &switch_time;
    input_channels.eight_bit_channels["input"] = &in_channel;

    std::map < std::string, std::string > configuration;
    configuration["nr_outputs"] = "3";

    std::map < std::string, std::string > initial_parameters;

    fil8.setup_filter(configuration, initial_parameters, input_channels);



    channel_mapping map = channel_mapping ();
    const std::string name = "t";
    for (int i = 0; i < 100; i++){
        time_s = (double) i;
        if (i == 40) {
            in_channel = 200;
        }
        if (i == 45) {
            in_channel = 250;
        }
        if (i == 50) {
            in_channel = 150;
        }
        if (i == 55) {
            switch_time = 20;
        }
        if (i == 80) {
            in_channel = 10;
        }
        uint8_t tester1;
        uint8_t tester2;
        uint8_t tester3;
        if (time_s < 10) {
            tester1 = 0;
            tester2 = 0;
            tester3 = 0;
        } else if (time_s < 20){
            tester1 = 100;
            tester2 = 0;
            tester3 = 0;
        } else if (time_s < 30){
            tester1 = 100;
            tester2 = 100;
            tester3 = 0;
        } else if (time_s < 40){
            tester1 = 100;
            tester2 = 100;
            tester3 = 100;
        } else if (time_s < 50){
            tester1 = 200;
            tester2 = 100;
            tester3 = 100;
        } else if (time_s < 70){
            tester1 = 150;
            tester2 = 200;
            tester3 = 100;
        } else if (time_s < 90){
            tester1 = 150;
            tester2 = 150;
            tester3 = 200;
        } else {
            tester1 = 10;
            tester2 = 150;
            tester3 = 150;
        }

        fil8.update();
        fil8.get_output_channels(map, name);
        std::string error =
                std::string("Channel ") + "t:output_1" + " should be " + std::to_string(tester1) + " , but is " +
                std::to_string(*map.eight_bit_channels["t:output_1"]) + " at time: " + std::to_string(time_s);
        BOOST_TEST(*map.eight_bit_channels["t:output_1"] == tester1, error);
        error =
                std::string("Channel ") + "t:output_2" + " should be " + std::to_string(tester2) + " , but is " +
                std::to_string(*map.eight_bit_channels["t:output_2"]) + " at time: " + std::to_string(time_s);
        BOOST_TEST(*map.eight_bit_channels["t:output_2"] == tester2, error);
        error =
                std::string("Channel ") + "t:output_3" + " should be " + std::to_string(tester3) + " , but is " +
                std::to_string(*map.eight_bit_channels["t:output_3"]) + " at time: " + std::to_string(time_s);
        BOOST_TEST(*map.eight_bit_channels["t:output_3"] == tester3, error);

        map.eight_bit_channels.clear();
        map.sixteen_bit_channels.clear();
        map.float_channels.clear();
        map.color_channels.clear();

    }
}