#include "../test/iomanager_t.hpp"

#include <iomanip>
#include <chrono>
#include <sstream>
#include <stdexcept>

#include "lib/logging.hpp"

#include "proto_src/MessageTypes.pb.h"
#include "proto_src/Console.pb.h"
#include "proto_src/DirectMode.pb.h"
#include "proto_src/FilterMode.pb.h"
#include "proto_src/RealTimeControl.pb.h"
#include "proto_src/UniverseControl.pb.h"
#include "google/protobuf/io/zero_copy_stream.h"



namespace dmxfish::test {

using namespace dmxfish::io;

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

IOManager::IOManager(client_handler::parse_message_cb_t parse_cb) :
		running(true),
		iothread(nullptr),
		loop(nullptr),
		gui_connections(std::make_shared<GUI_Connection_Handler>(parse_cb))

{

	if(!check_version_libev())
		throw std::runtime_error("Unable to initialize libev");
	this->loop = std::make_shared<::ev::default_loop>();

	this->iothread = std::make_shared<std::thread>(std::bind(&IOManager::run, this));
	const auto thread_id = std::hash<std::thread::id>{}(this->iothread->get_id());
	::spdlog::debug("Started IO manager with loop on thread with id {}.", thread_id);
}

void IOManager::start() {
	this->gui_connections->activate_tcp_connection(8085);
}

IOManager::~IOManager() {
	this->running = false;
	this->loop->break_loop(::ev::ALL);
	this->iothread->join();

	::spdlog::debug("Stopped IO manager");
}



}
// int main(int argc, char* argv[], char* env[]) {
// 	spdlog::set_level(spdlog::level::debug);
// 	::spdlog::debug("Only for others");
// }
