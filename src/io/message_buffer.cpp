#include "io/message_buffer.hpp"

namespace dmxfish::io {

message_buffer::message_buffer(found_message_cb_t found_message_cb_,
		std::string::size_type max_line_size) :
		found_message_cb(found_message_cb_),
		max(max_line_size),
		data_buffer(std::ostringstream::ate),
		buffer_length{0},
    size_left{0},
    msg_type{""},
    internal_state{0} {
		};


void message_buffer::clear() {
	this->data_buffer.str(std::string());
	this->data_buffer.clear();
	this->buffer_length = 0;
}

void message_buffer::conn_data_in_cb(const std::string& data_in) {
  const auto length = data_in.length();
  if (length > 0){
    switch (internal_state) {
      case 0:
        msg_type = data_in.substr(0,1);
        internal_state = 1;
        conn_data_in_cb(data_in.substr(1,length));
        break;
      case 1:
        size_left = data_in.substr(0,1);
        internal_state = 2;
        conn_data_in_cb(data_in.substr(1,length));
        break;
      case 2:
        if (length >= size_left){
        	const std::string data_to_send = this->data_buffer.str() + data_in.substr(0, size_left);
          this->found_message_cb(msg_type, data_to_send, true);
          internal_state = 0;
          this->clear();
          conn_data_in_cb(data_in.substr(size_left, length));
        } else {
      		this->data_buffer << data_in.substr(0, length);
      		this->buffer_length += length;
      		this->size_left -= length;
        }
        break;
      default:
        break;
    }
  }
	// for(std::string::size_type strpos = 0, nextpos = -1; strpos != std::string::npos; strpos = nextpos++) {
	// 	// Search for the next line break
	// 	nextpos = this->search(data_in, strpos);
  //
	// 	// Advance, if the line would be empty
	// 	if(nextpos == strpos) {
	// 		nextpos = this->search(data_in, ++strpos);
	// 	}
  //
	// 	if (nextpos == std::string::npos) {
	// 		// If we didn't find one we store the remaining data in the buffer
	// 		const auto length = data_in.length() - strpos;
	// 		this->data_buffer << data_in.substr(strpos, length);
	// 		this->buffer_length += length;
	// 		break;
	// 	} else {
	// 		// If we find one we send the buffer plus the incomming data up to the line break to the callback
	// 		const std::string data_to_send = this->data_buffer.str() + data_in.substr(strpos, nextpos - strpos);
	// 		this->found_message_cb(data_to_send, true);
	// 		// and clear the buffer
	// 		this->clear();
	// 	}
	// }

	// if (this->buffer_length > this->max) {
	// 	this->found_message_cb(this->data_buffer.str(), false);
	// 	this->clear();
	// }
}

}
