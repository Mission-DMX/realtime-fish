#include <functional>

#include "rmrf-net/ioqueue.hpp"

// #include "absl/strings/cord.h"
#include "google/protobuf/io/zero_copy_stream.h"

namespace dmxfish::io{

class message_buffer_input : public google::protobuf::io::ZeroCopyInputStream{
private:
	std::shared_ptr<::rmrf::net::ioqueue<::rmrf::net::iorecord>> io_buffer;
	int nr_of_read_msg;
	int localoffset;
	int localoffset_last;
	int64_t byte_count;
	int64_t byte_count_temp;
public:
	message_buffer_input(std::shared_ptr<::rmrf::net::ioqueue<::rmrf::net::iorecord>> io_buffer_);
	bool Next(const void** data, int* size);
	void BackUp(int count);
	bool Skip(int count);
	int64_t ByteCount() const;
	bool HandleReadResult(bool res);
	bool ReadVarint32(uint32_t *);
private:
	inline void Restore();
	inline void FinishRead();
	int64_t sizetemp() const;
	int64_t sizestream() const;
};

// This class should be deletet, memory leakage!!!
class message_buffer_output : public google::protobuf::io::ZeroCopyOutputStream{
private:
	std::shared_ptr<::rmrf::net::ioqueue<::rmrf::net::iorecord>> io_buffer;
	uint32_t* new_buff;
	int64_t byte_count;
public:
	message_buffer_output(std::shared_ptr<::rmrf::net::ioqueue<::rmrf::net::iorecord>> io_buffer_);
	void BackUp(int count);
	int64_t ByteCount() const;
	bool Next(void** data, int* size);
	void WriteVarint32(uint32_t);
};

}
