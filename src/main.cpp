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


#include "rmrf-net/tcp_client.hpp"
#include "stdin_watcher.hpp"


#include "proto_src/RealTimeControl.pb.h"

void perform_main_update(std::shared_ptr<runtime_state_t> t) {
	namespace stdc = std::chrono;
	while (t->running) {
		const auto start_time = stdc::system_clock::now().time_since_epoch();
		// TODO Fetch FPGA input data structure from iomanager and either lock or copy it
		if (t->is_direct_mode) {
			// TODO fetch and apply updates from GUI and FPGA
		} else { // Direct mode
			// TODO calculate filters using input data
		}
		// TODO Release input data structure if it was locked and not copied.

		dmxfish::io::push_all_registered_universes();
		// TODO push updates to UI

		// stop timer and wait 2ms until next cycle
		const auto end_time = stdc::system_clock::now().time_since_epoch();
		std::this_thread::sleep_for(stdc::milliseconds(18) - (end_time - start_time));
	}
	// auto client = std::make_shared<rmrf::net::tcp_client>(8085, AF_INET6);

	//auto curr_state_u = std::make_shared<missiondmx::fish::ipcmessages::current_state_update>();
}

int main(int argc, char* argv[], char* env[]) {

	GOOGLE_PROTOBUF_VERIFY_VERSION;
	MARK_UNUSED(argc);
	MARK_UNUSED(argv);
	MARK_UNUSED(env);

	spdlog::set_level(spdlog::level::debug);
	auto run_time_state = std::make_shared<runtime_state_t>();

	stdin_watcher sin_w([run_time_state](){
		run_time_state->running = false;
		::spdlog::info("Stopping server from keyboard now.");
	});
	dmxfish::io::IOManager manager(run_time_state, true);

	manager.start();

  // perform_main_update(u);
	//
	//
	// auto socket_address = rmrf::net::get_first_general_socketaddr("/tmp/9Lq7BNBnBycd6nxyz.socket", "", rmrf::net::socket_t::UNIX);
	// auto client = rmrf::net::connect(socket_address);
	//
	// start_time = time(NULL);
	// while (run_time_state->running && time(NULL) < start_time+2) {
	//
	// }
	//
	// // every cycle update
	// auto msg = std::make_shared<missiondmx::fish::ipcmessages::current_state_update>();
	// msg->set_current_state(::missiondmx::fish::ipcmessages::RM_DIRECT);
	// msg->set_showfile_apply_state(::missiondmx::fish::ipcmessages::SFAS_INVALID);
	// msg->set_current_scene(-1);
	// msg->set_last_cycle_time(10);
	// msg->set_last_error("No Error occured");
	//
	// iomanager.broadcast_message(msg, ::missiondmx::fish::ipcmessages::MSGT_CURRENT_STATE_UPDATE);
	//
	// start_time = time(NULL);
	// while (run_time_state->running && time(NULL) < start_time+20) {
	//
	// }

	perform_main_update(run_time_state);

	::spdlog::debug("Main End");
}
