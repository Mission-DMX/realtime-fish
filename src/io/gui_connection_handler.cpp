#include "io/gui_connection_handler.hpp"
#include "lib/logging.hpp"

#include "rmrf-net/sock_address_factory.hpp"
namespace dmxfish::io {

GUI_Connection_Handler::GUI_Connection_Handler(client_handler::parse_message_cb_t message_cb_):
message_cb(message_cb_)
{

}


GUI_Connection_Handler::~GUI_Connection_Handler() {
	this->external_control_server = nullptr;
	::spdlog::debug("Stopped GUI_Connection_Handler");
}

void GUI_Connection_Handler::activate_tcp_connection(uint16_t port){
	// this->external_control_server = std::make_shared<rmrf::net::tcp_server_socket>(port, std::bind(&dmxfish::io::GUI_Connection_Handler::client_cb, this, std::placeholders::_1, std::placeholders::_2));
	auto socket_address = rmrf::net::get_first_general_socketaddr("::1", port);
	this->external_control_server = std::make_shared<rmrf::net::tcp_server_socket>(socket_address, std::bind(&dmxfish::io::GUI_Connection_Handler::client_cb, this, std::placeholders::_1, std::placeholders::_2));
	::spdlog::debug("Opened control port.");
}

void GUI_Connection_Handler::client_cb(rmrf::net::async_server_socket::self_ptr_type server, std::shared_ptr<rmrf::net::connection_client> client){
	MARK_UNUSED(server);
	::spdlog::debug("A client connected to the external control port. Address: {0}", client->get_peer_address().str());
	this->clients.push_back(std::make_shared<client_handler>(this->message_cb, client));
	::spdlog::debug("Client found the server");
}

}
