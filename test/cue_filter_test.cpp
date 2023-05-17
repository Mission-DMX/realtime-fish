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
    for (int i = 0; i < 40000; i = i + 100)
    {
        time_s = (double) i;
        fil.update ();
        if (i == 1000)
        {
            const std::string key = "run_mode";
            const std::string _value = "play";
            fil.receive_update_from_gui (key, _value);
        }

        fil.get_output_channels (map, name);
        for (auto it = map.eight_bit_channels.begin ();
             it != map.eight_bit_channels.end (); ++it)
        {
            uint8_t tester = (time_s<2000)?0:100;
            std::string error = std::string("Channel ") + it->first + " should be " + std::to_string(tester) + " , but is " + std::to_string(*it->second) + " at time: " + std::to_string(time_s);
            BOOST_TEST(*map.eight_bit_channels["t:dimmer"] == tester, error);
        }
        for (auto it = map.sixteen_bit_channels.begin ();
             it != map.sixteen_bit_channels.end (); ++it)
        {
            std::cout << time_s /
                         1000 << " 16: " << it->first << ", " << (int) *it->second << '\n';
        }
        for (auto it = map.float_channels.begin ();
             it != map.float_channels.end (); ++it)
        {
            std::cout << time_s /
                         1000 << " fl: " << it->first << ", " << (double) *it->
                    second << '\n';
        }
        for (auto it = map.color_channels.begin ();
             it != map.color_channels.end (); ++it)
        {
            std::cout << time_s /
                         1000 << " co: " << it->first << ", " << it->
                    second->str () << '\n';
        }
        map.eight_bit_channels.clear ();
        map.sixteen_bit_channels.clear ();
        map.float_channels.clear ();
        map.color_channels.clear ();
    }



	BOOST_CHECK_EQUAL("abc", "abc");




}
