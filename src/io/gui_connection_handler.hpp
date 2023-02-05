#pragma once
#include <functional>
#include <memory>
#include "rmrf-net/tcp_server_socket.hpp"
#include "rmrf-net/tcp_client.hpp"

#include "io/client_handler.hpp"


namespace dmxfish::io {

	class GUI_Connection_Handler
  {

		private:
			std::shared_ptr<rmrf::net::tcp_server_socket> external_control_server;
			std::vector<std::shared_ptr<client_handler>> clients;
			client_handler::parse_message_cb_t message_cb;
		public:
  		GUI_Connection_Handler(client_handler::parse_message_cb_t);
			~GUI_Connection_Handler();
      void activate_tcp_connection(int);
		private:
			void client_cb(std::shared_ptr<rmrf::net::tcp_client>);
	};
}
