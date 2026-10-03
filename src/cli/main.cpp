#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "statlib/statistics.hpp"

/// \file main.cpp
/// \brief statcli entry point.
///
/// Reads whitespace-separated `double` tokens from stdin until EOF, then
/// prints descriptive statistics to stdout in the fixed `key: value` format
/// defined by the design document (§4.2). Exits non-zero on non-numeric or
/// empty input (§4.1).

int main() {
  std::vector<double> data;
  double value = 0.0;

  while (std::cin >> value) {
    data.push_back(value);
  }

  // Stream stopped: distinguish a parse failure from a clean EOF.
  if (std::cin.fail() && !std::cin.eof()) {
    std::cerr << "error: non-numeric input\n";
    return EXIT_FAILURE;
  }

  if (data.empty()) {
    std::cerr << "error: empty input\n";
    return EXIT_FAILURE;
  }

  try {
    std::cout << std::setprecision(6);
    std::cout << "count: " << statlib::count(data) << '\n';
    std::cout << "sum: " << statlib::sum(data) << '\n';
    std::cout << "mean: " << statlib::mean(data) << '\n';
    std::cout << "min: " << statlib::min(data) << '\n';
    std::cout << "max: " << statlib::max(data) << '\n';
    std::cout << "median: " << statlib::median(data) << '\n';
    std::cout << "stddev: " << statlib::population_stddev(data) << '\n';
  } catch (const std::invalid_argument& ex) {
    // Defense-in-depth: the empty case is handled above, but honor the
    // library's throw contract end-to-end regardless.
    std::cerr << "error: " << ex.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
