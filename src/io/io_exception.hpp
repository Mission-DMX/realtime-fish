#pragma once

#include <exception>
#include <string>

namespace dmxfish::io {
    class io_exception : public std::exception {
    private:
        std::string cause;
    public:
        io_exception(const std::string& cause_) : cause(cause_) {}
        virtual const char* what() const throw () {return cause.c_str();}
    };
}
