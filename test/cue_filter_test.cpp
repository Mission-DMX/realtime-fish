#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE FISH_TESTS
#include <boost/test/included/unit_test.hpp>
#include "filters/filter_cue.hpp"
#include "lib/logging.hpp"
#include "main.hpp"

#include <sstream>

using namespace dmxfish::filters;

struct Iomanager_Init {
    Iomanager_Init()  { construct_iomanager();}
    ~Iomanager_Init() { destruct_iomanager(); }
};

BOOST_FIXTURE_TEST_SUITE(cue_filter_with_iomanager, Iomanager_Init)

    struct cue_st_test{
        std::vector<uint8_t> eight_bit_frames;
        std::vector<uint16_t> sixteen_bit_frames;
        std::vector<double> float_frames;
        std::vector<dmxfish::dmx::pixel> color_frames;
    };

    struct cue_st_names{
        std::vector<std::string> eight_bit_frames;
        std::vector<std::string> sixteen_bit_frames;
        std::vector<std::string> float_frames;
        std::vector<std::string> color_frames;
    };

    void test_cue_function(
                std::map <std::string,
                std::string>& configuration,
                std::map <std::string, std::string>& initial_parameters,
                std::vector<int>& time_stamps,
                std::map<int, cue_st_test>& test_values,
                cue_st_names& channel_names,
                std::map<int, std::vector<std::tuple<std::string, std::string>>>& update_commands
                ){
        spdlog::set_level(spdlog::level::debug);
        dmxfish::filters::filter_cue fil = filter_cue();

        channel_mapping input_channels = channel_mapping();
        double time_s = 0;
        input_channels.float_channels["time"] = &time_s;

        fil.pre_setup(configuration, initial_parameters, "");
        fil.setup_filter (configuration, initial_parameters, input_channels, "");


        channel_mapping map = channel_mapping();
        const std::string name = "t";
        for (const int& i : time_stamps){
            time_s = (double) i;
            for (auto it = update_commands[i].begin();
                 it != update_commands[i].end(); ++it) {
                auto [key, value] = *it;
                fil.receive_update_from_gui(key, value);
            }


            fil.update();
            fil.get_output_channels(map, name);
            int num = 0;
            for (auto it = channel_names.eight_bit_frames.begin();
                 it != channel_names.eight_bit_frames.end(); ++it) {
                uint8_t tester = test_values[i].eight_bit_frames.at(num);
                BOOST_REQUIRE_MESSAGE(map.eight_bit_channels.contains("t:" + *it), "Output does not contain the 8Bit channel: " + *it);
                std::string error =
                        std::string("Channel (8Bit) ") + *it + " should be " + std::to_string(tester) + " , but is " +
                        std::to_string(*map.eight_bit_channels["t:" + *it]) + " at time: " + std::to_string(time_s);
                BOOST_TEST(*map.eight_bit_channels["t:" + *it] == tester, error);
                num++;
            }
            num = 0;
            for (auto it = channel_names.sixteen_bit_frames.begin();
                 it != channel_names.sixteen_bit_frames.end(); ++it) {
                uint16_t tester = test_values[i].sixteen_bit_frames.at(num);
                BOOST_REQUIRE_MESSAGE(map.sixteen_bit_channels.contains("t:" + *it), "Output does not contain the 16Bit channel: " + *it);
                std::string error =
                        std::string("Channel (16Bit) ") + *it + " should be " + std::to_string(tester) + " , but is " +
                        std::to_string(*map.sixteen_bit_channels["t:" + *it]) + " at time: " + std::to_string(time_s);
                BOOST_TEST(*map.sixteen_bit_channels["t:" + *it] == tester, error);
                num++;
            }
            num = 0;
            for (auto it = channel_names.float_frames.begin();
                 it != channel_names.float_frames.end(); ++it) {
                double tester = test_values[i].float_frames.at(num);
                BOOST_REQUIRE_MESSAGE(map.float_channels.contains("t:" + *it), "Output does not contain the float channel: " + *it);
                std::string error =
                        std::string("Channel (float) ") + *it + " should be " + std::to_string(tester) + " , but is " +
                        std::to_string(*map.float_channels["t:" + *it]) + " at time: " + std::to_string(time_s);
                BOOST_TEST(std::abs(*map.float_channels["t:" + *it] - tester) <= tester * 0.00001, error);
                num++;
            }
            num = 0;
            for (auto it = channel_names.color_frames.begin();
                 it != channel_names.color_frames.end(); ++it) {
                dmxfish::dmx::pixel& tester = test_values[i].color_frames.at(num);
                BOOST_REQUIRE_MESSAGE(map.color_channels.contains("t:" + *it), "Output does not contain the color channel: " + *it);
                std::string error =
                        std::string("Channel (Color.hue) ") + *it + " should be " + std::to_string(tester.hue) + " , but is " +
                        std::to_string((*map.color_channels["t:" + *it]).hue) + " at time: " + std::to_string(time_s);
                BOOST_TEST(std::abs((*map.color_channels["t:" + *it]).hue - tester.hue) <= tester.hue * 0.00001, error);
                error =
                        std::string("Channel (Color.sat) ") + *it + " should be " + std::to_string(tester.saturation) + " , but is " +
                        std::to_string((*map.color_channels["t:" + *it]).saturation) + " at time: " + std::to_string(time_s);
                BOOST_TEST(std::abs((*map.color_channels["t:" + *it]).saturation - tester.saturation) <= tester.saturation * 0.00001, error);
                error =
                        std::string("Channel (Color.ilu) ") + *it + " should be " + std::to_string(tester.iluminance) + " , but is " +
                        std::to_string((*map.color_channels["t:" + *it]).iluminance) + " at time: " + std::to_string(time_s);
                BOOST_TEST(std::abs((*map.color_channels["t:" + *it]).iluminance - tester.iluminance) <= tester.iluminance * 0.00001, error);
                num++;
            }
            map.eight_bit_channels.clear();
            map.sixteen_bit_channels.clear();
            map.float_channels.clear();
            map.color_channels.clear();

        }
    };




