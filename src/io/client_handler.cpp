#include "io/client_handler.hpp"

#include "main.hpp"

#include "events/event_source.hpp"
#include "lib/logging.hpp"
#include "lib/macros.hpp"
#include "google/protobuf/io/coded_stream.h"

COMPILER_SUPRESS("-Wuseless-cast")
#include "proto_src/MessageTypes.pb.h"
#include "proto_src/Events.pb.h"
COMPILER_RESTORE("-Wuseless-cast")

namespace dmxfish::io {

	client_handler::client_handler(parse_message_cb_t parse_message_cb_, std::shared_ptr<rmrf::net::connection_client> client):
		parse_message_cb(parse_message_cb_),
		connection_client(client),
		internal_state(NEXT_MSG),
		msg_type(0),
		msg_length(0),
        input_buffer(std::make_shared<client_input_buffer>())
	{
		this->connection_client->set_incomming_data_callback(std::bind(&dmxfish::io::client_handler::incomming_data_callback, this, std::placeholders::_1));
        size_t transmitted_event_senders = 0;
        for(const auto& sender : get_event_storage_instance()->get_registered_senders()) {
            const auto msg = sender->encode_proto_message();
            write_message(msg, ::missiondmx::fish::ipcmessages::MSGT_EVENT);
            transmitted_event_senders++;
        }
        ::spdlog::info("Transmitted {} event sender descriptions to new client.", transmitted_event_senders);
	}

	void client_handler::handle_messages(){
		switch (this->internal_state) {
			case NEXT_MSG:
            {
                if(this->input_buffer->ReadVarint32(&this->msg_type)){
                    this->msg_length = 0;
                    this->internal_state = GETLENGTH;
                    this->input_buffer->update_limit(5);
                }
                else {
//                    ::spdlog::debug("NEXT_MSG: Not finishing byte for Varint");
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
//                    ::spdlog::debug("GETLENGTH: Not finishing byte for Varint");
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
//                ::spdlog::debug("ReadMSG: Stream was not long enough: is {}, should: {}", this->input_buffer->get_streamsize(), this->msg_length);
                break;
            }
			default:
                ::spdlog::warn("Error: client_handler has reached an unknown state");
                break;
		}
	}

    bool client_handler::is_client_alive(){
        return this->connection_client->is_client_alive();
    }

	void client_handler::write_message(const google::protobuf::MessageLite& msg, uint32_t msgtype){
        auto buffer = std::vector<uint8_t>();
        size_t msgsize = msg.ByteSizeLong();
        buffer.resize(get_length_of_varint(msgtype) + get_length_of_varint(msgsize) + msgsize);
        auto stream = dmxfish::io::client_output_buffer(buffer);

        if(!stream.WriteVarint32(msgtype)){
            ::spdlog::warn("Writing Msg Type to stream failed");
            return;
        }
        if(!stream.WriteVarint32(msgsize)){
            ::spdlog::warn("Writing Msg Size to stream failed");
            return;
        }
        if(!msg.SerializeToZeroCopyStream(&stream)){
            ::spdlog::warn("Writing Msg to stream failed");
            return;
        }
        if(this->connection_client->is_client_alive()){
            try {
                this->connection_client->write_data(rmrf::net::iorecord(buffer.data(), buffer.size()));
            } catch (const std::exception& e) {
                ::spdlog::warn(std::string("Could not send message to client: ") + e.what());
                return;
            }
        } else {
            ::spdlog::info("Client Output Buffer: Could not send the message, because client is offline");
        }
	}

    void client_handler::incomming_data_callback(const rmrf::net::iorecord& data){
//        ::spdlog::debug("reached callback");
        this->input_buffer->append_data(data);
        this->handle_messages();
    }

    google::protobuf::io::ZeroCopyInputStream* client_handler::get_zero_copy_input_stream(){
        return this->input_buffer.get();
    }

    int client_handler::get_length_of_varint(size_t num){
        return num==0?1:(int)ceil(log2((double)(num+1))/7);
    }
}
