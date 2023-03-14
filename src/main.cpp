#include <chrono>
#include <memory>
#include <thread>

#include "lib/logging.hpp"
#include "lib/macros.hpp"

#include "io/iomanager.hpp"
#include "rmrf-net/client_factory.hpp"
#include "rmrf-net/ioqueue.hpp"

int main(int argc, char* argv[], char* env[]) {

	GOOGLE_PROTOBUF_VERIFY_VERSION;
	MARK_UNUSED(argc);
	MARK_UNUSED(argv);
	MARK_UNUSED(env);

	spdlog::set_level(spdlog::level::debug);
	auto run_time_state = std::make_shared<runtime_state_t>();

	dmxfish::io::IOManager manager(run_time_state, true);

	manager.start();

	time_t start_time = time(NULL);
	while (run_time_state->running && time(NULL) < start_time+2) {

	}

	auto client = rmrf::net::connect("::1", "8085", AF_INET6);

	start_time = time(NULL);
	while (run_time_state->running && time(NULL) < start_time+2) {

	}

	start_time = time(NULL);
	while (run_time_state->running && time(NULL) < start_time+20) {

	}
	::spdlog::debug("Main End");
}
