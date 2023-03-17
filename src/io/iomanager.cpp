#include "io/iomanager.hpp"

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

#include "lib/macros.hpp"
#include "net/sock_address_factory.hpp"
#include <netdb.h>


#include "proto_src/MessageTypes.pb.h"
#include "proto_src/Console.pb.h"
#include "proto_src/DirectMode.pb.h"
#include "proto_src/FilterMode.pb.h"
#include "proto_src/RealTimeControl.pb.h"
#include "proto_src/UniverseControl.pb.h"
#include "google/protobuf/io/zero_copy_stream.h"


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

void IOManager::parse_message_cb(uint32_t msg_type, google::protobuf::io::ZeroCopyInputStream& buff){
	switch ((::missiondmx::fish::ipcmessages::MsgType) msg_type) {
		case ::missiondmx::fish::ipcmessages::MSGT_UPDATE_STATE:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::update_state>();
				if (msg->ParseFromZeroCopyStream(&buff)){
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_CURRENT_STATE_UPDATE:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::current_state_update>();
				if (msg->ParseFromZeroCopyStream(&buff)){
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_UNIVERSE:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::Universe>();
				if (msg->ParseFromZeroCopyStream(&buff)){
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_UNIVERSE_LIST:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::universes_list>();
				if (msg->ParseFromZeroCopyStream(&buff)){
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_REQUEST_UNIVERSE_LIST:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::request_universe_list>();
				if (msg->ParseFromZeroCopyStream(&buff)){
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_DELETE_UNIVERSE:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::delete_universe>();
				if (msg->ParseFromZeroCopyStream(&buff)){
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_BUTTON_STATE_CHANGE:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::button_state_change>();
				if (msg->ParseFromZeroCopyStream(&buff)){
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_FADER_POSITION:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::fader_position>();
				if (msg->ParseFromZeroCopyStream(&buff)){
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_ROTARY_ENCODER_CHANGE:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::rotary_encoder_change>();
				if (msg->ParseFromZeroCopyStream(&buff)){
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_DMX_OUTPUT:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::dmx_output>();
				if (msg->ParseFromZeroCopyStream(&buff)){
					if (this->run_time_state->is_direct_mode){
						//send data to universe
					}
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_REQUEST_DMX_DATA:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::request_dmx_data>();
				if (msg->ParseFromZeroCopyStream(&buff)){
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_ENTER_SCENE:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::enter_scene>();
				if (msg->ParseFromZeroCopyStream(&buff)){
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_LOAD_SHOW_FILE:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::load_show_file>();
				if (msg->ParseFromZeroCopyStream(&buff)){
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_UPDATE_PARAMETER:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::update_parameter>();
				if (msg->ParseFromZeroCopyStream(&buff)){
					return;
				}
				return;
			}
		default:
				::spdlog::debug("Error: Got full message: C");
				return;
	}
}
}
