# Marseflection compile-time benchmarks

Each benchmark compiles one C++17 translation unit containing a class with N `int` properties. Intrusive and non-intrusive registration are measured separately.

The generated source checks `StaticPropertyMeta<Record>::size() == N` and instantiates `Meta::forEach` with property access. Measurements therefore include registration, metadata, and traversal compilation.

The default scenarios use **10, 50, and 100 properties**, in both Debug and Release. Add 250- and 500-property stress cases with `-DREFLECTION_BENCHMARK_STRESS=ON`. These larger cases may reach compiler or standard-library implementation limits and are not part of the default CI run.

## Run with the test suite

```sh
cmake -S tests -B build/tests -DBUILD_TESTING=ON
cmake --build build/tests --config Debug
ctest --test-dir build/tests -C Debug -L compile-benchmark --output-on-failure
```

## Run independently

```sh
cmake -S benchmarks -B build/benchmarks -DCMAKE_BUILD_TYPE=Release
ctest --test-dir build/benchmarks -C Release -L compile-benchmark --output-on-failure
```

Each case has its own `EXCLUDE_FROM_ALL` object-library target named `reflection_compile_benchmark_{intrusive,nonintrusive}_{N}`. CTest changes its generated source in the build directory to force recompilation, so repeated runs are not satisfied by cached object files. No linking is required.

The cases run sequentially to avoid competing for CPU and memory. Reported times include CMake/build-tool startup (including MSBuild on Windows), so they are **wall-clock build times**, not isolated compiler front-end times. Cross-machine comparisons are affected by runner hardware and system load.

Compilation errors fail the test; timing values are informational and do not have pass/fail thresholds.

JSON results are written under `build/.../benchmarks/results/<config>/` and included in compiler-job reports by `tools/run_compiler_matrix.py`.
