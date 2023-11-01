#pragma once

#include <string>

namespace dmxfish::execution {
    constexpr auto ERROR_FILTER_NOT_IMPLEMENTED_IN_ALLOCATION = "E001:";
    constexpr auto ERROR_FILTER_NOT_IMPLEMENTED_IN_CONSTRUCTION = "E002:";
    constexpr auto ERROR_FILTER_CONFIGURATION_EXCEPTION = "E003:";
    constexpr auto ERROR_FILTER_SCHEDULING_EXCEPTION = "E004:";
    constexpr auto ERROR_CYCLIC_OR_BROKEN_DEPENDENCY_WHILE_SCHEDULING = "E005:";
    constexpr auto ERROR_UNKNOWN_REQUESTED_OUTPUT_CHANNEL = "E006:";
}
