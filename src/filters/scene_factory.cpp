#include "filters/scene_factory.hpp"

#include <utility>

namespace dmxfish::filters {

    [[nodiscard]] inline size_t get_filter_memory_size(const ::MissionDMX::ShowFile::Scene& s) {
        // TODO implement by querying the filters and summing their size_of
        size_t sum = 0;
        return sum;
    }

    [[nodiscard]] inline std::tuple<scene_filter_vector_t, scene_boundry_vec_t, std::shared_ptr<LinearAllocator>> compute_filter(const ::MissionDMX::ShowFile::Scene& s) {
	    auto pac = std::make_shared<LinearAllocator>(get_filter_memory_size(s));
	    scene_filter_vector_t filters;
	    scene_boundry_vec_t boundries;
	    filters.reserve(s.filter().size());
	    // TODO perform scheduling algorithm to determine order
	    // TODO iterate over filters to emplace them in the allocated memory (allocate_shared),
	    // connect them and fill in the boundry vector
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
