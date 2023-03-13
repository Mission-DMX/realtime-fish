#include <chrono>
#include <memory>
#include <thread>

#include "lib/logging.hpp"
#include "lib/macros.hpp"

#include "io/iomanager.hpp"
// #include "io/universe_sender.hpp"
// #include "rmrf-net/tcp_client.hpp"
#include "rmrf-net/client_factory.hpp"
#include "rmrf-net/ioqueue.hpp"

// Test Stuff
#include "proto_src/RealTimeControl.pb.h"
#include "proto_src/MessageTypes.pb.h"
#include "google/protobuf/util/delimited_message_util.h"


// void perform_main_update(std::shared_ptr<dmxfish::dmx::universe> u) {
// 	time_t start_time = time(NULL);
// 	while (time(NULL) < start_time+15) {
// 		for(int i = 0; i < 24; i++)
// 			(*u)[i] += 1;
// 		//if(dmxfish::io::publish_universe_update(u)) spdlog::info("Posted Update.");
// 		dmxfish::io::publish_universe_update(u);
// 		std::this_thread::sleep_for(std::chrono::milliseconds(20));
// 	}
//
// 	start_time = time(NULL);
// 	while (time(NULL) < start_time+10) {
// 		for(int i = 0; i < 24; i++)
// 			(*u)[i] = 0;
// 		dmxfish::io::publish_universe_update(u);
// 		break;
// 	}
// }

// Sorry for forbidden stuff
void write_data(std::unique_ptr<rmrf::net::connection_client> cl) {
	// auto curr_state_u = std::make_shared<missiondmx::fish::ipcmessages::current_state_update>();
	// curr_state_u->set_current_state(missiondmx::fish::ipcmessages::RM_DIRECT);
	// curr_state_u->set_current_scene(23);
	// curr_state_u->set_last_cycle_time(25);
	// curr_state_u->set_last_error("Error Wathever");
	//
	// auto curr_state_u3 = std::make_shared<missiondmx::fish::ipcmessages::current_state_update>();
	// curr_state_u3->set_current_state(missiondmx::fish::ipcmessages::RM_STOP);
	// curr_state_u3->set_current_scene(22);
	// curr_state_u3->set_last_cycle_time(21);
	// curr_state_u3->set_last_error("Error Wathever2");


	uint32_t* msgtype = new uint32_t;
	// *msgtype = (uint8_t) ::missiondmx::fish::ipcmessages::MSGT_CURRENT_STATE_UPDATE;
	*msgtype = 25410;
	cl->write_data(rmrf::net::iorecord(msgtype, 4));

		// -> google Variante fuer Write Int, funktioniert nicht
		// ((google::protobuf::io::CodedOutputStream*) testBuffer->getOstream())->WriteVarint32(::missiondmx::fish::ipcmessages::MSGT_CURRENT_STATE_UPDATE);

	// uint32_t length = curr_state_u->ByteSizeLong();
	// mess_buff->WriteVarint32(length);
	// std::cout << "GC: finishedSerialize: " << curr_state_u->current_state() << " finished: " << curr_state_u->SerializeToZeroCopyStream(mess_buff.get()) << std::endl;
	// client_h->handle_messages();
	//
	// mess_buff->WriteVarint32(::missiondmx::fish::ipcmessages::MSGT_CURRENT_STATE_UPDATE);
	// std::cout << "GC: finishedSerialize: " << curr_state_u3->current_state() << " finished: " << google::protobuf::util::SerializeDelimitedToZeroCopyStream(*(curr_state_u3.get()), mess_buff.get()) << std::endl;

}

int main(int argc, char* argv[], char* env[]) {

	GOOGLE_PROTOBUF_VERIFY_VERSION;
	MARK_UNUSED(argc);
	MARK_UNUSED(argv);
	MARK_UNUSED(env);

	spdlog::set_level(spdlog::level::debug);
	auto run_time_state = std::make_shared<runtime_state_t>();
	// auto u = dmxfish::io::get_temporary_universe("10.0.15.1");

	dmxfish::io::IOManager manager(run_time_state, true);

	manager.start();

	// perform_main_update(u);

	time_t start_time = time(NULL);
	while (run_time_state->running && time(NULL) < start_time+2) {

	}

	auto client = rmrf::net::connect("::1", "8085", AF_INET6);

	start_time = time(NULL);
	while (run_time_state->running && time(NULL) < start_time+2) {

	}
	write_data(move(client));

	start_time = time(NULL);
	while (run_time_state->running && time(NULL) < start_time+20) {

	}
	::spdlog::debug("Main End");
}
