#include "executioners/threadpool.hpp"

#include <set>

namespace dmxfish::execution {

    threadpool::threadpool() : tasks{}, stopped_threads{}, threads{}, active_threads{}, cpu_count{std::thread::hardware_concurrency()}  {}

    threadpool::~threadpool() {
        this->join();
	this->cleanup();
    }

    void threadpool::join() {
        while(!tasks.empty()) {
            if(auto t = tasks.pop_front(); t.has_value()) {
                t.value()();
            }
        }
        for(auto& t : threads) {
            if(t.joinable()) {
                t.join();
            }
        }
    }

    void threadpool::enque(std::function<void()> task) {
        tasks.push_back(task);
        if(active_threads < cpu_count) {
            threads.emplace_front([this](){
                while(!this->tasks.empty()) {
                    try {
                        if(auto t = this->tasks.pop_front(); t.has_value()) {
                            t.value()();
                        }
                    } catch (const std::exception& e) {
                        ::spdlog::error("Task in thread pool crashed with error: {}", e.what());
                    }
                }
                this->active_threads--;
                this->stopped_threads.push_back(std::this_thread::get_id());
            });
            active_threads++;
        }
        cleanup();
    }

    void threadpool::cleanup() {
        if(!stopped_threads.empty()) {
            std::set<std::thread::id> s;
            while(!stopped_threads.empty()) {
                if(auto v = stopped_threads.pop_front(); v.has_value()) {
                    s.insert(v.value());
                }
            }
            // Join all threads to be removed to prevent race conditions
	    for (auto& t : threads) {
		if (s.contains(t.get_id())) {
		    t.join();
		}
	    }
	    threads.remove_if([s](const std::thread& t){return s.contains(t.get_id());});
        }
    }

}
