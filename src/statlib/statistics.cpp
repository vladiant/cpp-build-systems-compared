#include "statlib/statistics.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <string>

namespace statlib {
namespace {

void require_non_empty(const std::vector<double>& data, const char* fn) {
  if (data.empty()) {
    throw std::invalid_argument(std::string(fn) + ": empty input");
  }
}

}  // namespace

double sum(const std::vector<double>& data) {
  return std::accumulate(data.begin(), data.end(), 0.0);
}

std::size_t count(const std::vector<double>& data) {
  return data.size();
}

double mean(const std::vector<double>& data) {
  require_non_empty(data, "statlib::mean");
  return sum(data) / static_cast<double>(data.size());
}

double population_variance(const std::vector<double>& data) {
  require_non_empty(data, "statlib::population_variance");
  const double m = mean(data);
  double acc = 0.0;
  for (double x : data) {
    const double d = x - m;
    acc += d * d;
  }
  return acc / static_cast<double>(data.size());
}

double population_stddev(const std::vector<double>& data) {
  require_non_empty(data, "statlib::population_stddev");
  return std::sqrt(population_variance(data));
}

double min(const std::vector<double>& data) {
  require_non_empty(data, "statlib::min");
  return *std::min_element(data.begin(), data.end());
}

double max(const std::vector<double>& data) {
  require_non_empty(data, "statlib::max");
  return *std::max_element(data.begin(), data.end());
}

double median(const std::vector<double>& data) {
  require_non_empty(data, "statlib::median");
  std::vector<double> copy(data);
  const std::size_t n = copy.size();
  const std::size_t mid = n / 2;
  std::nth_element(copy.begin(), copy.begin() + mid, copy.end());
  const double upper = copy[mid];
  if (n % 2 == 1) {
    return upper;
  }
  // Even count: the lower median is the largest value in the partitioned
  // lower half [begin, mid). nth_element guarantees every element before
  // `mid` is <= copy[mid], so a max over that sub-range is correct.
  const double lower = *std::max_element(copy.begin(), copy.begin() + mid);
  return (lower + upper) / 2.0;
}

}  // namespace statlib
