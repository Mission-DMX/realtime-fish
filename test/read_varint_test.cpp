#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE RMRF_TESTS
#include <boost/test/included/unit_test.hpp>

#include "io/client_handler.hpp"
#include "io/iomanager.hpp"

#include <iostream>


using namespace dmxfish::io;


bool parse_message_cb(uint32_t msg_type, google::protobuf::io::ZeroCopyInputStream& buff){
	std::cout << "test" << std::endl;
}


BOOST_AUTO_TEST_CASE(helloworld) {

	auto run_time_state = std::make_shared<runtime_state_t>();
	IOManager manager(run_time_state, true);
	time_t start_time = time(NULL);
	while (run_time_state->running && time(NULL) < start_time+2) {

	}
	auto client = std::make_shared<rmrf::net::tcp_client>("::1", 8085);
	auto client_h = std::make_shared<client_handler>(parse_message_cb, client);
	std::cout << "helloworld" << std::endl;
	BOOST_CHECK_EQUAL("abc", "abc");
}
