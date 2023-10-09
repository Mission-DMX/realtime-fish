#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE FISH_TESTS
#include <boost/test/included/unit_test.hpp>
#include "filters/filter_lua_script.hpp"
#include "lib/logging.hpp"
#include "io/iomanager.hpp"

#include "io/universe_sender.hpp"
#include <filesystem>
#include <map>
#include <memory>
#include <sstream>



#include "rmrf-net/client_factory.hpp"
#include "rmrf-net/ioqueue.hpp"

#include "proto_src/RealTimeControl.pb.h"
#include "proto_src/MessageTypes.pb.h"
#include "proto_src/DirectMode.pb.h"
#include "google/protobuf/util/delimited_message_util.h"

#include <google/protobuf/text_format.h>

using namespace dmxfish::filters;

BOOST_AUTO_TEST_CASE(test_error_init) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_lua_script fil = filter_lua_script ();

    channel_mapping input_channels = channel_mapping();
    std::map <std::string, std::string> configuration;
    configuration["in_mapping"] = "";
    configuration["out_mapping"] = "";
    std::map <std::string, std::string> initial_parameters;

    initial_parameters["script"] = R"(
        function update()
            print('test2 ' .. empty_value)
	)";

    fil.pre_setup(configuration, initial_parameters);

    channel_mapping map = channel_mapping();
    fil.get_output_channels(map, "abc");

    bool test = false;
    try {
        fil.setup_filter (configuration, initial_parameters, input_channels);
    } catch (std::exception &e){
        test = true;
    }
    BOOST_TEST(test, "the filter lua should had thrown an exception while setting up the script");

}

BOOST_AUTO_TEST_CASE(test_error_code) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_lua_script fil = filter_lua_script ();

    channel_mapping input_channels = channel_mapping();
    std::map <std::string, std::string> configuration;
    configuration["in_mapping"] = "";
    configuration["out_mapping"] = "";
    std::map <std::string, std::string> initial_parameters;

    initial_parameters["script"] = R"(
        function update()
            print('test2 ' .. empty_value)
        end
        function scene_activated()
            print('test1 ' .. empty_value_init)
        end
	)";

    fil.pre_setup(configuration, initial_parameters);

    channel_mapping map = channel_mapping();
    fil.get_output_channels(map, "abc");

    fil.setup_filter(configuration, initial_parameters, input_channels);
    bool test = false;
    try {
        fil.scene_activated();
    } catch (std::exception &e){
        test = true;
    }
    BOOST_TEST(test, "the filter lua should had thrown an exception while scene_activated");

    test = false;
    try {
        fil.update();
    } catch (std::exception &e){
        test = true;
    }
    BOOST_TEST(test, "the filter lua should had thrown an exception while update");

}

BOOST_AUTO_TEST_CASE(testluadirectout) {
    spdlog::set_level(spdlog::level::debug);

    std::shared_ptr<runtime_state_t> run_time_state = nullptr;
    static std::shared_ptr<dmxfish::io::IOManager> manager = nullptr;

    run_time_state = std::make_shared<runtime_state_t>();
    manager = std::make_shared<dmxfish::io::IOManager>(run_time_state, true);
    manager->start();


    auto msg_universe_init = std::make_shared<missiondmx::fish::ipcmessages::Universe>();
    msg_universe_init->set_id(1);

    const auto universe_inner = msg_universe_init->mutable_remote_location();
    universe_inner->set_ip_address("192.168.125.23");
    universe_inner->set_port(6454);
    universe_inner->set_universe_on_device(1);
    dmxfish::io::register_universe_from_message(*msg_universe_init.get());



    dmxfish::filters::filter_lua_script fil = filter_lua_script ();

    channel_mapping input_channels = channel_mapping();
    std::map <std::string, std::string> configuration;
    configuration["in_mapping"] = "";
    configuration["out_mapping"] = "";
    std::map <std::string, std::string> initial_parameters;

    initial_parameters["script"] = "function update()\n"
                                   "    output[1]={}\n"
                                   "    output[\"2\"]={}\n"
                                   "    output[2]={}\n"
                                   "    output[1][2]=5\n"
                                   "    output[1][4]='avad'\n"
                                   "    output[1][3]=6\n"
                                   "end\n"
                                   "function scene_activated()\n"
                                   "    -- This method will be called every time the show is switched to this scene\n"
                                   "end";

    fil.pre_setup(configuration, initial_parameters);

    channel_mapping map = channel_mapping();
    fil.get_output_channels(map, "abc");
    fil.setup_filter (configuration, initial_parameters, input_channels);
    fil.update();

    fil.update();

}



