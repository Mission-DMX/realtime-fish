#include "io/client_handler.hpp"

#include "lib/logging.hpp"
#include "google/protobuf/io/coded_stream.h"

namespace dmxfish::io {

	client_handler::client_handler(parse_message_cb_t parse_message_cb_, std::shared_ptr<rmrf::net::connection_client> client):
		parse_message_cb(parse_message_cb_),
		io_buffer(std::make_shared<::rmrf::net::ioqueue<::rmrf::net::iorecord>>()),
		connection_client(client),
		internal_state(NEXT_MSG),
		msg_type(0),
		actual_record(this->io_buffer->begin()),
		byte_count(0),
		limit_(5),
		read_var_int_multiplier(1)
	{
		this->connection_client->set_incomming_data_callback(std::bind(&dmxfish::io::client_handler::push_msg, this, std::placeholders::_1));
	}

	void client_handler::push_msg(const rmrf::net::iorecord& data){
		this->io_buffer->push_back(data);
		this->handle_messages();
	}

	void client_handler::handle_messages(){
		switch (this->internal_state) {
			case NEXT_MSG:
				{
					if(ReadVarint32(&this->msg_type)){
						this->msg_length = 0;
						this->internal_state = GETLENGTH;
						this->limit_ = 5;
					}
					else {
						::spdlog::debug("NEXT_MSG: Not finishing byte for Varint");
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
						::spdlog::debug("GETLENGTH: Not finishing byte for Varint");
						return;
					}
					return handle_messages();
				}
			case READ_MSG:
				{
					if (streamsize() >= this->msg_length){
						parse_message_cb(msg_type, *this);
						this->msg_type = 0;
						this->internal_state = NEXT_MSG;
						this->limit_ = 5;
						return handle_messages();
					}
					::spdlog::debug("ReadMSG: Msg was not long enough");
					break;
				}
			default:
					::spdlog::debug("Error: Unknown State");
					break;
		}
	}

	bool client_handler::Next(const void** data, int* size){
		if (limit_ <= 0) return false;
		if (this->actual_record >= this->io_buffer->end()){
			return false;
		}
		if (this->io_buffer->begin() != this->actual_record){
			this->io_buffer->pop_front();
		}
		*data = (*this->actual_record).ptr();
		*size = (*this->actual_record).size();
		this->byte_count += *size;
		this->actual_record++;
		this->limit_ -= *size;
		if (limit_ < 0) {
		 *size += this->limit_;
		}

		return true;
	}


	void client_handler::BackUp(int count){
		if (limit_ < 0) {
	    this->BackUpLocal(count - this->limit_);
	    this->limit_ = count;
	  } else {
	    this->BackUpLocal(count);
    	this->limit_ += count;
		}
	}

	inline void client_handler::BackUpLocal(int count){
		if (count == 0){
			this->io_buffer->pop_front();
		} else {
			this->actual_record--;
			this->actual_record->advance(this->actual_record->size()-count);
			this->byte_count -= count;
		}
	}

	bool client_handler::Skip(int count){
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

	inline bool client_handler::SkipLocal(int count){
		::spdlog::debug("Run Skip...for skipping {} bytes", count);
		// if (count > streamsize()){
		// 	return false;
		// }

		while (count > 0){
			if (this->actual_record >= this->io_buffer->end()){
				return false;
			}
			if (count < this->actual_record->size()){
				this->byte_count += count;
				this->actual_record->advance(count);
				count = 0;
			} else{
				count -= this->actual_record->size();
				this->byte_count += this->actual_record->size();
				this->actual_record++;
				this->io_buffer->pop_front();
			}
		}
		return true;
	}

	int64_t client_handler::ByteCount() const{
		// ::spdlog::debug("ByteCount");
		if (this->limit_ < 0) {
	    return this->byte_count + this->limit_;
	  } else {
	    return this->byte_count;
	  }
	}

	bool client_handler::ReadVarint32(uint32_t* num){
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

	int client_handler::streamsize() const{
		int cnt = 0;
		std::deque<rmrf::net::iorecord>::iterator temp_it = this->actual_record;
		while (temp_it < this->io_buffer->end()){
			cnt += temp_it->size();
			temp_it++;
		}
		return cnt;
	}

}
