#include "logforge.hpp"
#include <iostream>
#include <thread>
#include <algorithm>

static std::size_t number(const std::string& value) {
    if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos)
        throw std::invalid_argument("expected positive integer: " + value);
    std::size_t consumed = 0;
    auto result = std::stoull(value, &consumed);
    if (!result || consumed != value.size()) throw std::invalid_argument("expected positive integer");
    return static_cast<std::size_t>(result);
}
int main(int argc, char** argv) {
    try {
        if (argc == 2 && std::string(argv[1]) == "--help") {
            std::cout << "Usage: logforge --input PATH [--threads 1..256] [--queue-capacity N]\n";
            return 0;
        }
        std::filesystem::path input;
        std::size_t threads = std::min(256u, std::max(1u, std::thread::hardware_concurrency()));
        std::size_t capacity = 64;
        for (int i = 1; i < argc; ++i) {
            const std::string option = argv[i];
            if (i + 1 >= argc) throw std::invalid_argument("missing value: " + option);
            const std::string value = argv[++i];
            if (option == "--input") input = value;
            else if (option == "--threads") threads = number(value);
            else if (option == "--queue-capacity") capacity = number(value);
            else throw std::invalid_argument("unknown option: " + option);
        }
        if (input.empty()) throw std::invalid_argument("--input is required; see --help");
        std::cout << logforge::json(logforge::analyze(input, threads, capacity));
    } catch (const std::exception& error) {
        std::cerr << "logforge: " << error.what() << '\n';
        return 1;
    }
}