BOOST_AUTO_TEST_CASE(testlua) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_lua_script fil = filter_lua_script ();

    channel_mapping input_channels = channel_mapping();
    uint8_t in_dimmer = 56;
    uint8_t test_val = 28;
    dmxfish::dmx::pixel color = dmxfish::dmx::pixel(0.1,0.2,0.3);
    dmxfish::dmx::pixel testercol = dmxfish::dmx::pixel(0.8,0.6,0.6);
    dmxfish::dmx::pixel testercol2 = dmxfish::dmx::pixel(0.01,0.1,0.3);
    input_channels.eight_bit_channels["in_dimmer"] = &in_dimmer;
    input_channels.color_channels["in_color"] = &color;
    std::map <std::string, std::string> configuration;
    configuration["in_mapping"] = "in_dimmer:8bit;in_color:color";
    configuration["out_mapping"] = "out_dimmer:8bit;out_color:color;out_color2:color";
    std::map <std::string, std::string> initial_parameters;

    initial_parameters["script"] = "function update()\n"
                                   "    -- This method will be called once per DMX output cycle\n"
                                   "    -- Put your effect here\n"
                                   "    out_color = {\n"
                                   "        h = in_color[\"h\"]*8,\n"
                                   "        s = in_color[\"s\"]*3,\n"
                                   "        i = in_color[\"i\"]*2}\n"
                                   "    out_dimmer = in_dimmer/2\n"
                                   "    out_color2 = in_color\n"
                                   "    out_color2.h = in_color.h/10\n"
                                   "    out_color2.s = in_color.s/2\n"
                                   "end\n"
                                   "function scene_activated()\n"
                                   "    -- This method will be called every time the show is switched to this scene\n"
                                   "end";

    fil.pre_setup(configuration, initial_parameters);

    channel_mapping map = channel_mapping();
    fil.get_output_channels(map, "abc");
    fil.setup_filter (configuration, initial_parameters, input_channels);
    fil.update();
    BOOST_TEST(*map.eight_bit_channels["abc:out_dimmer"] == test_val, "out_dimmer has wrong value: " + std::to_string((int) *map.eight_bit_channels["abc:out_dimmer"]) + " instead of " + std::to_string(test_val));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color"]->hue - testercol.hue) <= testercol.hue * 0.00001, "out_color:hue has wrong value: " + std::to_string(map.color_channels["abc:out_color"]->hue) + " instead of " + std::to_string(testercol.hue));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color"]->saturation - testercol.saturation) <= testercol.saturation * 0.00001, "out_color:saturation has wrong value: " + std::to_string(map.color_channels["abc:out_color"]->saturation) + " instead of " + std::to_string(testercol.saturation));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color"]->iluminance - testercol.iluminance) <= testercol.iluminance * 0.00001, "out_color:iluminance has wrong value: " + std::to_string(map.color_channels["abc:out_color"]->iluminance) + " instead of " + std::to_string(testercol.iluminance));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color2"]->hue - testercol2.hue) <= testercol2.hue * 0.00001, "out_color2:hue has wrong value: " + std::to_string(map.color_channels["abc:out_color2"]->hue) + " instead of " + std::to_string(testercol2.hue));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color2"]->saturation - testercol2.saturation) <= testercol2.saturation * 0.00001, "out_color2:saturation has wrong value: " + std::to_string(map.color_channels["abc:out_color2"]->saturation) + " instead of " + std::to_string(testercol2.saturation));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color2"]->iluminance - testercol2.iluminance) <= testercol2.iluminance * 0.00001, "out_color2:iluminance has wrong value: " + std::to_string(map.color_channels["abc:out_color2"]->iluminance) + " instead of " + std::to_string(testercol2.iluminance));


    in_dimmer = 24;
    test_val = 12;
    color = dmxfish::dmx::pixel(40.0,0.1,0.4);
    testercol = dmxfish::dmx::pixel(320.0,0.3,0.8);
    testercol2 = dmxfish::dmx::pixel(4.0,0.05,0.4);
    fil.update();

    BOOST_TEST(*map.eight_bit_channels["abc:out_dimmer"] == test_val, "out_dimmer has wrong value: " + std::to_string((int) *map.eight_bit_channels["abc:out_dimmer"]) + " instead of " + std::to_string(test_val));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color"]->hue - testercol.hue) <= testercol.hue * 0.00001, "out_color:hue has wrong value: " + std::to_string(map.color_channels["abc:out_color"]->hue) + " instead of " + std::to_string(testercol.hue));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color"]->saturation - testercol.saturation) <= testercol.saturation * 0.00001, "out_color:saturation has wrong value: " + std::to_string(map.color_channels["abc:out_color"]->saturation) + " instead of " + std::to_string(testercol.saturation));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color"]->iluminance - testercol.iluminance) <= testercol.iluminance * 0.00001, "out_color:iluminance has wrong value: " + std::to_string(map.color_channels["abc:out_color"]->iluminance) + " instead of " + std::to_string(testercol.iluminance));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color2"]->hue - testercol2.hue) <= testercol2.hue * 0.00001, "out_color2:hue has wrong value: " + std::to_string(map.color_channels["abc:out_color2"]->hue) + " instead of " + std::to_string(testercol2.hue));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color2"]->saturation - testercol2.saturation) <= testercol2.saturation * 0.00001, "out_color2:saturation has wrong value: " + std::to_string(map.color_channels["abc:out_color2"]->saturation) + " instead of " + std::to_string(testercol2.saturation));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color2"]->iluminance - testercol2.iluminance) <= testercol2.iluminance * 0.00001, "out_color2:iluminance has wrong value: " + std::to_string(map.color_channels["abc:out_color2"]->iluminance) + " instead of " + std::to_string(testercol2.iluminance));
}


