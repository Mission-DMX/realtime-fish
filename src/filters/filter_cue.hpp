#pragma once

/*
 * The filters hold a cue sequence
 */

#include <vector>
#include <functional>
#include "filters/filter.hpp"


namespace dmxfish::filters {
    COMPILER_SUPRESS("-Weffc++")


    class filter_cue: public filter {
    private:
        enum transition_t{
            EDGE,
            LINEAR,
            SIGMOIDAL,
            EASE_IN,
            EASE_OUT
        };
        enum handling_at_the_end{
            HOLD,
            START_AGAIN,
            NEXT_CUE,
            HOLDING
        };
        enum handling_at_restart{
            DO_NOTHING,
            START_FROM_BEGIN
        };

        enum run_state : uint8_t {
            STOP,
            PLAY,
            PAUSE
        };

        enum channel_t{
            EIGHT_BIT,
            SIXTEEN_BIT,
            FLOAT,
            COLOR
        };


        template <typename T>
        struct key_frame{
            T value;
            transition_t transition;
            key_frame(T val, transition_t tr): value(val), transition(tr) {}
        };

        template <typename T>
        void reserve_init_out(int amount);
        template <typename T>
        void init_values_out(std::string &channel_name);

        struct cue_st{
            std::vector<double> timestamps;
            std::vector<key_frame<uint8_t>> eight_bit_frames;
            std::vector<key_frame<uint16_t>> sixteen_bit_frames;
            std::vector<key_frame<double>> float_frames;
            std::vector<key_frame<dmxfish::dmx::pixel>> color_frames;
            handling_at_the_end end_handling;
            handling_at_restart restart_handling;
        };

        struct channel_str{
            channel_t channel_type;
            size_t index;
            channel_str(channel_t ch_t,  size_t i) : channel_type(ch_t), index(i) {}
        };

        struct frame_actual{
            bool updated;
            double time_stamp;
            size_t cue{};
            size_t frame{};
            std::vector<uint8_t> eight_bit_channels{};
            std::vector<uint16_t> sixteen_bit_channels{};
            std::vector<double> float_channels{};
            std::vector<dmxfish::dmx::pixel> color_channels{};
            frame_actual() :
                    updated(false),
                    time_stamp(0),
                    cue(65535),
                    frame(65535) {}
        };

        std::string own_filter_id;
        double* time = nullptr;
        double* time_scale_input = nullptr;
        double time_scale = 1;
        double current_time = 0;
        bool scale_valid = true;
        bool state_persistent = false;
        double start_time = 0;
        double pause_time = 0;
        frame_actual last_values = frame_actual();
        frame_actual actual_values = frame_actual();

        uint16_t next_cue = 0xffff;
	    long default_cue = -1;
        handling_at_the_end cue_end_handling_real = START_AGAIN;

        handling_at_the_end handle_end = HOLD;
        run_state running_state = STOP;

        std::vector<channel_str> channel;

        // containing the cues, incl, frames and transition types
        std::vector<cue_st> cues;

        // channel names for the output map
        std::vector<std::string> channel_names_eight;
        std::vector<std::string> channel_names_sixteen;
        std::vector<std::string> channel_names_float;
        std::vector<std::string> channel_names_color;

        /**
         * Sends current state to the ui
         */
        void update_parameter_gui();

        /**
         * Handles each part separated by the sep of the string from start to end with the given function
         * @param str the string to investigate
         * @param start the start position where the investigation should start
         * @param end the end position where the investigation should end
         * @param sep the separator to separate parts of the string to be handled individually
         * @param min_loops the number of parts at least to be found
         * @param func the function to handle each part
         * @return True when everything went fine, false otherwise or min_loops was not reached
         */
        inline bool do_with_substr(const std::string& str, size_t start, const size_t end, const char sep, size_t min_loops, const std::function<bool(const std::string&, size_t, size_t, size_t)> func);

        /**
         * Reads out the value and transition type of the one channel-frame
         * @param cue the number of the cue to which the frame belongs
         * @param str the string to investigate
         * @param start the start position where the investigation should start
         * @param end the end position where the investigation should end
         * @param nr_channel the number which channel is investigated
         * @return True when everything went fine
         */
        bool handle_channel_frame(size_t cue, const std::string& str, size_t start, size_t end, size_t nr_channel);

        /**
         * Reads out the timestamp and handles the channel-frames
         * @param cue the number of the cue to which the timestamp belongs
         * @param str the string to investigate
         * @param start the start position where the investigation should start
         * @param end the end position where the investigation should end
         * @param nr_timestamp the number of the timestamp in this cue
         * @return True when everything went fine
         */
        bool handle_timestamps(size_t cue, const std::string& str, size_t start, size_t end, size_t nr_timestamp);

        /**
         * Handles the configuration (end_handling) of the cue
         * @param cue the number of the cue
         * @param str the string to investigate
         * @param start the start position where the investigation should start
         * @param end the end position where the investigation should end
         * @param number (not used)
         * @return True when everything went fine
         */
        bool handle_cue_conf(size_t cue, const std::string& str, size_t start, size_t end, size_t number);

        /**
         * Handles the cue
         * @param str the string to investigate
         * @param start the start position where the investigation should start
         * @param end the end position where the investigation should end
         * @param number the number of the cue again
         * @return True when everything went fine
         */
        bool handle_cue(const std::string& str, size_t start, size_t end, size_t cue);

        /**
         * Calculation of the current output values
         * @param rel_time the relative time already proceed to the next timestamp
         * @param transition the transition type to the next timestamp
         * @param start_value the value where this transition started
         * @param end_value the value where this transition should end
         * @param ind the index which channel-number (of the specific type) is considered
         */
        template <typename T>
        void calc_transition(double rel_time, transition_t transition, T start_value, T end_value, size_t ind);

        /**
         * Puts the values of the last update to the output, for example if a cue ends with and hold
         */
        void update_hold_values();

        /**
         * Puts the values of the of the last timestamp of the cue to the last values, so the next transition starts with the right ones
         */
        void update_last_values();

        /**
         * Puts the values of the last frame from the cuelist to the last_values
         */
        void update_last_values_from_cuelist();

        /**
         * Resetting internal state for starting a new cue
         */
        void reset_for_starting_cue();

        /**
         * Updates all output values (and internal states for the current time
         */
        void calc_values();

        /**
         * Handling when the end of a cue is reached
         */
        bool last_frame_handling();

    public:
        filter_cue() : filter() {this->scale_valid = true;}
        virtual ~filter_cue() {}

        virtual void pre_setup(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const std::string& own_id) override;

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override;

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override;

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override;

        virtual void update() override;

        virtual void scene_activated() override;

        virtual void scene_deactivated() override;

    };

    COMPILER_RESTORE("-Weffc++")

}
