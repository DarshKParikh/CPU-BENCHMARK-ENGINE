#include "benchmarks.h"

#include <cmath>

void floating_worker(const std::atomic<bool>& stop, std::uint64_t& ops, std::uint64_t& checksum, std::uint64_t seed) {
    double x = 1.000001 + static_cast<double>(seed % 1000) / 100000.0;
    while (!stop.load(std::memory_order_relaxed)) {
        x = x * 1.000000119 + 0.000000031;
        x = x - std::floor(x * 0.000001) * 1000000.0;
        ++ops;
    }
    checksum = static_cast<std::uint64_t>(std::fabs(x) * 1000000000.0);
}