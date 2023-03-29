#pragma once

#include <memory>
#include <thread>

#include "lib/evpp.hpp"
#include "io/state_book.hpp"
#include "io/gui_connection_handler.hpp"
#include "io/client_handler.hpp"
#include "rmrf-net/unix_socket_server.hpp"
#include "dmx/universe.hpp"

namespace dmxfish::test {

	class Test_Client_Handler : public std::enable_shared_from_this<Test_Client_Handler> {

		private:
			bool running;
			std::shared_ptr<std::thread> iothread;
			std::shared_ptr<::ev::loop_ref> loop;
			std::shared_ptr<rmrf::net::unix_socket_server> external_control_server;
			std::shared_ptr<dmxfish::io::client_handler> client_handler;
		public:
			Test_Client_Handler();
			~Test_Client_Handler();
			void start();
		private:
			void run();
			void client_cb(rmrf::net::async_server_socket::self_ptr_type server_sock, std::shared_ptr<rmrf::net::connection_client> client);
			void parse_message_cb(uint32_t, google::protobuf::io::ZeroCopyInputStream&);
	};
}
