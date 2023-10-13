#include "io/iomanager.hpp"
#include "dmx/universe.hpp"
#include <cmath>

#include <iomanip>
#include <chrono>
#include <string>
#include <sstream>
#include <stdexcept>

#include "lib/logging.hpp"

#include "lib/macros.hpp"
#include "net/sock_address_factory.hpp"
#include <netdb.h>


COMPILER_SUPRESS("-Wuseless-cast")
#include "proto_src/MessageTypes.pb.h"
#include "proto_src/Console.pb.h"
#include "proto_src/DirectMode.pb.h"
#include "proto_src/FilterMode.pb.h"
#include "proto_src/RealTimeControl.pb.h"
#include "proto_src/UniverseControl.pb.h"
COMPILER_RESTORE("-Wuseless-cast")
#include "google/protobuf/io/zero_copy_stream.h"

#include "io/universe_sender.hpp"
#include "dmx/ftdi_universe.hpp"
#include "xml/show_files.hpp"

namespace dmxfish::io {


static void get_protobuf_msg_of_universe(missiondmx::fish::ipcmessages::Universe* universe_to_edit, std::shared_ptr<dmxfish::dmx::universe> universe_to_read){
    universe_to_edit->set_id(universe_to_read->getID());

    switch (universe_to_read->getUniverseType()) {
        case dmxfish::dmx::universe_type::PHYSICAL:
            // not finished
            universe_to_edit->set_physical_location(1);
            break;
        case dmxfish::dmx::universe_type::ARTNET:{
            auto universe_inner = universe_to_edit->mutable_remote_location();
            universe_inner->set_ip_address("Dummy data - we cant get real one right now");
            universe_inner->set_port(6454);
            universe_inner->set_universe_on_device(1);
            break;
        }
        case dmxfish::dmx::universe_type::sACN:
            // not finished, not supported in Protobuf
            break;
        case dmxfish::dmx::universe_type::FTDI: {
            auto universe_inner = universe_to_edit->mutable_ftdi_dongle();
	    auto ftdi_u = static_cast<dmxfish::dmx::ftdi_universe*>(universe_to_read.get());
            universe_inner->set_product_id(ftdi_u->get_product_id());
            universe_inner->set_vendor_id(ftdi_u->get_vendor_id());
            universe_inner->set_device_name("Hopefully Enttec USB DMX Pro");
            universe_inner->set_serial("Dummy Serial");
            break;
        }
        default:
            break;
    }
}

static void send_log_message_to_client(const std::string& str, client_handler& client){
    auto log_message = missiondmx::fish::ipcmessages::long_log_update();
    log_message.set_level(missiondmx::fish::ipcmessages::LL_WARNING);
    log_message.set_time_stamp(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    log_message.set_what(str);
    client.write_message(log_message, ::missiondmx::fish::ipcmessages::MSGT_LOG_MESSAGE);
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
	while(this->running) {
		try {
			this->loop->run(0);
		} catch (const std::exception& e) {
			::spdlog::error("Event loop crashed with exception: {}. Restarting event loop.", e.what());
		}
	}
	::spdlog::debug("Leaving ev defloop");
}

IOManager::IOManager(std::shared_ptr<runtime_state_t> run_time_state_, bool is_default_manager) :
		running(true),
		iothread(nullptr),
		show_loading_thread(nullptr),
		run_time_state(run_time_state_),
		loop(nullptr),
        loop_interrupter(),
		gui_connections(std::make_shared<GUI_Connection_Handler>(std::bind(&dmxfish::io::IOManager::parse_message_cb, this, std::placeholders::_1, std::placeholders::_2))),
		latest_error{"No Error occured"}
{
	this->loop_interrupter = std::make_unique<::ev::async>();
	this->loop_interrupter->set<IOManager, &IOManager::cb_interrupt_async>(this);
	if (is_default_manager) {
		if(!check_version_libev())
			throw std::runtime_error("Unable to initialize libev");
		this->loop = std::make_unique<::ev::default_loop>();
	} else {
		this->loop = std::make_unique<::ev::dynamic_loop>();
	}
	this->iothread = std::make_shared<std::thread>(std::bind(&IOManager::run, this));
	const auto thread_id = std::hash<std::thread::id>{}(this->iothread->get_id());
	::spdlog::debug("Started IO manager with loop on thread with id {}.", thread_id);
	this->loop_interrupter->start();
}

void IOManager::start() {
	this->gui_connections->activate_connection();
}

IOManager::~IOManager() {
	::spdlog::debug("Stopping IO manager");
	this->running = false;
	this->loop_interrupter->send();
	this->iothread->join();
	this->loop_interrupter->stop();
	::spdlog::debug("Stopped IO manager");
}

void IOManager::cb_interrupt_async(::ev::async& w, int events) {
	MARK_UNUSED(w);
	MARK_UNUSED(events);
	::spdlog::debug("Loop interrupt triggered.");
	this->loop->break_loop(::ev::ALL);
}

void IOManager::parse_message_cb(uint32_t msg_type, client_handler& client){
//	::spdlog::debug("Msg came in with type : {}", msg_type);
    auto buffer = client.get_zero_copy_input_stream();
    std::string error_message = "";
	switch ((::missiondmx::fish::ipcmessages::MsgType) msg_type) {
		case ::missiondmx::fish::ipcmessages::MSGT_UPDATE_STATE:
        // change the running mode
        {
            auto msg = missiondmx::fish::ipcmessages::update_state();
            if (msg.ParseFromZeroCopyStream(buffer)){
                switch (msg.new_state()) {
                    case ::missiondmx::fish::ipcmessages::RM_FILTER:
                    {
                        // this->run_time_state->running = true;
                        this->run_time_state->is_direct_mode = false;
			spdlog::info("Enabled filter execution.");
                        break;
                    }
                    case ::missiondmx::fish::ipcmessages::RM_DIRECT:
                    {
                        // this->run_time_state->running = true;
                        this->run_time_state->is_direct_mode = true;
			spdlog::info("Disabled filter execution.");
                        break;
                    }
                    case ::missiondmx::fish::ipcmessages::RM_STOP:
                    {
                        this->run_time_state->running = false;
                        break;
                    }
                    case ::missiondmx::fish::ipcmessages::RunMode_INT_MIN_SENTINEL_DO_NOT_USE_:
                    {
                        const auto error_msg = "IOManager Parse Message: MSGT_UPDATE_STATE: Used RunMode_INT_MIN_SENTINEL_DO_NOT_USE_ which should not be used";
                        this->latest_error = error_msg;
                        ::spdlog::warn(error_msg);
                        break;
                    }
                    case ::missiondmx::fish::ipcmessages::RunMode_INT_MAX_SENTINEL_DO_NOT_USE_:
                    {
                        const auto error_msg = "IOManager Parse Message: MSGT_UPDATE_STATE: Used RunMode_INT_MAX_SENTINEL_DO_NOT_USE_ which should not be used";
                        this->latest_error = error_msg;
                        ::spdlog::warn(error_msg);
                        break;
                    }
                    default:
                    {
                        const auto error_msg = "IOManager Parse Message: MSGT_UPDATE_STATE: Used Another unknown Run Mode Message";
                        this->latest_error = error_msg;
                        ::spdlog::warn(error_msg);
                        break;
                    }
                    }
                return;
            }
            error_message += "Could not parse the message of type: MSGT_UPDATE_STATE.";
            this->latest_error = error_message;
            ::spdlog::warn(error_message);
            return;
        }
		case ::missiondmx::fish::ipcmessages::MSGT_CURRENT_STATE_UPDATE:
        // Todo: What to do with that message? load showfile and scene with that Message ?
        {
            auto msg = missiondmx::fish::ipcmessages::current_state_update();
            if (msg.ParseFromZeroCopyStream(buffer)){
                return;
            }
            error_message += "Could not parse the message of type: MSGT_CURRENT_STATE_UPDATE.";
            this->latest_error = error_message;
            ::spdlog::warn(error_message);
            return;
        }
		case ::missiondmx::fish::ipcmessages::MSGT_UNIVERSE:
        // register a new universe
        {
            auto msg = missiondmx::fish::ipcmessages::Universe();
            if (msg.ParseFromZeroCopyStream(buffer)){
                try {
                    dmxfish::io::register_universe_from_message(msg);
                } catch (const dmx::ftdi_exception& e) {
                    const auto error_msg = "Could not create the usb-dmx universe with id " + std::to_string(msg.id()) + ". Reason: " + e.what();
                    this->latest_error = error_msg;
                    ::spdlog::warn(error_msg);
                    send_log_message_to_client(error_msg, client);
                } catch (const std::exception& e) {
                    const auto error_msg = "Could not create universe: with id " + std::to_string(msg.id()) + ". Reason: " + e.what();
                    this->latest_error = error_msg;
                    ::spdlog::warn(error_msg);
                    send_log_message_to_client(error_msg, client);
                }
                return;
            }
            error_message += "Could not parse the message of type: MSGT_UNIVERSE.";
            this->latest_error = error_message;
            ::spdlog::warn(error_message);
            return;
        }
		case ::missiondmx::fish::ipcmessages::MSGT_UNIVERSE_LIST:
        // register a list of universes
        {
            auto msg = missiondmx::fish::ipcmessages::universes_list();
            if (msg.ParseFromZeroCopyStream(buffer)){
                for(int i = 0 ; i < msg.list_of_universes_size(); i++){
                    auto universe_inner = msg.list_of_universes(i);
                    try {
                        dmxfish::io::register_universe_from_message(universe_inner);
                    } catch (const dmx::ftdi_exception& e) {
                        const auto error_msg = "Could not create the usb-dmx universe with id " + std::to_string(universe_inner.id()) + ". Reason: " + e.what();
                        this->latest_error = error_msg;
                        ::spdlog::warn(error_msg);
                        send_log_message_to_client(error_msg, client);
                    } catch (const std::exception& e) {
                        const auto error_msg = "Could not create universe: with id " + std::to_string(universe_inner.id()) + ". Reason: " + e.what();
                        this->latest_error = error_msg;
                        ::spdlog::warn(error_msg);
                        send_log_message_to_client(error_msg, client);
                    }
                }
                return;
            }
            error_message += "Could not parse the message of type: MSGT_UNIVERSE_LIST.";
            this->latest_error = error_message;
            ::spdlog::warn(error_message);
            return;
        }
		case ::missiondmx::fish::ipcmessages::MSGT_REQUEST_UNIVERSE_LIST:
        // send dmx data to UI (of one or multiple universes)
        {
            auto msg = missiondmx::fish::ipcmessages::request_universe_list();
            if (msg.ParseFromZeroCopyStream(buffer)){
                if(msg.universe_id()<0){
                    auto universes = dmxfish::io::get_universe_list();
                    auto msg_universes = missiondmx::fish::ipcmessages::universes_list();
                    for (std::weak_ptr<dmxfish::dmx::universe> universe : universes){
                        if (universe.use_count()>0){
                            auto universe_to_write_to = msg_universes.add_list_of_universes();
                            dmxfish::io::get_protobuf_msg_of_universe(universe_to_write_to, universe.lock());
                        }
                    }
                    client.write_message(msg_universes, ::missiondmx::fish::ipcmessages::MSGT_UNIVERSE_LIST);

                }else{
                    auto universe = dmxfish::io::get_universe(msg.universe_id());
                    if(universe){
                        auto universe_msg = missiondmx::fish::ipcmessages::Universe();
                        dmxfish::io::get_protobuf_msg_of_universe(&universe_msg, universe);
                        client.write_message(universe_msg, ::missiondmx::fish::ipcmessages::MSGT_UNIVERSE);
                    }
                    else {
                        ::spdlog::warn("did not find the universe with id: {}", msg.universe_id());
                    }
                }
                return;
            }
            error_message += "Could not parse the message of type: MSGT_REQUEST_UNIVERSE_LIST.";
            this->latest_error = error_message;
            ::spdlog::warn(error_message);
            return;
        }
		case ::missiondmx::fish::ipcmessages::MSGT_DELETE_UNIVERSE:
        // deletes the universe
        {
            auto msg = missiondmx::fish::ipcmessages::delete_universe();
            if (msg.ParseFromZeroCopyStream(buffer)){
                dmxfish::io::unregister_universe(msg.id());
                return;
            }
            error_message += "Could not parse the message of type: MSGT_DELETE_UNIVERSE.";
            this->latest_error = error_message;
            ::spdlog::warn(error_message);
            return;
        }
		case ::missiondmx::fish::ipcmessages::MSGT_BUTTON_STATE_CHANGE:
        {
            auto msg = missiondmx::fish::ipcmessages::button_state_change();
            if (msg.ParseFromZeroCopyStream(buffer)){
                if(control_desk_handle) {
                    try {
                        control_desk_handle->update_button_leds_from_protobuf(msg);
                    } catch(const std::exception& e) {
                        this->latest_error = e.what();
                    }
                }
                return;
            }
            error_message += "Could not parse the message of type: MSGT_BUTTON_STATE_CHANGE.";
            this->latest_error = error_message;
            ::spdlog::warn(error_message);
            return;
        }
		case ::missiondmx::fish::ipcmessages::MSGT_FADER_POSITION:
        {
            auto msg = missiondmx::fish::ipcmessages::fader_position();
            if (msg.ParseFromZeroCopyStream(buffer)){
                if(control_desk_handle) {
                    try {
                        control_desk_handle->update_fader_position_from_protobuf(msg);
                    } catch(const std::exception& e) {
                        this->latest_error = e.what();
                    }
                }
                return;
            }
            error_message += "Could not parse the message of type: MSGT_FADER_POSITION.";
            this->latest_error = error_message;
            ::spdlog::warn(error_message);
            return;
        }
		case ::missiondmx::fish::ipcmessages::MSGT_ROTARY_ENCODER_CHANGE:
        {
            auto msg = missiondmx::fish::ipcmessages::rotary_encoder_change();
            if (msg.ParseFromZeroCopyStream(buffer)){
                if(control_desk_handle) {
                    try {
                        control_desk_handle->update_encoder_state_from_protobuf(msg);
                    } catch(const std::exception& e) {
                        this->latest_error = e.what();
                    }
                }
                return;
            }
            error_message += "Could not parse the message of type: MSGT_ROTARY_ENCODER_CHANGE.";
            this->latest_error = error_message;
            ::spdlog::warn(error_message);
            return;
        }
		case ::missiondmx::fish::ipcmessages::MSGT_DMX_OUTPUT:
        // if running mode is direct, update given universe
        {
            auto msg = missiondmx::fish::ipcmessages::dmx_output();
            if (msg.ParseFromZeroCopyStream(buffer)){
                if (this->run_time_state->is_direct_mode){
                    auto universe = dmxfish::io::get_universe(msg.universe_id());
                    if(universe){
                        for (int i = 0; i< msg.channel_data_size() && i < DMX_UNIVERSE_SIZE; i++){
                            (*universe)[i] = (uint8_t) std::min(msg.channel_data(i), (int32_t) std::numeric_limits<uint8_t>::max());
                        }
                    }
                    else {
                        ::spdlog::info("did not find the universe with id: {}", msg.universe_id());
                    }
                }
                return;
            }
            error_message += "Could not parse the message of type: MSGT_DMX_OUTPUT.";
            this->latest_error = error_message;
            ::spdlog::warn(error_message);
            return;
        }
        case ::missiondmx::fish::ipcmessages::MSGT_REQUEST_DMX_DATA:
        // answer the request for the dmxdata
        {
            auto msg = missiondmx::fish::ipcmessages::request_dmx_data();
            if (msg.ParseFromZeroCopyStream(buffer)){
                auto universe = dmxfish::io::get_universe(msg.universe_id());
                // send universe back to UI
                if(universe){
                    auto msg_dmx_data = missiondmx::fish::ipcmessages::dmx_output();
                    msg_dmx_data.set_universe_id(msg.universe_id());
                    msg_dmx_data.add_channel_data(1);
                    for (int j = 0; j<DMX_UNIVERSE_SIZE; j++){
                        msg_dmx_data.add_channel_data((*universe)[j]);
                    }
                    client.write_message(msg_dmx_data, ::missiondmx::fish::ipcmessages::MSGT_DMX_OUTPUT);
                }
                else {
                    const auto error_msg = "did not find the universe with id: " + std::to_string(msg.universe_id());
                    this->latest_error = error_msg;
                    ::spdlog::info(error_msg);
                }
                return;
            }
            error_message += "Could not parse the message of type: MSGT_REQUEST_DMX_DATA.";
            this->latest_error = error_message;
            ::spdlog::warn(error_message);
            return;
        }
		case ::missiondmx::fish::ipcmessages::MSGT_ENTER_SCENE:
        {
            auto msg = missiondmx::fish::ipcmessages::enter_scene();
            if (msg.ParseFromZeroCopyStream(buffer)){
                if(this->active_show == nullptr) {
                    this->latest_error = "Request for scene switch couldn't be executed as there is currently no loaded scene.";
                    return;
                } else {
                    const auto sid = msg.scene_id();
                    if(!this->active_show->set_active_scene(sid)) {
                        this->latest_error = "The requested scene id (" + std::to_string(sid) + ") was not found.";
                    }
                }
                return;
            }
            error_message += "Could not parse the message of type: MSGT_ENTER_SCENE.";
            this->latest_error = error_message;
            ::spdlog::warn(error_message);
            return;
        }
		case ::missiondmx::fish::ipcmessages::MSGT_LOAD_SHOW_FILE:
        {
            auto msg = std::make_shared<missiondmx::fish::ipcmessages::load_show_file>();
            if (msg->ParseFromZeroCopyStream(buffer)){
                using namespace missiondmx::fish::ipcmessages;
                this->show_file_apply_state = this->active_show == nullptr ? SFAS_SHOW_LOADING : SFAS_SHOW_UPDATING;
                if(this->show_loading_thread) {
                    this->show_loading_thread->join();
                }
                this->show_loading_thread = std::make_shared<std::thread>(std::bind(&IOManager::load_show_file, this, msg));
                return;
            }
            error_message += "Could not parse the message of type: MSGT_LOAD_SHOW_FILE.";
            this->latest_error = error_message;
            ::spdlog::warn(error_message);
            return;
        }
		case ::missiondmx::fish::ipcmessages::MSGT_UPDATE_PARAMETER:
        {
            auto msg = missiondmx::fish::ipcmessages::update_parameter();
            if (msg.ParseFromZeroCopyStream(buffer)){
                try {
                    if(this->active_show == nullptr) {
                        this->latest_error = "Requested to update filter parameter, but no show is loaded.";
                    } else {
                        const auto scene = msg.scene_id();
                        const auto fid = msg.filter_id();
                        const auto k = msg.parameter_key();
                        const auto v = msg.parameter_value();
                        if(!this->active_show->update_filter_parameter(scene, fid, k, v)) {
                            this->latest_error = "The requested filter (" + fid + " in scene " + std::to_string(scene) + ") reported that it failed to update the parameter " + k + " to " + v + ".";
                            ::spdlog::info(this->latest_error);
                        }
                    }
                } catch (const std::invalid_argument& e) {
                    this->latest_error = e.what();
                    ::spdlog::warn(this->latest_error);
                }
                return;
            }
            error_message += "Could not parse the message of type: MSGT_UPDATE_PARAMETER.";
            this->latest_error = error_message;
            ::spdlog::warn(error_message);
            return;
        }
		case ::missiondmx::fish::ipcmessages::MSGT_LOG_MESSAGE:
        {
            auto msg = missiondmx::fish::ipcmessages::long_log_update();
            if (msg.ParseFromZeroCopyStream(buffer)){
                // TODO the GUI shouldn't send us log messages. What should we do with it?
                return;
            }
            error_message += "Could not parse the message of type: MSGT_LOG_MESSAGE.";
            this->latest_error = error_message;
            ::spdlog::warn(error_message);
            return;
        }
        case ::missiondmx::fish::ipcmessages::MSGT_NOTHING:
        {
            error_message += "IOManager Parse Message: Used MSGT_NOTHING as Msg Type. ";
            break;
        }
        case ::missiondmx::fish::ipcmessages::MsgType_INT_MIN_SENTINEL_DO_NOT_USE_:
        {
            error_message += "IOManager Parse Message: Used MsgType_INT_MIN_SENTINEL_DO_NOT_USE_ as Msg Type. ";
            break;
        }
        case ::missiondmx::fish::ipcmessages::MsgType_INT_MAX_SENTINEL_DO_NOT_USE_:
        {
            error_message += "IOManager Parse Message: Used MsgType_INT_MAX_SENTINEL_DO_NOT_USE_ as Msg Type. ";
            break;
        }
        case ::missiondmx::fish::ipcmessages::MSGT_REMOVE_FADER_BANK_SET:
            try {
                missiondmx::fish::ipcmessages::remove_fader_bank_set msg;
                if(!msg.ParseFromZeroCopyStream(buffer)) {
                    error_message += "Failed to decode MSGT_REMOVE_FADER_BANK_SET message.";
                }
                if(control_desk_handle) {
                    control_desk_handle->remove_bank_set(msg.bank_id());
                } else {
                    error_message += "No control desk handle has been currently set.";
                }
                return;
            } catch (const std::exception& e) {
                this->latest_error = e.what();
            }
            break;
        case ::missiondmx::fish::ipcmessages::MSGT_ADD_FADER_BANK_SET:
            try {
                missiondmx::fish::ipcmessages::add_fader_bank_set msg;
                if(!msg.ParseFromZeroCopyStream(buffer)) {
                    error_message += "Failed to decode MSGT_ADD_FADER_BANK_SET message.";
                }
                if(control_desk_handle) {
                    control_desk_handle->add_bank_set_from_protobuf_msg(msg);

                } else {
                    error_message += "No control desk handle has been currently set.";
                }
                return;
            } catch (const std::exception& e) {
                this->latest_error = e.what();
            }
            break;
        case ::missiondmx::fish::ipcmessages::MSGT_DESK_UPDATE:
            try {
                missiondmx::fish::ipcmessages::desk_update msg;
                if(!msg.ParseFromZeroCopyStream(buffer)) {
                    error_message += "Failed to decode MSGT_DESK_UPDATE message.";
                }
                if(control_desk_handle) {
                    control_desk_handle->process_desk_update_message(msg);
                } else {
                    error_message += "No control desk handle has been currently set.";
                }
                return;
            } catch (const std::exception& e) {
                this->latest_error = e.what();
            }
            break;
        case ::missiondmx::fish::ipcmessages::MSGT_UPDATE_COLUMN:
            try {
                missiondmx::fish::ipcmessages::fader_column msg;
                if(!msg.ParseFromZeroCopyStream(buffer)) {
                    error_message += "Failed to decode MSGT_UPDATE_COLUMN message.";
                }
                if(control_desk_handle) {
                    control_desk_handle->update_column_from_message(msg);
                } else {
                    error_message += "No control desk handle has been currently set.";
                }
                return;
            } catch (const std::exception& e) {
                this->latest_error = e.what();
            }
            break;
		default:
        {
            error_message += "IOManager Parse Message: Used a unknown Msg Type. ";
            break;
        }
        int count = 0;
        int size = 0;
        uint8_t* data;
        while(buffer->Next((const void**) &data, &size)){
            count += size;
        }
        error_message += "Not wanted Message had type " + std::to_string(msg_type) + " and size " + std::to_string(count);
        this->latest_error = error_message;
        ::spdlog::warn(error_message);
	}
}

void IOManager::push_msg_to_all_gui(google::protobuf::MessageLite& msg, uint32_t msg_type){
	this->gui_connections->push_msg_to_all_gui(msg, msg_type);
}

void IOManager::load_show_file(std::shared_ptr<missiondmx::fish::ipcmessages::load_show_file> msg) {
	using namespace missiondmx::fish::ipcmessages;
	std::stringstream loading_result_stream;
	bool success = false;
	try {
		std::istringstream istr(msg->show_data());
		auto candidate = MissionDMX::ShowFile::bord_configuration(istr, xml_schema::flags::dont_validate);
		loading_result_stream << "Show XML successfully parsed." << std::endl;
		auto show_candidate = std::make_shared<dmxfish::execution::project_configuration>(std::move(candidate), loading_result_stream);
		if(!msg->goto_default_scene()) {
			if(this->active_show) {
				const auto current_scene = this->active_show->get_active_scene();
				loading_result_stream << "Switching to last active scene " << current_scene << "." << std::endl;
				show_candidate->set_active_scene(current_scene);
			} else {
				loading_result_stream << "Failed to load last active scene as there was no previous show." << std::endl;
			}
		}
		this->last_active_show = this->active_show;
		this->active_show = show_candidate;
		loading_result_stream << "Switched to new show." << std::endl;
		success = true;
	} catch (const xml_schema::exception& e) {
		loading_result_stream << "\nAn error occurred while parsing the show file XML:" << std::endl;
		loading_result_stream << e << std::endl;
	} catch (const dmxfish::execution::project_config_exception& e) {
		loading_result_stream << "\nAn error occurred while scheduling the filters:" << std::endl;
		loading_result_stream << e.what() << std::endl;
	}
	if(!success) {
		if (this->active_show == nullptr) {
			this->latest_error = "An error occurred while loading the show configuration. No show file is currently loaded. Please review the detailed log message.";
			this->show_file_apply_state = SFAS_NO_SHOW_ERROR;
			::spdlog::error("Failed to load show file: No show currently running. Check GUI for log.");
		} else {
			this->latest_error = "An error occurred while loading the new show configuration. The last successfully loaded configuration is still active. Please review the detailed log message.";
			this->show_file_apply_state = SFAS_SHOW_ACTIVE;
			::spdlog::error(this->latest_error);
		}
	} else {
		this->show_file_apply_state = SFAS_SHOW_ACTIVE;
		::spdlog::info("Successfully loaded show file");
		this->latest_error = "Showfile Applied.";
		if(this->control_desk_handle) {
			this->control_desk_handle->notify_showfile_changed();
		}
	}
	long_log_update log_msg;
	log_msg.set_level(success ? LL_INFO : LL_ERROR);
	log_msg.set_time_stamp(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	log_msg.set_what(loading_result_stream.str());
	this->push_msg_to_all_gui(log_msg, MSGT_LOG_MESSAGE);
	std::cout << loading_result_stream.str() << std::endl;
}

void IOManager::handle_queued_io() {
	// TODO run io loop iteration or wait until 2 have passed.
	std::this_thread::sleep_for(std::chrono::milliseconds(100)); // TODO fixme
}

void IOManager::rollback() {       
        if(this->is_rollback_available()) {
        	this->active_show = this->last_active_show;
                this->last_active_show = nullptr;
                if(this->control_desk_handle) {
                        this->control_desk_handle->notify_showfile_changed();
                }
                ::spdlog::info("Rolled back to last valid show.");
        } else {
                ::spdlog::warn("Didn't roll back as there is no valid show avaiable.");
        }
}


}
