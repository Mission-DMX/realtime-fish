#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE FISH_TESTS
#include <boost/test/included/unit_test.hpp>
#include "filters/filter_cue.hpp"
#include "lib/logging.hpp"
#include "main.hpp"
#include <cmath>

#include <sstream>
#define SIGSTRECH 0.0024726231566347743340599073722557624510930726498587540880605924

using namespace dmxfish::filters;

struct Iomanager_Init {
    Iomanager_Init()  { construct_managers();}
    ~Iomanager_Init() { destruct_managers(); }
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

    static std::map<std::string, std::string> CONFIGS;
    static std::map<std::string, std::string> INIT_PARAMS;
    static std::map<int, std::vector<std::tuple<std::string, std::string>>> UPDATES;
    static std::vector<int> ACTIVATIONS;
    static std::map<int, double> TIME_SCALE;

    void test_cue_function(
            std::vector<int>& time_stamps,
            std::map<int, cue_st_test>& test_values,
            cue_st_names& channel_names,
            std::map<std::string, std::string>& configuration = CONFIGS,
            std::map<int, std::vector<std::tuple<std::string, std::string>>>& update_commands = UPDATES,
            std::vector<int>& activate_scenes = ACTIVATIONS,
            std::map<int, double>& time_scale = TIME_SCALE,
            std::map<std::string, std::string>& initial_parameters = INIT_PARAMS
            ){
        spdlog::set_level(spdlog::level::debug);
        dmxfish::filters::filter_cue fil = filter_cue();

        channel_mapping input_channels = channel_mapping();

        double time_s = 0;
        double factor = 1;
        input_channels.float_channels["time"] = &time_s;
        input_channels.float_channels["time_scale"] = &factor;

        fil.pre_setup(configuration, initial_parameters, "");
        fil.setup_filter (configuration, initial_parameters, input_channels, "");


        channel_mapping map = channel_mapping();
        const std::string name = "t";
        fil.get_output_channels(map, name);
        for (const int& i : time_stamps){
            time_s = (double) i;
            if(std::map<int, double>::iterator it = time_scale.find(i); it != time_scale.end()) {
                factor = it->second;
            }

            fil.update();

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
                BOOST_TEST(std::abs(*map.float_channels["t:" + *it] - tester) <= std::abs(tester) * 0.00001, error);
                num++;
            }
            num = 0;
            for (auto it = channel_names.color_frames.begin();
                 it != channel_names.color_frames.end(); ++it) {
                dmxfish::dmx::pixel& tester = test_values[i].color_frames.at(num);
                BOOST_REQUIRE_MESSAGE(map.color_channels.contains("t:" + *it), "Output does not contain the color channel: " + *it);
                std::string error =
                        std::string("Channel (Color.hue) ") + *it + " should be " + std::to_string(tester.getHue()) + " , but is " +
                        std::to_string((*map.color_channels["t:" + *it]).getHue()) + " at time: " + std::to_string(time_s);
                BOOST_TEST(std::abs((*map.color_channels["t:" + *it]).getHue() - tester.getHue()) <= tester.getHue() * 0.00001, error);
                error =
                        std::string("Channel (Color.sat) ") + *it + " should be " + std::to_string(tester.getSaturation()) + " , but is " +
                        std::to_string((*map.color_channels["t:" + *it]).getSaturation()) + " at time: " + std::to_string(time_s);
                BOOST_TEST(std::abs((*map.color_channels["t:" + *it]).getSaturation() - tester.getSaturation()) <= tester.getSaturation() * 0.00001, error);
                error =
                        std::string("Channel (Color.ilu) ") + *it + " should be " + std::to_string(tester.getIluminance()) + " , but is " +
                        std::to_string((*map.color_channels["t:" + *it]).getIluminance()) + " at time: " + std::to_string(time_s);
                BOOST_TEST(std::abs((*map.color_channels["t:" + *it]).getIluminance() - tester.getIluminance()) <= tester.getIluminance() * 0.00001, error);
                num++;
            }
            for (auto it = update_commands[i].begin();
                 it != update_commands[i].end(); ++it) {
                auto [key, value] = *it;
                fil.receive_update_from_gui(key, value);
            }
            if (std::find(activate_scenes.begin(), activate_scenes.end(), i) != activate_scenes.end()) {
                fil.scene_activated();
            }
        }
    };




    BOOST_AUTO_TEST_CASE(onechannelonecueoneframe) {
        std::map <std::string, std::string> configuration;

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";
        configuration["cuelist"] = "2:100@edg#hold#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;

        std::vector<int> time_s;
        for (int tester_time = 0; tester_time < 4000; tester_time = tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            test_values[tester_time].eight_bit_frames.push_back((tester_time < 3000) ? 0 : 100);
        }

        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }

    BOOST_AUTO_TEST_CASE(emptycue) {
        std::map <std::string, std::string> configuration;

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";
        configuration["cuelist"] = "#hold#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;


        std::vector<int> time_s;
        for (int tester_time = 0; tester_time < 4000; tester_time = tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            test_values[tester_time].eight_bit_frames.push_back(0);
        }

        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }
    BOOST_AUTO_TEST_CASE(betweenemptycues) {
        std::map <std::string, std::string> configuration;

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";
        configuration["cuelist"] = "2:100@lin|6:200@lin#next_cue#do_nothing$#next_cue#do_nothing$#next_cue#do_nothing$1:10@lin|3:150@lin#next_cue#do_nothing$2:0@lin|4:240@lin#next_cue#do_nothing$#hold#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;


        std::vector<int> time_s;
        for (int tester_time = 0; tester_time < 16000; tester_time = tester_time + 20) {
            time_s.push_back(tester_time);
            if (tester_time == 1000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            uint8_t tester8;
            if (tester_time < 1000){
                tester8 = 0;
            } else if (tester_time < 3000){
                tester8 = (uint8_t) std::round((double) 100 * ((double) tester_time - 1000) / 2000);
            } else if (tester_time < 7000){
                tester8 = (uint8_t) std::round((double) 100 + (double) 100 * ((double) tester_time - 3000) / 4000);
            } else if (tester_time < 7040){
                tester8 = 200; // due to the fact that i dont check on one update call iterative which cue has valid data
            } else if (tester_time < 8040){
                tester8 = (uint8_t) std::round((double) 200 - (double) 190 * ((double) tester_time - 7040) / 1000);
            } else if (tester_time < 10040){
                tester8 = (uint8_t) std::round((double) 10 + (double) 140 * ((double) tester_time - 8040) / 2000);
            } else if (tester_time == 11660) {
                tester8 = 28;
            } else if (tester_time < 12040){
                tester8 = (uint8_t) std::round((double) 150 - (double) 150 * ((double) tester_time - 10040) / 2000);
            } else if (tester_time < 14040){
                tester8 = (uint8_t) std::round((double) 0 + (double) 240 * ((double) tester_time - 12040) / 2000);
            } else if (tester_time < 14060) {
                tester8 = test_values[14020].eight_bit_frames.at(test_values[tester_time].eight_bit_frames.size());
            } else {
                tester8 = 240;
            }
            test_values[tester_time].eight_bit_frames.push_back(tester8);
        }

        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }

    BOOST_AUTO_TEST_CASE(emptycuestring) {
        std::map <std::string, std::string> configuration;

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";
        configuration["cuelist"] = "";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;


        std::vector<int> time_s;
        for (int tester_time = 0; tester_time < 4000; tester_time = tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            test_values[tester_time].eight_bit_frames.push_back(0);
        }

        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }


    BOOST_AUTO_TEST_CASE(oneframeeachchanneltype) {
        std::map <std::string, std::string> configuration;

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
        for (int tester_time= 0; tester_time < 4000; tester_time = tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            uint8_t tester8;
            uint16_t tester16;
            double testerfloat;
            dmxfish::dmx::pixel testercolor = (tester_time < 3000) ? dmxfish::dmx::pixel((float) 0, (float) 0, (float) 0): dmxfish::dmx::pixel((float) 120, (float) 1, (float) 1);
            if (tester_time < 1000){
                tester8 = 0;
                tester16 = 0;
                testerfloat = 0;
            } else if (tester_time < 3000) {
                tester8 = (uint8_t) std::round(100 * (1.0 / (1 + std::exp(6 - ((double) tester_time - 1000) / 2000 * 12)) - SIGSTRECH) / (1 - 2 * SIGSTRECH));
                tester16 = (uint16_t) std::round(16000 * (((double) tester_time - 1000) / 2000) * (((double) tester_time - 1000) / 2000));
                testerfloat = 0.8 * ((double) tester_time - 1000)/2000;
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
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }

    BOOST_AUTO_TEST_CASE(testcolor) {
        std::map <std::string, std::string> configuration;

        cue_st_names channel_names;
        channel_names.color_frames.push_back("color");
        channel_names.sixteen_bit_frames.push_back("dimmer");

        configuration["mapping"] = "color:color;dimmer:16bit";
        configuration["end_handling"] = "hold";

        configuration["cuelist"] =
                "2:0,1,0.5@lin&400@lin|7:120,1,1@lin&60000@lin#next_cue#do_nothing$3:180,0.2,0.8@lin&20000@lin|6:210,0.8,0.3@lin&0@lin#hold#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;


        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 15000; tester_time = tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            uint16_t tester16;
            dmxfish::dmx::pixel testercolor;
            dmxfish::dmx::pixel testercolor1 = dmxfish::dmx::pixel((float) 0, (float) 0, (float) 0);
            dmxfish::dmx::pixel testercolor2 = dmxfish::dmx::pixel((float) 0, (float) 1, (float) 0.5);
            dmxfish::dmx::pixel testercolor3 = dmxfish::dmx::pixel((float) 120, (float) 1, (float) 1);
            dmxfish::dmx::pixel testercolor4 = dmxfish::dmx::pixel((float) 180, (float) 0.2, (float) 0.8);
            dmxfish::dmx::pixel testercolor5 = dmxfish::dmx::pixel((float) 210, (float) 0.8, (float) 0.3);
            if (tester_time < 1000){
                testercolor = dmxfish::dmx::pixel((float) 0, (float) 0, (float) 0);
                tester16 = 0;
            } else if (tester_time < 3000) {
                testercolor = dmxfish::dmx::pixel((uint16_t) (testercolor1.getRed() + (testercolor2.getRed() - testercolor1.getRed()) * (double) (tester_time - 1000) / 2000),
                                                  (uint16_t) (testercolor1.getGreen() + (testercolor2.getGreen() - testercolor1.getGreen()) * (double) (tester_time - 1000) / 2000),
                                                  (uint16_t) (testercolor1.getBlue() + (testercolor2.getBlue() - testercolor1.getBlue()) * (double) (tester_time - 1000) / 2000));
                tester16 = (uint16_t) std::round(400 * (double) (tester_time - 1000) / 2000);
            } else if (tester_time < 8000) {
                testercolor = dmxfish::dmx::pixel((uint16_t) (testercolor2.getRed() + (testercolor3.getRed() - testercolor2.getRed()) * (double) (tester_time - 3000) / 5000),
                                                  (uint16_t) (testercolor2.getGreen() + (testercolor3.getGreen() - testercolor2.getGreen()) * (double) (tester_time - 3000) / 5000),
                                                  (uint16_t) (testercolor2.getBlue() + (testercolor3.getBlue() - testercolor2.getBlue()) * (double) (tester_time - 3000) / 5000));
                tester16 = (uint16_t) std::round(400 + 59600 * (double) (tester_time - 3000) / 5000);
            } else if (tester_time < 11000) {
                testercolor = dmxfish::dmx::pixel((uint16_t) (testercolor3.getRed() + (testercolor4.getRed() - testercolor3.getRed()) * (double) (tester_time - 8000) / 3000),
                                                  (uint16_t) (testercolor3.getGreen() + (testercolor4.getGreen() - testercolor3.getGreen()) * (double) (tester_time - 8000) / 3000),
                                                  (uint16_t) (testercolor3.getBlue() + (testercolor4.getBlue() - testercolor3.getBlue()) * (double) (tester_time - 8000) / 3000));
                tester16 = (uint16_t) std::round(60000 - 40000 * ((double) tester_time - 8000) / 3000);
            } else if (tester_time < 14000) {
                testercolor = dmxfish::dmx::pixel((uint16_t) (testercolor4.getRed() + (testercolor5.getRed() - testercolor4.getRed()) * (double) (tester_time - 11000) / 3000),
                                                  (uint16_t) (testercolor4.getGreen() + (testercolor5.getGreen() - testercolor4.getGreen()) * (double) (tester_time - 11000) / 3000),
                                                  (uint16_t) (testercolor4.getBlue() + (testercolor5.getBlue() - testercolor4.getBlue()) * (double) (tester_time - 11000) / 3000));
                tester16 = (uint16_t) std::round(20000 - 20000 * (double) (tester_time - 11000) / 3000);
            } else {
                testercolor = dmxfish::dmx::pixel(210, 0.8, 0.3);
                tester16 = 0;
            }
            test_values[tester_time].color_frames.push_back(testercolor);
            test_values[tester_time].sixteen_bit_frames.push_back(tester16);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }


    BOOST_AUTO_TEST_CASE(onechanneltwoframes) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["cuelist"] =
        "2:255@lin|4:50@lin#hold#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 4000; tester_time= tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000 || tester_time == 3000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            uint8_t tester8;
            if (tester_time < 1000){
                tester8 = 0;
            } else if (tester_time < 3000){
                tester8 = (uint8_t) std::round((double) 255 * ((double) tester_time - 1000) / 2000);
            } else if (tester_time < 5000){
                tester8 = (uint8_t) std::round((double) 255 - (double) 205 * ((double) tester_time - 3000) / 2000);
            } else {
                tester8 = 50;
            }
            test_values[tester_time].eight_bit_frames.push_back(tester8);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }


    BOOST_AUTO_TEST_CASE(teststop) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["cuelist"] =
                "2:250@lin|4:50@lin#hold#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 12000; tester_time= tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000 || tester_time == 6000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            if (tester_time == 4000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "stop"));
            }
            uint8_t tester8;
            if (tester_time < 1000){
                tester8 = 0;
            } else if (tester_time < 3000){
                tester8 = (uint8_t) std::round((double) 250 * ((double) tester_time - 1000) / 2000);
            } else if (tester_time < 4000){
                tester8 = (uint8_t) std::round((double) 250 - (double) 200 * ((double) tester_time - 3000) / 2000);
            } else if (tester_time < 6000){
                tester8 = (uint8_t) std::round(150);
            } else if (tester_time < 8000){
                tester8 = (uint8_t) std::round((double) 150 + 100 * ((double) tester_time - 6000) / 2000);
            } else if (tester_time < 10000){
                tester8 = (uint8_t) std::round((double) 250 - (double) 200 * ((double) tester_time - 8000) / 2000);
            } else {
                tester8 = 50;
            }
            test_values[tester_time].eight_bit_frames.push_back(tester8);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }

    BOOST_AUTO_TEST_CASE(sigmoidal) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "v1:8bit;v2:16bit;v3:float";
        configuration["end_handling"] = "hold";

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("v1");
        channel_names.sixteen_bit_frames.push_back("v2");
        channel_names.float_frames.push_back("v3");

        configuration["cuelist"] =
                "10:0@sig&1000@sig&-50@sig|20:255@sig&61000@sig&50@sig|40:55@sig&31000@sig&1050@sig#hold#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 43000; tester_time= tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            uint8_t tester8;
            uint16_t tester16;
            double testerfloat;
            if (tester_time < 1100) {
                tester8 = 0;
                tester16 = 0;
                testerfloat = 0;
            } else if (tester_time < 11000) {
                tester8 = (uint8_t) std::round(0 * (1.0 / (1 + std::exp(6 - ((double) tester_time - 1000) / 10000 * 12)) - SIGSTRECH) / (1 - 2 * SIGSTRECH));
                tester16 = (uint16_t) std::round((double) 1000 * (1.0 / (1 + std::exp(6 - ((double) tester_time - 1000) / 10000 * 12)) - SIGSTRECH) / (1 - 2 * SIGSTRECH));
                testerfloat = (double) -50 * (1.0 / (1 + std::exp(6 - ((double) tester_time - 1000) / 10000 * 12)) - SIGSTRECH) / (1 - 2 * SIGSTRECH);
            } else if (tester_time < 21000) {
                tester8 = (uint8_t) std::round(255 * (1.0 / (1 + std::exp(6 - ((double) tester_time - 11000) / 10000 * 12)) - SIGSTRECH) / (1 - 2 * SIGSTRECH));
                tester16 = (uint16_t) std::round((double) 1000 + 60000 * (1.0 / (1 + std::exp(6 - ((double) tester_time - 11000) / 10000 * 12)) - SIGSTRECH) / (1 - 2 * SIGSTRECH));
                testerfloat = (double) -50 + 100 * (1.0 / (1 + std::exp(6 - ((double) tester_time - 11000) / 10000 * 12)) - SIGSTRECH) / (1 - 2 * SIGSTRECH);
            } else if (tester_time < 41000){
                tester8 = (uint8_t) std::round(255 - 200 * (1.0 / (1 + std::exp(6 - ((double) tester_time - 21000) / 20000 * 12)) - SIGSTRECH) / (1 - 2 * SIGSTRECH));
                tester16 = (uint16_t) std::round(61000 - 30000 * (1.0 / (1 + std::exp(6 - ((double) tester_time - 21000) / 20000 * 12)) - SIGSTRECH) / (1 - 2 * SIGSTRECH));
                testerfloat = 50 + 1000 * (1.0 / (1 + std::exp(6 - ((double) tester_time - 21000) / 20000 * 12)) - SIGSTRECH) / (1 - 2 * SIGSTRECH);
            } else {
                tester8 = 55;
                tester16 = 31000;
                testerfloat = 1050;
            }
            test_values[tester_time].eight_bit_frames.push_back(tester8);
            test_values[tester_time].sixteen_bit_frames.push_back(tester16);
            test_values[tester_time].float_frames.push_back(testerfloat);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }

    BOOST_AUTO_TEST_CASE(sigmoidalzerofirst) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "v1:8bit;v2:16bit;v3:float";
        configuration["end_handling"] = "hold";

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("v1");
        channel_names.sixteen_bit_frames.push_back("v2");
        channel_names.float_frames.push_back("v3");

        configuration["cuelist"] =
                "0:0@sig&1000@sig&-50@sig|20:255@sig&61000@sig&50@sig|40:55@sig&31000@sig&1050@sig#hold#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 43000; tester_time= tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            uint8_t tester8;
            uint16_t tester16;
            double testerfloat;
            if (tester_time < 1100) {
                tester8 = 0;
                tester16 = 0;
                testerfloat = 0;
            } else if (tester_time < 21000) {
                tester8 = (uint8_t) std::round(255 * (1.0 / (1 + std::exp(6 - ((double) tester_time - 1000) / 20000 * 12)) - SIGSTRECH) / (1 - 2 * SIGSTRECH));
                tester16 = (uint16_t) std::round((double) 1000 + 60000 * (1.0 / (1 + std::exp(6 - ((double) tester_time - 1000) / 20000 * 12)) - SIGSTRECH) / (1 - 2 * SIGSTRECH));
                testerfloat = (double) -50 + 100 * (1.0 / (1 + std::exp(6 - ((double) tester_time - 1000) / 20000 * 12)) - SIGSTRECH) / (1 - 2 * SIGSTRECH);
            } else if (tester_time < 41000){
                tester8 = (uint8_t) std::round(255 - 200 * (1.0 / (1 + std::exp(6 - ((double) tester_time - 21000) / 20000 * 12)) - SIGSTRECH) / (1 - 2 * SIGSTRECH));
                tester16 = (uint16_t) std::round(61000 - 30000 * (1.0 / (1 + std::exp(6 - ((double) tester_time - 21000) / 20000 * 12)) - SIGSTRECH) / (1 - 2 * SIGSTRECH));
                testerfloat = 50 + 1000 * (1.0 / (1 + std::exp(6 - ((double) tester_time - 21000) / 20000 * 12)) - SIGSTRECH) / (1 - 2 * SIGSTRECH);
            } else {
                tester8 = 55;
                tester16 = 31000;
                testerfloat = 1050;
            }
            test_values[tester_time].eight_bit_frames.push_back(tester8);
            test_values[tester_time].sixteen_bit_frames.push_back(tester16);
            test_values[tester_time].float_frames.push_back(testerfloat);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }


    BOOST_AUTO_TEST_CASE(seccueatzero) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "v1:16bit";
        configuration["end_handling"] = "hold";

        cue_st_names channel_names;
        channel_names.sixteen_bit_frames.push_back("v1");

        configuration["cuelist"] =
                "2:61000@lin|5:10@lin#next_cue#do_nothing$0:16000@lin|4:200@lin#hold#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 11000; tester_time= tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            uint16_t tester16;
            if (tester_time < 1000) {
                tester16 = 0;
            } else if (tester_time < 3000) {
                tester16 = (uint16_t) std::round(61000 * ((double) tester_time - 1000) / 2000);
            } else if (tester_time < 6000) {
                tester16 = (uint16_t) std::round(61000 - 60990 * ((double) tester_time - 3000) / 3000);
            } else if (tester_time < 10000){
                tester16 = (uint16_t) std::round(16000 - 15800 * ((double) tester_time - 6000) / 4000);
            } else {
                tester16 = 200;
            }
            test_values[tester_time].sixteen_bit_frames.push_back(tester16);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }

    BOOST_AUTO_TEST_CASE(firstcueatzero) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "v1:16bit";
        configuration["end_handling"] = "hold";

        cue_st_names channel_names;
        channel_names.sixteen_bit_frames.push_back("v1");

        configuration["cuelist"] =
                "0:61000@lin|5:10@lin#next_cue#do_nothing$0:16000@lin|4:200@lin#hold#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 11000; tester_time= tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            uint16_t tester16;
            if (tester_time < 1100) { // here it also needs one step for the update
                tester16 = 0;
            } else if (tester_time < 6000) {
                tester16 = (uint16_t) std::round(61000 - 60990 * ((double) tester_time - 1000) / 5000);
            } else if (tester_time < 10000){
                tester16 = (uint16_t) std::round(16000 - 15800 * ((double) tester_time - 6000) / 4000);
            } else {
                tester16 = 200;
            }
            test_values[tester_time].sixteen_bit_frames.push_back(tester16);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }

    BOOST_AUTO_TEST_CASE(testpause) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["cuelist"] =
        "2:255@lin|4:50@lin#hold#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 8000; tester_time= tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000 || tester_time == 2600 || tester_time == 4300) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            if (tester_time == 2100 || tester_time == 3500) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "pause"));
            }
            uint8_t tester8;
            if (tester_time < 1000){
                tester8 = 0;
            } else if (tester_time < 2100){
                tester8 = (uint8_t) std::round((double) 255 * ((double) tester_time - 1000) / 2000);
            } else if (tester_time < 2600){
                tester8 = (uint8_t) std::round((double) 255 * (2100 - 1000) / 2000);
            } else if (tester_time < 3500){
                tester8 = (uint8_t) std::round((double) 255 * ((double) tester_time - 1500) / 2000);
            } else if (tester_time < 4300){
                tester8 = (uint8_t) std::round((double) 255);
            } else if (tester_time < 6300){
                tester8 = (uint8_t) std::round((double) 255 - (double) 205 * ((double) tester_time - 4300) / 2000);
            } else {
                tester8 = 50;
            }
            test_values[tester_time].eight_bit_frames.push_back(tester8);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }

    BOOST_AUTO_TEST_CASE(test_restart) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["cuelist"] =
        "2:200@lin|4:50@lin#hold#restart";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 10000; tester_time= tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000 || tester_time == 4000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            uint8_t tester8;
            if (tester_time < 1000){
                tester8 = 0;
            } else if (tester_time < 3000){
                tester8 = (uint8_t) std::round((double) 200 * ((double) tester_time - 1000) / 2000);
            } else if (tester_time < 4000){
                tester8 = (uint8_t) std::round((double) 200 - 150 * ((double) tester_time - 3000) / 2000);
            } else if (tester_time < 6000){
                tester8 = (uint8_t) std::round((double) 125 + 75 * ((double) tester_time - 4000) / 2000);
            } else if (tester_time < 8000){
                tester8 = (uint8_t) std::round((double) 200 - 150 * ((double) tester_time - 6000) / 2000);
            } else {
                tester8 = 50;
            }
            test_values[tester_time].eight_bit_frames.push_back(tester8);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }



    BOOST_AUTO_TEST_CASE(test_restart_2nd_cue) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["cuelist"] =
        "2:200@lin|4:50@lin#next_cue#restart$3:100@lin|6:250@lin#next_cue#restart";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 18000; tester_time= tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000 || tester_time == 10000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            uint8_t tester8;
            if (tester_time < 1000){
                tester8 = 0;
            } else if (tester_time < 3000){
                tester8 = (uint8_t) std::round((double) 200 * ((double) tester_time - 1000) / 2000);
            } else if (tester_time < 5000){
                tester8 = (uint8_t) std::round((double) 200 - 150 * ((double) tester_time - 3000) / 2000);
            } else if (tester_time < 8000){
                tester8 = (uint8_t) std::round((double) 50 + 50 * ((double) tester_time - 5000) / 3000);
            } else if (tester_time < 10000){
                tester8 = (uint8_t) std::round((double) 100 + 150 * ((double) tester_time - 8000) / 3000);
            } else if (tester_time < 13000){
                tester8 = (uint8_t) std::round((double) 200 - 100 * ((double) tester_time - 10000) / 3000);
            } else if (tester_time < 16000){
                tester8 = (uint8_t) std::round((double) 100 + 150 * ((double) tester_time - 13000) / 3000);
            } else {
                tester8 = 250;
            }
            test_values[tester_time].eight_bit_frames.push_back(tester8);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }


    BOOST_AUTO_TEST_CASE(test_start_again_whole_cuelist) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "start_again";

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["cuelist"] =
        "2:200@lin|4:50@lin#next_cue#restart$3:100@lin|6:250@lin#next_cue#restart";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 24000; tester_time= tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            uint8_t tester8;
            if (tester_time < 1000){
                tester8 = 0;
            } else if (tester_time < 3000){
                tester8 = (uint8_t) std::round((double) 200 * ((double) tester_time - 1000) / 2000);
            } else if (tester_time < 5000){
                tester8 = (uint8_t) std::round((double) 200 - 150 * ((double) tester_time - 3000) / 2000);
            } else if (tester_time < 8000){
                tester8 = (uint8_t) std::round((double) 50 + 50 * ((double) tester_time - 5000) / 3000);
            } else if (tester_time < 11000){
                tester8 = (uint8_t) std::round((double) 100 + 150 * ((double) tester_time - 8000) / 3000);
            } else if (tester_time < 13000){
                tester8 = (uint8_t) std::round((double) 250 - 50 * ((double) tester_time - 11000) / 2000);
            } else if (tester_time < 15000){
                tester8 = (uint8_t) std::round((double) 200 - 150 * ((double) tester_time - 13000) / 2000);
            } else if (tester_time < 18000){
                tester8 = (uint8_t) std::round((double) 50 + 50 * ((double) tester_time - 15000) / 3000);
            } else if (tester_time < 21000){
                tester8 = (uint8_t) std::round((double) 100 + 150 * ((double) tester_time - 18000) / 3000);
            } else if (tester_time < 23000){
                tester8 = (uint8_t) std::round((double) 250 - 50 * ((double) tester_time - 21000) / 2000);
            } else if (tester_time < 25000){
                tester8 = (uint8_t) std::round((double) 200 - 150 * ((double) tester_time - 23000) / 2000);
            }
            test_values[tester_time].eight_bit_frames.push_back(tester8);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }


    BOOST_AUTO_TEST_CASE(test_to_next_cue) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "start_again";

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["cuelist"] =
        "2:200@lin|4:50@lin#next_cue#restart$3:100@lin|6:250@lin#next_cue#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 24000; tester_time= tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "to_next_cue"));
            }
            if (tester_time == 6000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            uint8_t tester8;
            if (tester_time < 1000){
                tester8 = 0;
            } else if (tester_time < 3000){
                tester8 = (uint8_t) std::round((double) 200 * ((double) tester_time - 1000) / 2000);
            } else if (tester_time < 5000){
                tester8 = (uint8_t) std::round((double) 200 - 150 * ((double) tester_time - 3000) / 2000);
            } else if (tester_time < 6000){
                tester8 = (uint8_t) std::round((double) 50);
            } else if (tester_time < 9000){
                tester8 = (uint8_t) std::round((double) 50 + 50 * ((double) tester_time - 6000) / 3000);
            } else if (tester_time < 12000){
                tester8 = (uint8_t) std::round((double) 100 + 150 * ((double) tester_time - 9000) / 3000);
            } else if (tester_time < 14000){
                tester8 = (uint8_t) std::round((double) 250 - 50 * ((double) tester_time - 12000) / 2000);
            } else if (tester_time < 16000){
                tester8 = (uint8_t) std::round((double) 200 - 150 * ((double) tester_time - 14000) / 2000);
            } else if (tester_time < 19000){
                tester8 = (uint8_t) std::round((double) 50 + 50 * ((double) tester_time - 16000) / 3000);
            } else if (tester_time < 22000){
                tester8 = (uint8_t) std::round((double) 100 + 150 * ((double) tester_time - 19000) / 3000);
            } else if (tester_time < 24000){
                tester8 = (uint8_t) std::round((double) 250 - 50 * ((double) tester_time - 22000) / 2000);
            } else if (tester_time < 26000){
                tester8 = (uint8_t) std::round((double) 200 - 150 * ((double) tester_time - 24000) / 2000);
//                } else {
//                    tester8 = 250;
            }
            test_values[tester_time].eight_bit_frames.push_back(tester8);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }

    BOOST_AUTO_TEST_CASE(test_to_next_cue_twice) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "start_again";

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["cuelist"] =
        "2:200@lin|4:50@lin#next_cue#restart$3:100@lin|6:250@lin#next_cue#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 29000; tester_time= tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000 || tester_time == 6000 || tester_time == 13000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "to_next_cue"));
            }
            if (tester_time == 19000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            uint8_t tester8;
            if (tester_time < 1000){
                tester8 = 0;
            } else if (tester_time < 3000){
                tester8 = (uint8_t) std::round((double) 200 * ((double) tester_time - 1000) / 2000);
            } else if (tester_time < 5000){
                tester8 = (uint8_t) std::round((double) 200 - 150 * ((double) tester_time - 3000) / 2000);
            } else if (tester_time < 6000){
                tester8 = (uint8_t) std::round((double) 50);
            } else if (tester_time < 9000){
                tester8 = (uint8_t) std::round((double) 50 + 50 * ((double) tester_time - 6000) / 3000);
            } else if (tester_time < 12000){
                tester8 = (uint8_t) std::round((double) 100 + 150 * ((double) tester_time - 9000) / 3000);
            } else if (tester_time < 13000){
                tester8 = (uint8_t) std::round((double) 250);
            } else if (tester_time < 15000){
                tester8 = (uint8_t) std::round((double) 250 - 50 * ((double) tester_time - 13000) / 2000);
            } else if (tester_time < 17000){
                tester8 = (uint8_t) std::round((double) 200 - 150 * ((double) tester_time - 15000) / 2000);
            } else if (tester_time < 19000){
                tester8 = (uint8_t) std::round((double) 50);
            } else if (tester_time < 22000){
                tester8 = (uint8_t) std::round((double) 50 + 50 * ((double) tester_time - 19000) / 3000);
            } else if (tester_time < 25000){
                tester8 = (uint8_t) std::round((double) 100 + 150 * ((double) tester_time - 22000) / 3000);
            } else if (tester_time < 27000){
                tester8 = (uint8_t) std::round((double) 250 - 50 * ((double) tester_time - 25000) / 2000);
            } else if (tester_time < 29000){
                tester8 = (uint8_t) std::round((double) 200 - 150 * ((double) tester_time - 27000) / 2000);
//                } else {
//                    tester8 = 250;
            }
            test_values[tester_time].eight_bit_frames.push_back(tester8);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }



    BOOST_AUTO_TEST_CASE(twocuestwoframesnext) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["cuelist"] =
        "2:205@lin|4:50|6:100@lin#next_cue#do_nothing$3:20@lin|7:100@lin#hold#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 16000; tester_time= tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000 || tester_time == 7000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            uint8_t tester8;
            if (tester_time < 1000){
                tester8 = 0;
            } else if (tester_time < 3000){
                tester8 = (uint8_t) std::round((double) 205 * ((double) tester_time - 1000) / 2000);
            } else if (tester_time < 5000){
                tester8 = (uint8_t) std::round((double) 205 - (double) 155 * ((double) tester_time - 3000) / 2000);
            } else if (tester_time < 7000){
                tester8 = (uint8_t) std::round((double) 50 + (double) 50 * ((double) tester_time - 5000) / 2000);
            } else if (tester_time < 10000){
                tester8 = (uint8_t) std::round((double) 100 - 80 * ((double) tester_time - 7000) / 3000);
            } else if (tester_time < 14000){
                tester8 = (uint8_t) std::round((double) 20 + 80 * ((double) tester_time - 10000) / 4000);
            } else {
                tester8 = 100;
            }
            test_values[tester_time].eight_bit_frames.push_back(tester8);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }



    BOOST_AUTO_TEST_CASE(twocuestwoframestart_again) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["cuelist"] =
        "2:205@lin|4:50@lin#start_again#do_nothing$3:180@lin|7:90@lin#hold#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 16000; tester_time = tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000 || tester_time == 7000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            uint8_t tester8;
            if (tester_time < 1000) {
                tester8 = 0;
            } else if (tester_time < 3000) {
                tester8 = (uint8_t) std::round((double) 205 * ((double) tester_time - 1000) / 2000);
            } else if (tester_time < 5000) {
                tester8 = (uint8_t) std::round((double) 205 - (double) 155 * ((double) tester_time - 3000) / 2000);
            } else if (tester_time < 7000) {
                tester8 = (uint8_t) std::round((double) 50 + 155 * ((double) tester_time - 5000) / 2000);
            } else if (tester_time < 9000) {
                tester8 = (uint8_t) std::round((double) 205 - (double) 155 * ((double) tester_time - 7000) / 2000);
            } else if (tester_time < 11000) {
                tester8 = (uint8_t) std::round((double) 50 + 155 * ((double) tester_time - 9000) / 2000);
            } else if (tester_time < 13000) {
                tester8 = (uint8_t) std::round((double) 205 - (double) 155 * ((double) tester_time - 11000) / 2000);
            } else if (tester_time < 15000) {
                tester8 = (uint8_t) std::round((double) 50 + 155 * ((double) tester_time - 13000) / 2000);
            } else if (tester_time < 17000) {
                tester8 = (uint8_t) std::round((double) 205 - (double) 155 * ((double) tester_time - 15000) / 2000);
            } else {
                tester8 = 100;
            }
            test_values[tester_time].eight_bit_frames.push_back(tester8);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }




    BOOST_AUTO_TEST_CASE(twocuestwoframeshold) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["cuelist"] =
        "2:205@lin|4:50@lin#hold#do_nothing$3:20@lin|7:100@lin#hold#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 16000; tester_time= tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000 || tester_time == 7000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            uint8_t tester8;
            if (tester_time < 1000){
                tester8 = 0;
            } else if (tester_time < 3000){
                tester8 = (uint8_t) std::round((double) 205 * ((double) tester_time - 1000) / 2000);
            } else if (tester_time < 5000){
                tester8 = (uint8_t) std::round((double) 205 - (double) 155 * ((double) tester_time - 3000) / 2000);
            } else if (tester_time < 7000){
                tester8 = 50;
            } else if (tester_time < 10000){
                tester8 = (uint8_t) std::round((double) 50 - 30 * ((double) tester_time - 7000) / 3000);
            } else if (tester_time < 14000){
                tester8 = (uint8_t) std::round((double) 20 + 80 * ((double) tester_time - 10000) / 4000);
            } else {
                tester8 = 100;
            }
            test_values[tester_time].eight_bit_frames.push_back(tester8);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }


    BOOST_AUTO_TEST_CASE(anothercuenext) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["cuelist"] =
        "2:200@lin|4:50@lin#next_cue#do_nothing$3:20@lin|7:100@lin#hold#do_nothing$4:60@lin|7:180@lin#hold#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 16000; tester_time= tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            if (tester_time == 3500) {
                update_key_values[tester_time].push_back(std::tuple("next_cue", "2"));
            }
            uint8_t tester8;
            if (tester_time < 1000){
                tester8 = 0;
            } else if (tester_time < 3000){
                tester8 = (uint8_t) std::round((double) 200 * ((double) tester_time - 1000) / 2000);
            } else if (tester_time < 5000){
                tester8 = (uint8_t) std::round((double) 200 - (double) 150 * ((double) tester_time - 3000) / 2000);
            } else if (tester_time < 9000){
                tester8 = (uint8_t) std::round((double) 50 + 10 * ((double) tester_time - 5000) / 4000);
            } else if (tester_time < 12000){
                tester8 = (uint8_t) std::round((double) 60 + 120 * ((double) tester_time - 9000) / 3000);
            } else {
                tester8 = 180;
            }
            test_values[tester_time].eight_bit_frames.push_back(tester8);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }


    BOOST_AUTO_TEST_CASE(runcueimmidiatly) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["cuelist"] =
        "2:200@lin|5:50@lin#next_cue#do_nothing$3:20@lin|7:100@lin#hold#do_nothing$4:60@lin|7:180@lin#hold#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 16000; tester_time= tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            if (tester_time == 4000) {
                update_key_values[tester_time].push_back(std::tuple("run_cue", "2"));
            }
            uint8_t tester8;
            if (tester_time < 1000){
                tester8 = 0;
            } else if (tester_time < 3000){
                tester8 = (uint8_t) std::round((double) 200 * ((double) tester_time - 1000) / 2000);
            } else if (tester_time < 4000){
                tester8 = (uint8_t) std::round((double) 200 - (double) 150 * ((double) tester_time - 3000) / 3000);
            } else if (tester_time < 8000){
                tester8 = (uint8_t) std::round((double) 150 - 90 * ((double) tester_time - 4000) / 4000);
            } else if (tester_time < 11000){
                tester8 = (uint8_t) std::round((double) 60 + 120 * ((double) tester_time - 8000) / 3000);
            } else {
                tester8 = 180;
            }
            test_values[tester_time].eight_bit_frames.push_back(tester8);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }




    BOOST_AUTO_TEST_CASE(alotofstuff) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["cuelist"] =
        "2:200@lin|4:50@lin#next_cue#restart$3:20@lin|7:100@lin#start_again#do_nothing$4:60@lin|7:180@lin#hold#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 60000; tester_time= tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000 || tester_time == 4000 || tester_time == 16000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            if (tester_time == 43000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "to_next_cue"));
            }
            if (tester_time == 5000 || tester_time == 32000) {
                update_key_values[tester_time].push_back(std::tuple("next_cue", "2"));
            }
            if (tester_time == 38000) {
                update_key_values[tester_time].push_back(std::tuple("next_cue", "1"));
            }
            uint8_t tester8;
            if (tester_time < 1000){
                tester8 = 0;
            } else if (tester_time < 3000){
                tester8 = (uint8_t) std::round((double) 200 * ((double) tester_time - 1000) / 2000);
            } else if (tester_time < 4000){
                tester8 = (uint8_t) std::round((double) 200 - 150 * ((double) tester_time - 3000) / 2000);
            } else if (tester_time < 6000){
                tester8 = (uint8_t) std::round((double) 125 + 75 * ((double) tester_time - 4000) / 2000);
            } else if (tester_time < 8000){
                tester8 = (uint8_t) std::round((double) 200 - 150 * ((double) tester_time - 6000) / 2000);
            } else if (tester_time < 12000){
                tester8 = (uint8_t) std::round((double) 50 + 10 * ((double) tester_time - 8000) / 4000);
            } else if (tester_time < 15000){
                tester8 = (uint8_t) std::round((double) 60 + 120 * ((double) tester_time - 12000) / 3000);
            } else if (tester_time < 16000){
                tester8 = (uint8_t) std::round((double) 180);
            } else if (tester_time < 18000){
                tester8 = (uint8_t) std::round((double) 180 + 20 * ((double) tester_time - 16000) / 2000);
            } else if (tester_time < 20000){
                tester8 = (uint8_t) std::round((double) 200 - 150 * ((double) tester_time - 18000) / 2000);
            } else if (tester_time < 23000){
                tester8 = (uint8_t) std::round((double) 50 - 30 * ((double) tester_time - 20000) / 3000);
            } else if (tester_time < 27000){
                tester8 = (uint8_t) std::round((double) 20 + 80 * ((double) tester_time - 23000) / 4000);
            } else if (tester_time < 30000){
                tester8 = (uint8_t) std::round((double) 100 - 80 * ((double) tester_time - 27000) / 3000);
            } else if (tester_time < 34000){
                tester8 = (uint8_t) std::round((double) 20 + 80 * ((double) tester_time - 30000) / 4000);
            } else if (tester_time < 38000){
                tester8 = (uint8_t) std::round((double) 100 - 40 * ((double) tester_time - 34000) / 4000);
            } else if (tester_time < 41000){
                tester8 = (uint8_t) std::round((double) 60 + 120 * ((double) tester_time - 38000) / 3000);
            } else if (tester_time < 43000){
                tester8 = (uint8_t) std::round((double) 180);
            } else if (tester_time < 46000){
                tester8 = (uint8_t) std::round((double) 180 - 160 * ((double) tester_time - 43000) / 3000);
            } else if (tester_time < 50000){
                tester8 = (uint8_t) std::round((double) 20 + 80 * ((double) tester_time - 46000) / 4000);
            } else {
                tester8 = 100;
            }
            test_values[tester_time].eight_bit_frames.push_back(tester8);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }


    BOOST_AUTO_TEST_CASE(startdefaultcue) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["cuelist"] =
                "2:200@lin|4:50@lin#next_cue#restart$3:20@lin|7:100@lin#next_cue#do_nothing$4:60@lin|7:180@lin#hold#do_nothing";
        configuration["default_cue"] =
                "1";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;
        static std::vector<int> scene_activations;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 60000; tester_time= tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 5000 || tester_time == 9000 || tester_time == 11000 || tester_time == 13000 || tester_time == 30000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            if (tester_time == 10000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "pause"));
            }
            if (tester_time == 24000) {
                update_key_values[tester_time].push_back(std::tuple("next_cue", "0"));
            }
            if (tester_time == 54000 || tester_time == 58000) {
                update_key_values[tester_time].push_back(std::tuple("set_default_cue", "0"));
            }
            if (tester_time == 0 || tester_time == 20000 || tester_time == 47000 || tester_time == 57000 || tester_time == 60000) {
                scene_activations.push_back(tester_time);
            }
            uint8_t tester8;
            if (tester_time < 3000){
                tester8 = (uint8_t) std::round((double) 20 * ((double) tester_time) / 3000);
            } else if (tester_time < 7000) {
                tester8 = (uint8_t) std::round((double) 20 + 80 * ((double) tester_time - 3000) / 4000);
            } else if (tester_time < 10000) {
                tester8 = (uint8_t) std::round((double) 100 - 40 * ((double) tester_time - 7000) / 4000);
            } else if (tester_time < 11000) {
                tester8 = (uint8_t) std::round((double) 100 - 40 * (10000 - 7000) / 4000);
            } else if (tester_time < 12000) {
                tester8 = (uint8_t) std::round((double) 100 - 40 * ((double) tester_time - 8000) / 4000);
            } else if (tester_time < 15000) {
                tester8 = (uint8_t) std::round((double) 60 + 120 * ((double) tester_time - 12000) / 3000);
            } else if (tester_time < 20000) {
                tester8 = 180;
            } else if (tester_time < 23000){
                tester8 = (uint8_t) std::round((double) 180 - 160 * ((double) tester_time - 20000) / 3000);
            } else if (tester_time < 27000) {
                tester8 = (uint8_t) std::round((double) 20 + 80 * ((double) tester_time - 23000) / 4000);
            } else if (tester_time < 29000) {
                tester8 = (uint8_t) std::round((double) 100 + 100 * ((double) tester_time - 27000) / 2000);
            } else if (tester_time < 30000) {
                tester8 = (uint8_t) std::round((double) 200 - 150  * ((double) tester_time - 29000) / 2000);
            } else if (tester_time < 32000) {
                tester8 = (uint8_t) std::round((double) 125 + 75  * ((double) tester_time - 30000) / 2000);
            } else if (tester_time < 34000) {
                tester8 = (uint8_t) std::round((double) 200 - 150  * ((double) tester_time - 32000) / 2000);
            } else if (tester_time < 37000){
                tester8 = (uint8_t) std::round((double) 50 - 30 * ((double) tester_time - 34000) / 3000);
            } else if (tester_time < 41000) {
                tester8 = (uint8_t) std::round((double) 20 + 80 * ((double) tester_time - 37000) / 4000);
            } else if (tester_time < 45000) {
                tester8 = (uint8_t) std::round((double) 100 - 40 * ((double) tester_time - 41000) / 4000);
            } else if (tester_time < 47000) {
                tester8 = (uint8_t) std::round((double) 60 + 120 * ((double) tester_time - 45000) / 3000);
            } else if (tester_time < 50000){
                tester8 = (uint8_t) std::round((double) 140 - 120 * ((double) tester_time - 47000) / 3000);
            } else if (tester_time < 54000) {
                tester8 = (uint8_t) std::round((double) 20 + 80 * ((double) tester_time - 50000) / 4000);
            } else if (tester_time < 57000) {
                tester8 = (uint8_t) std::round((double) 100 - 40 * ((double) tester_time - 54000) / 4000);
            } else if (tester_time < 59000) {
                tester8 = (uint8_t) std::round((double) 70 + 130 * ((double) tester_time - 57000) / 2000);
            } else if (tester_time < 61000) {
                tester8 = (uint8_t) std::round((double) 200 - 150 * ((double) tester_time - 59000) / 2000);
            }
            test_values[tester_time].eight_bit_frames.push_back(tester8);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values, scene_activations);
    }


    BOOST_AUTO_TEST_CASE(startanothercuefromstart) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "hold";

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["cuelist"] =
                "2:80@lin|4:250@lin#next_cue#restart$3:40@lin|7:120@lin#next_cue#do_nothing$4:200@lin|7:20@lin#hold#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;
        static std::vector<int> scene_activations;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 60000; tester_time= tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 3000 || tester_time == 12000 || tester_time == 19000 || tester_time == 23000 || tester_time == 30000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            if (tester_time == 17000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "pause"));
            }
            if (tester_time == 20000 || tester_time == 26000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "stop"));
            }
            if (tester_time == 19500) {
                update_key_values[tester_time].push_back(std::tuple("next_cue", "0"));
            }
            if (tester_time == 4000) {
                update_key_values[tester_time].push_back(std::tuple("next_cue", "1"));
            }
            if (tester_time == 1000 || tester_time == 28000) {
                update_key_values[tester_time].push_back(std::tuple("next_cue", "2"));
            }
            if (tester_time == 0) {
                scene_activations.push_back(tester_time);
            }
            uint8_t tester8;
            if (tester_time < 3000){
                tester8 = 0;
            } else if (tester_time < 7000) {
                tester8 = (uint8_t) std::round((double) 0 + 200 * ((double) tester_time - 3000) / 4000);
            } else if (tester_time < 10000) {
                tester8 = (uint8_t) std::round((double) 200 - 180 * ((double) tester_time - 7000) / 3000);
            } else if (tester_time < 12000) {
                tester8 = 20;
            } else if (tester_time < 15000) {
                tester8 = (uint8_t) std::round((double) 20 + 20 * ((double) tester_time - 12000) / 3000);
            } else if (tester_time < 17000) {
                tester8 = (uint8_t) std::round((double) 40 + 80 * ((double) tester_time - 15000) / 4000);
            } else if (tester_time < 19000) {
                tester8 = 80;
            } else if (tester_time < 20000) {
                tester8 = (uint8_t) std::round((double) 40 + 80 * ((double) tester_time - 17000) / 4000);
            } else if (tester_time < 23000) {
                tester8 = 100;
            } else if (tester_time < 25000) {
                tester8 = (uint8_t) std::round((double) 100 - 20 * ((double) tester_time - 23000) / 2000);
            } else if (tester_time < 26000) {
                tester8 = (uint8_t) std::round((double) 80 + 170 * ((double) tester_time - 25000) / 2000);
            } else if (tester_time < 30000) {
                tester8 = 165;
            } else if (tester_time < 34000) {
                tester8 = (uint8_t) std::round((double) 165 + 35 * ((double) tester_time - 30000) / 4000);
            } else if (tester_time < 37000) {
                tester8 = (uint8_t) std::round((double) 200 - 180 * ((double) tester_time - 34000) / 3000);
            } else if (tester_time < 39000) {
                tester8 = 20;
            }
            test_values[tester_time].eight_bit_frames.push_back(tester8);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values, scene_activations);
    }


    BOOST_AUTO_TEST_CASE(changinginput) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "v1:16bit";
        configuration["end_handling"] = "hold";

        cue_st_names channel_names;
        channel_names.sixteen_bit_frames.push_back("v1");

        configuration["cuelist"] =
                "2:61000@lin|6:10@lin#next_cue#do_nothing$3:16000@lin|4:200@lin#hold#do_nothing";

        std::map<int, cue_st_test> test_values;
        std::map<int, double> time_scale;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 22000; tester_time= tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000 || tester_time == 11000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            if (tester_time == 9000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "pause"));
            }
            if (tester_time == 1000) {
                time_scale[tester_time] = 1;
            }
            if (tester_time == 2000) {
                time_scale[tester_time] = 0.5;
            }
            if (tester_time == 4000) {
                time_scale[tester_time] = 0.2;
            }
            if (tester_time == 10000) {
                time_scale[tester_time] = 1.5;
            }
            if (tester_time == 13000) {
                time_scale[tester_time] = 2;
            }
            if (tester_time == 14800) {
                time_scale[tester_time] = 0;
            }
            if (tester_time == 16000) {
                time_scale[tester_time] = 0.1;
            }
            uint16_t tester16;
            if (tester_time < 1000) {
                tester16 = 0;
            } else if (tester_time < 2000) {
                tester16 = (uint16_t) std::round(61000 * ((double) tester_time - 1000) / 2000);
            } else if (tester_time < 4000) {
                tester16 = (uint16_t) std::round(30500 + 30500 * ((double) tester_time - 2000) / 2000);
            } else if (tester_time < 9000) {
                tester16 = (uint16_t) std::round(61000 - 60990 * ((double) tester_time - 4000) / 20000);
            } else if (tester_time < 11000){
                tester16 = (uint16_t) std::round(45752.5);
            } else if (tester_time < 13000){
                tester16 = (uint16_t) std::round(45752.5 - 45742.5 * ((double) tester_time - 11000) / 2000);
            } else if (tester_time < 14500){
                tester16 = (uint16_t) std::round(10 + 15990 * ((double) tester_time - 13000) / 1500);
            } else if (tester_time < 14800){
                tester16 = (uint16_t) std::round(16000 - 15800 * ((double) tester_time - 14500) / 500);
            } else if (tester_time < 16000){
                tester16 = 6520;
            } else if (tester_time < 20000){
                tester16 = (uint16_t) std::round(6520 - 6320 * ((double) tester_time - 16000) / 4000);
            } else {
                tester16 = 200;
            }
            test_values[tester_time].sixteen_bit_frames.push_back(tester16);
        }
        std::vector<int> activations;
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values, activations, time_scale);
    }

