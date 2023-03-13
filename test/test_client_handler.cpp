#include "../test/test_client_handler.hpp"
#include "rmrf-net/sock_address_factory.hpp"
#include "rmrf-net/client_factory.hpp"
#include "lib/logging.hpp"


namespace dmxfish::test {

Test_Client_Handler::Test_Client_Handler()
{

}


Test_Client_Handler::~Test_Client_Handler() {
	::spdlog::debug("Test: Entry of test client handler");
	auto socket_address = rmrf::net::get_first_general_socketaddr("::1", 8086);
	this->external_control_server = std::make_shared<rmrf::net::tcp_server_socket>(socket_address, std::bind(&dmxfish::test::Test_Client_Handler::client_cb, this, std::placeholders::_1, std::placeholders::_2));
	::spdlog::debug("Test: Opened control port.");
}

void Test_Client_Handler::parse_message_cb(uint32_t msg_type, google::protobuf::io::ZeroCopyInputStream& buff){
	std::vector<uint8_t> sum_data;
	int size = 0;
	uint8_t* data = nullptr;
	while(buff.Next((const void**) &data, &size)){
		while(size>0){
			sum_data.push_back(*data);
			data++;
			size--;
		}
	}
	buff.BackUp(0);
	::spdlog::debug("callback reached type: {}, length: {}", msg_type, sum_data.size());
	return;
}

void Test_Client_Handler::client_cb(rmrf::net::async_server_socket::self_ptr_type server_sock, std::shared_ptr<rmrf::net::connection_client> client){
	this->clients.push_back(std::make_shared<dmxfish::io::client_handler>(std::bind(&dmxfish::test::Test_Client_Handler::parse_message_cb, this, std::placeholders::_1, std::placeholders::_2), client));
	::spdlog::debug("Test: Client found the server");
}

}
