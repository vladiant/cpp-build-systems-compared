# Changelog

All notable changes to this project are documented here.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

_Nothing yet._

## [0.1.0] - 2026-10-03

First release. One shared C++17 source tree built four ways to compare build
systems — "Same source, different builds."

### Added

- **`statlib`** — a dependency-free descriptive-statistics static library
  (`sum`, `count`, `mean`, `population_variance`, `population_stddev`, `min`,
  `max`, `median`). Functions requiring non-empty input throw
  `std::invalid_argument`; `sum([]) == 0.0` by the additive-identity convention.
  Standard-library-only, pure value semantics, header depends only on `<vector>`
  and `<cstddef>`.
- **`statcli`** — a command-line front-end that reads whitespace-separated
  doubles from stdin and prints fixed `key: value` lines
  (`count/sum/mean/min/max/median/stddev`) at `setprecision(6)` default-float.
  Exits non-zero with `error: empty input` / `error: non-numeric input` on bad
  input.
- **Dependency-free test harness** — a vendored, hand-rolled single header
  (`tests/testing/check.hpp`, MIT) providing `CHECK`, `CHECK_NEAR`,
  `CHECK_THROWS`, a self-registering test registry, and a `main` that returns a
  non-zero exit code on any failure. 20 unit tests covering odd/even median,
  single-element and unsorted inputs, empty-input throw contracts, population
  (not sample) variance, and median argument non-mutation.
- **Four build-system drivers** over the identical source tree:
  - **CMake** (`CMakeLists.txt`) — static `statlib`, `statcli`, `statlib_tests`
    via `ctest`.
  - **Bazel** (`WORKSPACE` + `BUILD.bazel` + `.bazelrc`) — classic WORKSPACE
    mode (not Bzlmod); `cc_library`/`cc_binary`/`cc_test`.
  - **Meson** (`meson.build`) — ninja backend; `static_library`/`executable`/`test`.
  - **xmake** (`xmake.lua`) — static/binary targets with `add_tests`.
- **`scripts/build-all.sh`** — builds and tests all four systems in isolated
  out-of-source directories, runs a `statcli` smoke check per system, and prints
  a PASS/FAIL summary.
- **CI workflow** (`.github/workflows/ci.yml`) — one job per build system plus a
  bonus clang-18 job, each ending with the `statcli` smoke check.
- **Release workflow** (`.github/workflows/release.yml`) — on a `v*` tag, builds
  Release, gates on tests + smoke check, packages `statcli` + headers + LICENSE
  into a versioned tarball with a `.sha256`, and publishes a GitHub Release.
- **Documentation** — SRS, design, QA report, CI/CD pipeline doc, project status,
  and this README with the four-way **trade-off matrix** (configuration language,
  measured config verbosity, dependency handling, build/incremental speed,
  tooling/IDE support, learning curve, cross-platform support, test integration,
  isolation footprint, and installation/availability).

[Unreleased]: https://github.com/vladiant/cpp-build-systems-compared/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/vladiant/cpp-build-systems-compared/releases/tag/v0.1.0
