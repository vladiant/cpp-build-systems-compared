# QA Report — cpp-build-systems-compared

**Project:** cpp-build-systems-compared ("Same source, different builds.")
**Reviewed commit:** `32a8458` — *semver(minor): implement statlib/statcli and CMake, Bazel, Meson, xmake builds*
**Date:** 2026-10-03
**QA engineer:** QA agent
**Verified against:** `docs/requirements/SRS.md` §9 (Acceptance Criteria) and `docs/design/DESIGN.md` §4.1/§4.2/§5/§6

---

## 1. Toolchain used (all local, offline)

| Tool | Version |
|------|---------|
| CMake | 3.28.3 |
| g++ (default) | 13.3.0 (Ubuntu 13.3.0) |
| clang++ (available, not required) | 18.1.3 |
| Bazel | `no_version` (classic WORKSPACE; bzlmod not used) |
| Meson | 1.12.1 (ninja 1.11.1 backend) |
| xmake | v3.1.1 |

Each build system was run in its own out-of-source directory (`build-cmake/`, `build-meson/`,
`build-xmake/` + `.xmake/`, Bazel `bazel-*` symlinks). All are covered by `.gitignore`.

---

## 2. Results matrix (4 build systems × {configure, build, test, cli-output})

| Build system | Configure | Build (statlib+statcli+tests) | Test (run + pass) | CLI output == §4.2 |
|--------------|:---------:|:-----------------------------:|:-----------------:|:------------------:|
| **CMake** 3.28.3 | ✅ PASS | ✅ PASS | ✅ PASS (1 ctest, 20 checks) | ✅ byte-for-byte |
| **Bazel** (WORKSPACE) | ✅ (BUILD files) | ✅ PASS | ✅ PASS (`//:statlib_tests`) | ✅ byte-for-byte |
| **Meson** 1.12.1 | ✅ PASS | ✅ PASS | ✅ PASS (1 meson test) | ✅ byte-for-byte |
| **xmake** 3.1.1 | ✅ PASS | ✅ PASS | ✅ PASS (`statlib_tests/default`) | ✅ byte-for-byte |

The test binary runs **20 registered test cases** (19 original + 1 QA-added, see §6).
Each build system reports 1 aggregate test target that wraps the harness; the harness prints
`PASSED 20 / FAILED 0`.

All four `statcli` binaries produced **identical** output on input `1 2 3 4 5`
(md5 `b5c97c1aa33f068d402ba77c510b633b` for all four).

---

## 3. Observed `statcli` output (input `printf '1 2 3 4 5'`)

Identical across CMake / Bazel / Meson / xmake, byte-for-byte equal to DESIGN §4.2:

```
count: 5
sum: 15
mean: 3
min: 1
max: 5
median: 3
stddev: 1.41421
```

Hex tail confirms exactly one trailing `\n` after `stddev: 1.41421` and no `version:` line,
no `std::fixed` (`3` prints `3`, not `3.000000`; `1.41421` at `setprecision(6)` default float):

```
...0a73 7464 6465 763a 2031 2e34 3134 3231   .stddev: 1.41421
 0a                                           .
```

### Error paths (verified on CMake-built `statcli`; confirmed again under ASan+UBSan build)

| Input | stdout | stderr | exit |
|-------|--------|--------|------|
| empty stdin (`printf ''`) | *(empty)* | `error: empty input` | 1 (non-zero) ✅ |
| non-numeric (`printf '1 2 abc'`) | *(empty)* | `error: non-numeric input` | 1 (non-zero) ✅ |

Messages match DESIGN §4.1 exactly; stdout is clean on error paths.

---

## 4. Acceptance Criteria — pass/fail

| AC | Description | Result | Evidence |
|----|-------------|:------:|----------|
| **AC-1** | Single shared source tree, no per-build-system duplication | ✅ PASS | All four drivers reference the *same* `src/statlib/statistics.cpp`, `src/cli/main.cpp`, `tests/statistics_test.cpp`. Exactly one copy of each file exists under `src/`, `include/`, `tests/`. |
| **AC-2** | CMake builds statlib+statcli+tests, tests pass | ✅ PASS | Build + `ctest` 100% pass. |
| **AC-3** | Bazel (classic WORKSPACE) builds all, tests pass | ✅ PASS | `bazel build //...` OK; `bazel test //:statlib_tests` → 1 test passes. `WORKSPACE` present, no `MODULE.bazel`. |
| **AC-4** | Meson (ninja) builds all, tests pass | ✅ PASS | `meson setup/compile/test` → Ok:1 Fail:0. |
| **AC-5** | xmake builds all, tests pass | ✅ PASS | `xmake f/build/test` → 100% tests passed. |
| **AC-6** | Every build system's statcli produces correct stats | ✅ PASS | All four byte-identical to the §4.2 worked example (§3 above). |
| **AC-7** | All build offline, local toolchain only | ✅ PASS | No `http_archive`/network fetch; all builds completed without network. |
| **AC-8** | Compiles under C++17 / g++ 13.3 by default | ✅ PASS | All four set `c++17`; default g++ 13.3 used, no warnings/errors. |
| **AC-9** | Isolated build dirs; `git status` shows no new tracked artifacts | ✅ PASS | `git status` clean except the intentionally-added test file (§6). All build dirs gitignored. |
| **AC-10** | Dependency-free test framework, no package manager | ✅ PASS | `tests/testing/check.hpp` includes only standard headers (`<cmath> <cstdlib> <functional> <iostream> <string> <vector>`). No third-party deps. |
| **AC-11** | README trade-off matrix (all four systems, MR-2 dims) | ⚠️ NOT ASSESSED | Out of QA scope for this pass (Technical Writer deliverable, MR-2). Flagged, not failed. |
| **AC-12** | Forced-fail test → non-zero exit code | ✅ PASS | Scratch test with failing `CHECK`/`CHECK_NEAR` (compiled against the real `check.hpp`, outside the repo) exits **1**; harness propagates non-zero on any failure. |

