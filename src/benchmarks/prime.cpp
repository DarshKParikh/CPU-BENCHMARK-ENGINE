#include "benchmarks.h"

namespace{

bool is_prime(std::uint64_t n){
    if(n < 2) return false;
    if(n % 2 == 0) return n == 2;
    for(std::uint64_t d = 3; d <=n /d; d+=2)
        if (n % d == 0) return false;
    return true;
}

}

void prime_worker(const std::atomic<bool>& stop, std::uint64_t& ops, std::uint64_t& checksum, std::uint64_t seed) {
    std::uint64_t candidate = 1000000007ULL + (seed % 10000ULL) * 2ULL;
    if ((candidate & 1ULL) == 0) ++candidate;
    while (!stop.load(std::memory_order_relaxed)) {
        (void)is_prime(candidate);
        candidate += 2;
        ++ops;
    }
    checksum = candidate;
}