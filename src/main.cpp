#include <memory>

#include "lib/logging.hpp"
#include "lib/macros.hpp"

#include "io/iomanager.hpp"
#include "rmrf-net/tcp_client.hpp"

#include <proto_src/RealTimeControl.pb.h>

#include <netdb.h>

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
	while (run_time_state->running && time(NULL) < start_time+4) {

	}

	// auto client = std::make_shared<rmrf::net::tcp_client>(8085, AF_INET6);

	// auto curr_state_u = std::make_shared<missiondmx::fish::ipcmessages::current_state_update>();

	start_time = time(NULL);
	while (run_time_state->running && time(NULL) < start_time+10) {

	}
	::spdlog::debug("Main End");
}
