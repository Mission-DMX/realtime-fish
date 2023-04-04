#include "executioners/scene_factory.hpp"

#include <deque>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace dmxfish::execution {

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
		//filter_info(const filter_info&) = default;
		//filter_info(filter_info&&) = default;
	};

    [[nodiscard]] inline size_t get_filter_memory_size(const ::MissionDMX::ShowFile::Scene& s) {
        size_t sum = 0;
		for(const auto& f : s.filter()) {
			// TODO add filters by checking for their type and adding the corresponding sizeof to sum
			throw scheduling_exception("The requested filter type is not yet implemented.");
		}
        return sum;
    }

    [[nodiscard]] inline std::shared_ptr<filter> construct_filter(int type, std::shared_ptr<LinearAllocator> pac) {
		// TODO implement using allocate_shared(pac) and custom deleter
		throw scheduling_exception("The requested filter type is not yet implemented.");
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

    inline void schedule_filters(const ::MissionDMX::ShowFile::Scene& s, scene_filter_vector_t& fv, scene_boundry_vec_t& b, std::shared_ptr<LinearAllocator> pac, std::map<size_t, filter_info>& filter_info_map) {
		auto missing_filter_stack = enque_filters(s.filter());
		std::set<std::string> resolved_filters;
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
					filter_info_map[filter_index] = filter_info(f_template.id(), convert_configuration(f_template.filterConfiguration()), convert_configuration(f_template.initialParameters()), convert_channel_mapping(f_template.channellink()));
				} else {
					missing_filter_stack.push_back(f_template);
				}
			}
			if(!placed_filter) {
				throw scheduling_exception("There were no filters with resolved dependencies within this round. Possible causes: broken or cyclic dependencies.");
			}
			b.emplace_back(fv.size());
		}
	}

	[[nodiscard]] inline channel_mapping construct_channel_input_mapping(channel_mapping& global_cm, const filter_info& i) {
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
				throw scheduling_exception("The filter " + i.name + " requested the output channel " + entry.first + " to populate its input channel " + entry.second + " but the corresponding output channel wasn't found.");
			}
		}
		return configuration_cm;
	}

	inline void connect_filters(scene_filter_vector_t& fv, std::map<size_t, filter_info>& filter_info_map) {
		channel_mapping cm;
		for(size_t i = 0; i < fv.size(); i++) {
			const auto& finfo = filter_info_map[i];
			fv[i]->get_output_channels(cm, finfo.name);
			channel_mapping input_channels = construct_channel_input_mapping(cm, finfo);
			fv[i]->setup_filter(finfo.configuration, finfo.initial_parameters, input_channels);
		}
	};

    [[nodiscard]] inline std::tuple<scene_filter_vector_t, scene_boundry_vec_t, std::shared_ptr<LinearAllocator>> compute_filter(const ::MissionDMX::ShowFile::Scene& s) {
	    auto pac = std::make_shared<LinearAllocator>(get_filter_memory_size(s));
	    scene_filter_vector_t filters;
	    scene_boundry_vec_t boundries;
	    filters.reserve(s.filter().size());
	    std::map<size_t, filter_info> filter_info_map;

	    schedule_filters(s, filters, boundries, pac, filter_info_map);
	    connect_filters(filters, filter_info_map);

	    return std::make_tuple(filters, boundries, pac);
    }

    void populate_scene_vector(std::vector<scene>& v, const MissionDMX::ShowFile::BordConfiguration::scene_sequence& ss) {
        v.reserve(ss.size());
		for(auto& stemplate : ss) {
			auto filter_tuple = compute_filter(stemplate);
			v.emplace_back(std::move(std::get<0>(filter_tuple)),
				std::move(std::get<1>(filter_tuple)),
				std::get<2>(filter_tuple));
		}
    }

}
