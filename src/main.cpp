#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <mutex>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using Clock = std::chrono::steady_clock;

struct BenchmarkResult {
    std::string workload;
    unsigned threads{};
    double seconds{};
    std::uint64_t operations{};
    double ops_per_second{};
    std::uint64_t checksum{};
};

struct Config {
    double duration_seconds = 2.0;
    unsigned max_threads = std::max(1u, std::thread::hardware_concurrency());
    std::string csv_path = "benchmark_results.csv";
};

static bool is_prime(std::uint64_t n) {
    if (n < 2) return false;
    if (n % 2 == 0) return n == 2;
    for (std::uint64_t d = 3; d <= n / d; d += 2)
        if (n % d == 0) return false;
    return true;
}

using Worker = void (*)(const std::atomic<bool>&, std::uint64_t&, std::uint64_t&, std::uint64_t);

static void integer_worker(const std::atomic<bool>& stop, std::uint64_t& ops, std::uint64_t& checksum, std::uint64_t seed) {
    std::uint64_t x = seed | 1ULL;
    while (!stop.load(std::memory_order_relaxed)) {
        // Xorshift-style integer workload with a data dependency.
        x ^= x << 13; x ^= x >> 7; x ^= x << 17;
        x *= 0x9E3779B185EBCA87ULL;
        ++ops;
    }
    checksum = x;
}

static void floating_worker(const std::atomic<bool>& stop, std::uint64_t& ops, std::uint64_t& checksum, std::uint64_t seed) {
    double x = 1.000001 + static_cast<double>(seed % 1000) / 100000.0;
    while (!stop.load(std::memory_order_relaxed)) {
        x = x * 1.000000119 + 0.000000031;
        x = x - std::floor(x * 0.000001) * 1000000.0;
        ++ops;
    }
    checksum = static_cast<std::uint64_t>(std::fabs(x) * 1000000000.0);
}

static void memory_worker(const std::atomic<bool>& stop, std::uint64_t& ops, std::uint64_t& checksum, std::uint64_t seed) {
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

static void prime_worker(const std::atomic<bool>& stop, std::uint64_t& ops, std::uint64_t& checksum, std::uint64_t seed) {
    std::uint64_t candidate = 1000000007ULL + (seed % 10000ULL) * 2ULL;
    if ((candidate & 1ULL) == 0) ++candidate;
    while (!stop.load(std::memory_order_relaxed)) {
        (void)is_prime(candidate);
        candidate += 2;
        ++ops;
    }
    checksum = candidate;
}

static BenchmarkResult run_benchmark(const std::string& name, Worker worker,
                                     unsigned thread_count, double duration) {
    std::atomic<bool> stop{false};
    std::vector<std::thread> threads;
    std::vector<std::uint64_t> counts(thread_count, 0);
    std::vector<std::uint64_t> checksums(thread_count, 0);
    threads.reserve(thread_count);

    const auto start = Clock::now();
    for (unsigned i = 0; i < thread_count; ++i) {
        threads.emplace_back([&, i] { worker(stop, counts[i], checksums[i], 0xABCDEF123456789ULL + i * 7919ULL); });
    }

    std::this_thread::sleep_for(std::chrono::duration<double>(duration));
    stop.store(true, std::memory_order_relaxed);
    for (auto& t : threads) t.join();

    const double elapsed = std::chrono::duration<double>(Clock::now() - start).count();
    std::uint64_t total_ops = 0;
    for (auto n : counts) total_ops += n;

    const auto combined_checksum = std::accumulate(checksums.begin(), checksums.end(), std::uint64_t{0},
        [](std::uint64_t a, std::uint64_t b) { return a ^ b; });
    return {name, thread_count, elapsed, total_ops,
            elapsed > 0.0 ? static_cast<double>(total_ops) / elapsed : 0.0, combined_checksum};
}

static void print_result(const BenchmarkResult& r) {
    std::cout << std::left << std::setw(13) << r.workload
              << " | threads " << std::setw(3) << r.threads
              << " | " << std::fixed << std::setprecision(2) << r.seconds << " s"
              << " | " << std::setprecision(0) << r.ops_per_second << " ops/s"
              << " | checksum " << r.checksum << '\n';
}

static void write_csv(const std::string& path, const std::vector<BenchmarkResult>& results) {
    std::ofstream out(path);
    if (!out) {
        std::cerr << "Warning: could not write CSV file: " << path << '\n';
        return;
    }
    out << "workload,threads,seconds,operations,operations_per_second,checksum\n";
    for (const auto& r : results)
        out << r.workload << ',' << r.threads << ',' << std::setprecision(10) << r.seconds
            << ',' << r.operations << ',' << r.ops_per_second << ',' << r.checksum << '\n';
    std::cout << "\nCSV results saved to: " << path << '\n';
}

static void print_usage(const char* exe) {
    std::cout << "Usage: " << exe << " [--duration SECONDS] [--max-threads N] [--csv FILE]\n"
              << "Example: " << exe << " --duration 3 --max-threads 8 --csv results.csv\n";
}

int main(int argc, char* argv[]) {
    Config config;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") { print_usage(argv[0]); return 0; }
        if (i + 1 >= argc) { print_usage(argv[0]); return 1; }
        const std::string value = argv[++i];
        try {
            if (arg == "--duration") config.duration_seconds = std::stod(value);
            else if (arg == "--max-threads") config.max_threads = static_cast<unsigned>(std::stoul(value));
            else if (arg == "--csv") config.csv_path = value;
            else { std::cerr << "Unknown option: " << arg << '\n'; print_usage(argv[0]); return 1; }
        } catch (...) {
            std::cerr << "Invalid value for " << arg << ": " << value << '\n';
            return 1;
        }
    }

    if (config.duration_seconds <= 0.0 || config.duration_seconds > 3600.0 ||
        config.max_threads == 0 || config.max_threads > 1024) {
        std::cerr << "Duration must be in (0, 3600] seconds and max threads in [1, 1024].\n";
        return 1;
    }

    const std::vector<std::pair<std::string, Worker>> workloads = {
        {"Integer", integer_worker},
        {"Floating", floating_worker},
        {"Memory", memory_worker},
        {"Prime", prime_worker}
    };

    std::cout << "Multithreaded CPU Performance Benchmarking Engine\n"
              << "Duration per test: " << config.duration_seconds << " seconds\n"
              << "Thread counts: 1 to " << config.max_threads << "\n"
              << "Note: close heavy background applications for more repeatable results.\n\n";

    std::vector<BenchmarkResult> results;
    for (const auto& workload : workloads) {
        std::cout << "== " << workload.first << " workload ==\n";
        for (unsigned n = 1; n <= config.max_threads; ++n) {
            auto result = run_benchmark(workload.first, workload.second, n, config.duration_seconds);
            print_result(result);
            results.push_back(result);
        }
        std::cout << '\n';
    }

    write_csv(config.csv_path, results);
    std::cout << "Benchmark complete. Higher ops/s means more work completed per second\n"
              << "for that workload; compare only like-for-like runs on the same settings.\n";
    return 0;
}
