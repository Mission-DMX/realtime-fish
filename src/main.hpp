#pragma once

#include <memory>

#include "io/iomanager.hpp"

std::shared_ptr<dmxfish::io::IOManager> get_iomanager_instance();

// only for testing purposes (and used in main_loop(), but only there!)
void construct_iomanager();
void destruct_iomanager();
