#pragma once
#include <functional>
#include <memory>

#include "io/client_handler.hpp"


namespace dmxfish::test {

	class Test_Client_Handler
	{
		public:
			Test_Client_Handler();
			~Test_Client_Handler();
      void parse_message_cb(uint32_t msg_type, google::protobuf::io::ZeroCopyInputStream& buff);
	};
}
