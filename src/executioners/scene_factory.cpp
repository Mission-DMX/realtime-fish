#include "executioners/scene_factory.hpp"

#include <deque>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

#include "executioners/threadpool.hpp"
#include "executioners/scheduling_errors.hpp"

#include "filters/types.hpp"
#include "filters/filter_constants.hpp"
#include "filters/filter_conversion.hpp"
#include "filters/filter_debug_output.hpp"
#include "filters/filter_fader.hpp"
#include "filters/filter_universe_output.hpp"
#include "filters/filter_trigonometric.hpp"
#include "filters/filter_math.hpp"
#include "filters/filter_time.hpp"
#include "filters/filter_cue.hpp"
#include "filters/filter_shift.hpp"
#include "filters/filter_lua_script.hpp"

#include <iostream>

namespace dmxfish::execution {

COMPILER_SUPRESS("-Weffc++")
	class ZeroDeletingLinearAllocator : public ::LinearAllocator {
	public:
		ZeroDeletingLinearAllocator(const std::size_t totalSize) : LinearAllocator(totalSize) {
			this->Init();
		}
		virtual void Free(void* ptr) override {
			// Do nothing as all memory will be deallocated soon anyway.
			// Basically the same destructor that would delete the filters
			// will also delete this allocator.
			MARK_UNUSED(ptr);
		}

		typedef ::dmxfish::filters::filter value_type;
		::dmxfish::filters::filter* allocate (std::size_t n) {
			return reinterpret_cast<::dmxfish::filters::filter*>(this->Allocate(n));
		}
		void deallocate (::dmxfish::filters::filter* p, std::size_t n) {
			MARK_UNUSED(n);
			this->Free((void*) p);
		}
	};
COMPILER_RESTORE("-Weffc++")

	struct filter_info {
		std::string name;
		std::map<std::string, std::string> configuration, initial_parameters;
		std::vector<std::pair<std::string, std::string>> channel_mapping;

		filter_info() : name{}, configuration{}, initial_parameters{}, channel_mapping{} {};
		filter_info(std::string _name,
					std::map<std::string, std::string> _configuration,
					std::map<std::string, std::string> _initial_parameters,
					std::vector<std::pair<std::string, std::string>> _channel_mapping
		) : name(_name), configuration(_configuration), initial_parameters(_initial_parameters), channel_mapping(_channel_mapping) {}

		[[nodiscard]] inline std::string str() const {
			std::stringstream ss;
			ss << "{\"name\": \"" << this->name << "\", \"conf\": {";
			for(const auto& [k, v] : this->configuration) {
				ss << "\"" << k << "\": \"" << v << "\",";
			}
			ss << "}, \"initialParameters\": {";
			for(const auto& [k, v] : this->initial_parameters) {
				ss << "\"" << k << "\": \"" << v << "\",";
			}
			ss << "} \"channel_mappings\": [";
			for(const auto& cp : this->channel_mapping) {
				ss << cp.first << " -> " << cp.second;
			}
			ss << "]}";
			return ss.str();
		}
	};

