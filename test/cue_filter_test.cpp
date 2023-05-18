#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE FISH_TESTS
#include <boost/test/included/unit_test.hpp>
#include "filters/filter_cue.hpp"
#include "lib/logging.hpp"

#include <sstream>

using namespace dmxfish::filters;

BOOST_AUTO_TEST_CASE(onechannelonecueoneframe) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_cue fil = filter_cue ();

    double time_s = 0;

    channel_mapping input_channels = channel_mapping ();
    input_channels.float_channels["time"] = &time_s;

    std::map < std::string, std::string > configuration;
    configuration["mapping"] = "dimmer:8bit";
    configuration["end_handling"] = "hold";

    configuration["cuelist"] =
    "2:100@edg#hold#do_nothing";

    std::map < std::string, std::string > initial_parameters;

    fil.setup_filter (configuration, initial_parameters, input_channels);



    channel_mapping map = channel_mapping ();
    const std::string name = "t";
    for (int i = 0; i < 40000; i = i + 100){
        time_s = (double) i;
        fil.update();
        if (i == 1000) {
            const std::string key = "run_mode";
            const std::string _value = "play";
            fil.receive_update_from_gui(key, _value);
        }

        fil.get_output_channels(map, name);
        for (auto it = map.eight_bit_channels.begin();
             it != map.eight_bit_channels.end(); ++it) {
            uint8_t tester = (time_s < 2000) ? 0 : 100;
            std::string error =
                    std::string("Channel ") + it->first + " should be " + std::to_string(tester) + " , but is " +
                    std::to_string(*it->second) + " at time: " + std::to_string(time_s);
            BOOST_TEST(*map.eight_bit_channels["t:dimmer"] == tester, error);
        }
        map.eight_bit_channels.clear();
        map.sixteen_bit_channels.clear();
        map.float_channels.clear();
        map.color_channels.clear();

    }
}


BOOST_AUTO_TEST_CASE(oneframeeachchanneltype) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_cue fil = filter_cue();

    double time_s = 0;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["time"] = &time_s;

    std::map <std::string, std::string> configuration;
    configuration["mapping"] = "dimmer:8bit;xpos:16bit;ypos:float;color:color";
    configuration["end_handling"] = "hold";

    configuration["cuelist"] =
            "2:100@sig&16000@e_i&0.8@lin&120,1,1@edg#hold#do_nothing";

    std::map <std::string, std::string> initial_parameters;

    fil.setup_filter(configuration, initial_parameters, input_channels);


    channel_mapping map = channel_mapping();
    const std::string name = "t";
    for (int i = 0; i < 40000; i = i + 100) {
        time_s = (double) i;
        fil.update();
        if (i == 1000) {
            const std::string key = "run_mode";
            const std::string _value = "play";
            fil.receive_update_from_gui(key, _value);
        }

        fil.get_output_channels(map, name);
        for (auto it = map.eight_bit_channels.begin();
             it != map.eight_bit_channels.end(); ++it) {

            uint8_t tester8;
            if (time_s < 1000){
                tester8 = 0;
            } else if (time_s < 3000) {
                tester8 = std::round(100 * 1.0 / (1 + std::exp(6 - ((double)time_s - 1000)/2000 *12)));
            } else {
                tester8 = 100;
            }
            std::string error = std::string("Channel ") + it->first + " should be " + std::to_string(tester8) +
                                " , but is " + std::to_string(*it->second) + " at time: " +
                                std::to_string(time_s);
            BOOST_TEST(*map.eight_bit_channels["t:dimmer"] == tester8, error);
        }

            for (auto it = map.sixteen_bit_channels.begin();
                 it != map.sixteen_bit_channels.end(); ++it) {

                uint16_t tester16;
                if (time_s < 1000){
                    tester16 = 0;
                } else if (time_s < 3000) {
                    tester16 = std::round(16000 * (((double)time_s - 1000)/2000)*(((double)time_s - 1000)/2000));
                } else {
                    tester16 = 16000;
                }
                std::string error = std::string("Channel ") + it->first + " should be " + std::to_string(tester16) +
                                    " , but is " + std::to_string(*it->second) + " at time: " +
                                    std::to_string(time_s);
                BOOST_TEST(*map.sixteen_bit_channels["t:xpos"] == tester16, error);
            }

            for (auto it = map.float_channels.begin();
                 it != map.float_channels.end(); ++it) {

                float testerfl;
                if (time_s < 1000){
                    testerfl = 0;
                } else if (time_s < 3000) {
                    testerfl = 0.8 * ((double)time_s - 1000)/2000;
                } else {
                    testerfl = 0.8;
                }
                std::string error = std::string("Channel ") + it->first + " should be " + std::to_string(testerfl) +
                                    " , but is " + std::to_string(*it->second) + " at time: " +
                                    std::to_string(time_s);
//                BOOST_TEST(*map.float_channels["t:ypos"] == testerfl, error);
            }
        map.eight_bit_channels.clear();
        map.sixteen_bit_channels.clear();
        map.float_channels.clear();
        map.color_channels.clear();
    }
}