#pragma once

#include <map>
#include <string>

#include "dmx/pixel.hpp"

namespace dmxfish::filters {

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
	 * This constructor is supposed to fill in the required input data pointers from the provided mapping table
	 * and throw an exception of type channel_mapping_exception if it fails to do so. It can assume its own names
	 * for the lookup as the scene factory is responsible for the correct mappings.
	 *
	 * @param configuration the key value pairs to configure the filter
	 * @param initial_parameters the initial values that should be set after the parent scene got activated
	 * @param input_channels The mapping table the UI provided
	 */
	virtual void setup_filter(const std::map<std::string, std::string>& configuration, const std::map<std::string, std::string>& initial_parameters, const channel_mapping& input_channels) = 0;

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
};
}
