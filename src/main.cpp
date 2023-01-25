#include <memory>

#include "lib/logging.hpp"
#include "lib/macros.hpp"

#include "io/iomanager.hpp"
#include "rmrf-net/tcp_client.hpp"

#include <proto_src/RealTimeControl.pb.h>

#include <netdb.h>

#include <iostream>
#include <fstream>
// #include <string>


int main(int argc, char* argv[], char* env[]) {

	GOOGLE_PROTOBUF_VERIFY_VERSION;
	MARK_UNUSED(argc);
	MARK_UNUSED(argv);
	MARK_UNUSED(env);

	spdlog::set_level(spdlog::level::debug);
	auto run_time_state = std::make_shared<runtime_state_t>();
	auto io_manager = std::make_shared<dmxfish::io::IOManager>(run_time_state, true);
	io_manager->start();


	time_t start_time = time(NULL);
	while (run_time_state->running && time(NULL) < start_time+2) {

	}

		// io_manager->writeData("1134");
		// io_manager->writeData("2131");
		// io_manager->writeData("7134");
		// io_manager->writeData("1234");
		// io_manager->writeData("7234");




	// auto client = std::make_shared<rmrf::net::tcp_client>(8085, AF_INET6);

	auto curr_state_u = std::make_shared<missiondmx::fish::ipcmessages::update_state>();

	::missiondmx::fish::ipcmessages::RunMode value = ::missiondmx::fish::ipcmessages::RM_DIRECT;
	// RM_DIRECT = 1,
	// RM_STOP = 2,
	curr_state_u->set_new_state(value);

	::spdlog::debug("A: b:{:b}  s:{:s}", curr_state_u->IsInitialized(), curr_state_u->DebugString());

		std::fstream test("obj/test.data", std::ios::out | std::ios::trunc | std::ios::binary);

    if (!curr_state_u->SerializeToOstream(&test)) {
			  std::cerr << "Failed to write stuff" << std::endl;
      return -1;
    }


		auto curr_state_rcv = std::make_shared<missiondmx::fish::ipcmessages::update_state>();


		::spdlog::debug("Sending...");
		test.close();

		std::fstream testin("obj/test.data", std::ios::in | std::ios::binary);

    if (!curr_state_rcv->ParseFromIstream(&testin)) {
			  std::cerr << "Failed to read stuff" << std::endl;
      return -1;
    }
		testin.close();
		::spdlog::debug("Got It...");

		::spdlog::debug("B: b:{:b}  s:{:s}", curr_state_rcv->IsInitialized(), curr_state_rcv->DebugString());

		::missiondmx::fish::ipcmessages::RunMode value_rsv = curr_state_rcv->new_state();
		switch (value_rsv) {
			case ::missiondmx::fish::ipcmessages::RM_DIRECT:
				::spdlog::debug("Test A");
				break;
			case ::missiondmx::fish::ipcmessages::RM_FILTER:
				::spdlog::debug("Test B");
				break;
			case ::missiondmx::fish::ipcmessages::RM_STOP:
				::spdlog::debug("Test C");
				break;
			default:
				::spdlog::debug("Test D");
		}


	start_time = time(NULL);
	while (run_time_state->running && time(NULL) < start_time+1000) {

	}
	::spdlog::debug("Main End");
}
