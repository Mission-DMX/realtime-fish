#include "io/iomanager.hpp"

#include "lib/logging.hpp"
#include "rmrf-net/tcp_server_socket.hpp"

#include "net/sock_address_factory.hpp"
#include <netdb.h>

namespace dmxfish::io {

void IOManager::run() {
	this->loop->run(0);
}

void client_cb(std::shared_ptr<rmrf::net::tcp_client> client){
		::spdlog::debug("Client Here");
}

void full_message_cb(std::string str, const std::string& s, bool msg_full){
		::spdlog::debug("Got full message: {:s}, {:s}, {:b}", str, s, msg_full);
}

IOManager::IOManager(std::shared_ptr<runtime_state_t> run_time_state_, bool is_default_manager) : running(true), iothread(nullptr), run_time_state(run_time_state_), loop(nullptr), msg_buffer(std::make_shared<message_buffer>(full_message_cb)) {
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

	const auto interface_addr = rmrf::net::get_first_general_socketaddr("::1", 8085);

		// auto server = std::make_shared<rmrf::net::tcp_server_socket>(interface_addr, client_cb);
	auto server = std::make_shared<rmrf::net::tcp_server_socket>(8085, client_cb);

}

void IOManager::writeData(std::string str){
	this->msg_buffer->conn_data_in_cb(str);
}

IOManager::~IOManager() {
	this->loop->break_loop(::ev::ALL);
	this->running = false;
	this->iothread->join();

	::spdlog::debug("Stopped IO manager");
}

}