BOOST_AUTO_TEST_CASE(testlua_missing_function) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_lua_script fil = filter_lua_script ();

    channel_mapping input_channels = channel_mapping();
    uint8_t in_dimmer = 56;
    dmxfish::dmx::pixel color = dmxfish::dmx::pixel(0.1,0.2,0.3);
    input_channels.eight_bit_channels["in_dimmer"] = &in_dimmer;
    input_channels.color_channels["in_color"] = &color;
    std::map <std::string, std::string> configuration;
    configuration["in_mapping"] = "in_dimmer:8bit;in_color:color";
    configuration["out_mapping"] = "out_dimmer:8bit;out_color:color";
    std::map <std::string, std::string> initial_parameters;

    initial_parameters["script"] = "";

    fil.pre_setup(configuration, initial_parameters);

    channel_mapping map = channel_mapping();
    fil.get_output_channels(map, "abc");
    fil.setup_filter (configuration, initial_parameters, input_channels);
    fil.update();

}

BOOST_AUTO_TEST_CASE(testlua_nil_value) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_lua_script fil = filter_lua_script ();

    channel_mapping input_channels = channel_mapping();
    uint8_t in_dimmer = 56;
    uint8_t test_val = 0;
    dmxfish::dmx::pixel color = dmxfish::dmx::pixel(0.1,0.2,0.3);
    dmxfish::dmx::pixel testercol = dmxfish::dmx::pixel(0.0,0.0,0.0);
    input_channels.eight_bit_channels["in_dimmer"] = &in_dimmer;
    input_channels.color_channels["in_color"] = &color;
    std::map <std::string, std::string> configuration;
    configuration["in_mapping"] = "in_dimmer:8bit;in_color:color";
    configuration["out_mapping"] = "out_dimmer:8bit;out_color:color";
    std::map <std::string, std::string> initial_parameters;

    initial_parameters["script"] = "function update()\n"
                                   "    out_color = nil\n"
                                   "end\n";

    fil.pre_setup(configuration, initial_parameters);

    channel_mapping map = channel_mapping();
    fil.get_output_channels(map, "abc");
    fil.setup_filter (configuration, initial_parameters, input_channels);
    fil.update();
    BOOST_TEST(*map.eight_bit_channels["abc:out_dimmer"] == test_val, "out_dimmer has wrong value: " + std::to_string((int) *map.eight_bit_channels["abc:out_dimmer"]) + " instead of " + std::to_string(test_val));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color"]->hue - testercol.hue) <= testercol.hue * 0.00001, "out_color:hue has wrong value: " + std::to_string(map.color_channels["abc:out_color"]->hue) + " instead of " + std::to_string(testercol.hue));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color"]->saturation - testercol.saturation) <= testercol.saturation * 0.00001, "out_color:saturation has wrong value: " + std::to_string(map.color_channels["abc:out_color"]->saturation) + " instead of " + std::to_string(testercol.saturation));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color"]->iluminance - testercol.iluminance) <= testercol.iluminance * 0.00001, "out_color:iluminance has wrong value: " + std::to_string(map.color_channels["abc:out_color"]->iluminance) + " instead of " + std::to_string(testercol.iluminance));
}

