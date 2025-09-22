#include <execinfo.h>
#include <signal.h>

#include "sound/fft.hpp"
#include "lib/logging.hpp"
#include "lib/macros.hpp"

int main_loop();

void segv_handler(int sig) {
    // unfortunately this needs to be plain C
    void *bt_array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(bt_array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d, handled with base exception handler:\n", sig);
    backtrace_symbols_fd(bt_array, size, STDERR_FILENO);
    exit(1);
}

int main(int argc, char* argv[], char* env[]) {
	MARK_UNUSED(argc);
	MARK_UNUSED(argv);
	MARK_UNUSED(env);

    signal(SIGSEGV, segv_handler);

	spdlog::set_level(spdlog::level::debug);

    dmxfish::audio::train_fft();

	auto ret_code = main_loop();
	::spdlog::debug("Main End");
	return ret_code;
}
