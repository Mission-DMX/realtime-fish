#include "io/gui_connection_handler.hpp"
#include "lib/logging.hpp"


// only for sending messages now
#include "proto_src/RealTimeControl.pb.h"
#include "proto_src/MessageTypes.pb.h"
#include "google/protobuf/util/delimited_message_util.h"

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
	::spdlog::debug("test client CB");
	this->clients.push_back(std::make_shared<client_handler>(message_cb, client));
	::spdlog::debug("Client found the server");

	auto curr_state_u = std::make_shared<missiondmx::fish::ipcmessages::current_state_update>();
	curr_state_u->set_current_state(missiondmx::fish::ipcmessages::RM_DIRECT);
	curr_state_u->set_current_scene(23);
	curr_state_u->set_last_cycle_time(25);
	curr_state_u->set_last_error("Error Wathever");

	auto curr_state_u3 = std::make_shared<missiondmx::fish::ipcmessages::current_state_update>();
	curr_state_u3->set_current_state(missiondmx::fish::ipcmessages::RM_STOP);
	curr_state_u3->set_current_scene(22);
	curr_state_u3->set_last_cycle_time(21);
	curr_state_u3->set_last_error("Error Wathever2");

	auto testBuffer = clients.front();
	testBuffer->getOstream().WriteVarint32(::missiondmx::fish::ipcmessages::MSGT_CURRENT_STATE_UPDATE);
		// -> google Variante fuer Write Int, funktioniert nicht
		// ((google::protobuf::io::CodedOutputStream*) testBuffer->getOstream())->WriteVarint32(::missiondmx::fish::ipcmessages::MSGT_CURRENT_STATE_UPDATE);

	uint32_t length = curr_state_u->ByteSizeLong();
	testBuffer->getOstream().WriteVarint32(length);
	std::cout << "GC: finishedSerialize: " << curr_state_u->current_state() << " finished: " << curr_state_u->SerializeToZeroCopyStream(&testBuffer->getOstream()) << std::endl;


	// std::cout << "GC: finishedSerialize: " << curr_state_u->current_state() << " finished: " << google::protobuf::util::SerializeDelimitedToZeroCopyStream(*(curr_state_u.get()), testBuffer->getOstream()) << std::endl;


	// testBuffer->handle_messages();

	testBuffer->getOstream().WriteVarint32(::missiondmx::fish::ipcmessages::MSGT_CURRENT_STATE_UPDATE);
	std::cout << "GC: finishedSerialize: " << curr_state_u3->current_state() << " finished: " << google::protobuf::util::SerializeDelimitedToZeroCopyStream(*(curr_state_u3.get()), &testBuffer->getOstream()) << std::endl;
	testBuffer->handle_messages();

}

}
