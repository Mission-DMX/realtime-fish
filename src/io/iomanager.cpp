#include "io/iomanager.hpp"

#include <iomanip>
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <string>

#include "lib/logging.hpp"
#include "lib/macros.hpp"
#include "net/sock_address_factory.hpp"
#include <netdb.h>

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

void client_cb(rmrf::net::async_server_socket::self_ptr_type server, std::shared_ptr<rmrf::net::connection_client> client){
	MARK_UNUSED(server);
	::spdlog::debug("A client connected to the external control port. Address: {0}", client->get_peer_address().str());
}

IOManager::IOManager(std::shared_ptr<runtime_state_t> run_time_state_, bool is_default_manager) : running(true), iothread(nullptr), run_time_state(run_time_state_), loop(nullptr), external_control_server{nullptr} {
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
	this->external_control_server = std::make_shared<rmrf::net::tcp_server_socket>(8085, client_cb);
	::spdlog::debug("Opened control port.");
}

IOManager::~IOManager() {
	this->external_control_server = nullptr;
	this->running = false;
	this->loop->break_loop(::ev::ALL);
	this->iothread->join();

	::spdlog::debug("Stopped IO manager");
}

}