    [[nodiscard]] inline size_t get_filter_memory_size(const ::MissionDMX::ShowFile::Scene& s) {
        using namespace ::dmxfish::filters;
        size_t sum = 0;
		for(const auto& f : s.filter()) {
			// add filters by checking for their type and adding the corresponding sizeof to sum
			switch(static_cast<filter_type>(f.type())) {
				case filter_type::constants_8bit:
					sum += sizeof(constant_8bit);
					break;
				case filter_type::constants_16bit:
					sum += sizeof(constant_16bit);
					break;
				case filter_type::constants_float:
					sum += sizeof(constant_float);
					break;
				case filter_type::constants_pixel:
					sum += sizeof(constant_color);
					break;
				case filter_type::debug_8bit:
					sum += sizeof(debug_8bit);
					break;
				case filter_type::debug_16bit:
					sum += sizeof(debug_16bit);
					break;
				case filter_type::debug_float:
					sum += sizeof(debug_float);
					break;
				case filter_type::debug_pixel:
					sum += sizeof(debug_pixel);
					break;
				case filter_type::filter_16bit_to_dual_byte:
					sum += sizeof(filter_16bit_to_dual_byte);
					break;
				case filter_type::filter_16bit_to_bool:
					sum += sizeof(filter_16bit_to_bool);
					break;
				case filter_type::filter_multiply_add:
					sum += sizeof(filter_multiply_add);
					break;
				case filter_type::filter_universe_output:
					sum += sizeof(filter_universe_output);
					break;
				case filter_type::filter_float_to_16bit:
					sum += sizeof(filter_float_to_16bit);
					break;
				case filter_type::filter_float_to_8bit:
					sum += sizeof(filter_float_to_8bit);
					break;
				case filter_type::filter_round_number:
					sum += sizeof(filter_round_number);
					break;
				case filter_type::filter_pixel_to_rgb_channels:
					sum += sizeof(filter_pixel_to_rgb_channels);
					break;
                case filter_type::filter_pixel_to_rgbw_channels:
                    sum += sizeof(filter_pixel_to_rgbw_channels);
                    break;
                case filter_type::filter_floats_to_pixel:
                    sum += sizeof(filter_floats_to_pixel);
                    break;
                case filter_type::filter_sine:
                    sum += sizeof(filter_sine);
                    break;
                case filter_type::filter_cosine:
                    sum += sizeof(filter_cosine);
                    break;
                case filter_type::filter_tangent:
                    sum += sizeof(filter_tangent);
                    break;
                case filter_type::filter_arcsine:
                    sum += sizeof(filter_arcsine);
                    break;
                case filter_type::filter_arccosine:
                    sum += sizeof(filter_arccosine);
                    break;
                case filter_type::filter_arctangent:
                    sum += sizeof(filter_arctangent);
                    break;
                case filter_type::filter_logarithm:
                    sum += sizeof(filter_logarithm);
                    break;
                case filter_type::filter_exponential:
                    sum += sizeof(filter_exponential);
                    break;
                case filter_type::filter_minimum:
                    sum += sizeof(filter_minimum);
                    break;
                case filter_type::filter_maximum:
                    sum += sizeof(filter_maximum);
                    break;
                case filter_type::filter_square:
                    sum += sizeof(filter_square);
                    break;
                case filter_type::filter_triangle:
                    sum += sizeof(filter_triangle);
                    break;
                case filter_type::filter_sawtooth:
                    sum += sizeof(filter_sawtooth);
                    break;
                case filter_type::filter_time:
                    sum += sizeof(filter_time);
                    break;
                case filter_type::delay_switch_on_8bit:
                    sum += sizeof(delay_switch_on_8bit);
                    break;
                case filter_type::delay_switch_on_16bit:
                    sum += sizeof(delay_switch_on_16bit);
                    break;
                case filter_type::delay_switch_on_float:
                    sum += sizeof(delay_switch_on_float);
                    break;
                case filter_type::delay_switch_off_8bit:
                    sum += sizeof(delay_switch_off_8bit);
                    break;
                case filter_type::delay_switch_off_16bit:
                    sum += sizeof(delay_switch_off_16bit);
                    break;
                case filter_type::delay_switch_off_float:
                    sum += sizeof(delay_switch_off_float);
                    break;
				case filter_type::filter_fader_column_raw:
					sum += sizeof(filter_fader_column_raw);
					break;
				case filter_type::filter_fader_column_hsi:
					sum += sizeof(filter_fader_column_hsi);
					break;
				case filter_type::filter_fader_column_hsia:
					sum += sizeof(filter_fader_column_hsia);
					break;
				case filter_type::filter_fader_column_hsiu:
					sum += sizeof(filter_fader_column_hsiu);
					break;
				case filter_type::filter_fader_column_hsiau:
					sum += sizeof(filter_fader_column_hsiau);
					break;
                case filter_type::filter_cue:
                    sum += sizeof(filter_cue);
                    break;
                case filter_type::filter_shift_8bit:
                    sum += sizeof(filter_shift_8bit);
                    break;
                case filter_type::filter_shift_16bit:
                    sum += sizeof(filter_shift_16bit);
                    break;
                case filter_type::filter_shift_float:
                    sum += sizeof(filter_shift_float);
                    break;
                case filter_type::filter_main_brightness_fader:
                    sum += sizeof(filter_main_brightness_fader);
                    break;
                case filter_type::filter_shift_color:
                    sum += sizeof(filter_shift_color);
                    break;
                case filter_type::filter_lua_script:
                    sum += sizeof(filter_lua_script);
                    break;
                case filter_type::filter_8bit_to_float:
                    sum += sizeof(filter_8bit_to_float);
                    break;
                case filter_type::filter_16bit_to_float:
                    sum += sizeof(filter_16bit_to_float);
                    break;
                case filter_type::filter_pixel_to_floats:
                    sum += sizeof(filter_pixel_to_floats);
                    break;
                case filter_type::filter_float_map_range_8bit:
                    sum += sizeof(filter_float_map_range_8bit);
                    break;
                case filter_type::filter_float_map_range_16bit:
                    sum += sizeof(filter_float_map_range_16bit);
                    break;
                case filter_type::filter_float_map_range_float:
                    sum += sizeof(filter_float_map_range_float);
                    break;
                case filter_type::filter_combine_bytes_to_16bit:
                    sum += sizeof(filter_combine_bytes_to_16bit);
                    break;
                case filter_type::filter_map_8bit_to_16bit:
                    sum += sizeof(filter_map_8bit_to_16bit);
                    break;
                case filter_type::filter_debug_remote_8bit:
                    sum += sizeof(debug_remote_8bit);
                    break;
                case filter_type::filter_debug_remote_16bit:
                    sum += sizeof(debug_remote_8bit);
                    break;
                case filter_type::filter_debug_remote_float:
                    sum += sizeof(debug_remote_float);
                    break;
                case filter_type::filter_debug_remote_pixel:
                    sum += sizeof(debug_remote_pixel);
                    break;
				default: {
						 std::stringstream ss;
						 ss << ERROR_FILTER_NOT_IMPLEMENTED_IN_ALLOCATION;
						 ss << "The requested filter type (";
						 ss << (int) f.type();
						 ss << ") is not yet implemented.";
					throw scheduling_exception(ss.str());
					 }
			}
		}
        return sum;
    }