BOOST_AUTO_TEST_CASE(onechannelonecueoneframe) {
        std::map <std::string, std::string> configuration;
        std::map <std::string, std::string> initial_parameters;

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";
        configuration["cuelist"] = "2:100@edg#hold#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;


        std::vector<int> time_s;
        for (int tester_time = 0; tester_time < 4000; tester_time = tester_time+ 100) {
            if (tester_time == 1000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            time_s.push_back(tester_time);
            test_values[tester_time].eight_bit_frames.push_back((tester_time < 2000) ? 0 : 100);
        }

        test_cue_function(configuration, initial_parameters, time_s, test_values, channel_names, update_key_values);
}


BOOST_AUTO_TEST_CASE(oneframeeachchanneltype) {
        std::map <std::string, std::string> configuration;
        std::map <std::string, std::string> initial_parameters;

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");
        channel_names.sixteen_bit_frames.push_back("xpos");
        channel_names.float_frames.push_back("ypos");
        channel_names.color_frames.push_back("color");

        configuration["mapping"] = "dimmer:8bit;xpos:16bit;ypos:float;color:color";
        configuration["end_handling"] = "hold";

        configuration["cuelist"] =
                "2:100@sig&16000@e_i&0.8@lin&120,1,1@edg#hold#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;


        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 4000; tester_time= tester_time + 100) {
            if (tester_time== 1000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            time_s.push_back(tester_time);
            uint8_t tester8;
            uint16_t tester16;
            double testerfloat;
            dmxfish::dmx::pixel testercolor = (tester_time < 2000) ? dmxfish::dmx::pixel(0,0,0): dmxfish::dmx::pixel(120,1,1);
            if (tester_time < 1000){
                tester8 = 0;
                tester16 = 0;
                testerfloat = 0;
            } else if (tester_time < 3000) {
                tester8 = (uint8_t) std::round(100 * 1.0 / (1 + std::exp(6 - ((double) tester_time - 1000) / 2000 * 12)));
                tester16 = (uint16_t) std::round(16000 * (((double) tester_time - 1000) / 2000) * (((double) tester_time - 1000) / 2000));
                testerfloat = 0.8 * (tester_time - 1000)/2000;
                ::spdlog::debug(" 8 at {}: {}", tester_time, tester8);
                ::spdlog::debug("16 at {}: {}", tester_time, tester16);
            } else {
                tester8 = 100;
                tester16 = 16000;
                testerfloat = 0.8;
            }
            test_values[tester_time].eight_bit_frames.push_back(tester8);
            test_values[tester_time].sixteen_bit_frames.push_back(tester16);
            test_values[tester_time].float_frames.push_back(testerfloat);
            test_values[tester_time].color_frames.push_back(testercolor);
        }
        test_cue_function(configuration, initial_parameters, time_s, test_values, channel_names, update_key_values);
}


BOOST_AUTO_TEST_CASE(onechanneltwoframes) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_cue fil = filter_cue();

    double time_s = 0;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["time"] = &time_s;

    std::map < std::string, std::string > configuration;
    configuration["mapping"] = "dimmer:8bit";
    configuration["end_handling"] = "hold";

    configuration["cuelist"] =
    "2:255@lin|4:50@lin#hold#do_nothing";

    std::map < std::string, std::string > initial_parameters;
	fil.pre_setup(configuration, initial_parameters, "");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");



    channel_mapping map = channel_mapping();
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
                tester = (uint8_t) std::round((double) 255 * (time_s-1000)/2000);
            } else if (time_s < 5000){
                tester = (uint8_t) std::round((double) 255 - (double) 205 * (time_s-3000)/2000);
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
    dmxfish::filters::filter_cue fil = filter_cue();

    double time_s = 0;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["time"] = &time_s;

    std::map < std::string, std::string > configuration;
    configuration["mapping"] = "dimmer:8bit";
    configuration["end_handling"] = "hold";

    configuration["cuelist"] =
    "2:250@lin|4:50@lin#hold#do_nothing";

    std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
	fil.setup_filter (configuration, initial_parameters, input_channels, "");



    channel_mapping map = channel_mapping();
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
                tester = (uint8_t) std::round((double) 250 * (time_s-1000)/2000);
            } else if (time_s < 4000){
                tester = (uint8_t) std::round((double) 250 - (double) 200 * (time_s-3000)/2000);
            } else if (time_s < 6000){
                tester = (uint8_t) std::round(150);
            } else if (time_s < 8000){
                tester = (uint8_t) std::round((double) 150 + 100 * (time_s-6000)/2000);
            } else if (time_s < 10000){
                tester = (uint8_t) std::round((double) 250 - (double) 200 * (time_s-8000)/2000);
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
    dmxfish::filters::filter_cue fil = filter_cue();

    double time_s = 0;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["time"] = &time_s;

    std::map < std::string, std::string > configuration;
    configuration["mapping"] = "dimmer:8bit";
    configuration["end_handling"] = "hold";

    configuration["cuelist"] =
    "2:255@lin|4:50@lin#hold#do_nothing";

    std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");



    channel_mapping map = channel_mapping();
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
                tester = (uint8_t) std::round((double) 255 * (time_s-1000)/2000);
            } else if (time_s < 2600){
                tester = (uint8_t) std::round((double) 255 * (2100-1000)/2000);
            } else if (time_s < 3500){
                tester = (uint8_t) std::round((double) 255 * (time_s-1500)/2000);
            } else if (time_s < 4300){
                tester = (uint8_t) std::round((double) 255);
            } else if (time_s < 6300){
                tester = (uint8_t) std::round((double) 255 - (double) 205 * (time_s-4300)/2000);
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
    dmxfish::filters::filter_cue fil = filter_cue();

    double time_s = 0;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["time"] = &time_s;

    std::map < std::string, std::string > configuration;
    configuration["mapping"] = "dimmer:8bit";
    configuration["end_handling"] = "hold";

    configuration["cuelist"] =
    "2:200@lin|4:50@lin#hold#restart";

    std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");



    channel_mapping map = channel_mapping();
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
                tester = (uint8_t) std::round((double) 200 * (time_s-1000)/2000);
            } else if (time_s < 4000){
                tester = (uint8_t) std::round((double) 200 - 150 * (time_s-3000)/2000);
            } else if (time_s < 6000){
                tester = (uint8_t) std::round((double) 125 + 75 * (time_s-4000)/2000);
            } else if (time_s < 8000){
                tester = (uint8_t) std::round((double) 200 - 150 * (time_s-6000)/2000);
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
    dmxfish::filters::filter_cue fil = filter_cue();

    double time_s = 0;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["time"] = &time_s;

    std::map < std::string, std::string > configuration;
    configuration["mapping"] = "dimmer:8bit";
    configuration["end_handling"] = "hold";

    configuration["cuelist"] =
    "2:200@lin|4:50@lin#next_cue#restart$3:100@lin|6:250@lin#next_cue#restart";

    std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");

    channel_mapping map = channel_mapping();
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
                tester = (uint8_t) std::round((double) 200 * (time_s-1000)/2000);
            } else if (time_s < 5000){
                tester = (uint8_t) std::round((double) 200 - 150 * (time_s-3000)/2000);
            } else if (time_s < 8000){
                tester = (uint8_t) std::round((double) 50 + 50 * (time_s-5000)/3000);
            } else if (time_s < 10000){
                tester = (uint8_t) std::round((double) 100 + 150 * (time_s-8000)/3000);
            } else if (time_s < 13000){
                tester = (uint8_t) std::round((double) 200 - 100 * (time_s-10000)/3000);
            } else if (time_s < 16000){
                tester = (uint8_t) std::round((double) 100 + 150 * (time_s-13000)/3000);
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
    dmxfish::filters::filter_cue fil = filter_cue();

    double time_s = 0;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["time"] = &time_s;

    std::map < std::string, std::string > configuration;
    configuration["mapping"] = "dimmer:8bit";
    configuration["end_handling"] = "start_again";

    configuration["cuelist"] =
    "2:200@lin|4:50@lin#next_cue#restart$3:100@lin|6:250@lin#next_cue#restart";

    std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");

    channel_mapping map = channel_mapping();
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
                tester = (uint8_t) std::round((double) 200 * (time_s-1000)/2000);
            } else if (time_s < 5000){
                tester = (uint8_t) std::round((double) 200 - 150 * (time_s-3000)/2000);
            } else if (time_s < 8000){
                tester = (uint8_t) std::round((double) 50 + 50 * (time_s-5000)/3000);
            } else if (time_s < 11000){
                tester = (uint8_t) std::round((double) 100 + 150 * (time_s-8000)/3000);
            } else if (time_s < 13000){
                tester = (uint8_t) std::round((double) 250 - 50 * (time_s-11000)/2000);
            } else if (time_s < 15000){
                tester = (uint8_t) std::round((double) 200 - 150 * (time_s-13000)/2000);
            } else if (time_s < 18000){
                tester = (uint8_t) std::round((double) 50 + 50 * (time_s-15000)/3000);
            } else if (time_s < 21000){
                tester = (uint8_t) std::round((double) 100 + 150 * (time_s-18000)/3000);
            } else if (time_s < 23000){
                tester = (uint8_t) std::round((double) 250 - 50 * (time_s-21000)/2000);
            } else if (time_s < 25000){
                tester = (uint8_t) std::round((double) 200 - 150 * (time_s-23000)/2000);
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
    dmxfish::filters::filter_cue fil = filter_cue();

    double time_s = 0;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["time"] = &time_s;

    std::map < std::string, std::string > configuration;
    configuration["mapping"] = "dimmer:8bit";
    configuration["end_handling"] = "start_again";

    configuration["cuelist"] =
    "2:200@lin|4:50@lin#next_cue#restart$3:100@lin|6:250@lin#next_cue#do_nothing";

    std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");

    channel_mapping map = channel_mapping();
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
                tester = (uint8_t) std::round((double) 200 * (time_s-1000)/2000);
            } else if (time_s < 5000){
                tester = (uint8_t) std::round((double) 200 - 150 * (time_s-3000)/2000);
            } else if (time_s < 6000){
                tester = (uint8_t) std::round((double) 50);
            } else if (time_s < 9000){
                tester = (uint8_t) std::round((double) 50 + 50 * (time_s-6000)/3000);
            } else if (time_s < 12000){
                tester = (uint8_t) std::round((double) 100 + 150 * (time_s-9000)/3000);
            } else if (time_s < 14000){
                tester = (uint8_t) std::round((double) 250 - 50 * (time_s-12000)/2000);
            } else if (time_s < 16000){
                tester = (uint8_t) std::round((double) 200 - 150 * (time_s-14000)/2000);
            } else if (time_s < 19000){
                tester = (uint8_t) std::round((double) 50 + 50 * (time_s-16000)/3000);
            } else if (time_s < 22000){
                tester = (uint8_t) std::round((double) 100 + 150 * (time_s-19000)/3000);
            } else if (time_s < 24000){
                tester = (uint8_t) std::round((double) 250 - 50 * (time_s-22000)/2000);
            } else if (time_s < 26000){
                tester = (uint8_t) std::round((double) 200 - 150 * (time_s-24000)/2000);
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
    dmxfish::filters::filter_cue fil = filter_cue();

    double time_s = 0;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["time"] = &time_s;

    std::map < std::string, std::string > configuration;
    configuration["mapping"] = "dimmer:8bit";
    configuration["end_handling"] = "start_again";

    configuration["cuelist"] =
    "2:200@lin|4:50@lin#next_cue#restart$3:100@lin|6:250@lin#next_cue#do_nothing";

    std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");

    channel_mapping map = channel_mapping();
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
                tester = (uint8_t) std::round((double) 200 * (time_s-1000)/2000);
            } else if (time_s < 5000){
                tester = (uint8_t) std::round((double) 200 - 150 * (time_s-3000)/2000);
            } else if (time_s < 6000){
                tester = (uint8_t) std::round((double) 50);
            } else if (time_s < 9000){
                tester = (uint8_t) std::round((double) 50 + 50 * (time_s-6000)/3000);
            } else if (time_s < 12000){
                tester = (uint8_t) std::round((double) 100 + 150 * (time_s-9000)/3000);
            } else if (time_s < 13000){
                tester = (uint8_t) std::round((double) 250);
            } else if (time_s < 15000){
                tester = (uint8_t) std::round((double) 250 - 50 * (time_s-13000)/2000);
            } else if (time_s < 17000){
                tester = (uint8_t) std::round((double) 200 - 150 * (time_s-15000)/2000);
            } else if (time_s < 19000){
                tester = (uint8_t) std::round((double) 50);
            } else if (time_s < 22000){
                tester = (uint8_t) std::round((double) 50 + 50 * (time_s-19000)/3000);
            } else if (time_s < 25000){
                tester = (uint8_t) std::round((double) 100 + 150 * (time_s-22000)/3000);
            } else if (time_s < 27000){
                tester = (uint8_t) std::round((double) 250 - 50 * (time_s-25000)/2000);
            } else if (time_s < 29000){
                tester = (uint8_t) std::round((double) 200 - 150 * (time_s-27000)/2000);
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
    dmxfish::filters::filter_cue fil = filter_cue();

    double time_s = 0;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["time"] = &time_s;

    std::map < std::string, std::string > configuration;
    configuration["mapping"] = "dimmer:8bit";
    configuration["end_handling"] = "hold";

    configuration["cuelist"] =
    "2:205@lin|4:50|6:100@lin#next_cue#do_nothing$3:20@lin|7:100@lin#hold#do_nothing";

    std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");

    channel_mapping map = channel_mapping();
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
                tester = (uint8_t) std::round((double) 205 * (time_s-1000)/2000);
            } else if (time_s < 5000){
                tester = (uint8_t) std::round((double) 205 - (double) 155 * (time_s-3000)/2000);
            } else if (time_s < 7000){
                tester = (uint8_t) std::round((double) 50 + (double) 50 * (time_s-5000)/2000);
            } else if (time_s < 10000){
                tester = (uint8_t) std::round((double) 100 - 80 * (time_s-7000)/3000);
            } else if (time_s < 14000){
                tester = (uint8_t) std::round((double) 20 + 80 * (time_s-10000)/4000);
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
    dmxfish::filters::filter_cue fil = filter_cue();

    double time_s = 0;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["time"] = &time_s;

    std::map < std::string, std::string > configuration;
    configuration["mapping"] = "dimmer:8bit";
    configuration["end_handling"] = "hold";

    configuration["cuelist"] =
    "2:205@lin|4:50@lin#start_again#do_nothing$3:180@lin|7:90@lin#hold#do_nothing";

    std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");



    channel_mapping map = channel_mapping();
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
                tester = (uint8_t) std::round((double) 205 * (time_s - 1000) / 2000);
            } else if (time_s < 5000) {
                tester = (uint8_t) std::round((double) 205 - (double) 155 * (time_s - 3000) / 2000);
            } else if (time_s < 7000) {
                tester = (uint8_t) std::round((double) 50 + 155 * (time_s - 5000) / 2000);
            } else if (time_s < 9000) {
                tester = (uint8_t) std::round((double) 205 - (double) 155 * (time_s - 7000) / 2000);
            } else if (time_s < 11000) {
                tester = (uint8_t) std::round((double) 50 + 155 * (time_s - 9000) / 2000);
            } else if (time_s < 13000) {
                tester = (uint8_t) std::round((double) 205 - (double) 155 * (time_s - 11000) / 2000);
            } else if (time_s < 15000) {
                tester = (uint8_t) std::round((double) 50 + 155 * (time_s - 13000) / 2000);
            } else if (time_s < 17000) {
                tester = (uint8_t) std::round((double) 205 - (double) 155 * (time_s - 15000) / 2000);
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
    dmxfish::filters::filter_cue fil = filter_cue();

    double time_s = 0;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["time"] = &time_s;

    std::map < std::string, std::string > configuration;
    configuration["mapping"] = "dimmer:8bit";
    configuration["end_handling"] = "hold";

    configuration["cuelist"] =
    "2:205@lin|4:50@lin#hold#do_nothing$3:20@lin|7:100@lin#hold#do_nothing";

    std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");



    channel_mapping map = channel_mapping();
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
                tester = (uint8_t) std::round((double) 205 * (time_s-1000)/2000);
            } else if (time_s < 5000){
                tester = (uint8_t) std::round((double) 205 - (double) 155 * (time_s-3000)/2000);
            } else if (time_s < 7000){
                tester = 50;
            } else if (time_s < 10000){
                tester = (uint8_t) std::round((double) 50 - 30 * (time_s-7000)/3000);
            } else if (time_s < 14000){
                tester = (uint8_t) std::round((double) 20 + 80 * (time_s-10000)/4000);
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
    dmxfish::filters::filter_cue fil = filter_cue();

    double time_s = 0;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["time"] = &time_s;

    std::map < std::string, std::string > configuration;
    configuration["mapping"] = "dimmer:8bit";
    configuration["end_handling"] = "hold";

    configuration["cuelist"] =
    "2:200@lin|4:50@lin#next_cue#do_nothing$3:20@lin|7:100@lin#hold#do_nothing$4:60@lin|7:180@lin#hold#do_nothing";

    std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");

    channel_mapping map = channel_mapping();
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
                tester = (uint8_t) std::round((double) 200 * (time_s-1000)/2000);
            } else if (time_s < 5000){
                tester = (uint8_t) std::round((double) 200 - (double) 150 * (time_s-3000)/2000);
            } else if (time_s < 9000){
                tester = (uint8_t) std::round((double) 50 + 10 * (time_s-5000)/4000);
            } else if (time_s < 12000){
                tester = (uint8_t) std::round((double) 60 + 120 * (time_s-9000)/3000);
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
    dmxfish::filters::filter_cue fil = filter_cue();

    double time_s = 0;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["time"] = &time_s;

    std::map < std::string, std::string > configuration;
    configuration["mapping"] = "dimmer:8bit";
    configuration["end_handling"] = "hold";

    configuration["cuelist"] =
    "2:200@lin|5:50@lin#next_cue#do_nothing$3:20@lin|7:100@lin#hold#do_nothing$4:60@lin|7:180@lin#hold#do_nothing";

    std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");

    channel_mapping map = channel_mapping();
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
                tester = (uint8_t) std::round((double) 200 * (time_s-1000)/2000);
            } else if (time_s < 4000){
                tester = (uint8_t) std::round((double) 200 - (double) 150 * (time_s-3000)/3000);
            } else if (time_s < 8000){
                tester = (uint8_t) std::round((double) 150 - 90 * (time_s-4000)/4000);
            } else if (time_s < 11000){
                tester = (uint8_t) std::round((double) 60 + 120 * (time_s-8000)/3000);
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
    dmxfish::filters::filter_cue fil = filter_cue();

    double time_s = 0;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["time"] = &time_s;

    std::map < std::string, std::string > configuration;
    configuration["mapping"] = "dimmer:8bit";
    configuration["end_handling"] = "hold";

    configuration["cuelist"] =
    "2:200@lin|4:50@lin#next_cue#restart$3:20@lin|7:100@lin#start_again#do_nothing$4:60@lin|7:180@lin#hold#do_nothing";

    std::map < std::string, std::string > initial_parameters;

	fil.pre_setup(configuration, initial_parameters, "");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");

    channel_mapping map = channel_mapping();
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
                tester = (uint8_t) std::round((double) 200 * (time_s-1000)/2000);
            } else if (time_s < 4000){
                tester = (uint8_t) std::round((double) 200 - 150 * (time_s-3000)/2000);
            } else if (time_s < 6000){
                tester = (uint8_t) std::round((double) 125 + 75 * (time_s-4000)/2000);
            } else if (time_s < 8000){
                tester = (uint8_t) std::round((double) 200 - 150 * (time_s-6000)/2000);
            } else if (time_s < 12000){
                tester = (uint8_t) std::round((double) 50 + 10 * (time_s-8000)/4000);
            } else if (time_s < 15000){
                tester = (uint8_t) std::round((double) 60 + 120 * (time_s-12000)/3000);
            } else if (time_s < 16000){
                tester = (uint8_t) std::round((double) 180);
            } else if (time_s < 18000){
                tester = (uint8_t) std::round((double) 180 + 20 * (time_s-16000)/2000);
            } else if (time_s < 20000){
                tester = (uint8_t) std::round((double) 200 - 150 * (time_s-18000)/2000);
            } else if (time_s < 23000){
                tester = (uint8_t) std::round((double) 50 - 30 * (time_s-20000)/3000);
            } else if (time_s < 27000){
                tester = (uint8_t) std::round((double) 20 + 80 * (time_s-23000)/4000);
            } else if (time_s < 30000){
                tester = (uint8_t) std::round((double) 100 - 80 * (time_s-27000)/3000);
            } else if (time_s < 34000){
                tester = (uint8_t) std::round((double) 20 + 80 * (time_s-30000)/4000);
            } else if (time_s < 38000){
                tester = (uint8_t) std::round((double) 100 - 40 * (time_s-34000)/4000);
            } else if (time_s < 41000){
                tester = (uint8_t) std::round((double) 60 + 120 * (time_s-38000)/3000);
            } else if (time_s < 43000){
                tester = (uint8_t) std::round((double) 180);
            } else if (time_s < 46000){
                tester = (uint8_t) std::round((double) 180 - 160 * (time_s-43000)/3000);
            } else if (time_s < 50000){
                tester = (uint8_t) std::round((double) 20 + 80 * (time_s-46000)/4000);
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


BOOST_AUTO_TEST_CASE(startdefaultcue) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_cue fil = filter_cue();

    double time_s = 0;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["time"] = &time_s;

    std::map < std::string, std::string > configuration;
    configuration["mapping"] = "dimmer:8bit";
    configuration["end_handling"] = "hold";

    configuration["cuelist"] =
            "2:200@lin|4:50@lin#next_cue#restart$3:20@lin|7:100@lin#next_cue#do_nothing$4:60@lin|7:180@lin#hold#do_nothing";
    configuration["default_cue"] =
            "1";

    std::map < std::string, std::string > initial_parameters;

    fil.pre_setup(configuration, initial_parameters, "");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");

    channel_mapping map = channel_mapping();
    const std::string name = "t";
    fil.scene_activated();
    for (int i = 0; i < 60000; i = i + 100){
        time_s = (double) i;

        fil.update();
        fil.get_output_channels(map, name);
        uint8_t tester;
        for (auto it = map.eight_bit_channels.begin();
             it != map.eight_bit_channels.end(); ++it) {
            if (time_s < 3000){
                tester = (uint8_t) std::round((double) 20 * (time_s)/3000);
            } else if (time_s < 7000) {
                tester = (uint8_t) std::round((double) 20 + 80 * (time_s - 3000) / 4000);
            } else if (time_s < 10000) {
                tester = (uint8_t) std::round((double) 100 - 40 * (time_s - 7000) / 4000);
            } else if (time_s < 11000) {
                tester = (uint8_t) std::round((double) 100 - 40 * (10000 - 7000) / 4000);
            } else if (time_s < 12000) {
                tester = (uint8_t) std::round((double) 100 - 40 * (time_s - 8000) / 4000);
            } else if (time_s < 15000) {
                tester = (uint8_t) std::round((double) 60 + 120 * (time_s - 12000) / 3000);
            } else if (time_s < 20000) {
                tester = 180;
            } else if (time_s < 23000){
                tester = (uint8_t) std::round((double) 180 - 160 * (time_s - 20000) / 3000);
            } else if (time_s < 27000) {
                tester = (uint8_t) std::round((double) 20 + 80 * (time_s - 23000) / 4000);
            } else if (time_s < 29000) {
                tester = (uint8_t) std::round((double) 100 + 100 * (time_s - 27000) / 2000);
            } else if (time_s < 30000) {
                tester = (uint8_t) std::round((double) 200 - 150  * (time_s - 29000) / 2000);
            } else if (time_s < 32000) {
                tester = (uint8_t) std::round((double) 125 + 75  * (time_s - 30000) / 2000);
            } else if (time_s < 34000) {
                tester = (uint8_t) std::round((double) 200 - 150  * (time_s - 32000) / 2000);
            } else if (time_s < 37000){
                tester = (uint8_t) std::round((double) 50 - 30 * (time_s - 34000) / 3000);
            } else if (time_s < 41000) {
                tester = (uint8_t) std::round((double) 20 + 80 * (time_s - 37000) / 4000);
            } else if (time_s < 45000) {
                tester = (uint8_t) std::round((double) 100 - 40 * (time_s - 41000) / 4000);
            } else if (time_s < 47000) {
                tester = (uint8_t) std::round((double) 60 + 120 * (time_s - 45000) / 3000);
            } else if (time_s < 50000){
                tester = (uint8_t) std::round((double) 140 - 120 * (time_s - 47000) / 3000);
            } else if (time_s < 54000) {
                tester = (uint8_t) std::round((double) 20 + 80 * (time_s - 50000) / 4000);
            } else if (time_s < 57000) {
                tester = (uint8_t) std::round((double) 100 - 40 * (time_s - 54000) / 4000);
            } else if (time_s < 59000) {
                tester = (uint8_t) std::round((double) 70 + 130 * (time_s - 57000) / 2000);
            } else if (time_s < 61000) {
                tester = (uint8_t) std::round((double) 200 - 150 * (time_s - 59000) / 2000);
            }
            std::string error =
                    std::string("Channel ") + it->first + " should be " + std::to_string(tester) + " , but is " +
                    std::to_string(*it->second) + " at time: " + std::to_string(time_s);
            BOOST_TEST(*map.eight_bit_channels["t:dimmer"] == tester, error);


            if (i == 5000 || i == 9000 || i == 11000 || i == 13000 || i == 30000) {
                fil.receive_update_from_gui("run_mode", "play");
            }
//
            if (i == 10000) {
                fil.receive_update_from_gui("run_mode", "pause");
            }
            if (i == 24000) {
                fil.receive_update_from_gui("next_cue", "0");
            }
            if (i == 54000) {
                fil.receive_update_from_gui("set_default_cue", "0");
            }
            if (i == 58000) {
                fil.receive_update_from_gui("set_default_cue", "0");
            }
            if (i == 20000 || i == 47000 || i == 57000 || i == 60000) {
                fil.scene_activated();
            }
        }
        map.eight_bit_channels.clear();
        map.sixteen_bit_channels.clear();
        map.float_channels.clear();
        map.color_channels.clear();
    }
}


