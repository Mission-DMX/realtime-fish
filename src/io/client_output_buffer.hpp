#pragma once
#include <vector>
#include "google/protobuf/io/zero_copy_stream.h"

namespace dmxfish::io {

    class client_output_buffer : public google::protobuf::io::ZeroCopyOutputStream {
    private:
        std::vector<uint8_t>& buffer;
        size_t local_offset;
    public:
        client_output_buffer(std::vector<uint8_t>&);
        bool Next(void** data, int* size);
        void BackUp(int count);
        int64_t ByteCount() const;
        bool WriteVarint32(size_t num);
    };
}