#include "io/iomanager.hpp"

#include <iomanip>
#include <chrono>
#include <sstream>
#include <stdexcept>

#include "lib/logging.hpp"

#include "proto_src/MessageTypes.pb.h"
#include "proto_src/Console.pb.h"
#include "proto_src/DirectMode.pb.h"
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

#include "io/universe_sender.hpp"

namespace dmxfish::io {


static void get_protobuf_msg_of_universe(missiondmx::fish::ipcmessages::Universe* universe_to_edit, std::shared_ptr<dmxfish::dmx::universe> universe_to_read){
	// auto msg_universe = std::make_shared<missiondmx::fish::ipcmessages::Universe>();
	universe_to_edit->set_id(universe_to_read->getID());

	switch (universe_to_read->getUniverseType()) {
		case dmxfish::dmx::universe_type::PHYSICAL:
			// not finished
			universe_to_edit->set_physical_location(1);
			break;
		case dmxfish::dmx::universe_type::ARTNET:
			{
				auto universe_inner = universe_to_edit->mutable_remote_location();
				universe_inner->set_ip_address("Dummy data - we cant get real one right now");
				universe_inner->set_port(6454);
				universe_inner->set_universe_on_device(1);
				break;
			}
		case dmxfish::dmx::universe_type::sACN:
			// not finished, not supported in Protobuf
			break;
		default:
			break;
	}
	// return universe_to_edit;
}

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
		show_loading_thread(nullptr),
		run_time_state(run_time_state_),
		loop(nullptr),
		gui_connections(std::make_shared<GUI_Connection_Handler>(std::bind(&dmxfish::io::IOManager::parse_message_cb, this, std::placeholders::_1, std::placeholders::_2))),
		latest_error{"No Error occured"}

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
	this->gui_connections->activate_connection();
}

IOManager::~IOManager() {
	this->running = false;
	this->loop->break_loop(::ev::ALL);
	this->iothread->join();

	::spdlog::debug("Stopped IO manager");
}

