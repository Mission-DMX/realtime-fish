#pragma once

#include <memory>
#include <thread>

#include "lib/evpp.hpp"
#include "executioners/project_configuration.hpp"
#include "io/state_book.hpp"
#include "io/gui_connection_handler.hpp"
#include "io/client_handler.hpp"

#include "proto_src/FilterMode.pb.h"

namespace dmxfish::io {

	class IOManager : public std::enable_shared_from_this<IOManager> {

		private:
			bool running;
			std::shared_ptr<std::thread> iothread;
			std::shared_ptr<runtime_state_t> run_time_state;
			std::shared_ptr<::ev::loop_ref> loop;
			std::shared_ptr<GUI_Connection_Handler> gui_connections;
			std::shared_ptr<dmxfish::execution::project_configuration> active_show = nullptr;
			std::string latest_error;
			::missiondmx::fish::ipcmessages::ShowFileApplyState show_file_apply_state = ::missiondmx::fish::ipcmessages::SFAS_INVALID;
		public:
			IOManager(std::shared_ptr<runtime_state_t> run_time_state_, bool is_default_manager = false);
			~IOManager();
			void start();
			void push_msg_to_all_gui(google::protobuf::MessageLite&, uint32_t);

			[[nodiscard]] inline std::shared_ptr<dmxfish::execution::project_configuration> get_active_show() {
				return this->active_show;
			}

			[[nodiscard]] inline std::string get_latest_error() {
				return this->latest_error;
			}

			[[nodiscard]] inline ::missiondmx::fish::ipcmessages::ShowFileApplyState get_show_file_loading_state() {
				return this->show_file_apply_state;
			}
		private:
			void run();
			void parse_message_cb(uint32_t, client_handler&);
	};
}
