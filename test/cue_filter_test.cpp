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

    fil.pre_setup(configuration, initial_parameters, "");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");



    channel_mapping map = channel_mapping ();
    const std::string name = "t";
    for (int i = 0; i < 4000; i = i + 100){
        time_s = (double) i;
        if (i == 1000) {
            const std::string key = "run_mode";
            const std::string _value = "play";
            fil.receive_update_from_gui(key, _value);
        }

        fil.update();
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

    fil.pre_setup(configuration, initial_parameters, "");
    fil.setup_filter(configuration, initial_parameters, input_channels, "");


    channel_mapping map = channel_mapping();
    const std::string name = "t";
    for (int i = 0; i < 4000; i = i + 100) {
        time_s = (double) i;
        if (i == 1000) {
            const std::string key = "run_mode";
            const std::string _value = "play";
            fil.receive_update_from_gui(key, _value);
        }

        fil.update();
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
                BOOST_TEST(std::abs(*map.float_channels["t:ypos"] - testerfl) <= testerfl * 0.00001, error);
            }

            for (auto it = map.float_channels.begin();
                 it != map.float_channels.end(); ++it) {

                dmxfish::dmx::pixel testercol;
                if (time_s < 2000){
                    testercol = dmxfish::dmx::pixel(0,0,0);
//                } else if (time_s < 3000) {
//                    testercol = 0.8 * ((double)time_s - 1000)/2000;
                } else {
                    testercol = dmxfish::dmx::pixel(120,1,1);
                }
                std::string error = std::string("Channel ") + it->first + " should be " + testercol.str() +
                                    " , but is " + std::to_string(*it->second) + " at time: " +
                                    std::to_string(time_s);
                BOOST_TEST(std::abs(map.color_channels["t:color"]->hue - testercol.hue) <= testercol.hue * 0.00001, error);
                BOOST_TEST(std::abs(map.color_channels["t:color"]->saturation - testercol.saturation) <= testercol.saturation * 0.00001, error);
                BOOST_TEST(std::abs(map.color_channels["t:color"]->iluminance - testercol.iluminance) <= testercol.iluminance * 0.00001, error);
            }
        map.eight_bit_channels.clear();
        map.sixteen_bit_channels.clear();
        map.float_channels.clear();
        map.color_channels.clear();
    }
}


