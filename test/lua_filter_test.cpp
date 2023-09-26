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

    std::map <std::string, std::string> configuration;
    configuration["in_mapping"] = "dimmer:8bit";
    configuration["out_mapping"] = "dimmer:8bit";
    std::map <std::string, std::string> initial_parameters;
//    initial_parameters["script"] = "print(color) color[\"s\"] = 0.2";
    initial_parameters["script"] = "print(' called update ') print(number) print(number2) number =42 number2 = 56.7 print(number) print(number2)";

    fil.pre_setup(configuration, initial_parameters);
    fil.setup_filter (configuration, initial_parameters, input_channels);
    fil.update();
    fil.update();
    BOOST_TEST(true, "ja");
}
