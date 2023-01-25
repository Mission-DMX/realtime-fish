#include "io/iomanager.hpp"

#include "lib/logging.hpp"
#include "rmrf-net/tcp_server_socket.hpp"

#include "net/sock_address_factory.hpp"
#include <netdb.h>
#include <functional>
#include <functional>

namespace dmxfish::io {

void IOManager::run() {
	this->loop->run(0);
}

void IOManager::client_cb(std::shared_ptr<rmrf::net::tcp_client> client){
	::spdlog::debug("Client Here");
}

void IOManager::full_message_cb(msg_t msg_type, const std::string& s, bool msg_full){
	switch (msg_type) {
		case dmxfish::io::msg_t::UPDATE_STATE:
				::spdlog::debug("Update-State: Got full message: {:s}" , s);
				break;
		case dmxfish::io::msg_t::CURRENT_STATE_UPDATE:
				::spdlog::debug("CurrentState: Got full message: {:s}" , s);
				break;
		default:
				::spdlog::debug("Error: Got full message: {:s}" , s);
				break;
	}
}

IOManager::IOManager(std::shared_ptr<runtime_state_t> run_time_state_, bool is_default_manager) : running(true), iothread(nullptr), run_time_state(run_time_state_), loop(nullptr), msg_buffer(std::make_shared<message_buffer>(std::bind(&dmxfish::io::IOManager::full_message_cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))) {
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

	const auto interface_addr = rmrf::net::get_first_general_socketaddr("127.0.0.1", 9861);
	auto server = std::make_shared<rmrf::net::tcp_server_socket>(interface_addr, std::bind(&dmxfish::io::IOManager::client_cb, this, std::placeholders::_1));


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
