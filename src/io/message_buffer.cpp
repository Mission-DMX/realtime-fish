#include "io/message_buffer.hpp"

namespace dmxfish::io {

message_buffer::message_buffer(found_message_cb_t found_message_cb_
		//, std::string::size_type max_line_size
	) :
		found_message_cb(found_message_cb_),
		// max(max_line_size),
		data_buffer(std::ostringstream::ate),
		// buffer_length{0},
    size_left(0),
    // msg_type(nullptr),
    msg_type{dmxfish::io::msg_t::NOTHING},
    internal_state(NEXT_MSG) {
		};


void message_buffer::clear() {
	this->internal_state = NEXT_MSG;
	this->msg_type = dmxfish::io::msg_t::NOTHING;
	this->size_left = 0;

	this->data_buffer.str(std::string());
	this->data_buffer.clear();
	// this->buffer_length = 0;
}

dmxfish::io::msg_t message_buffer::get_msg_type(const std::string& s){
	return dmxfish::io::msg_t::UPDATE_STATE;
}


void message_buffer::conn_data_in_cb(const std::string& data_in) {
  const auto length = data_in.length();
  if (length > 0){
    switch (internal_state) {
      case 0:
        msg_type = get_msg_type(data_in.substr(0,1));
        internal_state = GETLENGTH;
        return conn_data_in_cb(data_in.substr(1,length));
        break;
      case 1:
			{
				const char* ch = new char(data_in.at(0));
        size_left = atoi(ch);
        internal_state = READ_MSG;
        return conn_data_in_cb(data_in.substr(1,length));
        break;
			}
      case 2:
        if (length >= size_left){
        	const std::string data_to_send = this->data_buffer.str() + data_in.substr(0, size_left);
          this->found_message_cb(msg_type, data_to_send, true);
					size_t a = size_left;
          this->clear();
          return conn_data_in_cb(data_in.substr(a, length));
        } else {
      		this->data_buffer << data_in.substr(0, length);
      		// this->buffer_length += length;
      		this->size_left -= length;
        }
        break;
      default:
        break;
    }
  }

	// if (this->buffer_length > this->max) {
	// 	this->found_message_cb(this->data_buffer.str(), false);
	// 	this->clear();
	// }
}

}