---

## 5. Failing-assertion exit-code check (AC-10 / AC-12 / FR-15 / BR-3)

A scratch file (`/tmp/fail_test.cpp`, **not** added to the repo) was compiled against the real
`tests/testing/check.hpp` with two deliberately failing checks plus one passing test:

```
FAIL: /tmp/fail_test.cpp:3: CHECK(1 + 1 == 3) failed
PASSED 1 / FAILED 1
FAIL: /tmp/fail_test.cpp:4: CHECK_NEAR(0.0, 1.0, 1e-9) failed: 0.000000 vs 1.000000
  (in test: deliberate_fail)
EXIT=1
```

`STATLIB_TEST_MAIN()` → `run_all()` returns `EXIT_FAILURE` when `failed != 0`. Since every build
system runs the test target as a subprocess and keys pass/fail on its exit code, a forced failure
is correctly reported as a test failure by ctest / bazel test / meson test / xmake test. **Confirmed.**

---

## 6. Coverage assessment & QA-added test

The developer suite (19 cases) already covers: odd/even median, single-element, empty-throw for
every throwing function, `sum([]) == 0`, `count([]) == 0`, population variance with divisor **N**
(2.0 for 1..5, not the sample 2.5 — population-vs-sample distinction verified), all-equal/zero-variance,
negative values in min/max, and non-mutation of the median argument.

**Gap found and closed:** the even-count median branch — the trickiest path, which recovers the
lower median via `std::max_element` over the `nth_element`-partitioned lower half `[begin, mid)` —
was only exercised with **already-sorted** inputs (`median_even_count`), while the unsorted test
(`median_unsorted_input`) is **odd-count**. So correctness of the even-count branch on *unsorted*
data (where the partition actually matters) was untested.

**Added** one dependency-free test case to `tests/statistics_test.cpp`
(same harness, no library change):

```cpp
STATLIB_TEST(median_even_count_unsorted) {
  std::vector<double> a = {4.0, 1.0, 3.0, 2.0};
  const std::vector<double> a_before = a;
  CHECK_NEAR(statlib::median(a), 2.5, kEps);     // sorted 1,2,3,4 -> (2+3)/2
  CHECK(a == a_before);                          // argument not mutated
  CHECK_NEAR(statlib::median({10.0, 2.0, 8.0, 4.0, 6.0, 0.0}), 5.0, kEps); // (4+6)/2
}
```

New total: **20 tests, all passing under all four build systems.** This is the **only** repo change
(`git diff --stat`: `tests/statistics_test.cpp | 14 +++`). It does not alter library behavior.

### Integration & sanitizer testing
- **End-to-end chain** (stdin parse → statlib compute → §4.2 serialize) verified via `statcli` on
  success, empty, and non-numeric inputs for the CMake build and re-verified on an ASan+UBSan build.
- **ASan + UBSan** (`-fsanitize=address,undefined`, g++): library+tests and library+CLI built and run
  with `detect_leaks=1`. **Zero findings** — no memory errors, leaks, or UB. All 20 tests pass under
  sanitizers; CLI success/empty/non-numeric paths behave identically.
- **TSan:** not applicable — the code is single-threaded with no shared mutable state, thread pools,
  or async I/O.

---

## 7. Defects found

**None.** No functional, formatting, memory, or UB defects observed. No design flaws identified —
the empty-input contract (`sum([])==0`, others throw `std::invalid_argument`) is implemented and
tested consistently with DESIGN OQ-1.

Minor, non-blocking notes (informational, no action required):
- Bazel emits a benign warning: *"tests whose specified size is too big"* — cosmetic test-size hint,
  not a failure. Could be silenced with `size = "small"` on the `cc_test` if desired.
- AC-11 (README trade-off matrix) was not evaluated in this pass — it is a Technical Writer
  deliverable outside functional-QA scope.

---

## 8. Sign-off

**Result: PASS.**

All functional acceptance criteria in scope (AC-1 through AC-10, AC-12) are met across all four
build systems; `statcli` output is byte-for-byte identical to DESIGN §4.2 everywhere; error paths,
exit codes, and the forced-failure contract behave as specified; the suite is clean under ASan+UBSan.
AC-11 (README matrix) is deferred to the Technical Writer and was not failed.

The source tree remains clean apart from one QA-added, behavior-neutral test case
(`median_even_count_unsorted` in `tests/statistics_test.cpp`) — **do not commit from QA; the PM
commits.**

**Testing complete — ready for the Release Engineer agent to deploy.**
(Pending only the PM's decision to commit the added regression test and, optionally, the Technical
Writer's AC-11 README verification.)
