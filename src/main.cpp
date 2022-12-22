#include <memory>

#include "lib/logging.hpp"
#include "lib/macros.hpp"

#include "io/iomanager.hpp"

int main(int argc, char* argv[], char* env[]) {
	MARK_UNUSED(argc);
	MARK_UNUSED(argv);
	MARK_UNUSED(env);

	// spdlog::set_level(spdlog::level::debug);
	auto io_manager = std::make_shared<dmxfish::io::IOManager>(true);
	io_manager->start();
}
