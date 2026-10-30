#include "csv_export.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

void write_csv(const std::string& path, const std::vector<BenchmarkResult>& results) {
    namespace fs = std::filesystem;

    const fs::path file(path);
    if (file.has_parent_path()) {
        std::error_code ec;
        fs::create_directories(file.parent_path(), ec);  // failure is caught by the open check below
    }

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