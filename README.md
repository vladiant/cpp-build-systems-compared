# cpp-build-systems-compared

> **Same source, different builds.**

One shared C++17 source tree — a `statlib` statistics library, a `statcli`
command-line front-end, and a dependency-free unit-test suite — built four
different ways so you can compare the build systems side by side:
**CMake · Bazel · Meson · xmake.**

There are **no third-party dependencies**. Everything builds **offline** on
**Linux** with the system **g++** (clang++ works as a bonus). The headline
deliverable is the [**trade-off matrix**](#the-trade-off-matrix) near the bottom:
the same library + CLI + tests, expressed in each tool, compared across the
dimensions that actually matter when you pick a build system.

---

## Why this exists

Picking a build system is usually argued in the abstract. This repo makes it
concrete: a single, un-duplicated source tree (one copy of every `.cpp`/`.hpp`)
is consumed by four independent build drivers that each produce the same three
targets — a static library, a CLI, and a test runner — and run the same tests.
Because the *source* is held constant, every difference you see in the matrix is
a difference in the **build system**, not the application.

---

## Repository layout

```
cpp-build-systems-compared/
├── LICENSE                         # MIT
├── README.md                       # this file (trade-off matrix below)
├── CHANGELOG.md                    # Keep a Changelog / SemVer
│
├── include/
│   └── statlib/
│       ├── statistics.hpp          # public API (all statistics functions)
│       └── version.hpp             # statlib::kVersion string
│
├── src/
│   ├── statlib/
│   │   └── statistics.cpp          # library implementation
│   └── cli/
│       └── main.cpp                # statcli entry point (stdin → stats)
│
├── tests/
│   ├── testing/
│   │   └── check.hpp               # vendored, hand-rolled assertion harness (MIT)
│   └── statistics_test.cpp         # unit tests (dependency-free)
│
├── CMakeLists.txt                  # CMake driver
├── WORKSPACE                       # Bazel (classic WORKSPACE mode)
├── BUILD.bazel                     # Bazel targets
├── .bazelrc                        # Bazel ci config (--noenable_bzlmod)
├── meson.build                     # Meson driver
├── xmake.lua                       # xmake driver
│
├── scripts/
│   └── build-all.sh                # build + test all four, print PASS/FAIL
│
└── docs/
    ├── requirements/SRS.md
    ├── design/DESIGN.md
    ├── qa/QA-REPORT.md
    ├── ci-cd/cpp-build-systems-compared-pipeline.md
    └── status.md
```

Each build driver lives at the repo root and references the **same** relative
source paths — there are no per-build-system source copies.

---

## Prerequisites

Everything is standard-library-only C++17; no package manager or network access
is needed to build or test. Verified toolchain versions:

| Tool | Version used | Notes |
|------|--------------|-------|
| g++ | 13.3.0 | default compiler |
| clang++ | 18.1.3 | optional / bonus |
| CMake | 3.28.3 | `CMakeLists.txt` requires ≥ 3.16 |
| Bazel | classic **WORKSPACE** mode | not Bzlmod; CI pins bazelisk `7.4.1` |
| Meson | 1.12.1 | with **ninja** 1.11.1 backend |
| xmake | 3.1.1 | |

> `meson` and `xmake` are installed per-user in this project and land in
> `~/.local/bin`. If they are not on your `PATH`, add it first:
> `export PATH="$HOME/.local/bin:$PATH"`.

---

## Build & run

All commands run from the repo root. Each build system writes only to its own
isolated, git-ignored output directory.

### CMake

```bash
cmake -S . -B build-cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-cmake -j
ctest --test-dir build-cmake --output-on-failure
```

Run the CLI:

```bash
printf '1 2 3 4 5' | ./build-cmake/statcli
```

### Bazel (classic WORKSPACE)

```bash
bazel build //...
bazel test //:statlib_tests --test_output=errors
```

Run the CLI:

```bash
printf '1 2 3 4 5' | bazel run //:statcli
# or, after a build:  printf '1 2 3 4 5' | ./bazel-bin/statcli
```

> A modern Bazel/bazelisk defaults to Bzlmod, which ignores `WORKSPACE`. CI
> forces the classic path with `bazel ... --config=ci` (which applies
> `--noenable_bzlmod` from `.bazelrc`). A local classic Bazel needs no extra
> flag.

### Meson (+ ninja)

```bash
meson setup build-meson
meson compile -C build-meson
meson test -C build-meson --print-errorlogs
```

Run the CLI:

```bash
printf '1 2 3 4 5' | ./build-meson/statcli
```

### xmake

```bash
xmake f -y
xmake build
xmake test
```

Run the CLI:

```bash
printf '1 2 3 4 5' | xmake run statcli
```

> xmake nests the binary under `build-xmake/<plat>/<arch>/<mode>/`.

### Expected `statcli` output

For input `1 2 3 4 5`, **all four** build systems produce byte-for-byte
identical output (population standard deviation of 1..5 = √2 ≈ 1.41421):

```
count: 5
sum: 15
mean: 3
min: 1
max: 5
median: 3
stddev: 1.41421
```

Empty input prints `error: empty input` to stderr and exits non-zero;
non-numeric input prints `error: non-numeric input` and exits non-zero.

### Build & test all four at once

```bash
./scripts/build-all.sh
```

This builds + tests every system in its own out-of-source directory, runs the
`statcli` smoke check for each, and exits non-zero if any system fails. The same
four-way gate runs in CI on every push via
[`.github/workflows/ci.yml`](.github/workflows/ci.yml) (one job per build system,
plus a bonus clang-18 job); see
[`docs/ci-cd/cpp-build-systems-compared-pipeline.md`](docs/ci-cd/cpp-build-systems-compared-pipeline.md).

### Using clang++ instead of g++

| System | Command |
|--------|---------|
| CMake | `cmake -S . -B build-cmake-clang -DCMAKE_CXX_COMPILER=clang++` |
| Bazel | `bazel build //... --action_env=CC=clang --action_env=CXX=clang++` |
| Meson | `CXX=clang++ meson setup build-meson-clang` |
| xmake | `xmake f --toolchain=clang` |

---

## The trade-off matrix

The same `statlib` + `statcli` + tests, compared across the dimensions in the
[SRS](docs/requirements/SRS.md) §5 (MR-2). Measured rows state what was measured;
qualitative rows give a one-line justification rather than a bare score.

| Dimension | CMake | Bazel | Meson | xmake |
|-----------|-------|-------|-------|-------|
| **1. Config language / syntax** | Custom CMake DSL; imperative-with-targets. Ubiquitous but idiosyncratic. | Starlark (a Python subset); declarative targets loaded from `rules_cc`. | Custom declarative DSL, deliberately non-Turing-complete; reads like a simple config. | Lua-based DSL; imperative but terse and approachable. |
| **2. Verbosity (measured LOC)** | **33** lines (`CMakeLists.txt`). | **31** lines (`BUILD.bazel`) **+ 1** (`WORKSPACE`) **+ 15** (`.bazelrc` comments/flag) = **47** total for the classic-WORKSPACE setup. | **24** lines (`meson.build`) — the most compact here. | **27** lines (`xmake.lua`). |
| **3. Clean (cold) build speed** | Fast; ninja/make backend over 3 small TUs. *Not rigorously benchmarked* — see note below. | Highest fixed overhead at this scale (sandboxing + action graph + first-run analysis). *Not rigorously benchmarked.* | Fast; ninja backend. *Not rigorously benchmarked.* | Fast. *Not rigorously benchmarked.* |
| **4. Incremental (warm) build speed** | ninja-backed incremental is quick for a single-TU change. *Not benchmarked.* | Fine-grained action caching rebuilds only what changed; strong at scale, overhead-heavy at this size. *Not benchmarked.* | ninja-backed, quick incremental. *Not benchmarked.* | Fast incremental via its own cache. *Not benchmarked.* |
| **5. Dependency handling** (how a 3rd-party dep *would* be added — not exercised) | `find_package` / `FetchContent` / `ExternalProject`; broad ecosystem, several competing styles. | `http_archive` + `WORKSPACE` (classic) or Bzlmod registries; strong hermeticity, heavier setup. | WrapDB + `.wrap` files or `dependency()`/subprojects; clean built-in resolver. | Built-in `add_requires` package manager (xrepo); batteries-included. |
| **6. Built-in test integration** | `enable_testing()` + `add_test()`, run via `ctest`. | `cc_test` is a first-class rule; `bazel test` keys on exit code. | `test()` + `meson test` runner. | `add_tests()` + `xmake test`. |
| **7. IDE / tooling (`compile_commands.json`)** | `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`; best-supported in IDEs/clangd. | Needs an aspect/extractor (e.g. `hedron compile commands`); not built-in. | Generated automatically on `meson setup`. | `xmake project -k compile_commands`. |
| **8. Learning curve / ergonomics** | Widest material + answers, but the DSL's quirks (scopes, variables, `PUBLIC`/`PRIVATE`) take time. | Steepest here: Starlark, labels, sandbox model, WORKSPACE-vs-Bzlmod confusion. | Gentlest: readable DSL, sensible defaults, clear errors. | Easy if you know Lua; smallest community/doc base of the four. |
| **9. Cross-platform support** | Broadest: huge generator/toolchain matrix (Make, Ninja, VS, Xcode). | Strong (Linux/macOS/Windows), but the hermetic model has a learning tax per platform. | Strong; first-class MSVC/Xcode/Ninja; built for portability. | Strong; targets many platforms/toolchains from one file. *(Only Linux/g++/clang validated here.)* |
| **10. Isolation / out-of-source footprint** | Explicit `-B build-cmake`; clean out-of-source. | Outputs to an external cache; only `bazel-*` symlinks appear in the repo root. | Explicit `build-meson`; clean out-of-source. | `build-xmake/` for artifacts + `.xmake/` for state; both git-ignored. |
| **11. Installation / availability (version used)** | 3.28.3; preinstalled almost everywhere, system package managers. | classic WORKSPACE; typically via bazelisk (CI pins `7.4.1`). | 1.12.1 via pipx/pip (+ ninja 1.11.1). | 3.1.1 via its install script / per-user `~/.local/bin`. |

### Verbosity, honestly

The LOC counts above are the actual line counts of the shipped driver files
(blank and comment lines included, as written in this repo). They are a rough
proxy for "ceremony," not a quality score:

- **Meson (24)** is the most compact.
- **xmake (27)** and **CMake (33)** are close behind.
- **Bazel's** `BUILD.bazel` is **31** lines, but the classic-WORKSPACE setup also
  needs a `WORKSPACE` file (**1** line) and a `.bazelrc` (**15** lines, mostly the
  explanatory comment around the single `--noenable_bzlmod` flag). Counting only
  functional lines Bazel is comparable; counting the whole classic-mode setup it
  is the most verbose here.

### On build speed

Build speed was **not rigorously benchmarked** for this release. The project is
deliberately tiny (three translation units), so any cold/warm numbers would be
dominated by each tool's fixed startup/analysis overhead rather than compilation,
and would not generalize. The qualitative read is the well-known one: the
ninja-backed tools (CMake, Meson) and xmake start fast on a project this small,
while Bazel carries more fixed overhead (sandboxing + action-graph analysis) that
only pays off on much larger builds.

**Reproducible measurement method** (if you want numbers for your machine):

```bash
# cold: wipe the build dir, then time a full configure+build
rm -rf build-cmake && time (cmake -S . -B build-cmake && cmake --build build-cmake -j)

# warm: touch one source file, then time the incremental build
touch src/statlib/statistics.cpp && time cmake --build build-cmake -j
```

Define cold = clean build dir, warm = single-file touch, and report your
compiler and machine alongside the numbers.

---

## When to pick which

**CMake** — The safe default for portable C++. You get the widest toolchain and
IDE support, the most documentation and Stack Overflow answers, and
`compile_commands.json` for clangd with one flag. The cost is a quirky DSL whose
scoping and `PUBLIC`/`PRIVATE` rules take time to internalize. Pick it when you
want maximum reach and minimum "will this tool be here in five years" risk.

**Bazel** — Shines on large, multi-language monorepos that benefit from hermetic,
reproducible, fine-grained caching and remote execution. For a three-file project
like this one its fixed overhead and the WORKSPACE-vs-Bzlmod ceremony are pure
cost. Pick it when your build is big enough that cache correctness and
scalability dominate; skip it for something this small.

**Meson** — The most pleasant to write here: a clean, readable DSL, fast ninja
backend, automatic `compile_commands.json`, and the fewest lines of config.
Dependency handling via WrapDB is tidy. Pick it for new C/C++ projects where
developer ergonomics and speed matter and you don't need CMake's universal
ubiquity.

**xmake** — A compact, batteries-included option: a terse Lua config, a built-in
package manager (xrepo), and fast builds from a single file. Its trade-off is the
smallest community and documentation base of the four. Pick it when you value an
all-in-one tool and a minimal config and are comfortable with a younger ecosystem.

---

## Project docs

- [SRS](docs/requirements/SRS.md) — requirements, acceptance criteria, matrix dimensions
- [DESIGN](docs/design/DESIGN.md) — architecture, API, target mapping, command reference
- [QA report](docs/qa/QA-REPORT.md) — verified results (all four pass; byte-identical CLI output)
- [CI/CD pipeline](docs/ci-cd/cpp-build-systems-compared-pipeline.md) — workflows + local reproduction
- [Status](docs/status.md) — current project stage
- [CHANGELOG](CHANGELOG.md)

---

## License

[MIT](LICENSE). The vendored test harness (`tests/testing/check.hpp`) is our own
code under the same MIT license — no third-party attribution required.

This repository was produced through an SDLC-style agent pipeline (requirements →
design → implementation → QA → release/CI → documentation); the `docs/`
subfolders hold each stage's artifact.
