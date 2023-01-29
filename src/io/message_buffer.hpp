
#include <functional>
#include <memory>
#include <string>
#include <sstream>

#include "io/msg_type.hpp"

namespace dmxfish::io {

class message_buffer {
public:

	typedef std::function<void(msg_t msg_type, const std::string&, bool)> found_message_cb_t;
private:
	enum internal_state_t{
		NEXT_MSG,
		GETLENGTH,
		READ_MSG
	};
	found_message_cb_t found_message_cb;
	// std::string::size_type max;
	std::ostringstream data_buffer;
	// std::deque<uint8_t> data_buffer;
	size_t size_left;
	internal_state_t internal_state;
	dmxfish::io::msg_t msg_type;
public:

		// message_buffer(found_message_cb_t found_message_cb_, std::string::size_type max_line_size);
		message_buffer(found_message_cb_t found_message_cb_);
		void conn_data_in_cb(const std::string& data_in);
private:
	void clear();
	dmxfish::io::msg_t get_msg_type(const std::string& s);
};

}