void IOManager::parse_message_cb(uint32_t msg_type, client_handler& client){
	::spdlog::debug("Msg came in with type : {}", msg_type);
	switch ((::missiondmx::fish::ipcmessages::MsgType) msg_type) {
		case ::missiondmx::fish::ipcmessages::MSGT_UPDATE_STATE:
			// change the running mode
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::update_state>();
				if (msg->ParseFromZeroCopyStream(&client)){
					switch (msg->new_state()) {
						case ::missiondmx::fish::ipcmessages::RM_FILTER:
							{
								// this->run_time_state->running = true;
								this->run_time_state->is_direct_mode = false;
								break;
							}
						case ::missiondmx::fish::ipcmessages::RM_DIRECT:
							{
								// this->run_time_state->running = true;
								this->run_time_state->is_direct_mode = true;
								break;
							}
						case ::missiondmx::fish::ipcmessages::RM_STOP:
							{
								this->run_time_state->running = false;
								break;
							}
						}
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_CURRENT_STATE_UPDATE:
			// load showfile and scene with that Message ?
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::current_state_update>();
				if (msg->ParseFromZeroCopyStream(&client)){
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_UNIVERSE:
			// register a new universe
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::Universe>();
				if (msg->ParseFromZeroCopyStream(&client)){
					dmxfish::io::register_universe_from_message(*msg.get());
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_UNIVERSE_LIST:
			// register a list of universes
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::universes_list>();
				if (msg->ParseFromZeroCopyStream(&client)){
					for(int i = 0 ; i < msg->list_of_universes_size(); i++){
						dmxfish::io::register_universe_from_message(msg->list_of_universes(i));
					}
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_REQUEST_UNIVERSE_LIST:
//			 send dmx data to UI (of one or multiple universes)
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::request_universe_list>();
				if (msg->ParseFromZeroCopyStream(&client)){
					if(msg->universe_id()<0){
						auto universes = dmxfish::io::get_universe_list();
						auto msg_universes = std::make_shared<missiondmx::fish::ipcmessages::universes_list>();
						for (std::weak_ptr<dmxfish::dmx::universe> universe : universes){
							if (universe.use_count()>0){
								auto universe_to_write_to = msg_universes->add_list_of_universes();
								dmxfish::io::get_protobuf_msg_of_universe(universe_to_write_to, universe.lock());
							}
						}
						client.write_message(*msg_universes.get(), ::missiondmx::fish::ipcmessages::MSGT_UNIVERSE_LIST);

					}else{
						auto universe = dmxfish::io::get_universe(msg->universe_id());
						if(universe){
							auto universe_to_edit = missiondmx::fish::ipcmessages::Universe();
							dmxfish::io::get_protobuf_msg_of_universe(&universe_to_edit, universe);
							client.write_message(universe_to_edit, ::missiondmx::fish::ipcmessages::MSGT_UNIVERSE);
						}
						else {
							::spdlog::debug("did not find the universe with id: {}", msg->universe_id());
						}
					}
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_DELETE_UNIVERSE:
			// deletes the universe
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::delete_universe>();
				if (msg->ParseFromZeroCopyStream(&client)){
					dmxfish::io::unregister_universe(msg->id());
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_BUTTON_STATE_CHANGE:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::button_state_change>();
				if (msg->ParseFromZeroCopyStream(&client)){
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_FADER_POSITION:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::fader_position>();
				if (msg->ParseFromZeroCopyStream(&client)){
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_ROTARY_ENCODER_CHANGE:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::rotary_encoder_change>();
				if (msg->ParseFromZeroCopyStream(&client)){
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_DMX_OUTPUT:
			// if running mode is direct, update given universe
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::dmx_output>();
				if (msg->ParseFromZeroCopyStream(&client)){
					if (this->run_time_state->is_direct_mode){
						auto universe = dmxfish::io::get_universe(msg->universe_id());
						if(universe){
							for (int i = 0; i< msg->channel_data_size() && i < 512; i++){
								(*universe)[i] = msg->channel_data(i);
							}
						}
						else {
							::spdlog::debug("did not find the universe with id: {}", msg->universe_id());
						}
					}
					return;
				}
				::spdlog::debug("could not parse msg of type: dmx_output");
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_REQUEST_DMX_DATA:
			// answer the request for the dmxdata
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::request_dmx_data>();
				if (msg->ParseFromZeroCopyStream(&client)){
					auto universe = dmxfish::io::get_universe(msg->universe_id());
					// send universe back to UI
					if(universe){
						auto msg_dmx_data = std::make_shared<missiondmx::fish::ipcmessages::dmx_output>();
						msg_dmx_data->set_universe_id(msg->universe_id());
						msg_dmx_data->add_channel_data(1);
						for (int j = 0; j<512; j++){
							msg_dmx_data->add_channel_data((*universe)[j]);
						}
						client.write_message(*(msg_dmx_data.get()), ::missiondmx::fish::ipcmessages::MSGT_DMX_OUTPUT);
					}
					else {
						::spdlog::debug("did not find the universe with id: {}", msg->universe_id());
					}
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_ENTER_SCENE:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::enter_scene>();
				if (msg->ParseFromZeroCopyStream(&client)){
					return;
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_LOAD_SHOW_FILE:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::load_show_file>();
				if (msg->ParseFromZeroCopyStream(&client)){
					using namespace missiondmx::fish::ipcmessages;
					this->show_file_apply_state = SFAS_SHOW_LOADING;
					this->show_loading_thread = std::make_shared<std::thread>(std::bind(&IOManager::load_show_file, this, msg));
				}
				return;
			}
		case ::missiondmx::fish::ipcmessages::MSGT_UPDATE_PARAMETER:
			{
				auto msg = std::make_shared<missiondmx::fish::ipcmessages::update_parameter>();
				if (msg->ParseFromZeroCopyStream(&client)){
					return;
				}
				return;
			}
		default:
				::spdlog::debug("Error: Got full message: C");
				return;
	}
}

void IOManager::push_msg_to_all_gui(google::protobuf::MessageLite& msg, uint32_t msg_type){
	this->gui_connections->push_msg_to_all_gui(msg, msg_type);
}

void IOManager::load_show_file(std::shared_ptr<missiondmx::fish::ipcmessages::load_show_file> msg) {
	using namespace missiondmx::fish::ipcmessages;
	// TODO load project configuration with msg->show_data() in new thread and dispatch update code below
	// TODO switch to new project configuration
	this->show_file_apply_state = SFAS_SHOW_UPDATING;
	if(msg->goto_default_scene()) {
		this->active_show->set_active_scene(this->active_show->get_default_scene());
	}
	this->show_file_apply_state = SFAS_SHOW_ACTIVE;

}
}
