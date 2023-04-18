#include "../test/test_client_side.hpp"

#include "lib/logging.hpp"
#include "google/protobuf/io/coded_stream.h"
#include <sstream>


namespace dmxfish::test {

	client_side::client_side(parse_message_cb_t parse_message_cb_, std::unique_ptr<rmrf::net::connection_client> client):
		parse_message_cb(parse_message_cb_),
		io_buffer(std::make_shared<rmrf::net::ioqueue<::rmrf::net::iorecord>>()),
		// connection_client(client),
		internal_state(NEXT_MSG),
		msg_type(0),
		byte_count(0),
		limit_(5),
		read_var_int_multiplier(1),
		// output_buffer(std::move(client)),
		streamsize(0)
	{
		client->set_incomming_data_callback(std::bind(&dmxfish::test::client_side::incomming_data_callback, this, std::placeholders::_1));
		this->output_buffer = std::make_shared<message_buffer_output>(std::move(client));
	}

	void client_side::incomming_data_callback(const rmrf::net::iorecord& data){
		::spdlog::debug("TEST: reached callback");
		this->streamsize += data.size();
		this->io_buffer->push_back(data);
		this->handle_messages();
	}

	void client_side::handle_messages(){
		switch (this->internal_state) {
			case NEXT_MSG:
				{
					if(ReadVarint32(&this->msg_type)){
						this->msg_length = 0;
						this->internal_state = GETLENGTH;
						this->limit_ = 5;
					}
					else {
						::spdlog::debug("TEST: NEXT_MSG: Not finishing byte for Varint");
						return;
					}
					return handle_messages();
				}
			case GETLENGTH:
				{
					if(ReadVarint32(&this->msg_length)){
						this->internal_state = READ_MSG;
						this->limit_ =  this->msg_length;
					} else {
						::spdlog::debug("TEST: GETLENGTH: Not finishing byte for Varint");
						return;
					}
					return handle_messages();
				}
			case READ_MSG:
				{
					if (this->streamsize >= this->msg_length){
						parse_message_cb(msg_type, *this);
						this->msg_type = 0;
						this->internal_state = NEXT_MSG;
						this->limit_ = 5;
						return handle_messages();
					}
					::spdlog::debug("TEST: ReadMSG: Msg was not long enough: is {}, should: {}", this->streamsize, this->msg_length);
					break;
				}
			default:
					::spdlog::debug("TEST: Error: Unknown State");
					break;
		}
	}

	bool client_side::Next(const void** data, int* size){
		if (limit_ <= 0) return false;

		if (this->io_buffer->empty()){
			return false;
		}
		this->actual_record = std::make_unique<rmrf::net::iorecord>(this->io_buffer->pop_front());

		*data = this->actual_record->ptr();
		*size = this->actual_record->size();
		this->byte_count += *size;
		this->limit_ -= *size;
		if (limit_ < 0) {
		 *size += this->limit_;
		}
		this->streamsize -= *size;
		
		auto strstream = std::stringstream();
		strstream << "Test: Next:" << std::hex;
		for(int i = 0; i < *size; i++){
			strstream << " " << (int) *((*((uint8_t**)data))+i);
		}
		::spdlog::debug("{}", strstream.str());
		return true;
	}


	void client_side::BackUp(int count){
		if (limit_ < 0) {
	    this->BackUpLocal(count - this->limit_);
	    this->limit_ = count;
	  } else {
	    this->BackUpLocal(count);
    	this->limit_ += count;
		}
	}

	inline void client_side::BackUpLocal(int count){
		this->actual_record->advance(this->actual_record->size()-count);
		this->streamsize += count;
		this->byte_count -= count;
		this->io_buffer->push_front(*this->actual_record.get());
	}

	bool client_side::Skip(int count){
		if (count > this->limit_) {
	    if (limit_ < 0) return false;
	    this->SkipLocal(limit_);
	    this->limit_ = 0;
	    return false;
	  } else {
	    if (!this->SkipLocal(count)) return false;
	    this->limit_ -= count;
	    return true;
	  }
	}

	inline bool client_side::SkipLocal(int count){
		::spdlog::debug("Run Skip...for skipping {} bytes", count);
		while (count > 0){
			if (this->io_buffer->empty()){
				return false;
			}
			this->actual_record = std::make_unique<rmrf::net::iorecord>(this->io_buffer->pop_front());
			this->streamsize -= this->actual_record->size();
			count -= this->actual_record->size();
			this->byte_count += this->actual_record->size();
			if (count < 0){
				this->byte_count += count;
				this->actual_record->advance(-count);
				this->streamsize -= count;
				this->io_buffer->push_front(*this->actual_record.get());
				// count = 0;
			}
		}
		return true;
	}

	void client_side::write_message(google::protobuf::MessageLite& msg, uint32_t msg_type){
		this->output_buffer->WriteVarint32(msg_type);
		this->output_buffer->WriteVarint32(msg.ByteSizeLong());
		msg.SerializeToZeroCopyStream(this->output_buffer.get());
	}

	int64_t client_side::ByteCount() const{
		if (this->limit_ < 0) {
	    return this->byte_count + this->limit_;
	  } else {
	    return this->byte_count;
	  }
	}

	bool client_side::ReadVarint32(uint32_t* num){
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

}