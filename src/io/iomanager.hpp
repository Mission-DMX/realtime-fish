#pragma once

#include <memory>
#include <thread>

#include "lib/evpp.hpp"
#include "executioners/project_configuration.hpp"
#include "io/state_book.hpp"
#include "io/gui_connection_handler.hpp"
#include "io/client_handler.hpp"
#include "control_desk/desk.hpp"

#include "lib/macros.hpp"
COMPILER_SUPRESS("-Wuseless-cast")
#include "proto_src/FilterMode.pb.h"
COMPILER_RESTORE("-Wuseless-cast")

namespace dmxfish::io {

	class IOManager : public std::enable_shared_from_this<IOManager> {

		private:
			bool running;
			std::shared_ptr<std::thread> iothread, show_loading_thread;
			std::shared_ptr<runtime_state_t> run_time_state;
			std::unique_ptr<::ev::loop_ref> loop;
			std::unique_ptr<::ev::async> loop_interrupter;
			std::shared_ptr<GUI_Connection_Handler> gui_connections;
			std::shared_ptr<dmxfish::execution::project_configuration> active_show = nullptr, last_active_show = nullptr;
			std::unique_ptr<dmxfish::control_desk::desk> control_desk_handle = nullptr;
			std::string latest_error;
			::missiondmx::fish::ipcmessages::ShowFileApplyState show_file_apply_state = ::missiondmx::fish::ipcmessages::SFAS_INVALID;
		public:
			IOManager(std::shared_ptr<runtime_state_t> run_time_state_, bool is_default_manager = false);
			~IOManager();
			void start();
			void push_msg_to_all_gui(google::protobuf::MessageLite& msg, uint32_t msg_type);

			[[nodiscard]] inline std::shared_ptr<dmxfish::execution::project_configuration> get_active_show() {
				return this->active_show;
			}

			[[nodiscard]] inline std::string get_latest_error() const {
				return this->latest_error;
			}

			[[nodiscard]] inline ::missiondmx::fish::ipcmessages::ShowFileApplyState get_show_file_loading_state() const {
				return this->show_file_apply_state;
			}

			inline void set_latest_error(std::string error) {
				this->latest_error = error;
			}

			inline void mark_show_file_execution_error() {
				this->show_file_apply_state = ::missiondmx::fish::ipcmessages::SFAS_ERROR_SHOW_RUNNING;
			}

			inline void set_control_desk_handle(std::unique_ptr<dmxfish::control_desk::desk> handle) {
				this->control_desk_handle = std::move(handle);
			}

			inline void update_control_desk() {
				if(control_desk_handle) {
					control_desk_handle->update();
				}
			}

			inline std::shared_ptr<dmxfish::control_desk::bank_column> access_desk_column(const std::string& set_id, const std::string& column_id) {
				if (!control_desk_handle) {
					return nullptr;
				}
				return control_desk_handle->find_column(set_id, column_id);
			}

			inline uint16_t get_global_illumination() {
				if(control_desk_handle) {
					return control_desk_handle->get_global_illumination();
				} else {
					return 0;
				}
			}

			void handle_queued_io();

			[[nodiscard]] bool is_rollback_available() {
				return this->last_active_show != nullptr;
			}

			void rollback();
		private:
			void run();
			void load_show_file(std::shared_ptr<missiondmx::fish::ipcmessages::load_show_file> msg);
			void parse_message_cb(uint32_t, client_handler&);
			void cb_interrupt_async(::ev::async& w, int events);
	};
}
