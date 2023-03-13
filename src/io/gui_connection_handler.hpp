#pragma once
#include <functional>
#include <memory>
#include "rmrf-net/tcp_server_socket.hpp"
#include "rmrf-net/tcp_client.hpp"
#include "rmrf-net/async_server.hpp"

#include "io/client_handler.hpp"


namespace dmxfish::io {

	class GUI_Connection_Handler
	{

		private:
			std::shared_ptr<rmrf::net::tcp_server_socket> external_control_server;
			std::list<std::shared_ptr<client_handler>> clients;
			client_handler::parse_message_cb_t message_cb;
		public:
			GUI_Connection_Handler(client_handler::parse_message_cb_t);
			~GUI_Connection_Handler();
			void activate_tcp_connection(uint16_t);
		private:
			void client_cb(rmrf::net::async_server_socket::self_ptr_type, std::shared_ptr<rmrf::net::connection_client>);
	};
}
