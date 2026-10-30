cmake_minimum_required(VERSION 3.16)
project(cpu_benchmark_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(Threads REQUIRED)

add_executable(cpu_benchmark
  src/main.cpp
  src/benchmarks/integer.cpp
  src/benchmarks/floating_point.cpp
  src/benchmarks/memory.cpp
  src/benchmarks/prime.cpp
  src/utils/csv_export.cpp
)

target_include_directories(cpu_benchmark PRIVATE include)
target_link_libraries(cpu_benchmark PRIVATE Threads::Threads)

if(MSVC)
  target_compile_options(cpu_benchmark PRIVATE /W4)
else()
  target_compile_options(cpu_benchmark PRIVATE -Wall -Wextra -Wpedantic)
endif()