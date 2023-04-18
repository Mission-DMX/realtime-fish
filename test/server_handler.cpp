#include "../test/server_handler.hpp"

#include "lib/logging.hpp"
#include "google/protobuf/io/coded_stream.h"

namespace dmxfish::test {

	server_handler::server_handler(parse_message_cb_t parse_message_cb_, std::shared_ptr<rmrf::net::connection_client> client):
		parse_message_cb(parse_message_cb_),
		connection_client(client),
		internal_state(NEXT_MSG),
		msg_type(0),
		msg_length(0),
        input_buffer(std::make_shared<dmxfish::io::client_input_buffer>())
	{
		this->connection_client->set_incomming_data_callback(std::bind(&dmxfish::test::server_handler::incomming_data_callback, this, std::placeholders::_1));
	}

	void server_handler::handle_messages(){
		switch (this->internal_state) {
			case NEXT_MSG:
            {
                if(this->input_buffer->ReadVarint32(&this->msg_type)){
                    this->msg_length = 0;
                    this->internal_state = GETLENGTH;
                    this->input_buffer->update_limit(5);
                }
                else {
                    ::spdlog::debug("TEST: NEXT_MSG: Not finishing byte for Varint");
                    return;
                }
                return handle_messages();
            }
			case GETLENGTH:
            {
                if(this->input_buffer->ReadVarint32(&this->msg_length)){
                    this->internal_state = READ_MSG;
                    this->input_buffer->update_limit(this->msg_length);
                } else {
                    ::spdlog::debug("TEST: GETLENGTH: Not finishing byte for Varint");
                    return;
                }
                return handle_messages();
            }
            case READ_MSG:
            {
                if (this->input_buffer->get_streamsize() >= this->msg_length){
                    parse_message_cb(msg_type, *this);
                    this->msg_type = 0;
                    this->internal_state = NEXT_MSG;
                    this->input_buffer->update_limit(5);
                    return handle_messages();
                }
                ::spdlog::debug("TEST: ReadMSG: Stream was not long enough: is {}, should: {}", this->input_buffer->get_streamsize(), this->msg_length);
                break;
            }
			default:
                ::spdlog::debug("TEST: Error: Unknown State");
                break;
		}
	}

    bool server_handler::is_client_alive(){
        return this->connection_client->is_client_alive();
    }

	void server_handler::write_message(google::protobuf::MessageLite& msg, uint32_t msgtype){
        auto buffer = std::vector<uint8_t>();
        size_t msgsize = msg.ByteSizeLong();
        buffer.resize(get_length_of_varint(msgtype) + get_length_of_varint(msgsize) + msgsize);
        auto stream = dmxfish::io::client_output_buffer(buffer);

        if(!stream.WriteVarint32(msgtype)){
            ::spdlog::debug("Test: Writing Msg Type to stream failed");
        }
        if(!stream.WriteVarint32(msgsize)){
            ::spdlog::debug("Test: Writing Msg Size to stream failed");
        }
        if(!msg.SerializeToZeroCopyStream(&stream)){
            ::spdlog::debug("Test: Writing Msg to stream failed");
        }
        if(this->connection_client->is_client_alive()){
            this->connection_client->write_data(rmrf::net::iorecord(buffer.data(), buffer.size()));
        } else{
            ::spdlog::debug("Test: Output Buffer: Could not send the message, because server is offline");
        }
    }

    void server_handler::incomming_data_callback(const rmrf::net::iorecord& data){
        ::spdlog::debug("reached callback");
        this->input_buffer->append_data(data);
        this->handle_messages();
    }

    google::protobuf::io::ZeroCopyInputStream* server_handler::get_zero_copy_input_stream(){
        return this->input_buffer.get();
    }

    int server_handler::get_length_of_varint(size_t num){
        return num==0?1:(int)ceil(log2((double)(num+1))/7);
    }
}
