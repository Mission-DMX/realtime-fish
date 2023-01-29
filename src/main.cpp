#include <chrono>
#include <memory>
#include <thread>

#include "lib/logging.hpp"
#include "lib/macros.hpp"

#include "io/iomanager.hpp"
#include "io/universe_sender.hpp"
#include "rmrf-net/tcp_client.hpp"

#include <proto_src/RealTimeControl.pb.h>

#include <netdb.h>
#include <fstream>
#include <unistd.h>

#include "google/protobuf/util/delimited_message_util.h"

void perform_main_update(std::shared_ptr<dmxfish::dmx::universe> u) {
	time_t start_time = time(NULL);
	while (time(NULL) < start_time+15) {
		for(int i = 0; i < 24; i++)
			(*u)[i] += 1;
		//if(dmxfish::io::publish_universe_update(u)) spdlog::info("Posted Update.");
		dmxfish::io::publish_universe_update(u);
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
	// auto client = std::make_shared<rmrf::net::tcp_client>(8085, AF_INET6);

	//auto curr_state_u = std::make_shared<missiondmx::fish::ipcmessages::current_state_update>();

	start_time = time(NULL);
	while (time(NULL) < start_time+10) {
		for(int i = 0; i < 24; i++)
			(*u)[i] = 0;
		dmxfish::io::publish_universe_update(u);
		break;
	}
}

int main(int argc, char* argv[], char* env[]) {

	GOOGLE_PROTOBUF_VERIFY_VERSION;
	MARK_UNUSED(argc);
	MARK_UNUSED(argv);
	MARK_UNUSED(env);

	spdlog::set_level(spdlog::level::debug);
	auto run_time_state = std::make_shared<runtime_state_t>();
	auto u = dmxfish::io::get_temporary_universe("10.0.15.1");
	dmxfish::io::IOManager manager(run_time_state, true);
	manager.start();

	// perform_main_update(u);

	time_t start_time = time(NULL);
	while (run_time_state->running && time(NULL) < start_time+2) {

	}


	// client_listener(tcp_client(std::bind(&tcp_server_socket::client_destructed_cb, this, _1), auto_fd(client_fd_raw), address, port));

	// auto client = std::make_shared<rmrf::net::tcp_client>(std::bind(&tcp_server_socket::client_destructed_cb, this, _1), auto_fd(client_fd_raw), address, port);
	// auto client = std::make_shared<rmrf::net::tcp_client>("127.0.0.1", 8085);

	// auto client = std::make_shared<rmrf::net::tcp_client>("::1", 8085);


	// client->write_data("aadsfsafasef");

	auto curr_state_u = std::make_shared<missiondmx::fish::ipcmessages::update_state>();

	missiondmx::fish::ipcmessages::update_state* us = new missiondmx::fish::ipcmessages::update_state();
	us->set_new_state(::missiondmx::fish::ipcmessages::RM_DIRECT);

	::missiondmx::fish::ipcmessages::RunMode value = ::missiondmx::fish::ipcmessages::RM_DIRECT;
	// RM_DIRECT = 1,
	// RM_STOP = 2,
	curr_state_u->set_new_state(value);

	::spdlog::debug("A: b:{:b}  s:{:s}", curr_state_u->IsInitialized(), curr_state_u->DebugString());
	std::fstream test("obj/test.data", std::ios::out | std::ios::trunc | std::ios::binary);

	std::cout << "AB: " << test.tellg() << std::endl;

  // if (!curr_state_u->SerializeToOstream(&test)) {
	// 	  std::cerr << "Failed to write stuff" << std::endl;
  //   return -1;
  // }
	// std::cout << "ABC: " << test.tellg() << std::endl;
	// test.close();
	//
	//
	// ::spdlog::debug("Sending...");
	//
	//
	// auto curr_state_rcv = std::make_shared<missiondmx::fish::ipcmessages::update_state>();
	// std::fstream testin("obj/test.data", std::ios::in | std::ios::binary);
	//
  // if (!curr_state_rcv->ParseFromIstream(&testin)) {
	// 	  std::cerr << "Failed to read stuff" << std::endl;
  //   return -1;
  // }
	// testin.close();
	// ::spdlog::debug("Got It...");
	//
	//
	// ::spdlog::debug("B: b:{:b}  s:{:s}", curr_state_rcv->IsInitialized(), curr_state_rcv->DebugString());
	//
	// ::missiondmx::fish::ipcmessages::RunMode value_rsv = curr_state_rcv->new_state();
	// switch (value_rsv) {
	// 	case ::missiondmx::fish::ipcmessages::RM_DIRECT:
	// 		::spdlog::debug("Test A");
	// 		break;
	// 	case ::missiondmx::fish::ipcmessages::RM_FILTER:
	// 		::spdlog::debug("Test B");
	// 		break;
	// 	case ::missiondmx::fish::ipcmessages::RM_STOP:
	// 		::spdlog::debug("Test C");
	// 		break;
	// 	default:
	// 		::spdlog::debug("Test D");
	// }





  if (google::protobuf::util::SerializeDelimitedToOstream(*(curr_state_u.get()), &test)) {
		  std::cerr << "Failed to write stuff" << std::endl;
    return -1;
  }
	
	// if (!us->SerializeToOstream(&test)) {
	// 	  std::cerr << "Failed to write stuff" << std::endl;
  //   return -1;
  // }
	std::cout << "ABC: " << test.tellg() << std::endl;
	// test.close();
	//
	//
	// ::spdlog::debug("Sending...");
	//
	//
	// auto curr_state_rcv = std::make_shared<missiondmx::fish::ipcmessages::update_state>();
	// std::fstream testin("obj/test.data", std::ios::in | std::ios::binary);
	//
  // if (!curr_state_rcv->ParseFromIstream(&testin)) {
	// 	  std::cerr << "Failed to read stuff" << std::endl;
  //   return -1;
  // }
	// testin.close();
	// ::spdlog::debug("Got It...");
	//
	//
	// ::spdlog::debug("B: b:{:b}  s:{:s}", curr_state_rcv->IsInitialized(), curr_state_rcv->DebugString());
	//
	// ::missiondmx::fish::ipcmessages::RunMode value_rsv = curr_state_rcv->new_state();
	// switch (value_rsv) {
	// 	case ::missiondmx::fish::ipcmessages::RM_DIRECT:
	// 		::spdlog::debug("Test A");
	// 		break;
	// 	case ::missiondmx::fish::ipcmessages::RM_FILTER:
	// 		::spdlog::debug("Test B");
	// 		break;
	// 	case ::missiondmx::fish::ipcmessages::RM_STOP:
	// 		::spdlog::debug("Test C");
	// 		break;
	// 	default:
	// 		::spdlog::debug("Test D");
	// }





	::spdlog::debug("Main End");
}
