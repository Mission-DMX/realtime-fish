#include "io/iomanager.hpp"

#include <iomanip>
#include <chrono>
#include <sstream>
#include <stdexcept>

#include "lib/logging.hpp"

#include "proto_src/RealTimeControl.pb.h"
#include "google/protobuf/util/delimited_message_util.h"


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

IOManager::IOManager(std::shared_ptr<runtime_state_t> run_time_state_, bool is_default_manager) :
    running(true),
    iothread(nullptr),
    run_time_state(run_time_state_),
    loop(nullptr),
    gui_connections(std::make_shared<GUI_Connection_Handler>(std::bind(&dmxfish::io::IOManager::parse_message_cb, this, std::placeholders::_1, std::placeholders::_2)))

{
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
  this->gui_connections->activate_tcp_connection(8085);
}

IOManager::~IOManager() {
	this->running = false;
	this->loop->break_loop(::ev::ALL);
	this->iothread->join();

	::spdlog::debug("Stopped IO manager");
}

bool IOManager::parse_message_cb(uint32_t msg_type, message_buffer_input* buff){
	switch ((::missiondmx::fish::ipcmessages::MsgType) msg_type) {
		case ::missiondmx::fish::ipcmessages::MSGT_UPDATE_STATE:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::update_state>();
				bool cleanEOF;
				if (buff->HandleReadResult(google::protobuf::util::ParseDelimitedFromZeroCopyStream(msg.get(), buff, &cleanEOF))){
            std::cout << "Update State state: " << msg->new_state() << std::endl;
						return true;
				}
				return false;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_CURRENT_STATE_UPDATE:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::current_state_update>();
				bool cleanEOF;
				if (buff->HandleReadResult(google::protobuf::util::ParseDelimitedFromZeroCopyStream(msg.get(), buff, &cleanEOF))){
            std::cout << "CurUpState state: " << msg->current_state() << std::endl;
						return true;
				}

        std::cout << "CurUpState state: false" << std::endl;
				return false;
			}
		default:
				::spdlog::debug("Error: Got full message: C");
				return false;
	}
}

}
