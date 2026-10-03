# CI/CD Pipeline — cpp-build-systems-compared

This document describes the Deployment-stage automation for the project and how
to reproduce every stage locally. The project's whole point is that **one source
tree** (`statlib` + `statcli` + unit tests) builds under **four** build systems;
CI proves all four stay green on every push.

- Source of truth: `.github/workflows/ci.yml`, `.github/workflows/release.yml`
- Local counterpart: `scripts/build-all.sh`
- Bazel config: `.bazelrc` (the `ci` config)

---

## 1. CI workflow (`.github/workflows/ci.yml`)

**Triggers:** push to `main`/`master`, any pull request, and manual
`workflow_dispatch`. In-progress runs for the same ref are cancelled when a new
commit arrives (`concurrency`), to conserve free-tier minutes.

**Structure:** one **job per build system** (not a single matrix axis), so a
failure in the GitHub UI points straight at the offending tool. All run on
`ubuntu-latest` with **C++17** and **g++** (default), plus one bonus clang job.

| Job           | Runner        | Tool provisioning                                    | Build + test commands |
|---------------|---------------|------------------------------------------------------|-----------------------|
| `cmake`       | ubuntu-latest | preinstalled CMake + g++                             | `cmake -S . -B build-cmake -DCMAKE_BUILD_TYPE=Release` → `cmake --build` → `ctest --output-on-failure` |
| `bazel`       | ubuntu-latest | `bazelbuild/setup-bazelisk@v3`, `USE_BAZEL_VERSION=7.4.1` | `bazel build --config=ci //...` → `bazel test --config=ci //... --test_output=errors` |
| `meson`       | ubuntu-latest | `pipx install meson` (Ninja preinstalled, apt fallback) | `meson setup build-meson` → `meson compile` → `meson test --print-errorlogs` |
| `xmake`       | ubuntu-latest | `xmake-io/github-action-setup-xmake@v1` (pin 3.1.1) | `xmake f -y` → `xmake build` → `xmake test` |
| `cmake-clang` | ubuntu-24.04  | clang-18 from the runner image (apt fallback)       | same as `cmake`, with `CC=clang-18 CXX=clang++-18` (**bonus: compiler portability**) |

**Every** job finishes with the same **statcli smoke check**:

```bash
printf '1 2 3 4 5' | <statcli>   # assert output contains  stddev: 1.41421
```

The job fails if the test stage fails **or** if the smoke check's `grep` finds
no match — a red pipeline is a real defect to fix upstream, never something to
paper over.

### Why `--config=ci` for Bazel?

The repo uses the **classic WORKSPACE** model (`WORKSPACE` + `BUILD.bazel`, no
`MODULE.bazel`). Modern Bazel / bazelisk default to **Bzlmod**, which ignores
WORKSPACE and breaks the build. `.bazelrc` defines:

```
common:ci --noenable_bzlmod
```

CI activates it with `--config=ci`, forcing the WORKSPACE path. The flag is
scoped to a config (not global `common`) so that older local Bazel installs that
predate the option are unaffected — `scripts/build-all.sh` runs plain
`bazel build //...` without `--config=ci`. `USE_BAZEL_VERSION=7.4.1` pins a
release that still supports both WORKSPACE and `--noenable_bzlmod`.

---

## 2. Release workflow (`.github/workflows/release.yml`)

**Trigger:** pushing a SemVer tag matching `v*` (e.g. `v1.2.3`). Also runnable
via `workflow_dispatch` with a `ref` input (artifact only, no GitHub Release).

Stages (single `package` job, `ubuntu-latest`):

1. **Resolve version** — reads the `VERSION` file if present (PM-managed,
   SemVer); otherwise derives it from the tag name (strips the leading `v`).
2. **Build Release** — `cmake -DCMAKE_BUILD_TYPE=Release`, build, then
   `ctest --output-on-failure` + smoke check **gate** the release (never ship an
   untested binary).
3. **Package** — stage `statcli` (`bin/`), the public headers (`include/`), the
   `LICENSE`, and a `VERSION` stamp into
   `cpp-build-systems-compared-<version>-linux-x86_64/`, then `tar -czf` it and
   emit a `.sha256`.
4. **Publish** — upload the tarball + checksum as a workflow artifact
   (`actions/upload-artifact@v4`), and for real tag pushes create/attach a
   **GitHub Release** (`softprops/action-gh-release@v2`,
   `generate_release_notes: true`). `permissions: contents: write` enables this.

> The `VERSION` file and the git tag are created by the PM at the final release
> step (via the `semver-version-publish` skill). This workflow only *reads*
> `VERSION`; it never creates it.

---

## 3. Reproduce locally

Run the exact four-build gate CI runs, with a PASS/FAIL summary:

```bash
./scripts/build-all.sh
```

The script builds+tests each system in its own out-of-source dir, runs the
statcli smoke check for each, and exits non-zero if any system fails. It prefers
`~/.local/bin` for `meson`/`xmake` (where this repo installs them).

Individual systems:

```bash
# CMake
cmake -S . -B build-cmake && cmake --build build-cmake -j && (cd build-cmake && ctest --output-on-failure)

# Bazel (classic WORKSPACE; local Bazel needs no --config=ci)
bazel build //... && bazel test //... --test_output=errors

# Meson (+ ninja)
meson setup build-meson && meson compile -C build-meson && meson test -C build-meson

# xmake
xmake f -y && xmake build && xmake test
```

Simulate the release packaging locally:

```bash
cmake -S . -B build-cmake -DCMAKE_BUILD_TYPE=Release && cmake --build build-cmake -j
version="$(cat VERSION 2>/dev/null || echo 0.0.0-dev)"
pkg="cpp-build-systems-compared-${version}-linux-x86_64"
mkdir -p "dist/${pkg}/bin" "dist/${pkg}/include"
cp build-cmake/statcli "dist/${pkg}/bin/" && cp -r include/* "dist/${pkg}/include/" && cp LICENSE "dist/${pkg}/"
tar -czf "dist/${pkg}.tar.gz" -C dist "${pkg}"
```

---

## 4. Caveats

- **Runner tool-version drift.** `ubuntu-latest` ships rolling CMake/g++/Ninja
  versions; they may differ from the locally verified set (CMake 3.28.3, g++
  13.3, Ninja 1.11). The C++17 code is standard and tolerant of this, but exact
  compiler versions are not pinned for the g++ jobs.
- **Bazel pin.** `USE_BAZEL_VERSION=7.4.1` is deliberate: Bazel 9 removes
  WORKSPACE support and `--noenable_bzlmod`. Bump only to another 7.x/8.x
  release that still supports WORKSPACE.
- **clang job runner.** The bonus clang-18 job targets `ubuntu-24.04`, which
  bundles clang-18; if that image stops shipping it, the apt fallback installs
  it.
- **Local vs CI Bazel.** The local machine used for sign-off carries a minimal
  "development version" Bazel stub that does not recognise `--noenable_bzlmod`;
  that is exactly why the flag is config-scoped. CI's bazelisk-provided Bazel
  does support it.
