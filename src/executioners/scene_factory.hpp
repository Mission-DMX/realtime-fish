#pragma once

#include <utility>
#include <vector>

#include "executioners/scene.hpp"
#include "xml/show_files.hpp"

namespace dmxfish::execution {

class scheduling_exception : public std::exception {
private:
    std::string cause;

public:
    scheduling_exception(const std::string cause_) : cause(cause_) {}
    [[nodiscard]] inline virtual const char* what() const throw () {
        return this->cause.c_str();
    }
};

/**
 * This method populates the provided vector with ready-initialized scenes using the provided scene definitions.
 * @param v The vector to populate
 * @param ss The scene sequence to use
 * @returns the collected debug output
 */
[[nodiscard]] std::pair<std::string, bool> populate_scene_vector(std::vector<scene>& v, const MissionDMX::ShowFile::BordConfiguration::scene_sequence& ss, std::map<int32_t, size_t>& scene_index_map);

}
