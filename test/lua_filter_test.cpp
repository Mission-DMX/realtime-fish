#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE FISH_TESTS
#include <boost/test/included/unit_test.hpp>
#include "filters/filter_lua_script.hpp"
#include "lib/logging.hpp"

#include <sstream>

using namespace dmxfish::filters;

BOOST_AUTO_TEST_CASE(testlua) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_lua_script fil = filter_lua_script ();

    channel_mapping input_channels = channel_mapping();
    uint8_t in_dimmer = 56;
    uint8_t test_val = 28;
    dmxfish::dmx::pixel color = dmxfish::dmx::pixel(0.1,0.2,0.3);
    dmxfish::dmx::pixel testercol = dmxfish::dmx::pixel(0.8,0.6,0.6);
    input_channels.eight_bit_channels["in_dimmer"] = &in_dimmer;
    input_channels.color_channels["in_color"] = &color;
    std::map <std::string, std::string> configuration;
    configuration["in_mapping"] = "in_dimmer:8bit;in_color:color";
    configuration["out_mapping"] = "out_dimmer:8bit;out_color:color";
    std::map <std::string, std::string> initial_parameters;

    initial_parameters["script"] = "function update()\n"
                                   "    -- This method will be called once per DMX output cycle\n"
                                   "    -- Put your effect here\n"
                                   "    out_color = {\n"
                                   "        h = in_color[\"h\"]*8,\n"
                                   "        s = in_color[\"s\"]*3,\n"
                                   "        i = in_color[\"i\"]*2}\n"
                                   "    out_dimmer = in_dimmer/2\n"
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


    in_dimmer = 24;
    test_val = 12;
    color = dmxfish::dmx::pixel(40.0,0.1,0.4);
    testercol = dmxfish::dmx::pixel(320.0,0.3,0.8);
    fil.update();

    BOOST_TEST(*map.eight_bit_channels["abc:out_dimmer"] == test_val, "out_dimmer has wrong value: " + std::to_string((int) *map.eight_bit_channels["abc:out_dimmer"]) + " instead of " + std::to_string(test_val));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color"]->hue - testercol.hue) <= testercol.hue * 0.00001, "out_color:hue has wrong value: " + std::to_string(map.color_channels["abc:out_color"]->hue) + " instead of " + std::to_string(testercol.hue));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color"]->saturation - testercol.saturation) <= testercol.saturation * 0.00001, "out_color:saturation has wrong value: " + std::to_string(map.color_channels["abc:out_color"]->saturation) + " instead of " + std::to_string(testercol.saturation));
    BOOST_TEST(std::abs(map.color_channels["abc:out_color"]->iluminance - testercol.iluminance) <= testercol.iluminance * 0.00001, "out_color:iluminance has wrong value: " + std::to_string(map.color_channels["abc:out_color"]->iluminance) + " instead of " + std::to_string(testercol.iluminance));
}


BOOST_AUTO_TEST_CASE(testlua_missing_function) {
    spdlog::set_level(spdlog::level::debug);
    dmxfish::filters::filter_lua_script fil = filter_lua_script ();

    channel_mapping input_channels = channel_mapping();
    uint8_t in_dimmer = 56;
    uint8_t test_val = 28;
    dmxfish::dmx::pixel color = dmxfish::dmx::pixel(0.1,0.2,0.3);
    dmxfish::dmx::pixel testercol = dmxfish::dmx::pixel(0.8,0.6,0.6);
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
