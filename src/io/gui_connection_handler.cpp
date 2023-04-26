#include "io/gui_connection_handler.hpp"
#include "lib/logging.hpp"

#include "rmrf-net/sock_address_factory.hpp"
namespace dmxfish::io {

GUI_Connection_Handler::GUI_Connection_Handler(client_handler::parse_message_cb_t message_cb_):
    external_control_server(nullptr),
    clients(),
    message_cb(message_cb_)
{

}


GUI_Connection_Handler::~GUI_Connection_Handler() {
	this->external_control_server = nullptr;
	::spdlog::debug("Stopped GUI_Connection_Handler");
}

void GUI_Connection_Handler::activate_connection(){
	// auto socket_address = rmrf::net::get_first_general_socketaddr("/var/run/fish.sock", "", rmrf::net::socket_t::UNIX);
	auto socket_address = rmrf::net::get_first_general_socketaddr("/tmp/fish.sock", "", rmrf::net::socket_t::UNIX);
	this->external_control_server = std::make_shared<rmrf::net::unix_socket_server>(socket_address, std::bind(&dmxfish::io::GUI_Connection_Handler::client_cb, this, std::placeholders::_1, std::placeholders::_2));
	::spdlog::debug("Opened control port.");
}

void GUI_Connection_Handler::client_cb(rmrf::net::async_server_socket::self_ptr_type server, std::shared_ptr<rmrf::net::connection_client> client){
	MARK_UNUSED(server);
	::spdlog::debug("A client connected to the external control port. Address: {0}", client->get_peer_address().str());
	this->clients.push_back(std::make_shared<client_handler>(this->message_cb, client));
	::spdlog::debug("Client found the server");
}

void GUI_Connection_Handler::push_msg_to_all_gui(google::protobuf::MessageLite& msg, uint32_t msg_type){
    std::shared_ptr<client_handler> to_delete = nullptr;
	for (auto& c : this->clients){
        if(c->is_client_alive()) {
            c->write_message(msg, msg_type);
        }else{
            to_delete = c;
        }
	}

    if(to_delete){
        clients.remove(to_delete);
        ::spdlog::info("GUI Connection Handler: client was removed: now {} clients are connected", this->clients.size());
    }
}

}
