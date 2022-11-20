#include "io/iomanager.hpp"

namspace dmxfish::io {

IOManager::run() {
	this->loop->run(0);
}

IOManager::IOManager(bool is_default_manager = false) : running(true), iothread(nullptr) {
	if (is_default_manager) {
		this->loop = std::make_shared<::ev::default_loop>();
	} else {
		this->loop = std::make_shared<::ev::dynamic_loop>();
	}
}

IOManager::start() {
	this->iothread = std::make_shared<std::thread>([](std::weak_ptr<IOManager> v) {v->run();}, this);
	const auto thread_id = std::hash<std::thread::id>{}(this->iothread->get_id());
	::spdlog::debug("Started IO manager with loop on thread with id {0:d};.", thread_id);
}

IOManager::~IOManager() {
	this->loop->break_loop(::ev::ALL);
	this->running = false;
	this->iothread.join();
}

}
