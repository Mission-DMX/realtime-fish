#pragma once

#include <fcntl.h>
#include <memory>

#include "lib/evpp.hpp"

class stdin_watcher : std::enable_shared_from_this<stdin_watcher>
{
public:
    typedef std::function<void(void)> input_callback;
private:
    ::ev::io e_stdin;
    input_callback icb;

public:
    stdin_watcher(input_callback cb) : e_stdin{}, icb(cb) {
        fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
        e_stdin.set<stdin_watcher, &stdin_watcher::cb>(this);
        e_stdin.set(0, ::ev::READ);
        e_stdin.start();
    }
    ~stdin_watcher() {
        e_stdin.stop();
    }

private:
    void cb(::ev::io &w, int events) {
        (void)w;
        (void)events;
        this->icb();
        this->e_stdin.stop();
    }
};