BOOST_AUTO_TEST_CASE(test_lua_wrong_type) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_lua_script fil = filter_lua_script ();

    channel_mapping input_channels = channel_mapping();
    uint8_t in_dimmer = 56;
    uint8_t test_val = 0;
    dmxfish::dmx::pixel color = dmxfish::dmx::pixel(0.1,0.2,0.3);
    dmxfish::dmx::pixel testercol = dmxfish::dmx::pixel(90.0,0.4,0.0);
    dmxfish::dmx::pixel testercol2 = dmxfish::dmx::pixel(0.0,0.0,0.0);
    input_channels.eight_bit_channels["in_dimmer"] = &in_dimmer;
    input_channels.color_channels["in_color"] = &color;
    std::map <std::string, std::string> configuration;
    configuration["in_mapping"] = "in_dimmer:8bit;in_color:color";
    configuration["out_mapping"] = "out_dimmer:8bit;out_color:color;out_color2:color";
    std::map <std::string, std::string> initial_parameters;

    initial_parameters["script"] = "function update()\n"
                                   "    out_color = {h=90, s=0.4}\n"
                                   "    out_color2 = 3\n"
                                   "    out_dimmer = {z=8}\n"
                                   "end\n";

    fil.pre_setup(configuration, initial_parameters);

    channel_mapping map = channel_mapping();
    fil.get_output_channels(map, "abc");
    fil.setup_filter (configuration, initial_parameters, input_channels);
    fil.update();
    BOOST_TEST(*map.eight_bit_channels["abc:out_dimmer"] == test_val, "out_dimmer has wrong value: " + std::to_string((int) *map.eight_bit_channels["abc:out_dimmer"]) + " instead of " + std::to_string(test_val));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color"]->hue - testercol.hue) <= testercol.hue * 0.00001, "out_color:hue has wrong value: " + std::to_string(map.color_channels["abc:out_color"]->hue) + " instead of " + std::to_string(testercol.hue));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color"]->saturation - testercol.saturation) <= testercol.saturation * 0.00001, "out_color:saturation has wrong value: " + std::to_string(map.color_channels["abc:out_color"]->saturation) + " instead of " + std::to_string(testercol.saturation));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color"]->iluminance - testercol.iluminance) <= testercol.iluminance * 0.00001, "out_color:iluminance has wrong value: " + std::to_string(map.color_channels["abc:out_color"]->iluminance) + " instead of " + std::to_string(testercol.iluminance));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color2"]->hue - testercol2.hue) <= testercol2.hue * 0.00001, "out_color2:hue has wrong value: " + std::to_string(map.color_channels["abc:out_color2"]->hue) + " instead of " + std::to_string(testercol2.hue));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color2"]->saturation - testercol2.saturation) <= testercol2.saturation * 0.00001, "out_color2:saturation has wrong value: " + std::to_string(map.color_channels["abc:out_color2"]->saturation) + " instead of " + std::to_string(testercol2.saturation));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color2"]->iluminance - testercol2.iluminance) <= testercol2.iluminance * 0.00001, "out_color2:iluminance has wrong value: " + std::to_string(map.color_channels["abc:out_color2"]->iluminance) + " instead of " + std::to_string(testercol2.iluminance));
}


