#include <chrono>
#include <memory>
#include <thread>

#include "lib/logging.hpp"
#include "lib/macros.hpp"

#include "io/iomanager.hpp"
#include "io/universe_sender.hpp"
#include "rmrf-net/tcp_client.hpp"
#include "stdin_watcher.hpp"

#include "proto_src/RealTimeControl.pb.h"

#include <netdb.h>

#include <unistd.h>

void perform_main_update(std::shared_ptr<dmxfish::dmx::universe> u) {
	namespace stdc = std::chrono;
	while (*u != runtime_state_t::RM_STOP) {
		const auto start_time = stdc::system_clock::now().time_since_epoch();
		// TODO Fetch FPGA input data structure from iomanager and either lock or copy it
		if (*u == runtime_state_t::RM_FILTER) {
			// TODO calculate filters using input data
		} else { // Direct mode
			// TODO fetch and apply updates from GUI and FPGA
		}
		// TODO Release input data structure if it was locked and not copied.
		// TODO push universes
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

	stdin_watcher sin_w([](){
		*run_time_state = runtime_state_t::RM_STOP;
		::spdlog::info("Stopping server from keyboard now.");
	});

	dmxfish::io::IOManager manager(run_time_state, true);
	manager.start();

	perform_main_update(run_time_state);

	::spdlog::debug("Main End");
}
