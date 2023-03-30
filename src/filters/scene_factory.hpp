#pragma once

#include <vector>

#include "filters/scene.hpp"
#include "xml/show_files.hpp"

namespace dmxfish::filters {

    /**
     * This method populates the provided vector with ready-initialized scenes using the provided scene definitions.
     * @param v The vector to populate
     * @param ss The scene sequence to use
     */
    void populate_scene_vector(std::vector<scene>& v, const MissionDMX::ShowFile::BordConfiguration::scene_sequence& ss);

}
