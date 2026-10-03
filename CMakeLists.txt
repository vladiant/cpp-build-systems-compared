cmake_minimum_required(VERSION 3.16)

project(cpp_build_systems_compared LANGUAGES CXX VERSION 0.1.0)

# ---------------------------------------------------------------------------
# C++17 baseline for every target (NFR-3).
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# statlib: STATIC library, public include dir transitively exported.
# ---------------------------------------------------------------------------
add_library(statlib STATIC src/statlib/statistics.cpp)
target_include_directories(statlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(statlib PUBLIC cxx_std_17)

# ---------------------------------------------------------------------------
# statcli: executable linking statlib only.
# ---------------------------------------------------------------------------
add_executable(statcli src/cli/main.cpp)
target_link_libraries(statcli PRIVATE statlib)

# ---------------------------------------------------------------------------
# statlib_tests: unit tests; tests/ is PRIVATE so check.hpp is not leaked.
# ---------------------------------------------------------------------------
add_executable(statlib_tests tests/statistics_test.cpp)
target_link_libraries(statlib_tests PRIVATE statlib)
target_include_directories(statlib_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)

enable_testing()
add_test(NAME statlib_tests COMMAND statlib_tests)