    struct filter_deleter {
        void operator()(dmxfish::filters::filter* obj) {
	    MARK_UNUSED(obj);
	    // Do nothing as we're using linear allocators
        }
    };

    template<class T>
    [[nodiscard]] inline std::shared_ptr<T> calloc(std::shared_ptr<ZeroDeletingLinearAllocator> pac) {
	    const auto ptr = pac->Allocate(sizeof(T));
	    return std::shared_ptr<T>(new(ptr) T(), filter_deleter{});
    }

    [[nodiscard]] inline std::shared_ptr<dmxfish::filters::filter> construct_filter(int type, std::shared_ptr<ZeroDeletingLinearAllocator> pac) {
        using namespace ::dmxfish::filters;
		// implement using allocate_shared(pac)
		switch(static_cast<filter_type>(type)) {
			case filter_type::constants_8bit:
				return calloc<constant_8bit>(pac);
			case filter_type::constants_16bit:
				return calloc<constant_16bit>(pac);
			case filter_type::constants_float:
				return calloc<constant_float>(pac);
			case filter_type::constants_pixel:
				return calloc<constant_color>(pac);
			case filter_type::debug_8bit:
				return calloc<debug_8bit>(pac);
			case filter_type::debug_16bit:
				return calloc<debug_16bit>(pac);
			case filter_type::debug_float:
				return calloc<debug_float>(pac);
			case filter_type::debug_pixel:
				return calloc<debug_pixel>(pac);
			case filter_type::filter_16bit_to_dual_byte:
				return calloc<filter_16bit_to_dual_byte>(pac);
			case filter_type::filter_16bit_to_bool:
				return calloc<filter_16bit_to_bool>(pac);
			case filter_type::filter_multiply_add:
				return calloc<filter_multiply_add>(pac);
			case filter_type::filter_universe_output:
				return calloc<filter_universe_output>(pac);
			case filter_type::filter_float_to_16bit:
				return calloc<filter_float_to_16bit>(pac);
			case filter_type::filter_float_to_8bit:
				return calloc<filter_float_to_8bit>(pac);
			case filter_type::filter_round_number:
				return calloc<filter_round_number>(pac);
			case filter_type::filter_pixel_to_rgb_channels:
				return calloc<filter_pixel_to_rgb_channels>(pac);
			case filter_type::filter_pixel_to_rgbw_channels:
				return calloc<filter_pixel_to_rgbw_channels>(pac);
            case filter_type::filter_floats_to_pixel:
                return calloc<filter_floats_to_pixel>(pac);
            case filter_type::filter_sine:
                return calloc<filter_sine>(pac);
            case filter_type::filter_cosine:
                return calloc<filter_cosine>(pac);
            case filter_type::filter_tangent:
                return calloc<filter_tangent>(pac);
            case filter_type::filter_arcsine:
                return calloc<filter_arcsine>(pac);
            case filter_type::filter_arccosine:
                return calloc<filter_arccosine>(pac);
            case filter_type::filter_arctangent:
                return calloc<filter_arctangent>(pac);
            case filter_type::filter_logarithm:
                return calloc<filter_logarithm>(pac);
            case filter_type::filter_exponential:
                return calloc<filter_exponential>(pac);
            case filter_type::filter_minimum:
                return calloc<filter_minimum>(pac);
            case filter_type::filter_maximum:
                return calloc<filter_maximum>(pac);
            case filter_type::filter_square:
                return calloc<filter_square>(pac);
            case filter_type::filter_triangle:
                return calloc<filter_triangle>(pac);
            case filter_type::filter_sawtooth:
                return calloc<filter_sawtooth>(pac);
            case filter_type::filter_time:
                return calloc<filter_time>(pac);
            case filter_type::delay_switch_on_8bit:
                return calloc<delay_switch_on_8bit>(pac);
            case filter_type::delay_switch_on_16bit:
                return calloc<delay_switch_on_16bit>(pac);
            case filter_type::delay_switch_on_float:
                return calloc<delay_switch_on_float>(pac);
            case filter_type::delay_switch_off_8bit:
                return calloc<delay_switch_off_8bit>(pac);
            case filter_type::delay_switch_off_16bit:
                return calloc<delay_switch_off_16bit>(pac);
            case filter_type::delay_switch_off_float:
                return calloc<delay_switch_off_float>(pac);
			case filter_type::filter_fader_column_raw:
				return calloc<filter_fader_column_raw>(pac);
			case filter_type::filter_fader_column_hsi:
				return calloc<filter_fader_column_hsi>(pac);
			case filter_type::filter_fader_column_hsia:
				return calloc<filter_fader_column_hsia>(pac);
			case filter_type::filter_fader_column_hsiu:
				return calloc<filter_fader_column_hsiu>(pac);
			case filter_type::filter_fader_column_hsiau:
				return calloc<filter_fader_column_hsiau>(pac);
            case filter_type::filter_cue:
                return calloc<filter_cue>(pac);
            case filter_type::filter_shift_8bit:
                return calloc<filter_shift_8bit>(pac);
            case filter_type::filter_shift_16bit:
                return calloc<filter_shift_16bit>(pac);
            case filter_type::filter_shift_float:
                return calloc<filter_shift_float>(pac);
            case filter_type::filter_main_brightness_fader:
                return calloc<filter_main_brightness_fader>(pac);
            case filter_type::filter_shift_color:
                return calloc<filter_shift_color>(pac);
            case filter_type::filter_lua_script:
                return calloc<filter_lua_script>(pac);
            case filter_type::filter_8bit_to_float:
                return calloc<filter_8bit_to_float>(pac);
            case filter_type::filter_16bit_to_float:
                return calloc<filter_16bit_to_float>(pac);
            case filter_type::filter_pixel_to_floats:
                return calloc<filter_pixel_to_floats>(pac);
            case filter_type::filter_float_map_range_8bit:
                return calloc<filter_float_map_range_8bit>(pac);
            case filter_type::filter_float_map_range_16bit:
                return calloc<filter_float_map_range_16bit>(pac);
            case filter_type::filter_float_map_range_float:
                return calloc<filter_float_map_range_float>(pac);
            case filter_type::filter_combine_bytes_to_16bit:
                return calloc<filter_combine_bytes_to_16bit>(pac);
            case filter_type::filter_map_8bit_to_16bit:
                return calloc<filter_map_8bit_to_16bit>(pac);
            case filter_type::filter_debug_remote_8bit:
                return calloc<debug_remote_8bit>(pac);
            case filter_type::filter_debug_remote_16bit:
                return calloc<debug_remote_16bit>(pac);
            case filter_type::filter_debug_remote_float:
                return calloc<debug_remote_float>(pac);
            case filter_type::filter_debug_remote_pixel:
                return calloc<debug_remote_pixel>(pac);
	default:
		throw scheduling_exception(std::string(ERROR_FILTER_NOT_IMPLEMENTED_IN_CONSTRUCTION) + "Failed to construct filter. The requested filter type (" + std::to_string(type) + ") is not yet implemented.");
		}
		return nullptr;
	}

