#pragma once

#include <memory>
#include <thread>

#include "lib/evpp.hpp"
#include "io/state_book.hpp"


#include "rmrf-net/tcp_client.hpp"
#include "io/message_buffer.hpp"

namespace dmxfish::io {

	class IOManager : public std::enable_shared_from_this<IOManager> {
		private:
			bool running;
			std::shared_ptr<std::thread> iothread;
			std::shared_ptr<runtime_state_t> run_time_state;
			std::shared_ptr<::ev::loop_ref> loop;
			std::shared_ptr<message_buffer> msg_buffer;
		public:
			IOManager(std::shared_ptr<runtime_state_t> run_time_state_, bool is_default_manager = false);
			~IOManager();
			void start();
			void writeData(std::string str);
		private:
			void run();
			void full_message_cb(std::string& str, const std::string& s, bool msg_full);
			void client_cb(std::shared_ptr<rmrf::net::tcp_client> client);
	};
}
