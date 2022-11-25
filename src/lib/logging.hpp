#pragma once
#include "lib/macros.hpp"

COMPILER_SUPRESS("-Werror")
COMPILER_SUPRESS("-Wswitch-enum")
#include <spdlog/spdlog.h>
COMPILER_RESTORE("-Wswitch-enum")
COMPILER_RESTORE("-Werror")
