#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE RMRF_TESTS
#include <boost/test/included/unit_test.hpp>

#include <filesystem>
#include <memory>

#include "executioners/scene_factory.hpp"
#include "lib/logging.hpp"

using namespace dmxfish::filters;

inline bool ends_with(std::string const & value, std::string const & ending) {
    if (ending.size() > value.size())
		return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

BOOST_AUTO_TEST_CASE(scheduler_test) {
	spdlog::set_level(spdlog::level::debug);
	try {
		std::vector<dmxfish::execution::scene> v;
		xml_schema::properties properties;
		const auto cwd = std::filesystem::current_path().string();
		properties.no_namespace_schema_location("file:///" + cwd + "/submodules/Docs/FormatSchemes/ProjectFile/ShowFile_v0.xsd");
		properties.schema_location("http://www.w3.org/2001/XMLSchema", "file:///" + cwd + "/src/xml/schema/XMLSchema.xsd");
		//std::unique_ptr<MissionDMX::ShowFile::BordConfiguration> bc = MissionDMX::ShowFile::bord_configuration("./test/test_bord_config.xml", 0, properties);
		std::unique_ptr<MissionDMX::ShowFile::BordConfiguration> bc = MissionDMX::ShowFile::bord_configuration("./test/test_bord_config.xml", xml_schema::flags::dont_validate);
		std::cout << "Loaded XML file." << std::endl;

		if(const auto res = dmxfish::execution::populate_scene_vector(v, bc->scene()); ends_with(res.first, "\nDone.\n") && res.second) {
			std::cout << "Parsing complete." << std::endl;
		} else {
			std::cout << "Parsing failed. (reported: " << (res.second == true ? "true" : "false") << ") Parsing log:" << std::endl;
			std::cout << res.first << std::endl;
		}

		BOOST_CHECK_EQUAL(v.size(), 2);
		if(v.size() == 2) {
			BOOST_CHECK_EQUAL(v[0].get_filter_count(), 8);
			BOOST_CHECK_EQUAL(v[1].get_filter_count(), 0);
			v[0].on_start();
			v[0].invoke_filters();
			v[0].on_stop();
			v[1].on_start();
			v[1].invoke_filters();
			v[1].on_stop();
			// TODO catch spdlog output to search for correct output.
		}
	} catch (const xml_schema::exception& e) {
		std::cerr << e << std::endl;
		BOOST_CHECK(false);
	}
}
