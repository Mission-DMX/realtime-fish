#include <memory>

#include <spdlog-inl.h>

#include "io/iomanager.hpp"

int main(int argc, char* argv[], char* env[]) {
	spdlog::set_level(spdlog::level::debug);
	auto io_manager = std::make_shared<dmxfish::io::IOManager>(true);
	io_manager->start()
}
