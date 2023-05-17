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
            NEXT_CUE
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
        struct KeyFrame{
            T value;
            transition_t transition;
            KeyFrame(T val, transition_t tr): value(val), transition(tr) {}
        };

        struct Cue{
            std::vector<double> timestamps;
            std::vector<KeyFrame<uint8_t>> eight_bit_frames;
            std::vector<KeyFrame<uint16_t>> sixteen_bit_frames;
            std::vector<KeyFrame<double>> float_frames;
            std::vector<KeyFrame<dmxfish::dmx::pixel>> color_frames;
            handling_at_the_end end_handling;
            handling_at_restart restart_handling;
        };

        struct Channel{
//            std::string name;
            channel_t channel_type;
            size_t index;
            Channel(channel_t ch_t,  size_t i) : channel_type(ch_t), index(i) {}
        };

        double* time = nullptr;
        double start_time = 0;
        double pause_time = 0;
        uint16_t frame = 0;
        bool already_updated_last = false;
        bool already_updated_act = false;
        uint16_t active_cue = 0;

        uint16_t next_cue = 0xffff;
        bool stop_at_cue_end = false;

        handling_at_the_end handle_end = HOLD;
        run_state running_state = STOP;

        std::vector<Channel> channel;

        // containing the cues, incl, frames and transition types
        std::vector<Cue> cues;

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


        inline bool do_with_substr(const std::string& str, size_t start, const size_t end, const char sep, size_t min_loops, const std::function<bool(const std::string&, size_t, size_t, size_t)> func);

        bool handle_frame(size_t cue, const std::string& str, size_t start, size_t end, size_t nr_channel);

        bool handle_timestamps(size_t cue, const std::string& str, size_t start, size_t end, size_t nr_timestamp);

        bool handle_cue_conf(size_t cue, const std::string& str, size_t start, size_t end, size_t number);

        bool handle_cue(const std::string& str, size_t start, size_t end, size_t cue);

        template <typename T>
        void calc_transition(double rel_time, transition_t transition, T start_value, T end_value, size_t ind);

        void update_hold_values();

        void calc_values();


    public:
        filter_cue() : filter() {}
        virtual ~filter_cue() {}


        virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) override;

        virtual bool receive_update_from_gui(const std::string& key, const std::string& _value) override;

        virtual void get_output_channels(channel_mapping& map, const std::string& name) override;

        virtual void update() override;

        virtual void scene_activated() override;

    };

    COMPILER_RESTORE("-Weffc++")

}