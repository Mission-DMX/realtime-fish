#include "io/iomanager.hpp"

#include <iomanip>
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <string>

#include "lib/logging.hpp"
#include "net/sock_address_factory.hpp"
#include <netdb.h>
#include <functional>
#include <functional>

namespace dmxfish::io {

bool check_version_libev()
{
    auto ev_major{ev::version_major()};
    auto ev_minor{ev::version_minor()};

    constexpr auto exp_major{EV_VERSION_MAJOR};
    constexpr auto exp_minor{EV_VERSION_MINOR};

    std::stringstream str;
    str <<
        "Checking dependency: libev: detected " <<
        std::dec << ev_major << "." << std::setw(2) << std::setfill('0') << ev_minor <<
        ", compiled " <<
        std::dec << exp_major << "." << std::setw(2) << std::setfill('0') << exp_minor;

    if (ev_major != exp_major) {
		::spdlog::debug(str.str().c_str());
        ::spdlog::error("Checking dependency: libev: failed version check: Major API version mismatch.");
        return false;
    }

    if (ev_minor < exp_minor) {
		::spdlog::debug(str.str().c_str());
        ::spdlog::error("Checking dependency: libev: failed version check: Minor API version too old.");
        return false;
    }

    return true;
}

void IOManager::run() {
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	::spdlog::debug("Entering ev defloop");
	this->loop->run(0);
	::spdlog::debug("Leaving ev defloop");
}

void IOManager::client_cb(std::shared_ptr<rmrf::net::tcp_client> client){
	::spdlog::debug("Client Here");
  // gui_clients.emplace<GuiVerbindung>(client);
  client->write_data("abc");
  // client->write_data("avcvdfadsf");
  this->conn_client=client;
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
		if(!check_version_libev())
			throw std::runtime_error("Unable to initialize libev");
		this->loop = std::make_shared<::ev::default_loop>();
	} else {
		this->loop = std::make_shared<::ev::dynamic_loop>();
	}
	this->iothread = std::make_shared<std::thread>(std::bind(&IOManager::run, this));
	const auto thread_id = std::hash<std::thread::id>{}(this->iothread->get_id());
	::spdlog::debug("Started IO manager with loop on thread with id {}.", thread_id);
}

void IOManager::start() {
	this->external_control_server = std::make_shared<rmrf::net::tcp_server_socket>(8085, std::bind(&dmxfish::io::IOManager::client_cb, this, std::placeholders::_1));
	::spdlog::debug("Opened control port.");
}

void IOManager::writeData(std::string str){
	this->msg_buffer->conn_data_in_cb(str);
}

IOManager::~IOManager() {
	this->external_control_server = nullptr;
	this->running = false;
	this->loop->break_loop(::ev::ALL);
	this->iothread->join();

	::spdlog::debug("Stopped IO manager");
}

}
