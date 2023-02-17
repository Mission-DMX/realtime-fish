#include "io/message_buffer.hpp"

#include "lib/logging.hpp"

namespace dmxfish::io {

	message_buffer_input::message_buffer_input(std::shared_ptr<::rmrf::net::ioqueue<::rmrf::net::iorecord>> io_buffer_):
		io_buffer(io_buffer_),
		nr_of_read_msg(0),
		actual_record(this->io_buffer->begin()),
		localoffset(0),
		localoffset_last(0),
		byte_count(0),
		byte_count_temp(0)
	{
	}

	bool message_buffer_input::Next(const void** data, int* size){
		if (this->actual_record >= this->io_buffer->end()){
			return false;
		}
		*data = (*this->actual_record).ptr() + localoffset;
		*size = sizetemp();
		this->byte_count_temp += (*this->actual_record).size();
		this->localoffset_last = this->localoffset;
		this->localoffset = 0;
		this->actual_record++;
		return true;
	}

	void message_buffer_input::BackUp(int count){
		if (count > 0){
			this->actual_record--;
			this->localoffset = sizetemp() - count + this->localoffset_last;
		}
		this->byte_count_temp -= count;
	}

	bool message_buffer_input::HandleReadResult(bool res){
		if (res){
			FinishRead();
		}
		else {
			Restore();
		}
		this->byte_count_temp = 0;
		return res;
	}

	void message_buffer_input::Restore(){
		this->actual_record = this->io_buffer->begin();
		this->localoffset = 0;
	}

	void message_buffer_input::FinishRead(){
		while (this->io_buffer->begin() < this->actual_record) {
			this->io_buffer->pop_front();
		}
		if (this->localoffset > 0){
			(*this->actual_record).advance(this->localoffset);
		}
		this->localoffset = 0;
		this->byte_count += this->byte_count_temp;
	}

	bool message_buffer_input::Skip(int count){
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

	int64_t message_buffer_input::ByteCount() const{
		return this->byte_count + this->byte_count_temp;
	}


	bool message_buffer_input::ReadVarint32(uint32_t* num){
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

	int message_buffer_input::streamsize() const{
		int cnt = 0;
		std::deque<rmrf::net::iorecord>::iterator temp_it = this->actual_record;
		while (temp_it<=this->io_buffer->end()){
			cnt += (*temp_it).size();
			temp_it++;
		}
		return cnt - this->localoffset;
	}

	int message_buffer_input::sizetemp() const{
		if (this->actual_record < this->io_buffer->end()){
			return (*this->actual_record).size() - this->localoffset;
		}
		return 0;
	}





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
