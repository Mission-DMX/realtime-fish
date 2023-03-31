#include "filters/scene_factory.hpp"

#include <list>
#include <map>
#include <set>
#include <utility>

namespace dmxfish::filters {

    [[nodiscard]] inline size_t get_filter_memory_size(const ::MissionDMX::ShowFile::Scene& s) {
        // TODO implement by querying the filters and summing their size_of
        size_t sum = 0;
        return sum;
    }

    inline std::shared_ptr<filter> construct_filter(int type, std::shared_ptr<LinearAllocator> pac) {
		// TODO implement using allocate_shared(pac) and custom deleter
		return nullptr;
	}

	inline std::map<std::string, std::string> convert_configuration(const ::xsd::cxx::tree::sequence<::MissionDMX::ShowFile::KeyValuePair>& xml_conf) {
		std::map<std::string, std::string> kv;
		for(const auto& kve: xml_conf) {
			kv[kve.name()] = kve.value();
		}
		return std::move(kv);
	}

    inline void schedule_filters(const ::MissionDMX::ShowFile::Scene& s, scene_filter_vector_t& fv, scene_boundry_vec_t& b, std::shared_ptr<LinearAllocator> pac, std::map<size_t, std::string>& name_map, std::map<size_t, std::map<std::string, std::string>>& configuration_map) {
		auto missing_filter_stack = std::list<::MissionDMX::ShowFile::Filter>(s.filter());
		std::set<std::string> resolved_filters;
		while(!missing_filter_stack.empty()) {
			bool placed_filter = false;
			for(size_t i = 0; i < missing_filter_stack.size(); i++) {
				auto f_template = missing_filter_stack.pop_front();
				bool all_deps_clear = true;
				for(auto& links: f_template.channellink()) {
					const auto& required_filter = links.output_channel_id();
					all_deps_clear &= resolved_filters.contains(required_filter.substr(0, required_filter.find(":")));
				}
				if(all_deps_clear) {
					placed_filter = true;
					fv.emplace_back(std::move(construct_filter(f_template.type(), pac)));
					const auto filter_index = fv.size() - 1;
					name_map[filter_index] = f_template.id();
					configuration_map[filter_index] = convert_configuration(f_template.filterConfiguration());
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

	inline void connect_filters(const ::MissionDMX::ShowFile::Scene& s, scene_filter_vector_t& fv, std::map<size_t, std::string>& name_map, std::map<size_t, std::map<std::string, std::string>>& conf_map) {
		channel_mapping cm;
		for(size_t i = 0; i < fv.size(); i++) {
			fv[i]->get_output_channels(cm, name_map[i]);
			channel_mapping input_channels;
			// TODO fill input_channels map with data from cm picked by filters channel mapping configuration
			fv[i]->setup_filter(conf_map[i], input_channels);
			// TODO set initial_parameters setting of filter
		}
	};

    [[nodiscard]] inline std::tuple<scene_filter_vector_t, scene_boundry_vec_t, std::shared_ptr<LinearAllocator>> compute_filter(const ::MissionDMX::ShowFile::Scene& s) {
	    auto pac = std::make_shared<LinearAllocator>(get_filter_memory_size(s));
	    scene_filter_vector_t filters;
	    scene_boundry_vec_t boundries;
	    filters.reserve(s.filter().size());
		std::map<size_t, std::string> name_map;
		std::map<size_t, std::map<std::string, std::string>> configuration_map;

	    schedule_filters(s, filters, boundries, pac, name_map, configuration_map);
		connect_filters(s, filters, name_map, configuration_map);

	    return std::move(std::make_tuple(std::move(filters), std::move(boundries), pac));
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