BOOST_AUTO_TEST_CASE(test_lua_color_conversion) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_lua_script fil = filter_lua_script ();

    channel_mapping input_channels = channel_mapping();
    dmxfish::dmx::pixel color = dmxfish::dmx::pixel(120,1,1);
    uint8_t test_red1 = 0;
    uint8_t test_green1 = 255;
    uint8_t test_blue1 = 0;
    uint8_t test_red2 = 0;
    uint8_t test_green2 = 255;
    uint8_t test_blue2 = 0;
    uint8_t test_white2 = 0;
    input_channels.color_channels["in_color"] = &color;
    std::map <std::string, std::string> configuration;
    configuration["in_mapping"] = "in_color:color";
    configuration["out_mapping"] = "red1:8bit;green1:8bit;blue1:8bit;red2:8bit;green2:8bit;blue2:8bit;white2:8bit";
    std::map <std::string, std::string> initial_parameters;

    initial_parameters["script"] = "function decompose_rgb(color)\n"
                                   "    return color.r, color.g, color.b, color.w\n"
                                   "end\n"
                                   "function update()\n"
                                   "    red1, green1, blue1 = hsi_to_rgb(in_color)\n"
                                   "    red2, green2, blue2, white2 = hsi_to_rgbw(in_color)\n"
                                   "end\n";

    fil.pre_setup(configuration, initial_parameters);

    channel_mapping map = channel_mapping();
    fil.get_output_channels(map, "abc");
    fil.setup_filter (configuration, initial_parameters, input_channels);


    color = dmxfish::dmx::pixel(240,0.5001,1);
    test_red1 = 42;
    test_green1 = 42;
    test_blue1 = 170;
    test_red2 = 0;
    test_green2 = 0;
    test_blue2 = 128;
    test_white2 = 127;
    fil.update();
    BOOST_TEST(*map.eight_bit_channels["abc:red1"] == test_red1, "red1 has wrong value: " + std::to_string((int) *map.eight_bit_channels["abc:red1"]) + " instead of " + std::to_string(test_red1));
    BOOST_TEST(*map.eight_bit_channels["abc:green1"] == test_green1, "green1 has wrong value: " + std::to_string((int) *map.eight_bit_channels["abc:green1"]) + " instead of " + std::to_string(test_green1));
    BOOST_TEST(*map.eight_bit_channels["abc:blue1"] == test_blue1, "blue1 has wrong value: " + std::to_string((int) *map.eight_bit_channels["abc:blue1"]) + " instead of " + std::to_string(test_blue1));
    BOOST_TEST(*map.eight_bit_channels["abc:red2"] == test_red2, "red2 has wrong value: " + std::to_string((int) *map.eight_bit_channels["abc:red2"]) + " instead of " + std::to_string(test_red2));
    BOOST_TEST(*map.eight_bit_channels["abc:green2"] == test_green2, "green2 has wrong value: " + std::to_string((int) *map.eight_bit_channels["abc:green2"]) + " instead of " + std::to_string(test_green2));
    BOOST_TEST(*map.eight_bit_channels["abc:blue2"] == test_blue2, "blue2 has wrong value: " + std::to_string((int) *map.eight_bit_channels["abc:blue2"]) + " instead of " + std::to_string(test_blue2));
    BOOST_TEST(*map.eight_bit_channels["abc:white2"] == test_white2, "white2 has wrong value: " + std::to_string((int) *map.eight_bit_channels["abc:white2"]) + " instead of " + std::to_string(test_white2));
}
