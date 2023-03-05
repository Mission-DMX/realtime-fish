#include "io/client_handler.hpp"

#include "lib/logging.hpp"
#include "google/protobuf/io/coded_stream.h"

namespace dmxfish::io {

	client_handler::client_handler(parse_message_cb_t parse_message_cb_, std::shared_ptr<rmrf::net::tcp_client> client):
		parse_message_cb(parse_message_cb_),
		io_buffer(std::make_shared<::rmrf::net::ioqueue<::rmrf::net::iorecord>>()),
		// input_stream(std::make_shared<message_buffer_input>(this->io_buffer)),
		output_stream(std::make_shared<message_buffer_output>(this->io_buffer)),
		tcp_client(client),
		internal_state(NEXT_MSG),
		nr_of_read_msg(0),
		actual_record(this->io_buffer->begin()),
		localoffset(0),
		localoffset_last(0),
		byte_count(0),
		byte_count_temp(0),
		limit_(5)
	{

	}

	void client_handler::handle_messages(){
		switch (this->internal_state) {
			case NEXT_MSG:
				{
					if(ReadVarint32(&this->msg_type)){
					// googles lengh read function, does not work
					// if(getIstream().HandleReadResult(((google::protobuf::io::CodedInputStream*) getIstream())->ReadVarint32(&msg_type))){
						std::cout << "msg_type: " << this->msg_type << std::endl;
						this->internal_state = GETLENGTH;
						this->limit_ = 5;
					}
					else {
						::spdlog::debug("NEXT_MSG: Error");
						return;
					}
					// ::spdlog::debug("NextMsg");
					return handle_messages();
				}
			case GETLENGTH:
				{
					// // if(getIstream().HandleReadResult(getIstream().ReadVarint32(&this->msg_length))){
					// int size_before = streamsize();
					// if(ReadVarint32(&this->msg_length)){
					// 	this->pls_size = size_before - streamsize();
					// 	HandleReadResult(false);
					// 	std::cout << "msg_length: " << this->msg_length << std::endl;
					// 	this->internal_state = READ_MSG;
					// 	this->limit_ = this->pls_size + this->msg_length;
					// } else {
					// 	HandleReadResult(false);
					// 	return;
					// }
					// return handle_messages();
					// if(getIstream().HandleReadResult(getIstream().ReadVarint32(&this->msg_length))){
					int size_before = streamsize();
					if(ReadVarint32(&this->msg_length)){
						this->pls_size = size_before - streamsize();
						// HandleReadResult(true);
						std::cout << "msg_length: " << this->msg_length << std::endl;
						this->internal_state = READ_MSG;
						this->limit_ =  this->msg_length;
					} else {
						// HandleReadResult(false);
						return;
					}
					return handle_messages();
				}
			case READ_MSG:
				{
					// if (streamsize() >= this->msg_length + this->pls_size){
					if (streamsize() >= this->msg_length){
						if (parse_message_cb(msg_type, *this)){
							this->internal_state = NEXT_MSG;
							this->limit_ = 5;
						} else{
							::spdlog::debug("ReadMSG: Error");
							return;
						}
						// ::spdlog::debug("ReadMSG");
						return handle_messages();
					}
					::spdlog::debug("ReadMSG: Msg was not long enough");
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
		*data = (*this->actual_record).ptr() + localoffset;
		*size = sizetemp();
		this->byte_count_temp += (*this->actual_record).size();
		this->localoffset_last = this->localoffset;
		this->localoffset = 0;
		this->actual_record++;
		std::cout << "NextLimit: " << this->limit_ << std::endl;
		std::cout << "NextR:" << std::hex;
		for(int i = 0; i< *size; i++){
			std::cout << " " <<(int) *((uint8_t*)*data+i);
		}
		std::cout << std::dec << std::endl;
		limit_ -= *size;
		if (limit_ < 0) {
		 *size += limit_;
		}

		return true;
	}
	void client_handler::BackUp(int count){
		if (limit_ < 0) {
	    this->BackUpLocal(count - limit_);
	    limit_ = count;
	  } else {
	    this->BackUpLocal(count);
    	limit_ += count;
		}
		FinishRead();
	}

	void client_handler::BackUpLocal(int count){
		if (count > 0){
			this->actual_record--;
			this->localoffset = sizetemp() - count + this->localoffset_last;
		}
		this->byte_count_temp -= count;
	}

	// bool client_handler::HandleReadResult(bool res){
	// 	if (res){
	// 		FinishRead();
	// 	}
	// 	else {
	// 		Restore();
	// 	}
	// 	this->byte_count_temp = 0;
	// 	return res;
	// }
	//
	// void client_handler::Restore(){
	// 	this->actual_record = this->io_buffer->begin();
	// 	this->localoffset = 0;
	// }

	void client_handler::FinishRead(){
		while (this->io_buffer->begin() < this->actual_record) {
			this->io_buffer->pop_front();
		}
		if (this->localoffset > 0){
			(*this->actual_record).advance(this->localoffset);
		}
		this->localoffset = 0;
		this->byte_count += this->byte_count_temp;
	}

	bool client_handler::Skip(int count){
		if (count > limit_) {
	    if (limit_ < 0) return false;
	    this->Skip(limit_);
	    limit_ = 0;
	    return false;
	  } else {
	    if (!this->Skip(count)) return false;
	    limit_ -= count;
	    return true;
	  }
	}

	bool client_handler::SkipLocal(int count){
		::spdlog::debug("Run Skip...for skipping {} bytes", count);
		if (count > streamsize()){
			return false;
		}

		while (count > 0){
			if (this->actual_record >= this->io_buffer->end()){
				return false;
			}
			if (count < (*this->actual_record).size()){
				this->byte_count_temp += count;
				this->localoffset += count;
			} else{
				this->localoffset = 0;
				this->byte_count_temp += (*this->actual_record).size();
				this->actual_record++;
			}
		}
		return true;
	}

	int64_t client_handler::ByteCount() const{
		// return this->byte_count + this->byte_count_temp;
		if (limit_ < 0) {
	    return this->byte_count + this->byte_count_temp + limit_; // - prior_bytes_read_;
	  } else {
	    return this->byte_count + this->byte_count_temp; //  - prior_bytes_read_;
	  }
	}

	int64_t client_handler::ByteCountLocal() const{
		return this->byte_count + this->byte_count_temp;
	}


	bool client_handler::ReadVarint32(uint32_t* num){
		*num = 0;
		int size = 0;
		uint8_t* data;
		int cnt = 1;
		while(true){
			if(Next((const void**) &data, &size)){
				while(size>0){
					if(*data>=128){
						*num +=(*data % 128) * 128 * cnt;
						cnt *= 128;
						data++;
						size--;
					}
					else{
						*num += *data;
						size--;
						BackUp(size);
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
		while (temp_it<=this->io_buffer->end()){
			cnt += (*temp_it).size();
			temp_it++;
		}
		return cnt - this->localoffset;
	}

	int client_handler::sizetemp() const{
		if (this->actual_record < this->io_buffer->end()){
			return (*this->actual_record).size() - this->localoffset;
		}
		return 0;
	}

}
