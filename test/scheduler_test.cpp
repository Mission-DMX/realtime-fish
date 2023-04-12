#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE RMRF_TESTS
#include <boost/test/included/unit_test.hpp>

#include <filesystem>
#include <memory>

#include "executioners/scene_factory.hpp"

using namespace dmxfish::filters;

BOOST_AUTO_TEST_CASE(scheduler_test) {
	try {
		std::vector<dmxfish::execution::scene> v;
		xml_schema::properties properties;
		const auto cwd = std::filesystem::current_path().string();
		properties.no_namespace_schema_location("file:///" + cwd + "/submodules/Docs/FormatSchemes/ProjectFile/ShowFile_v0.xsd");
		properties.schema_location("http://www.w3.org/2001/XMLSchema", "file:///" + cwd + "/src/xml/schema/XMLSchema.xsd");
		std::unique_ptr<MissionDMX::ShowFile::BordConfiguration> bc = MissionDMX::ShowFile::bord_configuration("./test/test_bord_config.xml", 0, properties);
		//std::unique_ptr<MissionDMX::ShowFile::BordConfiguration> bc = MissionDMX::ShowFile::bord_configuration("./test/test_bord_config.xml", xml_schema::flags::dont_validate);
		std::cout << "Loaded XML file." << std::endl;

		if(dmxfish::execution::populate_scene_vector(v, bc->scene()))
			std::cout << "Parsing complete." << std::endl;
		else
			std::cout << "Parsing failed." << std::endl;

		BOOST_CHECK_EQUAL(v.size(), 2);
		if(v.size() == 2)
			BOOST_CHECK_EQUAL(v[0].get_filter_count(), 4);
	} catch (const xml_schema::exception& e) {
		std::cerr << e << std::endl;
		BOOST_CHECK(false);
	}
}
