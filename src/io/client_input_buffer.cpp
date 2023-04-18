#include "io/client_input_buffer.hpp"
#include "lib/logging.hpp"
#include <sstream>

namespace dmxfish::io {

    client_input_buffer::client_input_buffer():
            io_buffer(std::make_shared<rmrf::net::ioqueue<::rmrf::net::iorecord>>()),
            byte_count(0),
            limit_(5),
            read_var_int_multiplier(1),
            streamsize(0),
            actual_record(nullptr),
            local_offset(0)
    {
    }
    bool client_input_buffer::Next(const void** data, int* size){
        if (limit_ <= 0){
//            ::spdlog::debug("limit was reached");
            return false;
        }

        if (this->streamsize <= 0){
//            ::spdlog::debug("streamsize is {}", this->streamsize);
            return false;
        }

        if (this->actual_record == nullptr || this->local_offset >= this->actual_record->size()){
            if (this->io_buffer->empty()) {
//                ::spdlog::debug("Message_buffer is empty but streamsize is {}", this->streamsize);
                return false;
            }
            this->actual_record = std::make_unique<rmrf::net::iorecord>(this->io_buffer->pop_front());
            this->local_offset = 0;
        }

        *data = ((uint8_t*) this->actual_record->ptr()) + this->local_offset;
        *size = (int) this->actual_record->size() - (int) this->local_offset;

        this->byte_count += *size;
        this->limit_ -= *size;
        if (limit_ < 0) {
            *size += this->limit_;
        }
        this->local_offset += *size;
        this->streamsize -= *size;

        return true;
    }


    void client_input_buffer::BackUp(int count){
        if (this->limit_ < 0) {
            this->BackUpLocal(count - this->limit_);
            this->limit_ = count;
        } else {
            this->BackUpLocal(count);
            this->limit_ += count;
        }
    }

    inline void client_input_buffer::BackUpLocal(int count){
        this->actual_record->advance(this->actual_record->size() - count);
        this->local_offset = 0;
        this->streamsize += count;
        this->byte_count -= count;
    }

    bool client_input_buffer::Skip(int count){
        if (count > this->limit_) {
            this->SkipLocal(limit_);
            this->limit_ = 0;
            return false;
        } else {
            if (!this->SkipLocal(count)) {
                return false;
            }
            this->limit_ -= count;
            return true;
        }
    }

    bool client_input_buffer::SkipLocal(int count){
        ::spdlog::debug("Run Skip...for skipping {} bytes", count);
        // skip makes shure that count is always less equal limit_
        while (count > 0){
            if (limit_ <= 0){
//            ::spdlog::debug("limit was reached");
                return false;
            }

            if (this->streamsize <= 0){
//            ::spdlog::debug("streamsize is {}", this->streamsize);
                return false;
            }

            if (this->actual_record == nullptr || this->local_offset >= this->actual_record->size()){
                if (this->io_buffer->empty()) {
//                ::spdlog::debug("Message_buffer is empty but streamsize is {}", this->streamsize);
                    return false;
                }
                this->actual_record = std::make_unique<rmrf::net::iorecord>(this->io_buffer->pop_front());
                this->local_offset = 0;
            }
            int local_size = (int) this->actual_record->size() - (int) this->local_offset;
            this->streamsize -= local_size;
            count -= local_size;
            this->byte_count += local_size;
            this->limit_ -= local_size;
            if (count < 0){
                this->byte_count += count;
                this->actual_record->advance(local_size + count);
                this->local_offset = 0;
                this->streamsize -= count;
                this->limit_ -= count;
            }
        }
        return true;
    }

    int64_t client_input_buffer::ByteCount() const{
        if (this->limit_ < 0) {
            return this->byte_count + this->limit_;
        } else {
            return this->byte_count;
        }
    }

    bool client_input_buffer::ReadVarint32(uint32_t* num){
        int size = 0;
        uint8_t* data;
        while(true){
            if(Next((const void**) &data, &size)){
                while(size>0){
                    if(*data>=128){
                        *num +=(*data % 128) * this->read_var_int_multiplier;
                        this->read_var_int_multiplier *= 128;
                        data++;
                        size--;
                    }
                    else{
                        *num += *data * this->read_var_int_multiplier;
                        size--;
                        BackUp(size);
                        this->read_var_int_multiplier = 1;
                        return true;
                    }
                }
            }else{
                return false;
            }
        }
        return false;
    }

    void client_input_buffer::append_data(const rmrf::net::iorecord& data){
        this->streamsize += data.size();
        auto strstream = std::stringstream();
        strstream << "Got: Next:" << std::hex;
        for(size_t i = 0; i < data.size(); i++){
            strstream << " " << (int) *(((uint8_t*) data.ptr())+i);
        }
        ::spdlog::debug("{}", strstream.str());
        this->io_buffer->push_back(data);
    }

    void client_input_buffer::update_limit(int limit) {
        this->limit_ = limit;
    }

    size_t client_input_buffer::get_streamsize() {
        return streamsize;
    }
}