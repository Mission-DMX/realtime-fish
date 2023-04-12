#pragma once
#include <functional>
#include <memory>
#include <list>
#include "rmrf-net/unix_socket_server.hpp"
#include "rmrf-net/async_server.hpp"

#include "io/client_handler.hpp"


namespace dmxfish::io {
	class GUI_Connection_Handler
	{

		private:
			std::shared_ptr<rmrf::net::unix_socket_server> external_control_server;
			std::list<std::shared_ptr<client_handler>> clients;
			// std::list<client_handler> clients;
			client_handler::parse_message_cb_t message_cb;
		public:
			GUI_Connection_Handler(client_handler::parse_message_cb_t);
			~GUI_Connection_Handler();
			void activate_connection();
			void push_msg_to_all_gui(google::protobuf::MessageLite&, uint32_t);
		private:
			void client_cb(rmrf::net::async_server_socket::self_ptr_type, std::shared_ptr<rmrf::net::connection_client>);
	};
}
