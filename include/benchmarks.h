#pragma once

#include <atomic>
#include <cstdint>
#include <string>

struct BenchmarkResult {
    std::string workload;
    unsigned threads{};
    double seconds{};
    std::uint64_t operations{};
    double ops_per_second{};
    std::uint64_t checksum{};
};

// A worker runs until `stop` is set, counting operations and producing a checksum.
using Worker = void (*)(const std::atomic<bool>& stop,
                        std::uint64_t& ops,
                        std::uint64_t& checksum,
                        std::uint64_t seed);

void integer_worker(const std::atomic<bool>& stop, std::uint64_t& ops, std::uint64_t& checksum, std::uint64_t seed);
void floating_worker(const std::atomic<bool>& stop, std::uint64_t& ops, std::uint64_t& checksum, std::uint64_t seed);
void memory_worker(const std::atomic<bool>& stop, std::uint64_t& ops, std::uint64_t& checksum, std::uint64_t seed);
void prime_worker(const std::atomic<bool>& stop, std::uint64_t& ops, std::uint64_t& checksum, std::uint64_t seed);