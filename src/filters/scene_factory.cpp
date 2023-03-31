#include "filters/scene_factory.hpp"

#include <utility>

namespace dmxfish::filters {

    [[nodiscard]] inline std::pair<scene_filter_vector_t, scene_boundry_vec_t> compute_filter(const ::MissionDMX::ShowFile::Scene& s) {
	    scene_filter_vector_t filters;
	    scene_boundry_vec_t boundries;
	    // TODO calculate complete space required by filter types and allocate it
	    // TODO perform scheduling algorithm to determine order
	    // TODO iterate over filters to emplace them in the allocated memory,
	    // connect them and fill in the boundry vector
	    return std::move(std::make_pair(std::move(filters), std::move(boundries)));
    }

    void populate_scene_vector(std::vector<scene>& v, const MissionDMX::ShowFile::BordConfiguration::scene_sequence& ss) {
        v.reserve(ss.size());
	for(auto& stemplate : ss) {
		auto filter_pair = compute_filter(stemplate);
		v.emplace_back(std::move(filter_pair.first), std::move(filter_pair.second));
	}
    }

}
