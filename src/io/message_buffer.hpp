#include <functional>

#include "rmrf-net/ioqueue.hpp"
#include <istream>
// #include "absl/strings/cord.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/coded_stream.h"

namespace dmxfish::io{

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
