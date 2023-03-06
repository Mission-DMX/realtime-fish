#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE RMRF_TESTS
#include <boost/test/included/unit_test.hpp>

#include "lib/logging.hpp"
// #include "io/client_handler.hpp"
#include "../test/iomanager_t.hpp"

#include <iostream>


using namespace dmxfish::io;

bool parse_message_cb(uint32_t msg_type, google::protobuf::io::ZeroCopyInputStream& buff){
	::spdlog::debug("callback reached {}", msg_type);
	return true;
}


BOOST_AUTO_TEST_CASE(helloworld) {


	spdlog::set_level(spdlog::level::debug);
	auto io_manager_test = std::make_shared<dmxfish::test::IOManager>(std::bind(&parse_message_cb, std::placeholders::_1, std::placeholders::_2));
	io_manager_test->start();
		::spdlog::debug("test1");
	auto client = std::make_shared<rmrf::net::tcp_client>("::1", 8085);
		::spdlog::debug("test2");
	time_t start_time = time(NULL);
	while (time(NULL) < start_time+10) {

	}
		::spdlog::debug("test3");
	BOOST_CHECK_EQUAL("abc", "abc");
}
