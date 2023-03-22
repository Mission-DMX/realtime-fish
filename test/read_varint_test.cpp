#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE RMRF_TESTS
#include <boost/test/included/unit_test.hpp>

#include "lib/logging.hpp"

#include "../test/test_client_handler.hpp"
#include "../test/test_message_buffer.hpp"
// #include "io/message_buffer.hpp"

#include "rmrf-net/client_factory.hpp"
#include "rmrf-net/ioqueue.hpp"

#include "proto_src/RealTimeControl.pb.h"
#include "proto_src/MessageTypes.pb.h"
#include "proto_src/DirectMode.pb.h"
#include "google/protobuf/util/delimited_message_util.h"

using namespace dmxfish::io;

BOOST_AUTO_TEST_CASE(helloworld) {


	spdlog::set_level(spdlog::level::debug);

	auto iomanager = std::make_shared<dmxfish::test::Test_Client_Handler>();
	iomanager->start();

	time_t start_time = time(NULL);
	while (time(NULL) < start_time+2) {

	}

	auto client = rmrf::net::connect("::1", "8086");


	if(!client) {
		BOOST_CHECK_MESSAGE(false, "Es konnte keine Verbindung zu [::1]:8086 aufgebaut werden. Läuft der Server?");
	}

	start_time = time(NULL);
	while (time(NULL) < start_time+2) {

	}
	auto msg = std::make_shared<missiondmx::fish::ipcmessages::dmx_output>();
	msg->set_universe_id(1);

	::spdlog::debug("1msg size: {}", msg->ByteSizeLong());
	msg->add_channel_data(1);

	::spdlog::debug("2msg size: {}", msg->ByteSizeLong());
	for (int j = 0; j<20; j++){
		msg->add_channel_data(j+2);
		// ::spdlog::debug("3msg size: {}", msg->ByteSizeLong());
	}
	::spdlog::debug("4msg size: {}", msg->ByteSizeLong());

	auto io_buff = std::make_shared<::rmrf::net::ioqueue<::rmrf::net::iorecord>>();

	auto out_b = std::make_unique<dmxfish::test::message_buffer_output>(move(client));
	out_b->WriteVarint32(::missiondmx::fish::ipcmessages::MSGT_DMX_OUTPUT);
	out_b->WriteVarint32(msg->ByteSizeLong());
	msg->SerializeToZeroCopyStream(out_b.get());

	start_time = time(NULL);
	while (time(NULL) < start_time+60) {

	}
	BOOST_CHECK_EQUAL("abc", "abc");
}
