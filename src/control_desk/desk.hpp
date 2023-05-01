#pragma once

#include <list>
#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "control_desk/bank_column.hpp"
#include "control_desk/command.hpp"
#include "control_desk/device_handle.hpp"

#include "proto_src/Console.pb.h"

namespace dmxfish::io {
class IOManager;
}

namespace dmxfish::control_desk {

    class bank {
    private:
        std::vector<std::shared_ptr<bank_column>> columns;
    public:
        bank();
        ~bank() = default;

        /**
         * This method sets up the colums.
         */
        void activate();

        /**
         * This method deactivates all of its own colums and resets them if required.
         */
        void deactivate(size_t columns_in_next_row);

        inline size_t size() {
            return this->columns.size();
        }

        inline void reserve(int amount) {
            columns.reserve(amount);
        }

        inline std::shared_ptr<bank_column> emplace_back(std::weak_ptr<device_handle> device_connection, bank_mode mode, const std::string& id, uint8_t fader_index) {
            auto ptr = std::make_shared<bank_column>(device_connection, mode, id, fader_index);
            columns.push_back(ptr);
            return ptr;
        }
    };

    class desk {
    private:
        struct bank_set {
            std::vector<bank> fader_banks;
            std::unordered_map<std::string, std::shared_ptr<bank_column>> columns_map;
            std::unordered_set<std::string> columns_in_ready_state;
            size_t active_bank = 0;

            bank_set() : fader_banks{}, columns_map{}, columns_in_ready_state{} {};
            ~bank_set() = default;
        };
    private:
        std::shared_ptr<dmxfish::io::IOManager> iomanager = nullptr;
        std::vector<std::shared_ptr<device_handle>> devices;
        std::vector<bank_set> bank_sets;
        std::map<std::string, size_t> bankset_to_index_map;
        size_t max_number_of_colums = 0;
        size_t current_active_bank_set = 0;
    public:
        desk(std::list<std::pair<std::string, midi_device_id>> input_devices);
        ~desk();

        inline void set_iomanager(std::shared_ptr<dmxfish::io::IOManager> iom) {
            this->iomanager = iom;
        }

        /**
         * This method performs the updates of all HMI input devices
         */
        void update();

        bool set_active_bank_set(size_t index);

        inline bool set_active_bank_set(const std::string& id) {
            if(!bankset_to_index_map.contains(id)) {
                return false;
            }
            return set_active_bank_set(bankset_to_index_map.at(id));
        }

        inline bool remove_bank_set(const std::string& id) {
            if(!bankset_to_index_map.contains(id)) {
                return false;
            }
            remove_bank_set(bankset_to_index_map.at(id));
            return true;
        }

        inline size_t get_active_bank_set() {
            return current_active_bank_set;
        }

        bool set_active_fader_bank_on_current_set(size_t index);

        void add_bank_set_from_protobuf_msg(const ::missiondmx::fish::ipcmessages::add_fader_bank_set& definition);

        void update_fader_position_from_protobuf(const ::missiondmx::fish::ipcmessages::fader_position& msg);
        void update_encoder_state_from_protobuf(const ::missiondmx::fish::ipcmessages::rotary_encoder_change& msg);
        void update_button_leds_from_protobuf(const missiondmx::fish::ipcmessages::button_state_change& msg);

    private:
        void reset_devices();
        void remove_bank_set(size_t i);
        void process_incomming_command(midi_command& c, size_t device_index);
    };

}
