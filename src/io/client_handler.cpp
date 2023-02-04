#include "io/client_handler.hpp"

#include "lib/logging.hpp"
#include "google/protobuf/io/coded_stream.h"

// #include "google/protobuf/util/delimited_message_util.h"

namespace dmxfish::io {

  	// client_handler::client_handler(found_message_cb_t found_message_cb_):
  	// 	found_message_cb(found_message_cb_),
  	// 	io_buffer(std::make_shared<::rmrf::net::ioqueue<::rmrf::net::iorecord>>()),
  	// 	input_stream(std::make_shared<message_buffer_input>(this->io_buffer)),
  	// 	output_stream(std::make_shared<message_buffer_output>(this->io_buffer)),
  	// {
    //
  	// }

  	client_handler::client_handler(parse_message_cb_t parse_message_cb_, std::shared_ptr<rmrf::net::tcp_client> client):
  		parse_message_cb(parse_message_cb_),
  		io_buffer(std::make_shared<::rmrf::net::ioqueue<::rmrf::net::iorecord>>()),
  		input_stream(std::make_shared<message_buffer_input>(this->io_buffer)),
  		output_stream(std::make_shared<message_buffer_output>(this->io_buffer)),
      tcp_client(client),
      internal_state(NEXT_MSG)
  	{

  	}

    void client_handler::handle_messages(){
      switch (internal_state) {
        case NEXT_MSG:
            if(getIstream()->HandleReadResult(getIstream()->ReadVarint32(&msg_type))){
            // googles lengh read function, does not work
            // if(getIstream()->HandleReadResult(((google::protobuf::io::CodedInputStream*) getIstream())->ReadVarint32(&msg_type))){
              std::cout << "msg_type: " << msg_type << std::endl;
              this->internal_state = READ_MSG;
            }
            else {
              // ::spdlog::debug("NEXT_MSG: MsgWasNotLongEnough");
              return;
            }
            // ::spdlog::debug("NextMsg");
            return handle_messages();
        case GETLENGTH:
            ::spdlog::debug("GetLength");
            break;
        case READ_MSG:
          {
            if (parse_message_cb(msg_type, getIstream())){
              this->internal_state = NEXT_MSG;
            } else{
              // ::spdlog::debug("ReadMSG: MsgWasNotLongEnough");
              return;
            }
            // ::spdlog::debug("ReadMSG");
            return handle_messages();
            break;
          }
        default:
            ::spdlog::debug("Error: Unknown State");
            break;
      }
    }

}
