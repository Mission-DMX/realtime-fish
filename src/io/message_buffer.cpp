#include "io/message_buffer.hpp"
#include "lib/logging.hpp"

#define BUFF_SIZE 1024

namespace dmxfish::io {


	message_buffer_output::message_buffer_output(std::shared_ptr<rmrf::net::connection_client> conn_client_):
		byte_count(0),
		conn_client(conn_client_),
		data_buff((uint8_t*) malloc(BUFF_SIZE)),
		pre_used(0)
	{
	}

	message_buffer_output::~message_buffer_output()
	{
		free(this->data_buff);
	}

	bool message_buffer_output::Next(void** data, int* size){
		if (this->pre_used == BUFF_SIZE){
			this->datasend();
		}
		*data = (this->data_buff + this->pre_used);
		*size = BUFF_SIZE- this->pre_used;
		return true;
	}

	void message_buffer_output::BackUp(int count){
		this->pre_used = BUFF_SIZE - count;
		this->datasend();
	}

	int64_t message_buffer_output::ByteCount() const {
		return this->byte_count;
	}

	void message_buffer_output::WriteVarint32(uint32_t num){
		while(true){
			while (num>=128){
				if(this->pre_used >= BUFF_SIZE){
					this->datasend();
				}
				*(this->data_buff + this->pre_used) = num % 128 + 128;
				this->pre_used++;
				num = num / 128;
			}
			if(this->pre_used >= BUFF_SIZE){
				this->datasend();
			}
			*(this->data_buff + this->pre_used) = num;
			this->pre_used++;
			return;
		}
	}

	inline void message_buffer_output::datasend(){
		this->conn_client->write_data(rmrf::net::iorecord(this->data_buff, this->pre_used));
		this->byte_count += this->pre_used;
		this->pre_used = 0;
	}
}
