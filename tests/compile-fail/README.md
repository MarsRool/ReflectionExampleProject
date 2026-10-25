# Marseflection compile-fail tests

Compile-fail tests verify that invalid reflection usage produces the expected compiler diagnostics. Each case is a separate `.cpp` file and an object-library target excluded from the normal build.

CTest requires both conditions:

1. Compilation fails.
2. Build output contains the expected diagnostic.

The tests use the compiler and build generator selected for the test project. They run through CTest alongside the regular unit tests.

## Run

```sh
cmake -S tests -B build/tests -DBUILD_TESTING=ON
cmake --build build/tests --config Debug
ctest --test-dir build/tests -C Debug -R '^reflection_compile_fail\.' --output-on-failure
```

To run the entire test suite instead:

```sh
ctest --test-dir build/tests -C Debug --output-on-failure
```

The compile-fail tests share a build tree and use a CTest resource lock to prevent concurrent nested builds.
