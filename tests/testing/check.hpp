#pragma once

// ---------------------------------------------------------------------------
// check.hpp - minimal, dependency-free unit-test harness.
//
// Copyright (c) 2026 cpp-build-systems-compared contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// ---------------------------------------------------------------------------
//
// Usage:
//   #include "testing/check.hpp"
//
//   STATLIB_TEST(my_test) {
//     CHECK(1 + 1 == 2);
//     CHECK_NEAR(0.1 + 0.2, 0.3, 1e-9);
//     CHECK_THROWS(throw_something(), std::invalid_argument);
//   }
//
//   STATLIB_TEST_MAIN()  // defines main(); non-zero exit if any check fails.

#include <cmath>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace statlib_testing {

// A single registered test case: a name plus the body to execute.
struct TestCase {
  std::string name;
  std::function<void()> body;
};

// Process-wide test state. Function-local statics guarantee initialization
// order across translation units (Meyers singletons).
inline std::vector<TestCase>& registry() {
  static std::vector<TestCase> tests;
  return tests;
}

inline int& failure_count() {
  static int failures = 0;
  return failures;
}

// Self-registering helper constructed by the STATLIB_TEST macro.
struct Registrar {
  Registrar(const std::string& name, std::function<void()> body) {
    registry().push_back(TestCase{name, std::move(body)});
  }
};

inline void report_failure(const char* file, int line, const std::string& msg) {
  ++failure_count();
  std::cerr << "FAIL: " << file << ":" << line << ": " << msg << "\n";
}

inline int run_all() {
  int passed = 0;
  int failed = 0;
  for (const TestCase& test : registry()) {
    const int before = failure_count();
    try {
      test.body();
    } catch (const std::exception& ex) {
      report_failure(__FILE__, __LINE__,
                     "test '" + test.name +
                         "' threw unexpected std::exception: " + ex.what());
    } catch (...) {
      report_failure(__FILE__, __LINE__,
                     "test '" + test.name + "' threw unknown exception");
    }
    if (failure_count() == before) {
      ++passed;
    } else {
      ++failed;
      std::cerr << "  (in test: " << test.name << ")\n";
    }
  }
  std::cout << "PASSED " << passed << " / FAILED " << failed << "\n";
  return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

}  // namespace statlib_testing

// Define and self-register a test case. The body follows the macro as a block.
#define STATLIB_TEST(test_name)                                             \
  static void test_name##_body();                                          \
  static ::statlib_testing::Registrar test_name##_registrar(#test_name,    \
                                                            test_name##_body); \
  static void test_name##_body()

// Fail (and continue the current test) if `expr` is false.
#define CHECK(expr)                                                        \
  do {                                                                     \
    if (!(expr)) {                                                         \
      ::statlib_testing::report_failure(__FILE__, __LINE__,                \
                                        "CHECK(" #expr ") failed");        \
    }                                                                      \
  } while (false)

// Fail if |a - b| > eps.
#define CHECK_NEAR(a, b, eps)                                              \
  do {                                                                     \
    const double check_near_a_ = (a);                                      \
    const double check_near_b_ = (b);                                      \
    const double check_near_eps_ = (eps);                                  \
    if (std::fabs(check_near_a_ - check_near_b_) > check_near_eps_) {      \
      ::statlib_testing::report_failure(                                   \
          __FILE__, __LINE__,                                              \
          "CHECK_NEAR(" #a ", " #b ", " #eps ") failed: " +               \
              std::to_string(check_near_a_) + " vs " +                     \
              std::to_string(check_near_b_));                              \
    }                                                                      \
  } while (false)

// Fail if evaluating `expr` does not throw an exception of type `ExceptionType`.
#define CHECK_THROWS(expr, ExceptionType)                                  \
  do {                                                                     \
    bool check_threw_expected_ = false;                                    \
    try {                                                                  \
      (void)(expr);                                                        \
    } catch (const ExceptionType&) {                                       \
      check_threw_expected_ = true;                                        \
    } catch (...) {                                                        \
      ::statlib_testing::report_failure(                                   \
          __FILE__, __LINE__,                                              \
          "CHECK_THROWS(" #expr ", " #ExceptionType                        \
          ") threw a different type");                                     \
    }                                                                      \
    if (!check_threw_expected_) {                                          \
      ::statlib_testing::report_failure(                                   \
          __FILE__, __LINE__,                                              \
          "CHECK_THROWS(" #expr ", " #ExceptionType ") did not throw");    \
    }                                                                      \
  } while (false)

// Expand to a main() that runs all registered tests.
#define STATLIB_TEST_MAIN()                                                \
  int main() { return ::statlib_testing::run_all(); }
