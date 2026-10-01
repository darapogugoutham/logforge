#include "logforge.hpp"
#include <fstream>
#include <sstream>
#include <thread>
#include <algorithm>
#include <exception>

namespace logforge {
void Stats::merge(const Stats& other) {
    files += other.files; lines += other.lines;
    valid += other.valid; malformed += other.malformed;
    for (const auto& [level, count] : other.levels) levels[level] += count;
}
static Stats scan(const std::filesystem::path& path) {
    std::ifstream stream(path);
    if (!stream) throw std::runtime_error("cannot read: " + path.string());
    Stats result;
    result.files = 1;
    std::string line;
    while (std::getline(stream, line)) {
        ++result.lines;
        std::istringstream record(line);
        std::string timestamp, level, message;
        record >> timestamp >> level;
        std::getline(record, message);
        const bool known = level == "INFO" || level == "WARN" || level == "ERROR" || level == "DEBUG";
        bool digits = timestamp.size() == 20;
        for (std::size_t i = 0; i < timestamp.size(); ++i) {
            if (i == 4 || i == 7 || i == 10 || i == 13 || i == 16 || i == 19) continue;
            digits = digits && timestamp[i] >= '0' && timestamp[i] <= '9';
        }
        if (timestamp.size() != 20 || timestamp[4] != '-' || timestamp[7] != '-' ||
            timestamp[10] != 'T' || timestamp[13] != ':' || timestamp[16] != ':' ||
            timestamp[19] != 'Z' || !digits || !known || message.find_first_not_of(" \t\r") == std::string::npos) {
            ++result.malformed;
        } else {
            ++result.valid;
            ++result.levels[level];
        }
    }
    if (stream.bad()) throw std::runtime_error("read failed: " + path.string());
    return result;
}
Stats analyze(const std::filesystem::path& input, std::size_t threads, std::size_t capacity) {
    if (!threads || threads > 256) throw std::invalid_argument("threads must be between 1 and 256");
    BoundedQueue<std::filesystem::path> queue(capacity);
    std::vector<Stats> local(threads);
    std::vector<std::thread> workers;
    std::mutex error_mutex;
    std::exception_ptr error;
    auto record_error = [&] {
        std::lock_guard<std::mutex> lock(error_mutex);
        if (!error) error = std::current_exception();
        queue.close();
    };
    try {
        for (std::size_t i = 0; i < threads; ++i) {
            workers.emplace_back([&, i] {
                try {
                    std::filesystem::path path;
                    while (queue.pop(path)) local[i].merge(scan(path));
                } catch (...) { record_error(); }
            });
        }
        if (std::filesystem::is_regular_file(input)) {
            queue.push(input);
        } else if (std::filesystem::is_directory(input)) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(input)) {
                // Do not follow file symlinks into unrelated trees.
                if (!entry.is_symlink() && entry.is_regular_file() && entry.path().extension() == ".log")
                    if (!queue.push(entry.path())) break;
            }
        } else throw std::runtime_error("input must be an existing file or directory");
    } catch (...) { record_error(); }
    queue.close();
    for (auto& worker : workers) worker.join();
    if (error) std::rethrow_exception(error);
    Stats total;
    for (const auto& stats : local) total.merge(stats);
    return total;
}
std::string json(const Stats& stats) {
    std::ostringstream out;
    out << "{\n  \"files\": " << stats.files << ",\n  \"lines\": " << stats.lines
        << ",\n  \"valid\": " << stats.valid << ",\n  \"malformed\": " << stats.malformed << ",\n  \"levels\": {";
    bool first = true;
    for (const auto& [level, count] : stats.levels) {
        if (!first) out << ", ";
        first = false;
        out << '"' << level << "\": " << count;
    }
    out << "}\n}\n";
    return out.str();
}
}
