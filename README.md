# Multithreaded CPU Performance Benchmarking Engine (C++)

A small, cross-platform C++17 command-line project that measures how a CPU performs
under different thread counts. It runs four workloads and exports the measurements
to CSV for analysis.

## Features

- **Integer workload:** data-dependent 64-bit integer operations.
- **Floating-point workload:** repeated floating-point arithmetic.
- **Memory workload:** per-thread memory read/write activity.
- **Prime workload:** trial-division primality checks.
- **Thread scaling:** runs each workload with 1, 2, ... up to the selected maximum.
- **CSV export:** records elapsed time, operation count, throughput, and checksum.
- **Modular code:** each workload and the CSV exporter live in their own files.
- **Portable build:** CMake, C++17, and the standard threading library.

## Project structure

```text
cpu-benchmark-engine/
├── src/
│   ├── main.cpp                  # CLI parsing, benchmark runner, output
│   ├── benchmarks/
│   │   ├── integer.cpp
│   │   ├── floating_point.cpp
│   │   ├── memory.cpp
│   │   └── prime.cpp
│   └── utils/
│       └── csv_export.cpp
├── include/
│   ├── benchmarks.h              # BenchmarkResult struct and worker declarations
│   └── csv_export.h
├── results/                      # generated CSV output (contents are git-ignored)
├── CMakeLists.txt
├── README.md
└── .gitignore
```

## Requirements

- A C++17 compiler (Clang, GCC, or MSVC)
- CMake 3.16+

## Build on macOS

Install Apple's command-line tools if they are not already installed:

```bash
xcode-select --install
```

Then, from the project directory:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Run:

```bash
./build/cpu_benchmark --duration 2 --max-threads 8 --csv results/run1.csv
```

On macOS, you can use `sysctl -n hw.logicalcpu` to check the logical CPU count.
If `--max-threads` is omitted, the program uses the value reported by
`std::thread::hardware_concurrency()`.

## Build on Linux

```bash
sudo apt install build-essential cmake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/cpu_benchmark --duration 2 --max-threads 8 --csv results/run1.csv
```

## Build on Windows

Use a Visual Studio Developer Command Prompt with CMake available:

```powershell
cmake -S . -B build
cmake --build build --config Release
.\build\Release\cpu_benchmark.exe --duration 2 --max-threads 8 --csv results\run1.csv
```

## Command-line options

| Option | Meaning | Default |
|---|---|---|
| `--duration N` | Seconds each workload/thread-count test runs | `2` |
| `--max-threads N` | Test thread counts from 1 through N | Hardware concurrency |
| `--csv FILE` | Output CSV path (missing folders are created) | `results/benchmark_results.csv` |
| `--help` | Show usage | — |

Example shorter smoke test:

```bash
./build/cpu_benchmark --duration 0.5 --max-threads 4
```

## Output

Results are printed to the console and saved to CSV with these columns:

```text
workload,threads,seconds,operations,operations_per_second,checksum
```

## Understanding the results

The console and CSV report **operations per second (ops/s)**. Compare the same
workload across thread counts to see how throughput scales. A result that stops
improving or declines at higher thread counts can indicate saturation, memory
bandwidth limits, scheduling overhead, or thermal/power constraints.

The workloads do different amounts of work per operation, so do **not** compare
the raw ops/s number between different workload types as if they were equivalent.
Run several times under similar conditions and compare medians for more reliable
results. Close intensive background tasks and keep the machine's power mode
consistent.

## Engineering notes and limitations

- Each test creates a fresh set of worker threads and runs for a fixed wall-clock
  duration. The reported elapsed time includes thread startup and shutdown.
- The memory test uses a private 2 MiB buffer per worker. It is a simple access
  pattern, not a comprehensive memory-bandwidth benchmark.
- The prime workload uses trial division, so its throughput is intentionally
  different from the other workloads.
- This is an educational benchmarking project, not a replacement for established
  tools such as SPEC CPU or a carefully controlled performance lab.
- Results vary with CPU frequency scaling, thermal state, operating system,
  compiler, build flags, and background load.

## Possible extensions

- Add JSON output and charts for scaling efficiency.
- Add configurable working-set sizes and random memory access.
- Add warm-up runs, repeated trials, and median/percentile reporting.
- Record CPU model, OS, compiler, and build configuration alongside results.
- Add benchmark selection flags and per-core affinity controls.
