#include "io/gui_connection_handler.hpp"
#include "lib/logging.hpp"

namespace dmxfish::io {

GUI_Connection_Handler::GUI_Connection_Handler(client_handler::parse_message_cb_t message_cb_):
message_cb(message_cb_)
{

}


GUI_Connection_Handler::~GUI_Connection_Handler() {
	this->external_control_server = nullptr;
	::spdlog::debug("Stopped GUI_Connection_Handler");
}

void GUI_Connection_Handler::activate_tcp_connection(int port){
	this->external_control_server = std::make_shared<rmrf::net::tcp_server_socket>(port, std::bind(&dmxfish::io::GUI_Connection_Handler::client_cb, this, std::placeholders::_1));
	::spdlog::debug("Opened control port.");
}

void GUI_Connection_Handler::client_cb(std::shared_ptr<rmrf::net::tcp_client> client){
	this->clients.push_back(std::make_shared<client_handler>(message_cb, client));
	::spdlog::debug("Client found the server");
}

}
