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
        enum run_state{
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
//            std::string name;
            channel_t channel_type;
            size_t index;
            channel_str(channel_t ch_t,  size_t i) : channel_type(ch_t), index(i) {}
        };

        std::string _own_id;
        double* time = nullptr;
        double start_time = 0;
        double pause_time = 0;
        uint16_t frame = 0;
        bool already_updated_last = false;
        bool already_updated_act = false;
        uint16_t active_cue = 0;

        uint16_t next_cue = 0xffff;
	    long default_cue = -1;
        handling_at_the_end cue_end_handling_real = START_AGAIN;

        handling_at_the_end handle_end = HOLD;
        run_state running_state = STOP;

        std::vector<channel_str> channel;

        // containing the cues, incl, frames and transition types
        std::vector<cue_st> cues;

        // values for the output
        std::vector<uint8_t> eight_bit_channels;
        std::vector<uint16_t> sixteen_bit_channels;
        std::vector<double> float_channels;
        std::vector<dmxfish::dmx::pixel> color_channels;

        // last values, for calculating the transition
        double last_timestamp = 0;
        std::vector<uint8_t> last_eight_bit_channels;
        std::vector<uint16_t> last_sixteen_bit_channels;
        std::vector<double> last_float_channels;
        std::vector<dmxfish::dmx::pixel> last_color_channels;

        // channel names for the output map
        std::vector<std::string> channel_names_eight;
        std::vector<std::string> channel_names_sixteen;
        std::vector<std::string> channel_names_float;
        std::vector<std::string> channel_names_color;


        void update_parameter_gui();

        inline bool do_with_substr(const std::string& str, size_t start, const size_t end, const char sep, size_t min_loops, const std::function<bool(const std::string&, size_t, size_t, size_t)> func);

        bool handle_frame(size_t cue, const std::string& str, size_t start, size_t end, size_t nr_channel);

        bool handle_timestamps(size_t cue, const std::string& str, size_t start, size_t end, size_t nr_timestamp);

        bool handle_cue_conf(size_t cue, const std::string& str, size_t start, size_t end, size_t number);

        bool handle_cue(const std::string& str, size_t start, size_t end, size_t cue);

        template <typename T>
        void calc_transition(double rel_time, transition_t transition, T start_value, T end_value, size_t ind);

        void update_hold_values();

        void update_last_values();

        void start_new_cue();

        void calc_values();

    public:
        filter_cue() : filter() {}
        virtual ~filter_cue() {}

        virtual void pre_setup(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const std::string& own_id) override;

        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels, const std::string& own_id) override;

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override;

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override;

        virtual void update() override;

        virtual void scene_activated() override;

    };

    COMPILER_RESTORE("-Weffc++")

}
