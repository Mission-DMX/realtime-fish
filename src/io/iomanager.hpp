#pragma once

#include <memory>
#include <thread>

#include "lib/evpp.hpp"
#include "io/state_book.hpp"
#include "io/gui_connection_handler.hpp"

namespace dmxfish::io {

	class IOManager : public std::enable_shared_from_this<IOManager> {

		private:
			bool running;
			std::shared_ptr<std::thread> iothread;
			std::shared_ptr<runtime_state_t> run_time_state;
			std::shared_ptr<::ev::loop_ref> loop;
			std::shared_ptr<GUI_Connection_Handler> gui_connections;
		public:
			IOManager(std::shared_ptr<runtime_state_t> run_time_state_, bool is_default_manager = false);
			~IOManager();
			void start();
		private:
			void run();
			bool parse_message_cb(uint32_t msg_type, message_buffer_input* buff);
	};
}