	[[nodiscard]] inline std::map<std::string, std::string> convert_configuration(const ::xsd::cxx::tree::sequence<::MissionDMX::ShowFile::KeyValuePair>& xml_conf) {
		std::map<std::string, std::string> kv;
		for(const auto& kve: xml_conf) {
			kv[kve.name()] = kve.value();
		}
		return kv;
	}

	[[nodiscard]] inline std::vector<std::pair<std::string, std::string>> convert_channel_mapping(const ::MissionDMX::ShowFile::Filter::channellink_sequence& xml_conf) {
		std::vector<std::pair<std::string, std::string>> cvec;
		cvec.reserve(xml_conf.size());
		for(const auto& entry : xml_conf) {
			cvec.emplace_back(std::make_pair(entry.output_channel_id(), entry.input_channel_id()));
		}
		return cvec;
	}

	[[nodiscard]] inline std::deque<::MissionDMX::ShowFile::Filter> enque_filters(const ::MissionDMX::ShowFile::Scene::filter_sequence& fs) {
		std::deque<::MissionDMX::ShowFile::Filter> fq;
		for(const auto& filter_template : fs) {
			fq.emplace_back(filter_template);
		}
		return fq;
	}

	template<class T>
	[[nodiscard]] std::string iteratable_to_string(const T& s) {
		std::stringstream ss;
		bool first = true;
		for(const auto& element : s) {
			if(!first) {
				ss << ", ";
			} else {
				first = false;
			}
			if constexpr (std::is_same<T, std::deque<::MissionDMX::ShowFile::Filter>>::value) {
				ss << element.id();
			} else {
				ss << element;
			}
		}
		return ss.str();
	}

