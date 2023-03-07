#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE RMRF_TESTS
#include <boost/test/included/unit_test.hpp>

#include "lib/logging.hpp"
// #include "io/client_handler.hpp"
#include "../test/iomanager_t.hpp"
#include "../test/test_client_handler.hpp"
#include "../test/message_buffer.hpp"

#include "proto_src/RealTimeControl.pb.h"
#include "proto_src/MessageTypes.pb.h"
#include "google/protobuf/util/delimited_message_util.h"

using namespace dmxfish::io;

bool parse_message_cb(uint32_t msg_type, google::protobuf::io::ZeroCopyInputStream& buff){
	::spdlog::debug("Nothing here");
	return true;
	// std::vector<uint8_t> sum_data;
	// int size = 0;
	// uint8_t* data = nullptr;
	// while(buff.Next((const void**) &data, &size)){
	// 	while(size>0){
	// 		sum_data.push_back(*data);
	// 		data++;
	// 		size--;
	// 	}
	// }
	// buff.BackUp(0);
	// ::spdlog::debug("callback reached type: {}, length: {}", msg_type, sum_data.size());

	// switch ((::missiondmx::fish::ipcmessages::MsgType) msg_type) {
	// 	case ::missiondmx::fish::ipcmessages::MSGT_UPDATE_STATE:
	// 		{
	// 			auto msg = std::make_shared<missiondmx::fish::ipcmessages::update_state>();
	// 			bool cleanEOF;
	// 			if (msg->ParseFromZeroCopyStream(&buff)){
	// 					std::cout << "Update State state: " << msg->new_state() << std::endl;
	// 					return true;
	// 			}
	// 			return false;
	// 		}
	// 	case ::missiondmx::fish::ipcmessages::MSGT_CURRENT_STATE_UPDATE:
	// 		{
	// 			auto msg = std::make_shared<missiondmx::fish::ipcmessages::current_state_update>();
	// 			bool cleanEOF;
	// 			if (msg->ParseFromZeroCopyStream(&buff)){
	// 					std::cout << "CurUpState state: " << msg->current_state() << std::endl;
	// 					return true;
	// 			}
	//
	// 			std::cout << "CurUpState state: false" << std::endl;
	// 			return false;
	// 		}
	// 	default:
	// 			::spdlog::debug("Error: Got full message: C");
	// 			return false;
	// }
	// return true;
}


BOOST_AUTO_TEST_CASE(helloworld) {


	spdlog::set_level(spdlog::level::debug);
	auto io_manager_test = std::make_shared<dmxfish::test::IOManager>(std::bind(&parse_message_cb, std::placeholders::_1, std::placeholders::_2));
	io_manager_test->start();
	::spdlog::debug("test1");
	auto client = std::make_shared<rmrf::net::tcp_client>("::1", 8085);
	::spdlog::debug("test2");
	::spdlog::debug("test2.5");
	time_t start_time = time(NULL);
	while (time(NULL) < start_time+2) {

	}
	auto test_cl_h = std::make_shared<dmxfish::test::Test_Client_Handler>();
	auto client_h = std::make_shared<dmxfish::io::client_handler>(std::bind(&dmxfish::test::Test_Client_Handler::parse_message_cb, test_cl_h.get() , std::placeholders::_1, std::placeholders::_2), client);
	auto mess_buff = std::make_shared<dmxfish::test::message_buffer_output>(client_h->get_io_buffer());

	uint32_t test_num = 0;
	mess_buff->WriteVarint32(test_num);
	mess_buff->WriteVarint32(1);
	mess_buff->WriteVarint32(1);
	client_h->handle_messages();

	mess_buff->WriteVarint32(test_num);
	client_h->handle_messages();
	mess_buff->WriteVarint32(1);
	client_h->handle_messages();
	mess_buff->WriteVarint32(1);
	client_h->handle_messages();


	// auto curr_state_u = std::make_shared<missiondmx::fish::ipcmessages::current_state_update>();
	// curr_state_u->set_current_state(missiondmx::fish::ipcmessages::RM_DIRECT);
	// curr_state_u->set_current_scene(23);
	// curr_state_u->set_last_cycle_time(25);
	// curr_state_u->set_last_error("Error Wathever");
	//
	// auto curr_state_u3 = std::make_shared<missiondmx::fish::ipcmessages::current_state_update>();
	// curr_state_u3->set_current_state(missiondmx::fish::ipcmessages::RM_STOP);
	// curr_state_u3->set_current_scene(22);
	// curr_state_u3->set_last_cycle_time(21);
	// curr_state_u3->set_last_error("Error Wathever2");
	//
	// mess_buff->WriteVarint32(::missiondmx::fish::ipcmessages::MSGT_CURRENT_STATE_UPDATE);
	// 	// -> google Variante fuer Write Int, funktioniert nicht
	// 	// ((google::protobuf::io::CodedOutputStream*) testBuffer->getOstream())->WriteVarint32(::missiondmx::fish::ipcmessages::MSGT_CURRENT_STATE_UPDATE);
	//
	// uint32_t length = curr_state_u->ByteSizeLong();
	// mess_buff->WriteVarint32(length);
	// std::cout << "GC: finishedSerialize: " << curr_state_u->current_state() << " finished: " << curr_state_u->SerializeToZeroCopyStream(mess_buff.get()) << std::endl;
	// client_h->handle_messages();
	//
	// mess_buff->WriteVarint32(::missiondmx::fish::ipcmessages::MSGT_CURRENT_STATE_UPDATE);
	// std::cout << "GC: finishedSerialize: " << curr_state_u3->current_state() << " finished: " << google::protobuf::util::SerializeDelimitedToZeroCopyStream(*(curr_state_u3.get()), mess_buff.get()) << std::endl;


	// client_h->handle_messages();
	::spdlog::debug("test3");
	BOOST_CHECK_EQUAL("abc", "abc");
}
