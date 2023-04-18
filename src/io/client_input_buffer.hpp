#pragma once
#include "google/protobuf/io/zero_copy_stream.h"
#include "rmrf-net/ioqueue.hpp"

namespace dmxfish::io {

    class client_input_buffer : public google::protobuf::io::ZeroCopyInputStream {
    private:
        std::shared_ptr<rmrf::net::ioqueue<::rmrf::net::iorecord>> io_buffer;
        int64_t byte_count;
        int limit_;
        int read_var_int_multiplier;
        size_t streamsize;
        std::unique_ptr<rmrf::net::iorecord> actual_record;
        size_t local_offset;
    public:
        client_input_buffer();
        bool Next(const void** data, int* size);
        void BackUp(int count);
        bool Skip(int count);
        int64_t ByteCount() const;
        bool ReadVarint32(uint32_t *);
        void append_data(const rmrf::net::iorecord&);
        void update_limit(int limit);
        size_t get_streamsize();
    private:
        void BackUpLocal(int count);
        bool SkipLocal(int count);
    };
}