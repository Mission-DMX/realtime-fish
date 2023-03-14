#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE RMRF_TESTS
#include <boost/test/included/unit_test.hpp>

#include "lib/logging.hpp"
#include "../test/test_iomanager.hpp"

#include "rmrf-net/client_factory.hpp"

#include "proto_src/RealTimeControl.pb.h"
#include "proto_src/MessageTypes.pb.h"
#include "google/protobuf/util/delimited_message_util.h"

using namespace dmxfish::io;

BOOST_AUTO_TEST_CASE(helloworld) {


	spdlog::set_level(spdlog::level::debug);

	auto iomanager = std::make_shared<dmxfish::test::Test_IOManager>();
	iomanager->start();

	time_t start_time = time(NULL);
	while (time(NULL) < start_time+2) {

	}
	auto client = rmrf::net::connect("::1", "8086");

	if(!client) {
		BOOST_CHECK_MESSAGE(false, "Es konnte keine Verbindung zu [::1]:8086 aufgebaut werden. Läuft der Server?");
	}

	uint32_t msgtype = 25410;
	start_time = time(NULL);
	while (time(NULL) < start_time+2) {

	}
	::spdlog::debug("before writing");
	client->write_data(rmrf::net::iorecord(&msgtype, sizeof(msgtype)));
	::spdlog::debug("after writing");



	start_time = time(NULL);
	while (time(NULL) < start_time+20) {

	}

	BOOST_CHECK_EQUAL("abc", "abc");
}
