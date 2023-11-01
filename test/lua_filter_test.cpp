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

    fil.pre_setup(configuration, initial_parameters, "");

    channel_mapping map = channel_mapping();
    fil.get_output_channels(map, "abc");

    bool test = false;
    try {
        fil.setup_filter (configuration, initial_parameters, input_channels, "");
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

    fil.pre_setup(configuration, initial_parameters, "");

    channel_mapping map = channel_mapping();
    fil.get_output_channels(map, "abc");

    fil.setup_filter(configuration, initial_parameters, input_channels, "");
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

    initial_parameters["script"] = R"(
        function update()
            output[1]={}
            output["2"]={}
            output[2]={}
            output[1][2]=5
            output[1][4]='avad'
            output[1][3]=6
        end
        function scene_activated()
            -- This method will be called every time the show is switched to this scene
        end
    )";

    fil.pre_setup(configuration, initial_parameters, "");

    channel_mapping map = channel_mapping();
    fil.get_output_channels(map, "abc");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");
    fil.update();
    if(auto uptr = dmxfish::io::get_universe(1); uptr != nullptr) {
        BOOST_TEST((*uptr)[2] == 5, std::string("lua direct out to universe has value of ") + std::to_string((*uptr)[2]) + std::string(" instead of 5"));
        BOOST_TEST((*uptr)[3] == 6, std::string("lua direct out to universe has value of ") + std::to_string((*uptr)[3]) + std::string(" instead of 6"));
    } else {
        BOOST_TEST(false, "universe for lua filter test does not exist");
    }
    run_time_state->running = false;
    manager = nullptr;

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

    initial_parameters["script"] = R"(
        function update()
            -- This method will be called once per DMX output cycle
            -- Put your effect here
            out_color = {
                h = in_color["h"]*8,
                s = in_color["s"]*3,
                i = in_color["i"]*2}
            out_dimmer = in_dimmer/2
            out_color2 = in_color
            out_color2.h = in_color.h/10
            out_color2.s = in_color.s/2
        end
        function scene_activated()
            -- This method will be called every time the show is switched to this scene
        end
    )";

    fil.pre_setup(configuration, initial_parameters, "");

    channel_mapping map = channel_mapping();
    fil.get_output_channels(map, "abc");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");
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

    fil.pre_setup(configuration, initial_parameters, "");

    channel_mapping map = channel_mapping();
    fil.get_output_channels(map, "abc");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");
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

    initial_parameters["script"] = R"(
        function update()
           out_color = nil
       end
    )";

    fil.pre_setup(configuration, initial_parameters, "");

    channel_mapping map = channel_mapping();
    fil.get_output_channels(map, "abc");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");
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

    initial_parameters["script"] = R"(
        function update()
           out_color = {h=90, s=0.4}
           out_color2 = 3
           out_dimmer = {z=8}
       end
    )";

    fil.pre_setup(configuration, initial_parameters, "");

    channel_mapping map = channel_mapping();
    fil.get_output_channels(map, "abc");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");
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

    initial_parameters["script"] = R"(
        function decompose_rgb(color)
            return color.r, color.g, color.b, color.w
        end
        function update()
            red1, green1, blue1 = hsi_to_rgb(in_color)
            red2, green2, blue2, white2 = hsi_to_rgbw(in_color)
        end
    )";

    fil.pre_setup(configuration, initial_parameters, "");

    channel_mapping map = channel_mapping();
    fil.get_output_channels(map, "abc");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");


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



BOOST_AUTO_TEST_CASE(test_update_gui_empty) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_lua_script fil = filter_lua_script ();

    channel_mapping input_channels = channel_mapping();
    std::map <std::string, std::string> configuration;
    configuration["in_mapping"] = "";
    configuration["out_mapping"] = "";
    std::map <std::string, std::string> initial_parameters;

    initial_parameters["script"] = "";

    fil.pre_setup(configuration, initial_parameters, "");

    channel_mapping map = channel_mapping();
    fil.get_output_channels(map, "abc");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");

    fil.update();
    fil.receive_update_from_gui("false", "test");
}

