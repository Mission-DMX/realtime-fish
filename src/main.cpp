#include "lib/logging.hpp"
#include "lib/macros.hpp"

int main_loop();

int main(int argc, char* argv[], char* env[]) {
	MARK_UNUSED(argc);
	MARK_UNUSED(argv);
	MARK_UNUSED(env);

	spdlog::set_level(spdlog::level::debug);

	auto ret_code = main_loop();
	::spdlog::debug("Main End");
	return ret_code;
}
