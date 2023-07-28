#pragma once

#include <memory>

#include "io/iomanager.hpp"

std::shared_ptr<dmxfish::io::IOManager> get_iomanager_instance();
