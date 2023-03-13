#pragma once
#include <functional>
#include <memory>

#include "rmrf-net/tcp_server_socket.hpp"
#include "io/client_handler.hpp"


namespace dmxfish::test {

	class Test_Client_Handler
	{
	private:
		std::shared_ptr<rmrf::net::tcp_server_socket> external_control_server;
		std::list<std::shared_ptr<dmxfish::io::client_handler>> clients;
		public:
			Test_Client_Handler();
			~Test_Client_Handler();
      void parse_message_cb(uint32_t msg_type, google::protobuf::io::ZeroCopyInputStream& buff);
		private:
			void client_cb(rmrf::net::async_server_socket::self_ptr_type, std::shared_ptr<rmrf::net::connection_client>);
	};
}
