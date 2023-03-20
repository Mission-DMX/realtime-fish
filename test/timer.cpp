#include "../test/timer.hpp"
#include <ev++.h>
#include "lib/logging.hpp"

// #include <utility>

namespace fish::test {

int fish::test::timer::nr = 0;

timer::timer(std::shared_ptr<::ev::loop_ref> _loop) : loop(_loop)
     {

}

timer::~timer() {
    // Remove this socket from libev ...
    // io.stop();
}


void timer::start(){
  ev_timer_init (&timeout_watcher, timeout_cb, 0, 0.);
  ev_timer_start (*loop.get(), &timeout_watcher);
}

void timer::stop(){
  ev_timer_stop(*loop.get(), &timeout_watcher);
}

void timer::timeout_cb (EV_P_ ev_timer *w, int revents)
{
	::spdlog::debug("again one minute gone: {0:d}, {1:d};.", (nr), revents);

  // if(nr<5){
    // ev_timer_stop (loop, w);
    // ev_timer_set (w, 2., 0.);
    // ev_timer_start (loop, w);
    w->repeat = 2.;
    ev_timer_again (loop, w);
    // ev_break (EV_A_ EVBREAK_ONE);
  // }
  // else {
  //   ev_timer_stop (loop, w);
  //   ev_break (EV_A_ EVBREAK_ONE);
  // }
  nr++;


}
}
