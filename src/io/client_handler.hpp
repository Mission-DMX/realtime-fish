#pragma once
#include <functional>
#include <memory>

#include "rmrf-net/ioqueue.hpp"
#include "io/message_buffer.hpp"
#include "rmrf-net/connection_client.hpp"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/message_lite.h"

namespace dmxfish::io {

class client_handler: public google::protobuf::io::ZeroCopyInputStream {
	public:
		typedef std::function<void(uint32_t, client_handler&)> parse_message_cb_t;
	private:
		enum internal_state_t{
			NEXT_MSG,
			GETLENGTH,
			READ_MSG
		};
		parse_message_cb_t parse_message_cb;
		std::shared_ptr<rmrf::net::ioqueue<::rmrf::net::iorecord>> io_buffer;
		std::shared_ptr<rmrf::net::connection_client> connection_client;
		internal_state_t internal_state;
		uint32_t msg_type;
		uint32_t msg_length;
		// std::deque<rmrf::net::iorecord>::iterator actual_record;
		int64_t byte_count;
		int limit_;
		int read_var_int_multiplier;
		std::shared_ptr<message_buffer_output> output_buffer;
		size_t streamsize;
		std::unique_ptr<rmrf::net::iorecord> actual_record;
        int last_limit;
	public:
		client_handler(parse_message_cb_t found_message_cb_, std::shared_ptr<rmrf::net::connection_client>);
		void handle_messages();
		bool Next(const void** data, int* size);
		void BackUp(int count);
		bool Skip(int count);
		int64_t ByteCount() const;
		void write_message(google::protobuf::MessageLite&, uint32_t);
	private:
		void incomming_data_callback(const rmrf::net::iorecord&);
		void BackUpLocal(int count);
		bool SkipLocal(int count);
		bool ReadVarint32(uint32_t *);
};

}
