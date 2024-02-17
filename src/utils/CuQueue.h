#pragma once

#include "libcu.h"

template <typename T>
class CuQueue {
    public:
        CuQueue(size_t maxSize) : queue_(), maxSize_(maxSize) { }
        ~CuQueue() { }

        void add(const T &item) 
        {
            if (queue_.size() == maxSize_) {
                queue_.erase(queue_.begin());
            }
            queue_.emplace_back(item);
        }

        const T &get() const
        {
            if (queue_.size() == 0) {
                throw std::runtime_error("Empty queue.");
            }
            return queue_.front();
        }

        size_t size() const
        {
            return queue_.size();
        }

        size_t maxSize() const
        {
            return maxSize_;
        }

        bool full() const 
        {
            return (queue_.size() == maxSize_);
        }

        const std::vector<T> &data() const
        {
            return queue_;
        }

        void clear()
        {
            queue_.clear();
        }

    private:
        std::vector<T> queue_;
        size_t maxSize_;
};
