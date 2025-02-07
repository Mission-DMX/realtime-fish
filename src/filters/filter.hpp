#pragma once

#include <map>
#include <string>

#include "dmx/pixel.hpp"
#include "filters/types.hpp"

#include "lib/macros.hpp"


namespace dmxfish::filters {

class filter_config_exception : public std::exception {
private:
    const std::string cause;

public:
    filter_config_exception(const std::string& cause_, const filter_type type, const std::string& filter_id);

    [[nodiscard]] inline const char* what() const noexcept override {
        return this->cause.c_str();
    }
};

class filter_runtime_exception : public std::exception {
private:
    const std::string cause;

public:
    filter_runtime_exception(const std::string& cause_, const filter_type type);

    [[nodiscard]] inline const char* what() const noexcept override {
        return this->cause.c_str();
    }
};

struct channel_mapping {
public:
	std::map<std::string, uint8_t*> eight_bit_channels;
	std::map<std::string, uint16_t*> sixteen_bit_channels;
	std::map<std::string, double*> float_channels;
	std::map<std::string, dmxfish::dmx::pixel*> color_channels;

	channel_mapping() : eight_bit_channels{}, sixteen_bit_channels{}, float_channels{}, color_channels{} {};
	channel_mapping(const channel_mapping&) = default;
	channel_mapping(channel_mapping&&) = default;
};

class filter {
public:

	filter() {}
	virtual ~filter() {}

	/**
	 * This method is called prior to calling get_output_channels. It is meant in case a filter needs to dynamically construct the output channels based
	 * on the configuration.
	 *
	 * @param configuration the key value pairs to configure the filter
	 * @param initial_parameters the initial values that should be set after the parent scene got activated
	 * @param filter_id The own ID of the filter
	 */
	virtual void pre_setup(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const std::string& filter_id) {
        MARK_UNUSED(initial_parameters);
        MARK_UNUSED(configuration);
        MARK_UNUSED(filter_id);
    }

	/**
	 * This constructor is supposed to fill in the required input data pointers from the provided mapping table
	 * and throw an exception of type channel_mapping_exception if it fails to do so. It can assume its own names
	 * for the lookup as the scene factory is responsible for the correct mappings.
	 *
	 * @param configuration the key value pairs to configure the filter
	 * @param initial_parameters the initial values that should be set after the parent scene got activated
	 * @param input_channels The mapping table the UI provided
	 * @param filter_id The own id of the filter
	 */
	virtual void setup_filter(const std::map<std::string, std::string>& configuration,
                              const std::map<std::string, std::string>& initial_parameters,
                              const channel_mapping& input_channels,
                              const std::string& filter_id) = 0;

	/**
	 * This method will be called in order to get the output channels. It is supposed
	 * to fill in the required maps with the avaiable pointers of the result storage.
	 * @param map The global channel mapping
	 * @param name The prefix that this channel should use
	 */
	virtual void get_output_channels(channel_mapping& map, const std::string& name) = 0;

	/**
	 * This method gets called once every update cycle and should be used to calculate the values.
	 */
	virtual void update() = 0;

	/**
	 * This method gets called when the scene got activated.
	 */
	virtual void scene_activated() = 0;

    /**
     * This method may be used to store any state as it gets called prior to scene switches.
     */
    virtual void scene_deactivated() {}

	/**
	 * This method gets called if the GUI published a parameter update.
	 * @param key the parameter to change
	 * @param value the new value to set
	 * @return true if the update was successful
	 */
	virtual bool receive_update_from_gui(const std::string& key, const std::string& value) {
		MARK_UNUSED(key);
		MARK_UNUSED(value);
		return false;
	}
};

}