BOOST_AUTO_TEST_CASE(onechanneltwoframes) {
        spdlog::set_level(spdlog::level::debug);
        dmxfish::filters::filter_cue fil = filter_cue ();

        double time_s = 0;

        channel_mapping input_channels = channel_mapping ();
        input_channels.float_channels["time"] = &time_s;

        std::map < std::string, std::string > configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        configuration["cuelist"] =
        "2:255@lin|4:50@lin#hold#do_nothing";

        std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
        fil.setup_filter (configuration, initial_parameters, input_channels, "");



        channel_mapping map = channel_mapping ();
        const std::string name = "t";
        for (int i = 0; i < 6000; i = i + 100){
            time_s = (double) i;
            if (i == 1000 || i == 3000) {
                const std::string key = "run_mode";
                const std::string _value = "play";
                fil.receive_update_from_gui(key, _value);
            }

            fil.update();
            fil.get_output_channels(map, name);
            for (auto it = map.eight_bit_channels.begin();
                 it != map.eight_bit_channels.end(); ++it) {
                uint8_t tester;
                if (time_s < 1000){
                    tester = 0;
                } else if (time_s < 3000){
                    tester = std::round((double) 255 * (time_s-1000)/2000);
                } else if (time_s < 5000){
                    tester = std::round((double) 255 - (double) 205 * (time_s-3000)/2000);
                } else {
                    tester = 50;
                }
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


BOOST_AUTO_TEST_CASE(teststop) {
        spdlog::set_level(spdlog::level::debug);
        dmxfish::filters::filter_cue fil = filter_cue ();

        double time_s = 0;

        channel_mapping input_channels = channel_mapping ();
        input_channels.float_channels["time"] = &time_s;

        std::map < std::string, std::string > configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        configuration["cuelist"] =
        "2:250@lin|4:50@lin#hold#do_nothing";

        std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
	fil.setup_filter (configuration, initial_parameters, input_channels, "");



        channel_mapping map = channel_mapping ();
        const std::string name = "t";
        for (int i = 0; i < 12000; i = i + 100){
            time_s = (double) i;
            if (i == 1000 || i == 6000) {
                const std::string key = "run_mode";
                const std::string _value = "play";
                fil.receive_update_from_gui(key, _value);
            }
            fil.update();

            if (i == 4000) {
                const std::string key = "run_mode";
                const std::string _value = "stop";
                fil.receive_update_from_gui(key, _value);
            }

            fil.get_output_channels(map, name);
            for (auto it = map.eight_bit_channels.begin();
                 it != map.eight_bit_channels.end(); ++it) {
                uint8_t tester;
                if (time_s < 1000){
                    tester = 0;
                } else if (time_s < 3000){
                    tester = std::round((double) 250 * (time_s-1000)/2000);
                } else if (time_s < 4000){
                    tester = std::round((double) 250 - (double) 200 * (time_s-3000)/2000);
                } else if (time_s < 6000){
                    tester = std::round(150);
                } else if (time_s < 8000){
                    tester = std::round((double) 150 + 100 * (time_s-6000)/2000);
                } else if (time_s < 10000){
                    tester = std::round((double) 250 - (double) 200 * (time_s-8000)/2000);
                } else {
                    tester = 50;
                }
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

BOOST_AUTO_TEST_CASE(testpause) {
        spdlog::set_level(spdlog::level::debug);
        dmxfish::filters::filter_cue fil = filter_cue ();

        double time_s = 0;

        channel_mapping input_channels = channel_mapping ();
        input_channels.float_channels["time"] = &time_s;

        std::map < std::string, std::string > configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        configuration["cuelist"] =
        "2:255@lin|4:50@lin#hold#do_nothing";

        std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
        fil.setup_filter (configuration, initial_parameters, input_channels, "");



        channel_mapping map = channel_mapping ();
        const std::string name = "t";
        for (int i = 0; i < 8000; i = i + 100){
            time_s = (double) i;
            fil.update();

            if (i == 1000) {
                const std::string key = "run_mode";
                const std::string _value = "play";
                fil.receive_update_from_gui(key, _value);
            }

            if (i == 2100 || i == 3500) {
                const std::string key = "run_mode";
                const std::string _value = "pause";
                fil.receive_update_from_gui(key, _value);
            }

            if (i == 2600 || i == 4300) {
                const std::string key = "run_mode";
                const std::string _value = "play";
                fil.receive_update_from_gui(key, _value);
            }

            fil.get_output_channels(map, name);
            for (auto it = map.eight_bit_channels.begin();
                 it != map.eight_bit_channels.end(); ++it) {
                uint8_t tester;
                if (time_s < 1000){
                    tester = 0;
                } else if (time_s < 2100){
                    tester = std::round((double) 255 * (time_s-1000)/2000);
                } else if (time_s < 2600){
                    tester = std::round((double) 255 * (2100-1000)/2000);
                } else if (time_s < 3500){
                    tester = std::round((double) 255 * (time_s-1500)/2000);
                } else if (time_s < 4300){
                    tester = std::round((double) 255);
                } else if (time_s < 6300){
                    tester = std::round((double) 255 - (double) 205 * (time_s-4300)/2000);
                } else {
                    tester = 50;
                }
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

BOOST_AUTO_TEST_CASE(test_restart) {
        spdlog::set_level(spdlog::level::debug);
        dmxfish::filters::filter_cue fil = filter_cue ();

        double time_s = 0;

        channel_mapping input_channels = channel_mapping ();
        input_channels.float_channels["time"] = &time_s;

        std::map < std::string, std::string > configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        configuration["cuelist"] =
        "2:200@lin|4:50@lin#hold#restart";

        std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
        fil.setup_filter (configuration, initial_parameters, input_channels, "");



        channel_mapping map = channel_mapping ();
        const std::string name = "t";
        for (int i = 0; i < 10000; i = i + 100){
            time_s = (double) i;

            fil.update();


            if (i == 1000 || i == 4000) {
                const std::string key = "run_mode";
                const std::string _value = "play";
                fil.receive_update_from_gui(key, _value);
            }

            fil.get_output_channels(map, name);
            for (auto it = map.eight_bit_channels.begin();
                 it != map.eight_bit_channels.end(); ++it) {
                uint8_t tester;
                if (time_s < 1000){
                    tester = 0;
                } else if (time_s < 3000){
                    tester = std::round((double) 200 * (time_s-1000)/2000);
                } else if (time_s < 4000){
                    tester = std::round((double) 200 - 150 * (time_s-3000)/2000);
                } else if (time_s < 6000){
                    tester = std::round((double) 125 + 75 * (time_s-4000)/2000);
                } else if (time_s < 8000){
                    tester = std::round((double) 200 - 150 * (time_s-6000)/2000);
                } else {
                    tester = 50;
                }
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



BOOST_AUTO_TEST_CASE(test_restart_2nd_cue) {
        spdlog::set_level(spdlog::level::debug);
        dmxfish::filters::filter_cue fil = filter_cue ();

        double time_s = 0;

        channel_mapping input_channels = channel_mapping ();
        input_channels.float_channels["time"] = &time_s;

        std::map < std::string, std::string > configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        configuration["cuelist"] =
        "2:200@lin|4:50@lin#next_cue#restart$3:100@lin|6:250@lin#next_cue#restart";

        std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
        fil.setup_filter (configuration, initial_parameters, input_channels, "");

        channel_mapping map = channel_mapping ();
        const std::string name = "t";
        for (int i = 0; i < 18000; i = i + 100){
            time_s = (double) i;

            fil.update();


            if (i == 1000 || i == 10000) {
                const std::string key = "run_mode";
                const std::string _value = "play";
                fil.receive_update_from_gui(key, _value);
            }

            fil.get_output_channels(map, name);
            for (auto it = map.eight_bit_channels.begin();
                 it != map.eight_bit_channels.end(); ++it) {
                uint8_t tester;
                if (time_s < 1000){
                    tester = 0;
                } else if (time_s < 3000){
                    tester = std::round((double) 200 * (time_s-1000)/2000);
                } else if (time_s < 5000){
                    tester = std::round((double) 200 - 150 * (time_s-3000)/2000);
                } else if (time_s < 8000){
                    tester = std::round((double) 50 + 50 * (time_s-5000)/3000);
                } else if (time_s < 10000){
                    tester = std::round((double) 100 + 150 * (time_s-8000)/3000);
                } else if (time_s < 13000){
                    tester = std::round((double) 200 - 100 * (time_s-10000)/3000);
                } else if (time_s < 16000){
                    tester = std::round((double) 100 + 150 * (time_s-13000)/3000);
                } else {
                    tester = 250;
                }
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


BOOST_AUTO_TEST_CASE(test_start_again_whole_cuelist) {
        spdlog::set_level(spdlog::level::debug);
        dmxfish::filters::filter_cue fil = filter_cue ();

        double time_s = 0;

        channel_mapping input_channels = channel_mapping ();
        input_channels.float_channels["time"] = &time_s;

        std::map < std::string, std::string > configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "start_again";

        configuration["cuelist"] =
        "2:200@lin|4:50@lin#next_cue#restart$3:100@lin|6:250@lin#next_cue#restart";

        std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
        fil.setup_filter (configuration, initial_parameters, input_channels, "");

        channel_mapping map = channel_mapping ();
        const std::string name = "t";
        for (int i = 0; i < 24000; i = i + 100){
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
                uint8_t tester;
                if (time_s < 1000){
                    tester = 0;
                } else if (time_s < 3000){
                    tester = std::round((double) 200 * (time_s-1000)/2000);
                } else if (time_s < 5000){
                    tester = std::round((double) 200 - 150 * (time_s-3000)/2000);
                } else if (time_s < 8000){
                    tester = std::round((double) 50 + 50 * (time_s-5000)/3000);
                } else if (time_s < 11000){
                    tester = std::round((double) 100 + 150 * (time_s-8000)/3000);
                } else if (time_s < 13000){
                    tester = std::round((double) 250 - 50 * (time_s-11000)/2000);
                } else if (time_s < 15000){
                    tester = std::round((double) 200 - 150 * (time_s-13000)/2000);
                } else if (time_s < 18000){
                    tester = std::round((double) 50 + 50 * (time_s-15000)/3000);
                } else if (time_s < 21000){
                    tester = std::round((double) 100 + 150 * (time_s-18000)/3000);
                } else if (time_s < 23000){
                    tester = std::round((double) 250 - 50 * (time_s-21000)/2000);
                } else if (time_s < 25000){
                    tester = std::round((double) 200 - 150 * (time_s-23000)/2000);
                }
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


BOOST_AUTO_TEST_CASE(test_to_next_cue) {
        spdlog::set_level(spdlog::level::debug);
        dmxfish::filters::filter_cue fil = filter_cue ();

        double time_s = 0;

        channel_mapping input_channels = channel_mapping ();
        input_channels.float_channels["time"] = &time_s;

        std::map < std::string, std::string > configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "start_again";

        configuration["cuelist"] =
        "2:200@lin|4:50@lin#next_cue#restart$3:100@lin|6:250@lin#next_cue#do_nothing";

        std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
        fil.setup_filter (configuration, initial_parameters, input_channels, "");

        channel_mapping map = channel_mapping ();
        const std::string name = "t";
        for (int i = 0; i < 24000; i = i + 100){
            time_s = (double) i;

            if (i == 1000) {
                const std::string key = "run_mode";
                const std::string _value = "to_next_cue";
                fil.receive_update_from_gui(key, _value);
            }

            if (i == 6000) {
                const std::string key = "run_mode";
                const std::string _value = "play";
                fil.receive_update_from_gui(key, _value);
            }

            fil.update();
            fil.get_output_channels(map, name);
            for (auto it = map.eight_bit_channels.begin();
                 it != map.eight_bit_channels.end(); ++it) {
                uint8_t tester;
                if (time_s < 1000){
                    tester = 0;
                } else if (time_s < 3000){
                    tester = std::round((double) 200 * (time_s-1000)/2000);
                } else if (time_s < 5000){
                    tester = std::round((double) 200 - 150 * (time_s-3000)/2000);
                } else if (time_s < 6000){
                    tester = std::round((double) 50);
                } else if (time_s < 9000){
                    tester = std::round((double) 50 + 50 * (time_s-6000)/3000);
                } else if (time_s < 12000){
                    tester = std::round((double) 100 + 150 * (time_s-9000)/3000);
                } else if (time_s < 14000){
                    tester = std::round((double) 250 - 50 * (time_s-12000)/2000);
                } else if (time_s < 16000){
                    tester = std::round((double) 200 - 150 * (time_s-14000)/2000);
                } else if (time_s < 19000){
                    tester = std::round((double) 50 + 50 * (time_s-16000)/3000);
                } else if (time_s < 22000){
                    tester = std::round((double) 100 + 150 * (time_s-19000)/3000);
                } else if (time_s < 24000){
                    tester = std::round((double) 250 - 50 * (time_s-22000)/2000);
                } else if (time_s < 26000){
                    tester = std::round((double) 200 - 150 * (time_s-24000)/2000);
//                } else {
//                    tester = 250;
                }
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

BOOST_AUTO_TEST_CASE(test_to_next_cue_twice) {
        spdlog::set_level(spdlog::level::debug);
        dmxfish::filters::filter_cue fil = filter_cue ();

        double time_s = 0;

        channel_mapping input_channels = channel_mapping ();
        input_channels.float_channels["time"] = &time_s;

        std::map < std::string, std::string > configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "start_again";

        configuration["cuelist"] =
        "2:200@lin|4:50@lin#next_cue#restart$3:100@lin|6:250@lin#next_cue#do_nothing";

        std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
        fil.setup_filter (configuration, initial_parameters, input_channels, "");

        channel_mapping map = channel_mapping ();
        const std::string name = "t";
        for (int i = 0; i < 29000; i = i + 100){
            time_s = (double) i;

            if (i == 1000) {
                const std::string key = "run_mode";
                const std::string _value = "to_next_cue";
                fil.receive_update_from_gui(key, _value);
            }

            if (i == 6000) {
                const std::string key = "run_mode";
                const std::string _value = "to_next_cue";
                fil.receive_update_from_gui(key, _value);
            }

            if (i == 13000) {
                const std::string key = "run_mode";
                const std::string _value = "to_next_cue";
                fil.receive_update_from_gui(key, _value);
            }

            if (i == 19000) {
                const std::string key = "run_mode";
                const std::string _value = "play";
                fil.receive_update_from_gui(key, _value);
            }

            fil.update();
            fil.get_output_channels(map, name);
            for (auto it = map.eight_bit_channels.begin();
                 it != map.eight_bit_channels.end(); ++it) {
                uint8_t tester;
                if (time_s < 1000){
                    tester = 0;
                } else if (time_s < 3000){
                    tester = std::round((double) 200 * (time_s-1000)/2000);
                } else if (time_s < 5000){
                    tester = std::round((double) 200 - 150 * (time_s-3000)/2000);
                } else if (time_s < 6000){
                    tester = std::round((double) 50);
                } else if (time_s < 9000){
                    tester = std::round((double) 50 + 50 * (time_s-6000)/3000);
                } else if (time_s < 12000){
                    tester = std::round((double) 100 + 150 * (time_s-9000)/3000);
                } else if (time_s < 13000){
                    tester = std::round((double) 250);
                } else if (time_s < 15000){
                    tester = std::round((double) 250 - 50 * (time_s-13000)/2000);
                } else if (time_s < 17000){
                    tester = std::round((double) 200 - 150 * (time_s-15000)/2000);
                } else if (time_s < 19000){
                    tester = std::round((double) 50);
                } else if (time_s < 22000){
                    tester = std::round((double) 50 + 50 * (time_s-19000)/3000);
                } else if (time_s < 25000){
                    tester = std::round((double) 100 + 150 * (time_s-22000)/3000);
                } else if (time_s < 27000){
                    tester = std::round((double) 250 - 50 * (time_s-25000)/2000);
                } else if (time_s < 29000){
                    tester = std::round((double) 200 - 150 * (time_s-27000)/2000);
//                } else {
//                    tester = 250;
                }
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



BOOST_AUTO_TEST_CASE(twocuestwoframesnext) {
        spdlog::set_level(spdlog::level::debug);
        dmxfish::filters::filter_cue fil = filter_cue ();

        double time_s = 0;

        channel_mapping input_channels = channel_mapping ();
        input_channels.float_channels["time"] = &time_s;

        std::map < std::string, std::string > configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        configuration["cuelist"] =
        "2:205@lin|4:50|6:100@lin#next_cue#do_nothing$3:20@lin|7:100@lin#hold#do_nothing";

        std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
        fil.setup_filter (configuration, initial_parameters, input_channels, "");

        channel_mapping map = channel_mapping ();
        const std::string name = "t";
        for (int i = 0; i < 16000; i = i + 100){
            time_s = (double) i;
            if (i == 1000 || i == 7000) {
                const std::string key = "run_mode";
                const std::string _value = "play";
                fil.receive_update_from_gui(key, _value);
            }

            fil.update();
            fil.get_output_channels(map, name);
            for (auto it = map.eight_bit_channels.begin();
                 it != map.eight_bit_channels.end(); ++it) {
                uint8_t tester;
                if (time_s < 1000){
                    tester = 0;
                } else if (time_s < 3000){
                    tester = std::round((double) 205 * (time_s-1000)/2000);
                } else if (time_s < 5000){
                    tester = std::round((double) 205 - (double) 155 * (time_s-3000)/2000);
                } else if (time_s < 7000){
                    tester = std::round((double) 50 + (double) 50 * (time_s-5000)/2000);
                } else if (time_s < 10000){
                    tester = std::round((double) 100 - 80 * (time_s-7000)/3000);
                } else if (time_s < 14000){
                    tester = std::round((double) 20 + 80 * (time_s-10000)/4000);
                } else {
                    tester = 100;
                }
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



BOOST_AUTO_TEST_CASE(twocuestwoframestart_again) {
        spdlog::set_level(spdlog::level::debug);
        dmxfish::filters::filter_cue fil = filter_cue ();

        double time_s = 0;

        channel_mapping input_channels = channel_mapping ();
        input_channels.float_channels["time"] = &time_s;

        std::map < std::string, std::string > configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        configuration["cuelist"] =
        "2:205@lin|4:50@lin#start_again#do_nothing$3:180@lin|7:90@lin#hold#do_nothing";

        std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
        fil.setup_filter (configuration, initial_parameters, input_channels, "");



        channel_mapping map = channel_mapping ();
        const std::string name = "t";
        for (int i = 0; i < 16000; i = i + 100){
            time_s = (double) i;
            if (i == 1000 || i == 7000) {
                const std::string key = "run_mode";
                const std::string _value = "play";
                fil.receive_update_from_gui(key, _value);
            }

            fil.update();
            fil.get_output_channels(map, name);
            for (auto it = map.eight_bit_channels.begin();
                 it != map.eight_bit_channels.end(); ++it) {
                uint8_t tester;
                if (time_s < 1000) {
                    tester = 0;
                } else if (time_s < 3000) {
                    tester = std::round((double) 205 * (time_s - 1000) / 2000);
                } else if (time_s < 5000) {
                    tester = std::round((double) 205 - (double) 155 * (time_s - 3000) / 2000);
                } else if (time_s < 7000) {
                    tester = std::round((double) 50 + 155 * (time_s - 5000) / 2000);
                } else if (time_s < 9000) {
                    tester = std::round((double) 205 - (double) 155 * (time_s - 7000) / 2000);
                } else if (time_s < 11000) {
                    tester = std::round((double) 50 + 155 * (time_s - 9000) / 2000);
                } else if (time_s < 13000) {
                    tester = std::round((double) 205 - (double) 155 * (time_s - 11000) / 2000);
                } else if (time_s < 15000) {
                    tester = std::round((double) 50 + 155 * (time_s - 13000) / 2000);
                } else if (time_s < 17000) {
                    tester = std::round((double) 205 - (double) 155 * (time_s - 15000) / 2000);
                } else {
                    tester = 100;
                }
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




BOOST_AUTO_TEST_CASE(twocuestwoframeshold) {
        spdlog::set_level(spdlog::level::debug);
        dmxfish::filters::filter_cue fil = filter_cue ();

        double time_s = 0;

        channel_mapping input_channels = channel_mapping ();
        input_channels.float_channels["time"] = &time_s;

        std::map < std::string, std::string > configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        configuration["cuelist"] =
        "2:205@lin|4:50@lin#hold#do_nothing$3:20@lin|7:100@lin#hold#do_nothing";

        std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
        fil.setup_filter (configuration, initial_parameters, input_channels, "");



        channel_mapping map = channel_mapping ();
        const std::string name = "t";
        for (int i = 0; i < 16000; i = i + 100){
            time_s = (double) i;


            if (i == 1000 || i == 7000) {
                const std::string key = "run_mode";
                const std::string _value = "play";
                fil.receive_update_from_gui(key, _value);
            }

            fil.update();
            fil.get_output_channels(map, name);
            for (auto it = map.eight_bit_channels.begin();
                 it != map.eight_bit_channels.end(); ++it) {
                uint8_t tester;
                if (time_s < 1000){
                    tester = 0;
                } else if (time_s < 3000){
                    tester = std::round((double) 205 * (time_s-1000)/2000);
                } else if (time_s < 5000){
                    tester = std::round((double) 205 - (double) 155 * (time_s-3000)/2000);
                } else if (time_s < 7000){
                    tester = 50;
                } else if (time_s < 10000){
                    tester = std::round((double) 50 - 30 * (time_s-7000)/3000);
                } else if (time_s < 14000){
                    tester = std::round((double) 20 + 80 * (time_s-10000)/4000);
                } else {
                    tester = 100;
                }
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


BOOST_AUTO_TEST_CASE(anothercuenext) {
        spdlog::set_level(spdlog::level::debug);
        dmxfish::filters::filter_cue fil = filter_cue ();

        double time_s = 0;

        channel_mapping input_channels = channel_mapping ();
        input_channels.float_channels["time"] = &time_s;

        std::map < std::string, std::string > configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        configuration["cuelist"] =
        "2:200@lin|4:50@lin#next_cue#do_nothing$3:20@lin|7:100@lin#hold#do_nothing$4:60@lin|7:180@lin#hold#do_nothing";

        std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
        fil.setup_filter (configuration, initial_parameters, input_channels, "");

        channel_mapping map = channel_mapping ();
        const std::string name = "t";
        for (int i = 0; i < 16000; i = i + 100){
            time_s = (double) i;


            if (i == 1000) {
                const std::string key = "run_mode";
                const std::string _value = "play";
                fil.receive_update_from_gui(key, _value);
            }
            if (i == 3500) {
                const std::string key = "next_cue";
                const std::string _value = "2";
                fil.receive_update_from_gui(key, _value);
            }

            fil.update();
            fil.get_output_channels(map, name);
            for (auto it = map.eight_bit_channels.begin();
                 it != map.eight_bit_channels.end(); ++it) {
                uint8_t tester;
                if (time_s < 1000){
                    tester = 0;
                } else if (time_s < 3000){
                    tester = std::round((double) 200 * (time_s-1000)/2000);
                } else if (time_s < 5000){
                    tester = std::round((double) 200 - (double) 150 * (time_s-3000)/2000);
                } else if (time_s < 9000){
                    tester = std::round((double) 50 + 10 * (time_s-5000)/4000);
                } else if (time_s < 12000){
                    tester = std::round((double) 60 + 120 * (time_s-9000)/3000);
                } else {
                    tester = 180;
                }
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


BOOST_AUTO_TEST_CASE(runcueimmidiatly) {
        spdlog::set_level(spdlog::level::debug);
        dmxfish::filters::filter_cue fil = filter_cue ();

        double time_s = 0;

        channel_mapping input_channels = channel_mapping ();
        input_channels.float_channels["time"] = &time_s;

        std::map < std::string, std::string > configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        configuration["cuelist"] =
        "2:200@lin|5:50@lin#next_cue#do_nothing$3:20@lin|7:100@lin#hold#do_nothing$4:60@lin|7:180@lin#hold#do_nothing";

        std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
        fil.setup_filter (configuration, initial_parameters, input_channels, "");

        channel_mapping map = channel_mapping ();
        const std::string name = "t";
        for (int i = 0; i < 16000; i = i + 100){
            time_s = (double) i;

            fil.update();

            if (i == 1000) {
                const std::string key = "run_mode";
                const std::string _value = "play";
                fil.receive_update_from_gui(key, _value);
            }
            if (i == 4000) {
                const std::string key = "run_cue";
                const std::string _value = "2";
                fil.receive_update_from_gui(key, _value);
            }

            fil.get_output_channels(map, name);
            for (auto it = map.eight_bit_channels.begin();
                 it != map.eight_bit_channels.end(); ++it) {
                uint8_t tester;
                if (time_s < 1000){
                    tester = 0;
                } else if (time_s < 3000){
                    tester = std::round((double) 200 * (time_s-1000)/2000);
                } else if (time_s < 4000){
                    tester = std::round((double) 200 - (double) 150 * (time_s-3000)/3000);
                } else if (time_s < 8000){
                    tester = std::round((double) 150 - 90 * (time_s-4000)/4000);
                } else if (time_s < 11000){
                    tester = std::round((double) 60 + 120 * (time_s-8000)/3000);
                } else {
                    tester = 180;
                }
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




BOOST_AUTO_TEST_CASE(alotofstuff) {
        spdlog::set_level(spdlog::level::debug);
        dmxfish::filters::filter_cue fil = filter_cue ();

        double time_s = 0;

        channel_mapping input_channels = channel_mapping ();
        input_channels.float_channels["time"] = &time_s;

        std::map < std::string, std::string > configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        configuration["cuelist"] =
        "2:200@lin|4:50@lin#next_cue#restart$3:20@lin|7:100@lin#start_again#do_nothing$4:60@lin|7:180@lin#hold#do_nothing";

        std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
        fil.setup_filter (configuration, initial_parameters, input_channels, "");

        channel_mapping map = channel_mapping ();
        const std::string name = "t";
        for (int i = 0; i < 60000; i = i + 100){
            time_s = (double) i;


            fil.update();

            if (i == 1000 || i == 4000 || i == 16000) {
                const std::string key = "run_mode";
                const std::string _value = "play";
                fil.receive_update_from_gui(key, _value);
            }
            if (i == 43000) {
                const std::string key = "run_mode";
                const std::string _value = "to_next_cue";
                fil.receive_update_from_gui(key, _value);
            }

            if (i == 5000 || i == 32000) {
                const std::string key = "next_cue";
                const std::string _value = "2";
                fil.receive_update_from_gui(key, _value);
            }

            if (i == 38000) {
                const std::string key = "next_cue";
                const std::string _value = "1";
                fil.receive_update_from_gui(key, _value);
            }

            fil.update();
            fil.get_output_channels(map, name);
            for (auto it = map.eight_bit_channels.begin();
                 it != map.eight_bit_channels.end(); ++it) {
                uint8_t tester;
                if (time_s < 1000){
                    tester = 0;
                } else if (time_s < 3000){
                    tester = std::round((double) 200 * (time_s-1000)/2000);
                } else if (time_s < 4000){
                    tester = std::round((double) 200 - 150 * (time_s-3000)/2000);
                } else if (time_s < 6000){
                    tester = std::round((double) 125 + 75 * (time_s-4000)/2000);
                } else if (time_s < 8000){
                    tester = std::round((double) 200 - 150 * (time_s-6000)/2000);
                } else if (time_s < 12000){
                    tester = std::round((double) 50 + 10 * (time_s-8000)/4000);
                } else if (time_s < 15000){
                    tester = std::round((double) 60 + 120 * (time_s-12000)/3000);
                } else if (time_s < 16000){
                    tester = std::round((double) 180);
                } else if (time_s < 18000){
                    tester = std::round((double) 180 + 20 * (time_s-16000)/2000);
                } else if (time_s < 20000){
                    tester = std::round((double) 200 - 150 * (time_s-18000)/2000);
                } else if (time_s < 23000){
                    tester = std::round((double) 50 - 30 * (time_s-20000)/3000);
                } else if (time_s < 27000){
                    tester = std::round((double) 20 + 80 * (time_s-23000)/4000);
                } else if (time_s < 30000){
                    tester = std::round((double) 100 - 80 * (time_s-27000)/3000);
                } else if (time_s < 34000){
                    tester = std::round((double) 20 + 80 * (time_s-30000)/4000);
                } else if (time_s < 38000){
                    tester = std::round((double) 100 - 40 * (time_s-34000)/4000);
                } else if (time_s < 41000){
                    tester = std::round((double) 60 + 120 * (time_s-38000)/3000);
                } else if (time_s < 43000){
                    tester = std::round((double) 180);
                } else if (time_s < 46000){
                    tester = std::round((double) 180 - 160 * (time_s-43000)/3000);
                } else if (time_s < 50000){
                    tester = std::round((double) 20 + 80 * (time_s-46000)/4000);
                } else {
                    tester = 100;
                }
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
