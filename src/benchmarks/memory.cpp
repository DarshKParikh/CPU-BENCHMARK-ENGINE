#include "benchmarks.h"

#include <cstddef>
#include <numeric>
#include <vector>

void memory_worker(const std::atomic<bool>& stop, std::uint64_t& ops, std::uint64_t& checksum, std::uint64_t seed) {
    constexpr std::size_t count = 1u << 18; // 2 MiB per worker
    std::vector<std::uint64_t> data(count, seed + 1);
    std::size_t index = 0;
    std::uint64_t value = seed + 0x1234567ULL;
    while (!stop.load(std::memory_order_relaxed)) {
        data[index] = value;
        value += data[index] + 1;
        index = (index + 1) % count;
        ++ops; // one write/read-modify operation
    }
    checksum = std::accumulate(data.begin(), data.end(), std::uint64_t{0});
}