#include <stdexcept>
#include <vector>

#include "statlib/statistics.hpp"
#include "testing/check.hpp"

namespace {

// The worked example from DESIGN.md §4.2: input "1 2 3 4 5".
const std::vector<double> kExample = {1.0, 2.0, 3.0, 4.0, 5.0};

constexpr double kEps = 1e-9;

}  // namespace

// --- sum ------------------------------------------------------------------

STATLIB_TEST(sum_basic) {
  CHECK_NEAR(statlib::sum(kExample), 15.0, kEps);
  CHECK_NEAR(statlib::sum({-2.0, 2.0}), 0.0, kEps);
  CHECK_NEAR(statlib::sum({1.5}), 1.5, kEps);
}

STATLIB_TEST(sum_empty_is_zero) {
  // sum is the single documented exception: defined on empty input (FR-5).
  CHECK_NEAR(statlib::sum({}), 0.0, kEps);
}

// --- count ----------------------------------------------------------------

STATLIB_TEST(count_basic) {
  CHECK(statlib::count(kExample) == 5u);
  CHECK(statlib::count({42.0}) == 1u);
}

STATLIB_TEST(count_empty_is_zero) {
  CHECK(statlib::count({}) == 0u);
}

// --- mean -----------------------------------------------------------------

STATLIB_TEST(mean_basic) {
  CHECK_NEAR(statlib::mean(kExample), 3.0, kEps);
  CHECK_NEAR(statlib::mean({2.0, 4.0}), 3.0, kEps);
  CHECK_NEAR(statlib::mean({7.0}), 7.0, kEps);
}

STATLIB_TEST(mean_empty_throws) {
  CHECK_THROWS(statlib::mean({}), std::invalid_argument);
}

// --- population_variance --------------------------------------------------

STATLIB_TEST(population_variance_basic) {
  // Population variance of 1..5 = 2.0 (divisor N, not N-1).
  CHECK_NEAR(statlib::population_variance(kExample), 2.0, kEps);
  // Single element has zero variance.
  CHECK_NEAR(statlib::population_variance({5.0}), 0.0, kEps);
  // All-equal input has zero variance.
  CHECK_NEAR(statlib::population_variance({3.0, 3.0, 3.0}), 0.0, kEps);
}

STATLIB_TEST(population_variance_empty_throws) {
  CHECK_THROWS(statlib::population_variance({}), std::invalid_argument);
}

// --- population_stddev ----------------------------------------------------

STATLIB_TEST(population_stddev_basic) {
  // sqrt(2) ~= 1.4142135623730951 (worked example stddev).
  CHECK_NEAR(statlib::population_stddev(kExample), 1.4142135623730951, kEps);
  CHECK_NEAR(statlib::population_stddev({5.0}), 0.0, kEps);
}

STATLIB_TEST(population_stddev_empty_throws) {
  CHECK_THROWS(statlib::population_stddev({}), std::invalid_argument);
}

// --- min / max ------------------------------------------------------------

STATLIB_TEST(min_max_basic) {
  CHECK_NEAR(statlib::min(kExample), 1.0, kEps);
  CHECK_NEAR(statlib::max(kExample), 5.0, kEps);
  CHECK_NEAR(statlib::min({-3.0, -1.0, -2.0}), -3.0, kEps);
  CHECK_NEAR(statlib::max({-3.0, -1.0, -2.0}), -1.0, kEps);
  // Single element is both min and max.
  CHECK_NEAR(statlib::min({9.0}), 9.0, kEps);
  CHECK_NEAR(statlib::max({9.0}), 9.0, kEps);
}

STATLIB_TEST(min_empty_throws) {
  CHECK_THROWS(statlib::min({}), std::invalid_argument);
}

STATLIB_TEST(max_empty_throws) {
  CHECK_THROWS(statlib::max({}), std::invalid_argument);
}

// --- median ---------------------------------------------------------------

STATLIB_TEST(median_odd_count) {
  CHECK_NEAR(statlib::median(kExample), 3.0, kEps);
  CHECK_NEAR(statlib::median({7.0, 1.0, 3.0}), 3.0, kEps);
}

STATLIB_TEST(median_even_count) {
  // Even count: average of the two middle values.
  CHECK_NEAR(statlib::median({1.0, 2.0, 3.0, 4.0}), 2.5, kEps);
  CHECK_NEAR(statlib::median({10.0, 20.0}), 15.0, kEps);
}

STATLIB_TEST(median_even_count_unsorted) {
  // QA-added: exercises the even-count branch on UNSORTED input, where the
  // lower median is recovered via std::max_element over the partitioned lower
  // half [begin, mid). The existing median_even_count cases use already-sorted
  // inputs, and median_unsorted_input is odd-count, so this trickiest path was
  // previously untested. Must not mutate the argument.
  std::vector<double> a = {4.0, 1.0, 3.0, 2.0};
  const std::vector<double> a_before = a;
  CHECK_NEAR(statlib::median(a), 2.5, kEps);  // sorted: 1,2,3,4 -> (2+3)/2
  CHECK(a == a_before);
  // Six elements, scrambled: sorted 0,2,4,6,8,10 -> (4+6)/2 = 5.
  CHECK_NEAR(statlib::median({10.0, 2.0, 8.0, 4.0, 6.0, 0.0}), 5.0, kEps);
}

STATLIB_TEST(median_unsorted_input) {
  // Must not assume sorted input, and must not mutate the argument.
  std::vector<double> data = {5.0, 1.0, 4.0, 2.0, 3.0};
  const std::vector<double> before = data;
  CHECK_NEAR(statlib::median(data), 3.0, kEps);
  CHECK(data == before);  // argument left untouched
}

STATLIB_TEST(median_single_element) {
  CHECK_NEAR(statlib::median({42.0}), 42.0, kEps);
}

STATLIB_TEST(median_empty_throws) {
  CHECK_THROWS(statlib::median({}), std::invalid_argument);
}

// --- combined worked example ---------------------------------------------

STATLIB_TEST(worked_example_1_to_5) {
  CHECK(statlib::count(kExample) == 5u);
  CHECK_NEAR(statlib::sum(kExample), 15.0, kEps);
  CHECK_NEAR(statlib::mean(kExample), 3.0, kEps);
  CHECK_NEAR(statlib::min(kExample), 1.0, kEps);
  CHECK_NEAR(statlib::max(kExample), 5.0, kEps);
  CHECK_NEAR(statlib::median(kExample), 3.0, kEps);
  CHECK_NEAR(statlib::population_stddev(kExample), 1.4142135623730951, kEps);
}

STATLIB_TEST_MAIN()
