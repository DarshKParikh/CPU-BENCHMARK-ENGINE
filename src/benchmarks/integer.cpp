#include "benchmarks.h"

void integer_worker(const std::atomic<bool>& stop, std::uint64_t& ops, std::uint64_t& checksum, std::uint64_t seed) {
    std::uint64_t x = seed | 1ULL;
    while (!stop.load(std::memory_order_relaxed)) {
        // Xorshift-style integer workload with a data dependency.
        x ^= x << 13; x ^= x >> 7; x ^= x << 17;
        x *= 0x9E3779B185EBCA87ULL;
        ++ops;
    }
    checksum = x;
}