    inline void schedule_filters(const ::MissionDMX::ShowFile::Scene& s, scene_filter_vector_t& fv, scene_boundry_vec_t& b, std::shared_ptr<ZeroDeletingLinearAllocator> pac, std::map<size_t, filter_info>& filter_info_map, std::stringstream& msg_stream, scene_filter_index_t& scene_filter_index) {
		auto missing_filter_stack = enque_filters(s.filter());
		std::set<std::string> resolved_filters{};
		size_t round = 0;
		while(!missing_filter_stack.empty()) {
			bool placed_filter = false;
			for(size_t i = 0; i < missing_filter_stack.size(); i++) {
				auto f_template = missing_filter_stack.front();
				missing_filter_stack.pop_front();
				bool all_deps_clear = true;
				for(auto& links : f_template.channellink()) {
					const auto& required_filter = links.output_channel_id();
					all_deps_clear &= resolved_filters.contains(required_filter.substr(0, required_filter.find(":")));
				}
				if(all_deps_clear) {
					placed_filter = true;
					fv.emplace_back(std::move(construct_filter(f_template.type(), pac)));
					const auto filter_index = fv.size() - 1;
					const auto& fid = f_template.id();
					const auto conf = convert_configuration(f_template.filterConfiguration());
					const auto initial_params = convert_configuration(f_template.initialParameters());
					const auto mapping = convert_channel_mapping(f_template.channellink());
					const filter_info fi(fid, conf, initial_params, mapping);
					msg_stream << "Loading configuration of filter " << filter_index << " of type " << fid << " with config: " << fi.str() << ". Scheduled." << std::endl;
					filter_info_map[filter_index] = fi;
					scene_filter_index[fid] = fv[filter_index];
					resolved_filters.insert(fid);
					fv[fv.size()-1]->pre_setup(conf, initial_params, fi.name);
				} else {
					missing_filter_stack.push_back(f_template);
				}
			}
			if(!placed_filter) {
				std::stringstream exc_ss;
				exc_ss << std::string(ERROR_CYCLIC_OR_BROKEN_DEPENDENCY_WHILE_SCHEDULING);
				exc_ss << "There were no filters with resolved dependencies within this round (";
				exc_ss << std::to_string(round);
				exc_ss << "). Possible causes: broken or cyclic dependencies.\nAlready scheduled filters: ";
				exc_ss << iteratable_to_string(resolved_filters);
				exc_ss << "\nStill missing filters: ";
				exc_ss << iteratable_to_string(missing_filter_stack);
				for (const auto& f : missing_filter_stack) {
					exc_ss << "\n" << f.id() << ":\n";
					for(const auto& link : f.channellink()) {
						const auto& dep_filter = link.output_channel_id();
						if (!resolved_filters.contains(dep_filter.substr(0, dep_filter.find(":")))) {
							exc_ss << '\t' << dep_filter << '\n';
						}
					}
				}
				throw scheduling_exception(exc_ss.str());
			}
			b.emplace_back(fv.size());
			msg_stream << "Next round." << std::endl;
			round++;
		}
		msg_stream << "Finished scheduling " << resolved_filters.size() << " filters in " << round << " rounds." << std::endl;
	}

