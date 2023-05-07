#include "main.hpp"

#include <chrono>
#include <memory>
#include <thread>

#include "lib/logging.hpp"
#include "lib/macros.hpp"

#include "global_vars.hpp"

#include "rmrf-net/client_factory.hpp"
#include "rmrf-net/ioqueue.hpp"

#include "io/universe_sender.hpp"

#include "rmrf-net/client_factory.hpp"
#include "rmrf-net/ioqueue.hpp"


#include "rmrf-net/tcp_client.hpp"
#include "stdin_watcher.hpp"

#include "lib/macros.hpp"
COMPILER_SUPRESS("-Wuseless-cast")
#include "proto_src/RealTimeControl.pb.h"
#include "proto_src/MessageTypes.pb.h"
COMPILER_RESTORE("-Wuseless-cast")

#include "control_desk/desk.hpp"
#include "control_desk/device_enumeration.hpp"

static std::shared_ptr<dmxfish::io::IOManager> manager = nullptr;

void push_updates_to_ui(std::shared_ptr<runtime_state_t> t, unsigned long c_time) {
	auto msg = std::make_shared<missiondmx::fish::ipcmessages::current_state_update>();
	msg->set_current_state(t->running?(t->is_direct_mode?(::missiondmx::fish::ipcmessages::RM_DIRECT):(::missiondmx::fish::ipcmessages::RM_FILTER)):(::missiondmx::fish::ipcmessages::RM_STOP));
	msg->set_showfile_apply_state(manager->get_show_file_loading_state());
	if (t->is_direct_mode) {
		msg->set_current_scene(-1);
	} else if(auto s = manager->get_active_show(); s != nullptr) {
		msg->set_current_scene(s->get_active_scene());
	} else {
		msg->set_current_scene(-2);
	}
	msg->set_last_cycle_time((int32_t) c_time);
	msg->set_last_error(manager->get_latest_error());
	manager->push_msg_to_all_gui(*msg.get(), ::missiondmx::fish::ipcmessages::MSGT_CURRENT_STATE_UPDATE);
}


void perform_main_update(std::shared_ptr<runtime_state_t> t, std::unique_ptr<dmxfish::control_desk::desk> control_desk) {
	namespace stdc = std::chrono;
	manager->set_control_desk_handle(std::move(control_desk));
	while (t->running) {
		const auto start_time = stdc::system_clock::now().time_since_epoch();
		manager->update_control_desk();
		if (t->is_direct_mode) {
			// TODO fetch and apply updates from FPGA, also send values to GUI
		} else { // Direct mode
			// TODO apply data from input structure on show.
			if(auto sptr = manager->get_active_show(); sptr != nullptr) {
				try {
					sptr->run_cycle_update();
				} catch (const std::exception& e) {
					manager->set_latest_error(e.what());
					manager->mark_show_file_execution_error();
				}
			}
		}

		dmxfish::io::push_all_registered_universes();


		// stop timer and wait 2ms until next cycle
		const auto end_time = stdc::system_clock::now().time_since_epoch();

		const auto cycle_time = (end_time - start_time);
		const auto cycle_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(cycle_time).count();
		push_updates_to_ui(t, cycle_time_ms);

		if(cycle_time_ms < 18)
			std::this_thread::sleep_for(stdc::milliseconds(18) - cycle_time);
	}
	manager->set_control_desk_handle(nullptr);
}

int main(int argc, char* argv[], char* env[]) {

	GOOGLE_PROTOBUF_VERIFY_VERSION;
	MARK_UNUSED(argc);
	MARK_UNUSED(argv);
	MARK_UNUSED(env);

	spdlog::set_level(spdlog::level::debug);
    reset_start_time();
	auto run_time_state = std::make_shared<runtime_state_t>();

	stdin_watcher sin_w([run_time_state](){
		if(run_time_state->running) {
			run_time_state->running = false;
			::spdlog::info("Stopping server from keyboard now.");
		}
	});
	// dmxfish::io::IOManager manager(run_time_state, true);
	manager = std::make_shared<dmxfish::io::IOManager>(run_time_state, true);

	manager->start();

	auto control_desk = std::make_unique<dmxfish::control_desk::desk>(dmxfish::control_desk::enumerate_control_devices());

	::spdlog::info("Fish started. Press ENTER to close the server.");

	perform_main_update(run_time_state, std::move(control_desk));
	manager = nullptr;

	::spdlog::debug("Main End");
}

std::shared_ptr<dmxfish::io::IOManager> get_iomanager_instance() {
	return manager;
}
