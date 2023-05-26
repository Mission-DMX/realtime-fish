#pragma once

#include <functional>
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

#include "lib/macros.hpp"
COMPILER_SUPRESS("-Wuseless-cast")
#include "proto_src/Console.pb.h"
COMPILER_RESTORE("-Wuseless-cast")

namespace dmxfish::io {
class IOManager;
}

namespace dmxfish::control_desk {

    class bank {
    private:
        std::vector<std::shared_ptr<bank_column>> columns;
	// If this turns out to be a performance issue we may switch to
	// not storing this function pointer here again and use direct
	// objects in the vector from bank_set again. This would require
	// changing the factory though.
        const std::function<void(std::string const&, bool)> set_ready_state_handler;
        const std::function<void(std::string const&, bool)> select_state_handler;
    public:
        bank(std::function<void(std::string const&, bool)> _set_ready_state_handler, std::function<void(std::string const&, bool)> _select_state_handler);
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
            auto ptr = std::make_shared<bank_column>(device_connection, set_ready_state_handler, select_state_handler, mode, id, fader_index);
            columns.push_back(ptr);
            return ptr;
        }

        inline std::shared_ptr<bank_column> get(size_t pos) {
        if(pos < this->columns.size())
            return columns[pos];
        else
            return nullptr;
        }
    };

    class desk {
    private:
        struct bank_set {
            size_t active_bank = 0;
            std::vector<std::shared_ptr<bank>> fader_banks;
            std::unordered_map<std::string, std::shared_ptr<bank_column>> columns_map;
            std::unordered_set<std::string> columns_in_ready_state;
            std::string id;

            bank_set(const std::string& _id) : fader_banks{}, columns_map{}, columns_in_ready_state{}, id{_id} {};
            ~bank_set() = default;
        };
    private:
        std::vector<std::shared_ptr<device_handle>> devices;
        std::vector<bank_set> bank_sets;
        std::map<std::string, size_t> bankset_to_index_map;
        std::string selected_column_id = "";
        std::string find_enabled_on_column_id = "";
        size_t max_number_of_colums = 0;
        size_t current_active_bank_set = 0;
        int jogwheel_change = 0;
        bool update_message_required = false;
        bool bank_set_modification_happened = false;
        bool global_dark = false;
        uint16_t global_illumination = 0;
    public:
        desk(std::list<std::pair<std::string, midi_device_id>> input_devices);
        ~desk();

        /**
         * This method performs the updates of all HMI input devices
         */
        void update();

        bool set_active_bank_set(size_t index);

        inline bool set_active_bank_set(const std::string& id) {
            if(!bankset_to_index_map.contains(id)) {
                return false;
            }
            update_message_required = true;
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

        [[nodiscard]] inline bool is_global_black_enabled() {
            return global_dark;
        }

        [[nodiscard]] inline uint8_t get_global_illumination() {
            return global_dark ? 0 : global_illumination;
        }

        bool set_active_fader_bank_on_current_set(size_t index);
        [[nodiscard]] size_t get_active_fader_bank_on_current_set();

        void add_bank_set_from_protobuf_msg(const ::missiondmx::fish::ipcmessages::add_fader_bank_set& definition);
        void process_desk_update_message(const ::missiondmx::fish::ipcmessages::desk_update& msg);
        void update_column_from_message(const ::missiondmx::fish::ipcmessages::fader_column& msg);

        void update_fader_position_from_protobuf(const ::missiondmx::fish::ipcmessages::fader_position& msg);
        void update_encoder_state_from_protobuf(const ::missiondmx::fish::ipcmessages::rotary_encoder_change& msg);
        void update_button_leds_from_protobuf(const missiondmx::fish::ipcmessages::button_state_change& msg);
        void set_seven_seg_display_data(const std::string& data);
        std::shared_ptr<bank_column> find_column(const std::string& set_id, const std::string& column_id);
    private:
        void reset_devices();
        void remove_bank_set(size_t i);
        void process_incomming_command(const midi_command& c, size_t device_index);
        void handle_bord_buttons(button b, button_change c);
        void handle_ready_state_update_from_bank(const std::string& column_id, bool new_state);
        void handle_select_state_update_from_bank(const std::string& column_id, bool new_state);
        void update_fader_bank_leds();
        void commit_readymode();
    };

}
