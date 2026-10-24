# Compile-fail tests

Each `.cpp` file is deliberately invalid. CMake creates a separate object-library
build target for each case, excluded from the normal build (`EXCLUDE_FROM_ALL`).
CTest invokes `cmake --build --target ...` for the chosen target and requires both:

1. Compilation must fail.
2. The compiler/build output must contain the expected diagnostic.

The tests use the **same generator, compiler, toolchain and dependency include
paths** as the rest of the project. No Python or Visual Studio batch files are
required for CTest.

## With CTest

Configure and build the regular test executable:

```sh
cmake -S tests -B build/tests
cmake --build build/tests --config Debug
```

Run compile-fail cases (15 individually reported tests):

```sh
ctest --test-dir build/tests -C Debug -R '^reflection_compile_fail\.' --output-on-failure
```

Or run all tests:

```sh
ctest --test-dir build/tests -C Debug --output-on-failure
```

When using the Visual Studio CMake generator, CMake selects the MSVC installation
at **configuration time**. `CMAKE_GENERATOR_INSTANCE` can pin a specific installed
instance. A different generator or instance requires a new build directory.

Tests share the build tree. Their `RESOURCE_LOCK` prevents simultaneous nested
build commands when CTest is run with `-j`.
