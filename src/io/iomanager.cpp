#include "io/iomanager.hpp"

#include "lib/logging.hpp"

namespace dmxfish::io {

void IOManager::run() {
	this->loop->run(0);
}

IOManager::IOManager(std::shared_ptr<runtime_state_t> run_time_state_, bool is_default_manager) : running(true), iothread(nullptr), run_time_state(run_time_state_), loop(nullptr) {
	if (is_default_manager) {
		this->loop = std::make_shared<::ev::default_loop>();
	} else {
		this->loop = std::make_shared<::ev::dynamic_loop>();
	}
}

void IOManager::start() {
	this->iothread = std::make_shared<std::thread>(std::bind(&IOManager::run, this));
	const auto thread_id = std::hash<std::thread::id>{}(this->iothread->get_id());
	::spdlog::debug("Started IO manager with loop on thread with id {0:d};.", thread_id);
}

IOManager::~IOManager() {
	this->loop->break_loop(::ev::ALL);
	this->running = false;
	this->iothread->join();
}

}
