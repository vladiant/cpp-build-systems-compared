# Software Requirements Specification — cpp-build-systems-compared

**Project:** cpp-build-systems-compared
**Tagline:** "Same source, different builds."
**Status:** Draft for handoff to System Architect
**Last updated:** 2026-10-03

---

## 1. Purpose & Scope

### 1.1 Purpose
Build **one** small, self-contained C++ project whose **identical source tree** is compiled,
linked, and tested by **four** different build systems — **CMake, Bazel, Meson, and xmake** —
so they can be compared side by side. The headline deliverable is a **trade-off matrix**
(in the repo README) contrasting the four build systems across practical dimensions.

### 1.2 In Scope
- A single shared C++ source tree (library + CLI + tests), domain-trivial but structurally non-trivial.
- Four parallel build configurations (one per build system), each driving the **same** sources.
- A comparison/trade-off matrix documenting build-system differences.
- Local, offline, Linux-based builds with g++ (default) and optionally clang++.

### 1.3 Out of Scope
- Any third-party library dependency (math, CLI parsing, logging, package-manager test framework, etc.).
- Packaging, installation, distribution artifacts, or publishing to registries.
- Cross-compilation and non-Linux platform validation (may be *noted* as a matrix dimension, not a deliverable).
- CI pipelines, remote caching farms, container images.
- Runtime performance benchmarking of the C++ application (only *build* characteristics are compared).

---

## 2. The Single Shared Application

### 2.1 Chosen Domain (recommendation)
A small, dependency-free **descriptive statistics library** named `statlib`, exercised by a CLI
named `statcli`, with a unit-test suite. Statistics is self-contained, needs no third-party code,
produces deterministic outputs that are easy to assert, and is non-trivial enough to justify a
real library/CLI/test split.

> The domain is a recommendation to make the SRS concrete; the Architect may substitute an
> equivalent dependency-free domain (e.g., simple string/text processing) provided it still yields
> a reusable library + CLI + tests of equivalent structural complexity.

### 2.2 Structure (shared by all four build systems)
1. A reusable **library** target (`statlib`) built from shared headers + sources.
2. A **command-line executable** (`statcli`) that links `statlib`.
3. A **unit-test suite** that links `statlib` and exercises its public API.

No source file may be copied or duplicated per build system; all four build systems consume the
exact same files under a single shared source tree.

---

## 3. Functional Requirements

### 3.1 Library (`statlib`)
- **FR-1:** Compute the **arithmetic mean** of a non-empty sequence of `double` values.
- **FR-2:** Compute the **population variance** and **standard deviation** of a non-empty sequence.
- **FR-3:** Compute the **minimum** and **maximum** of a non-empty sequence.
- **FR-4:** Compute the **median** of a non-empty sequence (correct for both even and odd counts).
- **FR-5:** Compute the **sum** of a sequence of `double` values.
- **FR-6:** Define a well-specified, consistent behavior for **empty input** across all functions
  (e.g., error / throw / error type). Exact mechanism is an open question (see §8).
- **FR-7:** Declare the public API in public headers under the shared source tree, usable by both the
  CLI and tests without modification.

### 3.2 Command-Line Executable (`statcli`)
- **FR-8:** Accept a series of numeric values as input (args and/or stdin — mechanism is open, see §8).
- **FR-9:** Print the computed statistics (at minimum: count, sum, mean, min, max, median, std-dev) in
  human-readable form to standard output.
- **FR-10:** Exit `0` on success, non-zero on invalid input (non-numeric tokens, empty input).
- **FR-11:** Perform all computation by calling `statlib` (no duplicated math logic in the CLI).

### 3.3 Unit Tests
- **FR-12:** Verify each public `statlib` function against known, hand-computed expected values
  (including even/odd median cases and a single-element sequence).
- **FR-13:** Verify the defined empty-input behavior (FR-6).
- **FR-14:** Use floating-point comparisons with an explicit tolerance (epsilon), not exact equality.
- **FR-15:** The test runner shall **exit non-zero if any assertion fails**, so each build system can
  detect failure from the process exit code.
- **FR-16:** Use a **dependency-free** testing approach — either a single-header assertion framework
  **vendored in the repo** or a hand-rolled minimal assertion harness — so none of the four build
  systems needs package-manager or network integration to build/run tests. (Reinforced as NFR-7.)

---

## 4. Build-System Requirements

Each build system — **CMake, Bazel, Meson, xmake** — shall, **from the same shared source tree**:

- **BR-1:** Build the `statlib` library target.
- **BR-2:** Build the `statcli` executable, linking against `statlib`.
- **BR-3:** Build and **run** the unit-test suite, surfacing pass/fail via exit code.
- **BR-4:** Consume identical, un-duplicated source files (no per-build-system `.cpp`/`.h` copies).
- **BR-5:** Produce outputs in its **own isolated build directory**, with no cross-contamination and no
  artifacts committed to source control.
- **BR-6:** Build fully **offline** using only the locally installed toolchain (no configure/build downloads).
- **BR-7:** Default to **g++ 13.3**; optionally be configurable to **clang++ 18**.
- **BR-8 (Bazel-specific):** Use **classic WORKSPACE mode** (a `WORKSPACE` file), **not** bzlmod /
  `MODULE.bazel`, matching the verified local Bazel.
