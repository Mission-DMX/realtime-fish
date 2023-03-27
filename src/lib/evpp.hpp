#pragma once
#include "lib/macros.hpp"

//COMPILER_SUPRESS("-Werror")
COMPILER_SUPRESS("-Wunused-parameter")
COMPILER_SUPRESS("-Wshadow")
COMPILER_SUPRESS("-Wundef")
#include <ev++.h>
COMPILER_RESTORE("-Wundef")
COMPILER_RESTORE("-Wshadow")
COMPILER_RESTORE("-Wunused-parameter")
//COMPILER_RESTORE("-Werror")
