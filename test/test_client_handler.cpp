#include "../test/test_client_handler.hpp"
#include "lib/logging.hpp"


namespace dmxfish::test {

Test_Client_Handler::Test_Client_Handler()
{

}


Test_Client_Handler::~Test_Client_Handler() {
}

bool Test_Client_Handler::parse_message_cb(uint32_t msg_type, google::protobuf::io::ZeroCopyInputStream& buff){
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
	return true;
}

}
