#pragma once

#include <memory>
#include <thread>

#include "lib/evpp.hpp"
#include "io/state_book.hpp"
#include "io/gui_connection_handler.hpp"
#include "io/client_handler.hpp"

namespace dmxfish::test {

	class Test_IOManager : public std::enable_shared_from_this<Test_IOManager> {

		private:
			bool running;
			std::shared_ptr<std::thread> iothread;
			std::shared_ptr<::ev::loop_ref> loop;
			std::shared_ptr<rmrf::net::tcp_server_socket> external_control_server;
			std::shared_ptr<dmxfish::io::client_handler> client_handler;
		public:
			Test_IOManager();
			~Test_IOManager();
			void start();
		private:
			void run();
			void client_cb(rmrf::net::async_server_socket::self_ptr_type server_sock, std::shared_ptr<rmrf::net::connection_client> client);
			void parse_message_cb(uint32_t, google::protobuf::io::ZeroCopyInputStream&);
	};
}
