cmake_minimum_required(VERSION 3.16)
project(CPUPerformanceBenchmark LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_executable(cpu_benchmark src/main.cpp)
target_link_libraries(cpu_benchmark PRIVATE Threads::Threads)

if(MSVC)
    target_compile_options(cpu_benchmark PRIVATE /O2 /W4)
else()
    target_compile_options(cpu_benchmark PRIVATE -O2 -Wall -Wextra -Wpedantic)
endif()
