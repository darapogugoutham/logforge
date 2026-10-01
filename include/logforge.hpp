#pragma once
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <mutex>
#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <utility>

namespace logforge {
// Closing wakes both producers and consumers. Accepted items drain before pop
// returns false; producers cannot add more work after close.
template <typename T> class BoundedQueue {
    std::mutex mutex_;
    std::condition_variable readable_, writable_;
    std::deque<T> items_;
    std::size_t capacity_;
    bool closed_ = false;
public:
    explicit BoundedQueue(std::size_t capacity) : capacity_(capacity) {
        if (!capacity) throw std::invalid_argument("queue capacity must be positive");
    }
    bool push(T item) {
        std::unique_lock<std::mutex> lock(mutex_);
        writable_.wait(lock, [&] { return closed_ || items_.size() < capacity_; });
        if (closed_) return false;
        items_.push_back(std::move(item));
        readable_.notify_one();
        return true;
    }
    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        readable_.wait(lock, [&] { return closed_ || !items_.empty(); });
        if (items_.empty()) return false;
        item = std::move(items_.front());
        items_.pop_front();
        writable_.notify_one();
        return true;
    }
    void close() {
        std::lock_guard<std::mutex> lock(mutex_);
        closed_ = true;
        readable_.notify_all();
        writable_.notify_all();
    }
};
struct Stats {
    std::uint64_t files = 0, lines = 0, valid = 0, malformed = 0;
    std::map<std::string, std::uint64_t> levels;
    void merge(const Stats& other);
};
Stats analyze(const std::filesystem::path& input, std::size_t threads, std::size_t capacity);
std::string json(const Stats& stats);
}
