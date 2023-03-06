#pragma once

#include <memory>
#include <thread>

#include "lib/evpp.hpp"
#include "io/state_book.hpp"
#include "io/gui_connection_handler.hpp"
#include "io/client_handler.hpp"

namespace dmxfish::test {

	class IOManager : public std::enable_shared_from_this<IOManager> {

		private:
			bool running;
			std::shared_ptr<std::thread> iothread;
			std::shared_ptr<::ev::loop_ref> loop;
			std::shared_ptr<dmxfish::io::GUI_Connection_Handler> gui_connections;
		public:
			IOManager(dmxfish::io::client_handler::parse_message_cb_t);
			~IOManager();
			void start();
		private:
			void run();
	};
}
