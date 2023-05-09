#pragma once

#include <deque>
#include <mutex>
#include <optional>

namespace dmxfish::execution {
    template <typename T>
    class locked_queue {
        private:
            using mutex_type = std::mutex;
            std::deque<T> data{};
            mutable mutex_type mutex{};
        public:
            using value_type = T;
            using size_type = typename std::deque<T>::size_type;

            locked_queue() = default;

            void push_back(T&& value) {
                std::lock_guard lock(mutex);
                data.push_back(std::forward<T>(value));
            }

            void push_back(T& value) {
                std::lock_guard lock(mutex);
                data.push_back(std::forward<T>(value));
            }

            [[nodiscard]] bool empty() const {
                std::lock_guard lock(mutex);
                return data.empty();
            }

            [[nodiscard]] std::optional<T> pop_front() {
                std::lock_guard lock(mutex);
                if (data.empty()) {
                    return std::nullopt;
                }

                auto front = data.front();
                data.pop_front();
                return front;
            }

            [[nodiscard]] std::optional<T> pop_back() {
                std::lock_guard lock(mutex);
                if (data.empty()) {
                    return std::nullopt;
                }

                auto back = data.back();
                data.pop_back();
                return back;
            }
    };
}
