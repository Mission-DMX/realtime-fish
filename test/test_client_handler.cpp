#include "../test/test_client_handler.hpp"

#include "rmrf-net/sock_address_factory.hpp"

#include <iomanip>
#include <chrono>
#include <sstream>
#include <stdexcept>

#include "lib/logging.hpp"
#include "io/universe_sender.hpp"

#include "proto_src/MessageTypes.pb.h"
#include "proto_src/Console.pb.h"
#include "proto_src/DirectMode.pb.h"
#include "proto_src/FilterMode.pb.h"
#include "proto_src/RealTimeControl.pb.h"
#include "proto_src/UniverseControl.pb.h"
#include "google/protobuf/io/zero_copy_stream.h"

namespace dmxfish::test {

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

void Test_Client_Handler::run() {
	std::this_thread::sleep_for(std::chrono::milliseconds(400));
	::spdlog::debug("Entering ev defloop");
	this->loop->run(0);
	::spdlog::debug("Leaving ev defloop");
}

Test_Client_Handler::Test_Client_Handler() :
		running(true),
		iothread(nullptr),
		loop(nullptr),
		external_control_server(nullptr)

{
	if(!check_version_libev())
		throw std::runtime_error("Unable to initialize libev");
	this->loop = std::make_shared<::ev::default_loop>();
	this->iothread = std::make_shared<std::thread>(std::bind(&Test_Client_Handler::run, this));
	const auto thread_id = std::hash<std::thread::id>{}(this->iothread->get_id());
	::spdlog::debug("Test: Started Test_Client_Handler with loop on thread with id {}.", thread_id);
}

void Test_Client_Handler::start() {
	auto socket_address = rmrf::net::get_first_general_socketaddr("/tmp/9Lq7BNBnBycd6nxyz.socket", "", rmrf::net::socket_t::UNIX);
	this->external_control_server = std::make_shared<rmrf::net::unix_socket_server>(socket_address, std::bind(&dmxfish::test::Test_Client_Handler::client_cb, this, std::placeholders::_1, std::placeholders::_2));
	::spdlog::debug("Test: Opened control port.");
}

void Test_Client_Handler::client_cb(rmrf::net::async_server_socket::self_ptr_type server, std::shared_ptr<rmrf::net::connection_client> client){
	MARK_UNUSED(server);
	::spdlog::debug("Test: A client connected to the external control port. Address: {0}", client->get_peer_address().str());
	this->client_handler = std::make_shared<dmxfish::io::client_handler>(std::bind(&dmxfish::test::Test_Client_Handler::parse_message_cb, this, std::placeholders::_1, std::placeholders::_2), client);
	::spdlog::debug("Test: Client found the server");
}

Test_Client_Handler::~Test_Client_Handler() {
	::spdlog::debug("Test: Test_Client_Handler");
	this->running = false;
	this->loop->break_loop(::ev::ALL);
	this->iothread->join();

	::spdlog::debug("Test: Test_Client_Handler");
}

void Test_Client_Handler::parse_message_cb(uint32_t msg_type, google::protobuf::io::ZeroCopyInputStream& buff){
	::spdlog::debug("got message with type: {}", msg_type);
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
					auto universe = dmxfish::io::get_universe(msg->universe_id());
					if (universe){
						for (int i = 0; i< msg->channel_data_size(); i++){
							(*universe)[i] = msg->channel_data(i);
						}
						dmxfish::io::publish_universe_update(universe);
					}
					else {
						::spdlog::debug("did not find the universe with id: {}", msg->universe_id());
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
