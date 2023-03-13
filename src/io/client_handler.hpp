#pragma once
#include <functional>
#include <memory>

#include "rmrf-net/ioqueue.hpp"
// #include "rmrf-net/tcp_client.hpp"
#include "rmrf-net/connection_client.hpp"
#include "google/protobuf/io/zero_copy_stream.h"

namespace dmxfish::io {

class client_handler: public google::protobuf::io::ZeroCopyInputStream {
	public:
		typedef std::function<void(uint32_t, google::protobuf::io::ZeroCopyInputStream&)> parse_message_cb_t;
	private:
		enum internal_state_t{
			NEXT_MSG,
			GETLENGTH,
			READ_MSG
		};
		parse_message_cb_t parse_message_cb;
		std::shared_ptr<::rmrf::net::ioqueue<::rmrf::net::iorecord>> io_buffer;
		std::shared_ptr<rmrf::net::connection_client> connection_client;
		internal_state_t internal_state;
		uint32_t msg_type;
		uint32_t msg_length;
		std::deque<rmrf::net::iorecord>::iterator actual_record;
		int byte_count;
		int64_t limit_;
		int read_var_int_multiplier;
	public:
		client_handler(parse_message_cb_t found_message_cb_, std::shared_ptr<rmrf::net::connection_client>);
		void handle_messages();
		bool Next(const void** data, int* size);
		void BackUp(int count);
		bool Skip(int count);
		int64_t ByteCount() const;
		std::shared_ptr<::rmrf::net::ioqueue<::rmrf::net::iorecord>> get_io_buffer(){return this->io_buffer;};
	private:
		void push_msg(const rmrf::net::iorecord&);
		void BackUpLocal(int count);
		bool SkipLocal(int count);
		bool ReadVarint32(uint32_t *);
		int streamsize() const;
};

}
