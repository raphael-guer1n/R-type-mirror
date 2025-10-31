#pragma once
#include <queue>

namespace engine::net
{
    template <typename T>
    class ThreadSafeQueue
    {
        public:
            void push(const T &item)
            {
                std::lock_guard<std::mutex> lock(mutex_);
                queue_.push(item);
            }

            bool pop(T &item)
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (queue_.empty())
                    return false;
                item = std::move(queue_.front());
                queue_.pop();
                return true;
            }

        private:
            std::queue<T> queue_;
            std::mutex mutex_;
    };
}
