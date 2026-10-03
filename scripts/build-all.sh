#!/usr/bin/env bash
#
# build-all.sh — build AND test the project under all four build systems in
# isolated out-of-source directories, then print a PASS/FAIL summary.
#
# This is the local counterpart to .github/workflows/ci.yml. It is what a
# developer runs before pushing, and what the Technical Writer references.
#
# Usage:   scripts/build-all.sh
# Exit:    0 if every build system builds, tests, and passes the CLI smoke
#          check; non-zero otherwise.
#
# Notes:
#   * meson and xmake are picked up from ~/.local/bin if present (that is where
#     this project installs them); otherwise they must be on PATH.
#   * Bazel here uses the plain WORKSPACE build (no --config=ci): CI pins a
#     modern bazelisk that defaults to Bzlmod and therefore needs
#     --config=ci / --noenable_bzlmod; a local classic Bazel does not.
#   * The CLI smoke check feeds "1 2 3 4 5" to statcli and asserts the
#     population standard deviation line, mirroring DESIGN §4.2.
set -euo pipefail

# --- locate repo root (script lives in <root>/scripts) ------------------------
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

# Prefer per-user tool installs (meson/xmake land in ~/.local/bin for this repo).
export PATH="$HOME/.local/bin:$PATH"

SMOKE_INPUT='1 2 3 4 5'
SMOKE_EXPECT='stddev: 1.41421'

# Track results for the final summary table.
declare -A RESULT
OVERALL=0

# run_stage <name> -- runs the remaining args as a command, recording pass/fail
# without aborting the whole script (so every build system gets a turn).
run_stage() {
  local name="$1"; shift
  echo "::: ${name} :::"
  if "$@"; then
    RESULT["$name"]="PASS"
  else
    RESULT["$name"]="FAIL"
    OVERALL=1
  fi
  echo
}

# smoke_check <path-to-statcli> -- feed SMOKE_INPUT, assert SMOKE_EXPECT present.
smoke_check() {
  local bin="$1"
  local out
  out="$(printf '%s' "$SMOKE_INPUT" | "$bin")"
  echo "$out"
  grep -qF "$SMOKE_EXPECT" <<<"$out"
}

# --- CMake --------------------------------------------------------------------
build_cmake() {
  cmake -S . -B build-cmake
  cmake --build build-cmake -j
  ( cd build-cmake && ctest --output-on-failure )
  smoke_check build-cmake/statcli
}

# --- Bazel (classic WORKSPACE) ------------------------------------------------
build_bazel() {
  bazel build //...
  bazel test //... --test_output=errors
  smoke_check bazel-bin/statcli
}

# --- Meson (+ ninja) ----------------------------------------------------------
build_meson() {
  # --reconfigure keeps setup idempotent across repeated local runs.
  if [ -d build-meson ]; then
    meson setup --reconfigure build-meson
  else
    meson setup build-meson
  fi
  meson compile -C build-meson
  meson test -C build-meson
  smoke_check build-meson/statcli
}

# --- xmake --------------------------------------------------------------------
build_xmake() {
  xmake f -y
  xmake build
  xmake test
  # xmake nests the binary under <builddir>/<plat>/<arch>/<mode>/.
  local xbin
  xbin="$(find build-xmake -type f -name statcli | head -n1)"
  smoke_check "$xbin"
}

run_stage cmake build_cmake
run_stage bazel build_bazel
run_stage meson build_meson
run_stage xmake build_xmake

# --- summary ------------------------------------------------------------------
echo "================ build-all summary ================"
for bs in cmake bazel meson xmake; do
  printf '  %-6s : %s\n' "$bs" "${RESULT[$bs]:-SKIPPED}"
done
echo "==================================================="

if [ "$OVERALL" -eq 0 ]; then
  echo "ALL BUILD SYSTEMS PASSED"
else
  echo "ONE OR MORE BUILD SYSTEMS FAILED" >&2
fi
exit "$OVERALL"