- **BR-9:** Expose a documented, minimal-command path for configure → build → test, keeping the
  comparison fair and reproducible.

---

## 5. Trade-off Matrix Requirement

- **MR-1:** The repo README shall contain a **trade-off matrix** comparing CMake, Bazel, Meson, and xmake.
- **MR-2:** The matrix shall cover **at least** these dimensions:
  1. Configuration language / syntax (declarative vs. imperative; language used).
  2. Verbosity / lines-of-config to express the same library+CLI+tests.
  3. Clean (cold) build speed on this project.
  4. Incremental (warm) build speed after a single-file change.
  5. Dependency-handling model (how a third-party dep *would* be added — noted, not exercised).
  6. Built-in test integration (how running tests is wired up).
  7. IDE / editor / tooling support (e.g., `compile_commands.json` generation).
  8. Learning curve / ergonomics (qualitative).
  9. Cross-platform support (qualitative, documentation-based).
  10. Isolation / out-of-source build behavior and generated-file footprint.
  11. Installation / availability (how the tool is obtained; version used).
- **MR-3:** Measured dimensions (build speeds) shall state the measurement method and environment
  (compiler, machine, warm/cold definitions) for reproducibility.
- **MR-4:** Qualitative ratings shall include a one-line justification, not just a score.

---

## 6. Non-Functional Requirements

- **NFR-1:** **No external or network dependencies** at configure, build, or test time.
- **NFR-2:** **Offline-buildable** using only pre-installed local tooling.
- **NFR-3:** C++ baseline standard is **C++17** for all targets and all build systems.
- **NFR-4:** **Shall build with g++ 13.3** (default); clang++ 18 support is optional/bonus.
- **NFR-5:** Builds shall be **reproducible**: same inputs yield same outputs on repeated runs.
- **NFR-6:** Each build shall be **isolated** in its own build directory; source tree stays clean.
- **NFR-7:** The test framework shall be **dependency-free** (vendored single-header or hand-rolled).
- **NFR-8:** The shared source tree shall be **build-system-agnostic** (no build-system `#ifdef`s or
  conditional source selection).
- **NFR-9:** Licensing: project stays **MIT**; any vendored test header must carry a compatible
  permissive license and attribution.

---

## 7. Constraints & Assumptions

- **C-1:** Target OS is **Linux** only for the deliverable builds.
- **C-2:** Verified tooling: CMake 3.28.3, Bazel (classic WORKSPACE), Meson 1.12.1 + ninja 1.11.1,
  xmake 3.1.1; g++ 13.3 and clang++ 18. `meson` and `xmake` live in `~/.local/bin`.
- **C-3:** Greenfield repo: currently only `LICENSE` (MIT), `.gitignore`, and `.github/` configs.
- **A-1:** The grader/QA environment matches the verified tooling versions above.
- **A-2:** Single-developer/portfolio context — enterprise concerns are intentionally excluded.
- **A-3:** Build-speed numbers are indicative of this project's scale, not general benchmarks.

---

## 8. Open Questions

- **OQ-1:** Empty-input contract (FR-6): throw, return `std::optional`, or error code/struct?
- **OQ-2:** `statcli` input mechanism (FR-8): args, stdin, file path, or a combination?
- **OQ-3:** Exact CLI output format (FR-9): plain lines, aligned table, or key=value? Precision rules?
- **OQ-4:** Vendored single-header framework vs. hand-rolled harness (FR-16/NFR-7) — Architect's choice,
  but must stay dependency-free.
- **OQ-5:** Is clang++ 18 support a required deliverable or a bonus matrix dimension?
- **OQ-6:** Build the library **static**, **shared**, or **both** across build systems (and is this itself
  a matrix dimension)?

---

## 9. Acceptance Criteria

1. **AC-1:** A single shared source tree contains `statlib`, `statcli`, and test sources with **no
   per-build-system duplication** of any source file.
2. **AC-2:** **CMake** builds `statlib`, builds `statcli`, and builds+runs tests; all tests pass.
3. **AC-3:** **Bazel** (classic WORKSPACE) builds `statlib`, builds `statcli`, and builds+runs tests; all pass.
4. **AC-4:** **Meson** (with ninja) builds `statlib`, builds `statcli`, and builds+runs tests; all pass.
5. **AC-5:** **xmake** builds `statlib`, builds `statcli`, and builds+runs tests; all pass.
6. **AC-6:** For every build system, `statcli` on a known input sample produces correct statistics
   (matching hand-computed expected values).
7. **AC-7:** All four build systems build successfully **offline** using only the verified local toolchain.
8. **AC-8:** All targets compile under **C++17** with **g++ 13.3** by default.
9. **AC-9:** Each build system writes only to its **own isolated build directory**; `git status` shows no
   new tracked build artifacts.
10. **AC-10:** The test suite uses a **dependency-free** framework — no package manager/download required.
11. **AC-11:** The README contains the **trade-off matrix** covering all four build systems and all MR-2
    dimensions, with the measurement method noted for any build-speed figures.
12. **AC-12:** A test intentionally forced to fail makes the corresponding build system's test command
    report failure via a **non-zero exit code** (validating FR-15/BR-3).

---

*Requirements are ready for the System Architect agent to design against.*
