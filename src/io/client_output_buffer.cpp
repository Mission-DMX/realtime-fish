#include "io/client_output_buffer.hpp"
#include "io/io_exception.hpp"
#include "lib/logging.hpp"
#include <math.h>

namespace dmxfish::io {

    client_output_buffer::client_output_buffer(std::vector<uint8_t>& buffer_):
            buffer(buffer_),
            local_offset(0)
    {
    }

    bool client_output_buffer::Next(void** data, int* size){

        if (this->local_offset >= this->buffer.size()){
            ::spdlog::info("Output buffer: iorecord was not big enough, why did this happen?");
            return false;
        }
        *data = this->buffer.data() + this->local_offset;
        *size = (int) this->buffer.size() - (int) this->local_offset;
        this->local_offset += *size;
        return true;
    }


    void client_output_buffer::BackUp(int count){
        if (count < 0 || (size_t) count > this->local_offset){
            ::spdlog::info("OutputBuffer: Backup had wrong size");
        }
        this->local_offset -= count;
    }


    int64_t client_output_buffer::ByteCount() const{
        return this->local_offset;
    }

    bool client_output_buffer::WriteVarint32(size_t num){
        while (num>=128){
            if(this->local_offset >= this->buffer.size()){
                return false;
            }
            *(this->buffer.data() + this->local_offset) = num % 128 + 128;
            this->local_offset++;
            num = num / 128;
        }
        if(this->local_offset >= this->buffer.size()){
            return false;
        }
        *(this->buffer.data() + this->local_offset) = (uint8_t) num;
        this->local_offset++;
        return true;
    }
}