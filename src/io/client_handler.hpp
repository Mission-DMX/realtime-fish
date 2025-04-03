#pragma once
#include <functional>
#include <memory>

#include "rmrf-net/ioqueue.hpp"
#include "io/client_input_buffer.hpp"
#include "io/client_output_buffer.hpp"
#include "rmrf-net/connection_client.hpp"
#include "google/protobuf/message_lite.h"

namespace dmxfish::io {

class client_handler{
	public:
		typedef std::function<void(uint32_t, client_handler&)> parse_message_cb_t;
	private:
		enum internal_state_t{
			NEXT_MSG,
			GETLENGTH,
			READ_MSG
		};
		parse_message_cb_t parse_message_cb;
		std::shared_ptr<rmrf::net::connection_client> connection_client;
		internal_state_t internal_state;
		uint32_t msg_type;
		uint32_t msg_length;
        std::shared_ptr<client_input_buffer> input_buffer;
	public:
		client_handler(parse_message_cb_t found_message_cb_, std::shared_ptr<rmrf::net::connection_client>);
		void handle_messages();
        bool is_client_alive();
        void write_message(const google::protobuf::MessageLite&, uint32_t);
        google::protobuf::io::ZeroCopyInputStream* get_zero_copy_input_stream();
    private:
        void incomming_data_callback(const rmrf::net::iorecord&);
        int get_length_of_varint(size_t);
};

}
