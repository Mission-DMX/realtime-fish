#pragma once
#include <functional>

#include "rmrf-net/ioqueue.hpp"
#include "rmrf-net/connection_client.hpp"
#include <istream>
// #include "absl/strings/cord.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/coded_stream.h"

namespace dmxfish::io{

class message_buffer_output : public google::protobuf::io::ZeroCopyOutputStream{
private:
	int64_t byte_count;
	std::shared_ptr<rmrf::net::connection_client> conn_client;
	uint8_t* data_buff;
	int64_t pre_used;
public:
	message_buffer_output(std::shared_ptr<rmrf::net::connection_client>);
	~message_buffer_output();
	void BackUp(int count);
	int64_t ByteCount() const;
	bool Next(void** data, int* size);
	void WriteVarint32(uint32_t);
private:
	void datasend();
};

}