BOOST_AUTO_TEST_CASE(test_update_gui) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_lua_script fil = filter_lua_script ();

    channel_mapping input_channels = channel_mapping();
    std::map <std::string, std::string> configuration;
    configuration["in_mapping"] = "";
    configuration["out_mapping"] = "";
    std::map <std::string, std::string> initial_parameters;

    initial_parameters["script"] = R"(
        function receive_update_from_gui(key, value)
            if key == 'true' then
                return true
            end
            return false
        end
	)";

    fil.pre_setup(configuration, initial_parameters, "");

    channel_mapping map = channel_mapping();
    fil.get_output_channels(map, "abc");
    fil.setup_filter (configuration, initial_parameters, input_channels, "");

    fil.update();
    bool test1 = fil.receive_update_from_gui("false", "test1");
    BOOST_TEST(!test1, "lua receive update from gui should return false but is" + std::to_string(test1));

    fil.update();
    bool test2 = fil.receive_update_from_gui("true", "test2");
    BOOST_TEST(test2, "lua receive update from gui should return false but is" + std::to_string(test2));
}



BOOST_AUTO_TEST_CASE(lua_script_ersti_party) {
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


    dmxfish::filters::filter_lua_script fil = filter_lua_script();

    channel_mapping input_channels = channel_mapping();
    std::map<std::string, std::string> configuration;
    configuration["in_mapping"] = "color:color;change_nr:float";
    configuration["out_mapping"] = "";
    std::map<std::string, std::string> initial_parameters;


    dmxfish::dmx::pixel color = dmxfish::dmx::pixel(120, 1, 1);
    input_channels.color_channels["color"] = &color;
    double change_nr = 25.1;
    input_channels.float_channels["change_nr"] = &change_nr;

    initial_parameters["script"] = R"(
        function shuffle()
            shuffled_fix = {}
            for i, v in ipairs(fix) do
            	local pos = math.random(1, #shuffled_fix+1)
            	table.insert(shuffled_fix, pos, v)
            end
            old_change_nr = change_nr
        end

        function update()
            local r
            local g
            local b
            if old_change_nr ~= change_nr then
                shuffle()
            end
            for key,value in ipairs(shuffled_fix) do
                if key <= nr_of_fix_on then
                    r, g, b = hsi_to_rgb(color)
                else
                    r = 0
                    g = 0
                    b = 0
                end
                output[value["uni"]][value["red"]] = r
                output[value["uni"]][value["green"]] = g
                output[value["uni"]][value["blue"]] = b
            end
        end

        function scene_activated()
            nr_of_fix = 20
            nr_of_fix_on = 5
            fix = {}
            for i = 1,nr_of_fix,1 do
                local univ = 1
                fix[i] = {uni= univ,
                          red = i*5+0,
                          green = i*5+1,
                          blue = i*5+2}
                output[univ] = {}
            end
            shuffle()
        end
	)";

    fil.pre_setup(configuration, initial_parameters, "");

    channel_mapping map = channel_mapping();
    fil.get_output_channels(map, "abc");
    fil.setup_filter(configuration, initial_parameters, input_channels, "");
    fil.scene_activated();
    fil.update();
    std::cout << "universe ";
    if (auto uptr = dmxfish::io::get_universe(1); uptr != nullptr) {
        for (int i = 1; i <= 512; i++) {
            std::cout << i << ": " << (int)(*uptr)[i] << ",    ";
        }
    }
    std::cout << std::endl;


    color.hue = 180.0;
    fil.update();
    std::cout << "universe ";
    if (auto uptr = dmxfish::io::get_universe(1); uptr != nullptr) {
        for (int i = 1; i <= 512; i++) {
            std::cout << i << ": " << (int)(*uptr)[i] << ",    ";
        }
    }
    std::cout << std::endl;

    color.hue = 180.0;
    change_nr = 0.0;
    fil.update();
    std::cout << "universe ";
    if (auto uptr = dmxfish::io::get_universe(1); uptr != nullptr) {
        for (int i = 1; i <= 512; i++) {
            std::cout << i << ": " << (int)(*uptr)[i] << ",    ";
        }
    }
    std::cout << std::endl;


    run_time_state->running = false;
    manager = nullptr;
}