	[[nodiscard]] inline dmxfish::filters::channel_mapping construct_channel_input_mapping(dmxfish::filters::channel_mapping& global_cm, const filter_info& i) {
		using namespace dmxfish::filters;
		channel_mapping configuration_cm;
		for(const auto& entry : i.channel_mapping) {
			bool found = false;
			if(global_cm.eight_bit_channels.contains(entry.first)) {
				configuration_cm.eight_bit_channels[entry.second] = global_cm.eight_bit_channels[entry.first];
				found = true;
			}
			if(global_cm.sixteen_bit_channels.contains(entry.first)) {
				configuration_cm.sixteen_bit_channels[entry.second] = global_cm.sixteen_bit_channels[entry.first];
				found = true;
			}
			if(global_cm.float_channels.contains(entry.first)) {
				configuration_cm.float_channels[entry.second] = global_cm.float_channels[entry.first];
				found = true;
			}
			if(global_cm.color_channels.contains(entry.first)) {
				configuration_cm.color_channels[entry.second] = global_cm.color_channels[entry.first];
				found = true;
			}
			if(!found) {
				throw scheduling_exception(std::string(ERROR_UNKNOWN_REQUESTED_OUTPUT_CHANNEL) + "The filter '" + i.name + "' requested the output channel '" + entry.first + "' to populate its input channel '" + entry.second + "' but the corresponding output channel wasn't found.");
			}
		}
		return configuration_cm;
	}

