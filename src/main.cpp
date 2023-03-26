#include <chrono>
#include <memory>
#include <thread>

#include "lib/logging.hpp"
#include "lib/macros.hpp"

#include "io/iomanager.hpp"

#include "rmrf-net/client_factory.hpp"
#include "rmrf-net/ioqueue.hpp"

#include "io/universe_sender.hpp"
#include "rmrf-net/client_factory.hpp"
#include "rmrf-net/ioqueue.hpp"


#include "proto_src/RealTimeControl.pb.h"

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
	auto u = dmxfish::io::get_temporary_universe("10.15.0.1");
	dmxfish::io::IOManager manager(run_time_state, true);

	manager.start();
  perform_main_update(u);

	time_t start_time = time(NULL);
	while (run_time_state->running && time(NULL) < start_time+2) {

	}

	auto client = rmrf::net::connect("::1", "8085", AF_INET6);

	start_time = time(NULL);
	while (run_time_state->running && time(NULL) < start_time+2) {

	}

	// every cycle update
	auto msg = std::make_shared<missiondmx::fish::ipcmessages::current_state_update>();
	msg->set_current_state(::missiondmx::fish::ipcmessages::RM_DIRECT);
	msg->set_showfile_apply_state(::missiondmx::fish::ipcmessages::SFAS_INVALID);
	// can't we run multiple scenes at the same time?
	msg->set_current_scene(-1);
	msg->set_last_cycle_time(10);
	msg->set_last_error("No Error occured");

	iomanager.broadcast_message(msg, ::missiondmx::fish::ipcmessages::MSGT_CURRENT_STATE_UPDATE);

	start_time = time(NULL);
	while (run_time_state->running && time(NULL) < start_time+20) {

	}

	::spdlog::debug("Main End");
}
