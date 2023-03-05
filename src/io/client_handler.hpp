#include <functional>
#include <memory>

#include "rmrf-net/ioqueue.hpp"
#include "io/message_buffer.hpp"
#include "rmrf-net/tcp_client.hpp"

namespace dmxfish::io {

class client_handler: public google::protobuf::io::ZeroCopyInputStream {
	public:
		typedef std::function<bool(uint32_t, client_handler&)> parse_message_cb_t;
	private:
		enum internal_state_t{
			NEXT_MSG,
			GETLENGTH,
			READ_MSG
		};
		parse_message_cb_t parse_message_cb;
		std::shared_ptr<::rmrf::net::ioqueue<::rmrf::net::iorecord>> io_buffer;
		// std::shared_ptr<message_buffer_input> input_stream;
		std::shared_ptr<message_buffer_output> output_stream;
		std::shared_ptr<rmrf::net::tcp_client> tcp_client;
		internal_state_t internal_state;
		uint32_t msg_type;
		uint32_t msg_length;
		int pls_size;
		int nr_of_read_msg;
		std::deque<rmrf::net::iorecord>::iterator actual_record;
		int localoffset;
		int localoffset_last;
		int byte_count;
		int byte_count_temp;
		int64_t limit_;
	public:
		client_handler(parse_message_cb_t found_message_cb_, std::shared_ptr<rmrf::net::tcp_client>);
		message_buffer_output& getOstream(){return *output_stream.get();}
		// message_buffer_input& getIstream(){return *input_stream.get();}
		void handle_messages();
		bool Next(const void** data, int* size);
		void BackUp(int count);
		bool Skip(int count);
		int64_t ByteCount() const;
		bool HandleReadResult(bool res);
	private:
		void BackUpLocal(int count);
		bool SkipLocal(int count);
		int64_t ByteCountLocal() const;
		bool ReadVarint32(uint32_t *);
		int streamsize() const;
		inline void Restore();
		inline void FinishRead();
		int sizetemp() const;
};

}
