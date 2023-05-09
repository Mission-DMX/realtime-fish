#pragma once

#include <atomic>
#include <forward_list>
#include <functional>
#include <thread>

#include "executioners/locked_queue.hpp"
#include "lib/logging.hpp"

namespace dmxfish::execution {

    class threadpool {
    private:
        locked_queue<std::function<void()>> tasks;
        locked_queue<std::thread::id> stopped_threads;
        std::forward_list<std::thread> threads;
        std::atomic_uint64_t active_threads;
        const size_t cpu_count;
    public:
        threadpool();

        ~threadpool();

        threadpool(const threadpool &) = delete;
        threadpool &operator=(const threadpool &) = delete;

        [[nodiscard]] inline bool currently_done() {
            return tasks.empty() && active_threads == 0;
        }

        void join();

        void enque(std::function<void()> task);
    private:
        void cleanup();
    };

}
