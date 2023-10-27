//
// Created by leondietrich on 10/27/23.
//

#include "filters/filter.hpp"

#include <sstream>

namespace dmxfish::filters {

    std::string compute_cause(const std::string& cause_, const filter_type type, const std::string& filter_id) {
        std::stringstream ss;
        ss << "TYPE:" << ((int) type) << "ID:'" << filter_id << "'REASON:" << cause_;
        return ss.str();
    }

    filter_config_exception::filter_config_exception(const std::string& cause_, const filter_type type, const std::string& filter_id)
        : cause(compute_cause(cause_, type, filter_id)) {}

    filter_runtime_exception::filter_runtime_exception(const std::string& cause_, const filter_type type) : cause(compute_cause(cause_, type, "???")) {}

}