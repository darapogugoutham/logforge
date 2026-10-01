#include "logforge.hpp"
#include <atomic>
#include <iostream>
#include <thread>
#include <stdexcept>
#include <vector>

void check(bool value) { if (!value) throw std::runtime_error("test failed"); }
int main() {
    using logforge::BoundedQueue;
    bool rejected = false;
    try { BoundedQueue<int> invalid(0); } catch (const std::invalid_argument&) { rejected = true; }
    check(rejected);
    BoundedQueue<int> drain(1);
    check(drain.push(42)); drain.close(); drain.close();
    int item = 0;
    check(!drain.push(99)); check(drain.pop(item) && item == 42); check(!drain.pop(item));
    BoundedQueue<int> queue(3);
    std::atomic<int> count{0};
    std::atomic<long long> sum{0};
    std::vector<std::thread> consumers, producers;
    for (int i = 0; i < 6; ++i) consumers.emplace_back([&] {
        int value;
        while (queue.pop(value)) { ++count; sum += value; }
    });
    for (int i = 0; i < 4; ++i) producers.emplace_back([&, i] {
        for (int j = 1; j <= 5000; ++j) check(queue.push(i * 5000 + j));
    });
    for (auto& thread : producers) thread.join();
    queue.close();
    for (auto& thread : consumers) thread.join();
    check(count == 20000); check(sum == 20000LL * 20001 / 2);
    BoundedQueue<int> full(1);
    full.push(1);
    std::thread blocked_producer([&] { check(!full.push(2)); });
    full.close(); blocked_producer.join();
    BoundedQueue<int> empty(1);
    std::thread blocked_consumer([&] { int value; check(!empty.pop(value)); });
    empty.close(); blocked_consumer.join();
    std::cout << "PASS queue validation, draining, close wakeups, 20,000 concurrent items\n";
}
