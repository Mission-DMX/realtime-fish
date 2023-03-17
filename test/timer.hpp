
#pragma once

#include <ev++.h>

#include <functional>
#include <memory>

namespace fish::test  {

class timer : public std::enable_shared_from_this<timer> {
// public:
    // typedef std::shared_ptr<async_server_socket> self_ptr_type;
    //
    // typedef std::function<void(self_ptr_type, const auto_fd &)> accept_handler_type;
    // typedef std::function<void(self_ptr_type)> error_handler_type;

private:
    // auto_fd socket;
    //
    // accept_handler_type on_accept;
    // error_handler_type on_error;

    // ::ev::io io;
    ::ev_timer timeout_watcher;
    std::shared_ptr<::ev::loop_ref> loop;
    static int nr;

public:
    timer(std::shared_ptr<::ev::loop_ref>);
    ~timer();
    void stop();
    void start();

private:
    // void cb_ev(::ev::io &w, int events);
    void cb_ev_new();
    static void timeout_cb (EV_P_ ev_timer *w, int revents);
    // static void timeout_cb (EV_P_ ev_timer *w, int revents);
};

}
