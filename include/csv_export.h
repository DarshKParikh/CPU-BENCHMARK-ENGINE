#pragma once

#include "benchmarks.h"

#include <string>
#include <vector>

// Writes results to `path` (creating parent directories if needed).
// Prints a warning and returns if the file cannot be written.
void write_csv(const std::string& path, const std::vector<BenchmarkResult>& results);