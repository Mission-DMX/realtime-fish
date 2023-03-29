#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE RMRF_TESTS
#include <boost/test/included/unit_test.hpp>

#include "lib/logging.hpp"

#include "../test/test_client_side.hpp"
// #include "../test/test_message_buffer.hpp"
#include "io/universe_sender.hpp"
#include "io/iomanager.hpp"

#include "rmrf-net/client_factory.hpp"
#include "rmrf-net/ioqueue.hpp"

#include "proto_src/RealTimeControl.pb.h"
#include "proto_src/MessageTypes.pb.h"
#include "proto_src/DirectMode.pb.h"
#include "google/protobuf/util/delimited_message_util.h"

#include <google/protobuf/text_format.h>

using namespace dmxfish::io;

void parse_message_cb(uint32_t msg_type, dmxfish::test::client_side& client){
	int count = 0;
	int size = 0;
	uint8_t* data;
	while(client.Next((const void**) &data, &size)){
		count += size;
	}
	::spdlog::debug("Msg has type {} and size {}", msg_type, count);

}


BOOST_AUTO_TEST_CASE(helloworld) {


	spdlog::set_level(spdlog::level::debug);

	auto u = dmxfish::io::get_temporary_universe("10.15.0.1");

	auto run_time_state = std::make_shared<runtime_state_t>();

	auto iomanager = std::make_shared<IOManager>(run_time_state, true);
	// auto iomanager = std::make_shared<iomanager>(run_time_state, true);
	iomanager->start();

	time_t start_time = time(NULL);
	while (time(NULL) < start_time+2) {

	}

	auto socket_address = rmrf::net::get_first_general_socketaddr("/tmp/9Lq7BNBnBycd6nxyz.socket", "", rmrf::net::socket_t::UNIX);
	auto client = rmrf::net::connect(socket_address);


	if(!client) {
		BOOST_CHECK_MESSAGE(false, "Es konnte keine Verbindung zu /tmp/9Lq7BNBnBycd6nxyz.socket aufgebaut werden. Läuft der Server?");
	}

	start_time = time(NULL);
	while (time(NULL) < start_time+2) {

	}

	auto client_ha = std::make_shared<dmxfish::test::client_side>(std::bind(&parse_message_cb, std::placeholders::_1, std::placeholders::_2), std::move(client));


	auto msg = std::make_shared<missiondmx::fish::ipcmessages::request_dmx_data>();
	msg->set_universe_id(-1);
	client_ha->write_message(*msg.get(), ::missiondmx::fish::ipcmessages::MSGT_REQUEST_DMX_DATA);


	start_time = time(NULL);
	while (time(NULL) < start_time+5) {

	}
	BOOST_CHECK_EQUAL("abc", "abc");
}
