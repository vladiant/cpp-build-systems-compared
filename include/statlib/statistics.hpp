#pragma once

#include <cstddef>
#include <vector>

/// \file statistics.hpp
/// \brief Public API for the dependency-free descriptive statistics library.
///
/// All functions operate on a `const std::vector<double>&` (pure value
/// semantics, no ownership transfer) and return plain arithmetic types.
/// Functions that mathematically require a non-empty input throw
/// `std::invalid_argument` when given an empty vector. `sum` and `count`
/// are defined on empty input (additive identity / zero).
///
/// All functions are pure, stateless and therefore trivially thread-safe.

namespace statlib {

/// Sum of all values.
/// \note Defined for empty input: returns 0.0 (the additive identity).
double sum(const std::vector<double>& data);

/// Number of elements.
/// \note Defined for empty input: returns 0.
std::size_t count(const std::vector<double>& data);

/// Arithmetic mean.
/// \throws std::invalid_argument if \p data is empty.
double mean(const std::vector<double>& data);

/// Population variance (divisor N).
/// \throws std::invalid_argument if \p data is empty.
double population_variance(const std::vector<double>& data);

/// Population standard deviation (sqrt of population_variance).
/// \throws std::invalid_argument if \p data is empty.
double population_stddev(const std::vector<double>& data);

/// Minimum value.
/// \throws std::invalid_argument if \p data is empty.
double min(const std::vector<double>& data);

/// Maximum value.
/// \throws std::invalid_argument if \p data is empty.
double max(const std::vector<double>& data);

/// Median value (average of the two middle elements for even counts).
/// Does not assume sorted input; computes on a local copy and never mutates
/// its argument.
/// \throws std::invalid_argument if \p data is empty.
double median(const std::vector<double>& data);

}  // namespace statlib
