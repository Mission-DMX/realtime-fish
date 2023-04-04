#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE RMRF_TESTS
#include <boost/test/included/unit_test.hpp>

#include <memory>

#include "executioners/scene_factory.hpp"

using namespace dmxfish::filters;
using namespace dmxfish::execution;

BOOST_AUTO_TEST_CASE(scheduler_test) {
	std::vector<scene> v;
	std::unique_ptr<MissionDMX::ShowFile::BordConfiguration> bc = std::make_unique<MissionDMX::ShowFile::BordConfiguration>(std::string("test/test_bord_config.xml"));

	populate_scene_vector(v, bc->scene());

	BOOST_CHECK_EQUAL(v.size(), 2);
}
