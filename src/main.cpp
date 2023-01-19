#include <memory>

#include "lib/logging.hpp"
#include "lib/macros.hpp"

#include "io/iomanager.hpp"

int main(int argc, char* argv[], char* env[]) {
	MARK_UNUSED(argc);
	MARK_UNUSED(argv);
	MARK_UNUSED(env);

	spdlog::set_level(spdlog::level::debug);
	auto run_time_state = std::make_shared<runtime_state_t>();
	auto io_manager = std::make_shared<dmxfish::io::IOManager>(run_time_state, true);
	io_manager->start();
	while (run_time_state->running) {

	}
}
