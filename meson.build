project('cpp-build-systems-compared', 'cpp',
  version : '0.1.0',
  default_options : ['cpp_std=c++17'])

inc = include_directories('include')

# statlib: static library with public include dir.
statlib = static_library('statlib',
  'src/statlib/statistics.cpp',
  include_directories : inc)

# statcli: executable linking statlib.
executable('statcli',
  'src/cli/main.cpp',
  link_with : statlib,
  include_directories : inc)

# statlib_tests: unit tests; tests/ added privately for testing/check.hpp.
test_exe = executable('statlib_tests',
  'tests/statistics_test.cpp',
  link_with : statlib,
  include_directories : [inc, include_directories('tests')])

test('statlib_tests', test_exe)
