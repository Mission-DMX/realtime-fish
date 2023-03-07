#include "../test/message_buffer.hpp"
#include "google/protobuf/io/zero_copy_stream.h"
#include "lib/logging.hpp"

namespace dmxfish::test {


	message_buffer_output::message_buffer_output(std::shared_ptr<::rmrf::net::ioqueue<::rmrf::net::iorecord>> io_buffer_):
		io_buffer(io_buffer_),
		new_buff(nullptr),
		byte_count(0)
	{}

		bool message_buffer_output::Next(void** data, int* size){
			if (new_buff != nullptr){
				const ::rmrf::net::iorecord& new_rec = ::rmrf::net::iorecord(new_buff, 4);
				this->io_buffer->push_back(new_rec);
				byte_count += 4;
			}
			new_buff = new uint32_t;
			*size = 4;
			*data = new_buff;
			return true;
		}

	void message_buffer_output::BackUp(int count){
		if (count>4){
			return;
		}
		const ::rmrf::net::iorecord& new_rec = ::rmrf::net::iorecord(new_buff, 4-count);
		this->io_buffer->push_back(new_rec);
		byte_count += (4-count);
		new_buff = nullptr;
	}

	int64_t message_buffer_output::ByteCount() const {
		return this->byte_count;
	}

	void message_buffer_output::WriteVarint32(uint32_t num){
		std::deque<uint8_t> array;
		while (num>=128){
			array.push_back(num%128+128);
			num = num / 128;
		}
		array.push_back(num);
		while (array.size()>0){
			int size;
			uint8_t * data;
			Next((void**) &data, &size);
			while (size > 0 && array.size()>0) {
				*data = array.front();
				data++;
				array.pop_front();
				size--;
			}
			BackUp(size);
		}
	}
}