	inline void connect_filters(scene_filter_vector_t& fv, std::map<size_t, filter_info>& filter_info_map) {
		dmxfish::filters::channel_mapping cm;
		for(size_t i = 0; i < fv.size(); i++) {
			auto& finfo = filter_info_map[i];
			fv[i]->get_output_channels(cm, finfo.name);
			auto input_channels = construct_channel_input_mapping(cm, finfo);
			fv[i]->setup_filter(finfo.configuration, finfo.initial_parameters, input_channels, finfo.name);
		}
	}

    [[nodiscard]] inline std::tuple<scene_filter_vector_t, scene_boundry_vec_t, std::shared_ptr<ZeroDeletingLinearAllocator>, scene_filter_index_t> compute_filter(const ::MissionDMX::ShowFile::Scene& s, std::stringstream& msg_stream) {
	    auto pac = std::make_shared<ZeroDeletingLinearAllocator>(get_filter_memory_size(s));
	    scene_filter_vector_t filters;
	    scene_boundry_vec_t boundries;
		scene_filter_index_t filter_index;
	    filters.reserve(s.filter().size());
	    std::map<size_t, filter_info> filter_info_map;

	    schedule_filters(s, filters, boundries, pac, filter_info_map, msg_stream, filter_index);
	    connect_filters(filters, filter_info_map);

	    return std::make_tuple(filters, boundries, pac, filter_index);
    }

    [[nodiscard]] std::pair<std::string, bool> populate_scene_vector(std::vector<scene>& v, const MissionDMX::ShowFile::BordConfiguration::scene_sequence& ss, std::map<int32_t, size_t>& scene_index_map) {
		if(ss.size() == 0) {
			return std::make_pair("There were no scenes defined. Skipping.", false);
		}
        v.reserve(ss.size());
		std::stringstream global_msg_stream;
		std::shared_ptr<bool> worked = std::make_shared<bool>(true);

		threadpool tp;
		std::mutex msg_stream_mutex{}, v_mutex{};

		for(auto& stemplate : ss) {
			tp.enque([&global_msg_stream, &v, &scene_index_map, &msg_stream_mutex, &v_mutex, stemplate, worked](){
				std::stringstream msg_stream;
				try {
					auto filter_tuple = compute_filter(stemplate, msg_stream);
					scene s{std::move(std::get<0>(filter_tuple)),
							std::move(std::get<1>(filter_tuple)),
							std::get<2>(filter_tuple),
							std::get<3>(filter_tuple)
						};
					{
						std::lock_guard lock(v_mutex);
						v.push_back(std::move(s));
						const auto last_index = v.size() - 1;
						const int32_t sid = (int32_t) (stemplate.id().present() ? stemplate.id().get() : last_index);
						scene_index_map[sid] = last_index;
					}
				} catch (const ::dmxfish::filters::filter_config_exception& e) {
					msg_stream << ERROR_FILTER_CONFIGURATION_EXCEPTION << "SCENE_ID:" << stemplate.id() << "/Failed to configure filters in scene '" << stemplate.human_readable_name() << "'. Reason: " << e.what() << std::endl;
					*worked = false;
				} catch (const scheduling_exception& e) {
					msg_stream << ERROR_FILTER_SCHEDULING_EXCEPTION << "SCENE_ID:" << stemplate.id() << "Failed to schedule filters in scene '" << stemplate.human_readable_name() << "'. Reason: " << e.what() << std::endl;
					*worked = false;
				}
				{
					std::lock_guard lock(msg_stream_mutex);
					global_msg_stream << msg_stream.str();
				}
			});
		}
		tp.join();
		global_msg_stream << "Done." << std::endl;
		return std::make_pair(global_msg_stream.str(), *worked);
    }

}