BOOST_AUTO_TEST_CASE(test_parsing_metadata) {
        std::map <std::string, std::string> configuration;
        configuration["mapping"] = "dimmer:8bit";
        configuration["end_handling"] = "start_again";

        cue_st_names channel_names;
        channel_names.eight_bit_frames.push_back("dimmer");

        configuration["cuelist"] =
        "2:200@lin|4:50@lin#next_cue#restart$3:100@lin|6:250@lin#next_cue#do_nothing#just a cue name";

        std::map<int, cue_st_test> test_values;
        std::map<int, std::vector<std::tuple<std::string, std::string>>> update_key_values;

        std::vector<int> time_s;
        for (int tester_time= 0; tester_time < 24000; tester_time= tester_time + 100) {
            time_s.push_back(tester_time);
            if (tester_time == 1000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "to_next_cue"));
            }
            if (tester_time == 6000) {
                update_key_values[tester_time].push_back(std::tuple("run_mode", "play"));
            }
            uint8_t tester8;
            if (tester_time < 1000){
                tester8 = 0;
            } else if (tester_time < 3000){
                tester8 = (uint8_t) std::round((double) 200 * ((double) tester_time - 1000) / 2000);
            } else if (tester_time < 5000){
                tester8 = (uint8_t) std::round((double) 200 - 150 * ((double) tester_time - 3000) / 2000);
            } else if (tester_time < 6000){
                tester8 = (uint8_t) std::round((double) 50);
            } else if (tester_time < 9000){
                tester8 = (uint8_t) std::round((double) 50 + 50 * ((double) tester_time - 6000) / 3000);
            } else if (tester_time < 12000){
                tester8 = (uint8_t) std::round((double) 100 + 150 * ((double) tester_time - 9000) / 3000);
            } else if (tester_time < 14000){
                tester8 = (uint8_t) std::round((double) 250 - 50 * ((double) tester_time - 12000) / 2000);
            } else if (tester_time < 16000){
                tester8 = (uint8_t) std::round((double) 200 - 150 * ((double) tester_time - 14000) / 2000);
            } else if (tester_time < 19000){
                tester8 = (uint8_t) std::round((double) 50 + 50 * ((double) tester_time - 16000) / 3000);
            } else if (tester_time < 22000){
                tester8 = (uint8_t) std::round((double) 100 + 150 * ((double) tester_time - 19000) / 3000);
            } else if (tester_time < 24000){
                tester8 = (uint8_t) std::round((double) 250 - 50 * ((double) tester_time - 22000) / 2000);
            } else if (tester_time < 26000){
                tester8 = (uint8_t) std::round((double) 200 - 150 * ((double) tester_time - 24000) / 2000);
            }
            test_values[tester_time].eight_bit_frames.push_back(tester8);
        }
        test_cue_function(time_s, test_values, channel_names, configuration, update_key_values);
    }

BOOST_AUTO_TEST_SUITE_END()
