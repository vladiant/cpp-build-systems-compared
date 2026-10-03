set_project("cpp-build-systems-compared")
set_version("0.1.0")

set_languages("c++17")

-- Keep all generated output inside build-xmake/ so the tree stays clean.
set_config("builddir", "build-xmake")

-- statlib: static library, public include dir exported to dependents.
target("statlib")
    set_kind("static")
    add_files("src/statlib/statistics.cpp")
    add_includedirs("include", {public = true})

-- statcli: executable linking statlib.
target("statcli")
    set_kind("binary")
    add_files("src/cli/main.cpp")
    add_deps("statlib")

-- statlib_tests: unit tests; tests/ is private to this target.
target("statlib_tests")
    set_kind("binary")
    add_files("tests/statistics_test.cpp")
    add_includedirs("tests")
    add_deps("statlib")
    add_tests("default")
