
#include <functional>
#include <memory>
#include <string>
#include <sstream>

// #include "io/message_client.hpp"

namespace dmxfish::io {

enum msg_t {
	ABC,
	DEF
};

class message_buffer {
public:

	typedef std::function<void(std::string, const std::string&, bool)> found_message_cb_t;
private:
	found_message_cb_t found_message_cb;
	std::string::size_type max;
	std::ostringstream data_buffer;
	size_t buffer_length;
	size_t size_left;
	std::string msg_type;
	int internal_state;
public:

	message_buffer(found_message_cb_t found_message_cb_, std::string::size_type max_line_size);
private:
	void conn_data_in_cb(const std::string& data_in);
	void clear();
};

}