BOOST_AUTO_TEST_CASE(startanothercuefromstart) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_cue fil = filter_cue();

    double time_s = 0;

    channel_mapping input_channels = channel_mapping();
    input_channels.float_channels["time"] = &time_s;

    std::map < std::string, std::string > configuration;
    configuration["mapping"] = "dimmer:8bit";
    configuration["end_handling"] = "hold";

    configuration["cuelist"] =
            "2:80@lin|4:250@lin#next_cue#restart$3:40@lin|7:120@lin#next_cue#do_nothing$4:200@lin|7:20@lin#hold#do_nothing";

    std::map < std::string, std::string > initial_parameters;

    fil.pre_setup(configuration, initial_parameters, "");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");

    channel_mapping map = channel_mapping();
    const std::string name = "t";
    fil.scene_activated();
    for (int i = 0; i < 60000; i = i + 100){
        time_s = (double) i;

        fil.update();
        fil.get_output_channels(map, name);
        uint8_t tester;
        for (auto it = map.eight_bit_channels.begin();
             it != map.eight_bit_channels.end(); ++it) {
            if (time_s < 3000){
                tester = 0;
            } else if (time_s < 7000) {
                tester = (uint8_t) std::round((double) 0 + 200 * (time_s - 3000) / 4000);
            } else if (time_s < 10000) {
                tester = (uint8_t) std::round((double) 200 - 180 * (time_s - 7000) / 3000);
            } else if (time_s < 12000) {
                tester = 20;
            } else if (time_s < 15000) {
                tester = (uint8_t) std::round((double) 20 + 20 * (time_s - 12000) / 3000);
            } else if (time_s < 17000) {
                tester = (uint8_t) std::round((double) 40 + 80 * (time_s - 15000) / 4000);
            } else if (time_s < 19000) {
                tester = 80;
            } else if (time_s < 20000) {
                tester = (uint8_t) std::round((double) 40 + 80 * (time_s - 17000) / 4000);
            } else if (time_s < 23000) {
                tester = 100;
            } else if (time_s < 25000) {
                tester = (uint8_t) std::round((double) 100 - 20 * (time_s - 23000) / 2000);
            } else if (time_s < 26000) {
                tester = (uint8_t) std::round((double) 80 + 170 * (time_s - 25000) / 2000);
            } else if (time_s < 30000) {
                tester = 165;
            } else if (time_s < 34000) {
                tester = (uint8_t) std::round((double) 165 + 35 * (time_s - 30000) / 4000);
            } else if (time_s < 37000) {
                tester = (uint8_t) std::round((double) 200 - 180 * (time_s - 34000) / 3000);
            } else if (time_s < 39000) {
                tester = 20;
            }
            std::string error =
                    std::string("Channel ") + it->first + " should be " + std::to_string(tester) + " , but is " +
                    std::to_string(*it->second) + " at time: " + std::to_string(time_s);
            BOOST_TEST(*map.eight_bit_channels["t:dimmer"] == tester, error);


            if (i == 3000 || i == 12000 || i == 19000 || i == 23000 || i == 30000) {
                fil.receive_update_from_gui("run_mode", "play");
            }
            if (i == 17000) {
                fil.receive_update_from_gui("run_mode", "pause");
            }
            if (i == 20000 || i == 26000) {
                fil.receive_update_from_gui("run_mode", "stop");
            }
            if (i == 19500) {
                fil.receive_update_from_gui("next_cue", "0");
            }
            if (i == 4000) {
                fil.receive_update_from_gui("next_cue", "1");
            }
            if (i == 1000 || i == 28000) {
                fil.receive_update_from_gui("next_cue", "2");
            }
        }
        map.eight_bit_channels.clear();
        map.sixteen_bit_channels.clear();
        map.float_channels.clear();
        map.color_channels.clear();
    }
}

BOOST_AUTO_TEST_SUITE_END()