# Marseflection

**Version:** `1.0.0` · **License:** MIT

**Header-only static reflection for C++17.**

Marseflection provides compile-time registration and inspection of C++ object properties without a runtime registry, generated registration code, or compiler extensions. It supports intrusive and non-intrusive registration, class templates, explicit base-class registration, and property lookup by name.

The core requires only the C++17 standard library. For direct integration, use the [generated single header](reflection/single_include/reflection/reflection.h).

## Repository structure

| Directory | Contents | Dependencies |
| --- | --- | --- |
| [`reflection/`](reflection/) | Core reflection library, modular headers, and single-header distribution | C++17 standard library |
| [`extensions/`](extensions/) | JSON conversion, string conversion, and comparison examples | Reflection; nlohmann/json 3.12.0 for JSON |
| [`tests/`](tests/) | GoogleTest, compile-fail, cross-translation-unit, and single-header tests | Reflection, extensions, GoogleTest, nlohmann/json |
| [`benchmarks/`](benchmarks/) | Compile-time benchmarks for both registration styles | Reflection, CMake, Python 3.9+ |
| [`tools/`](tools/) | Single-header generator and compiler-matrix runner | Python |

The core is independent of the extensions. You can use its metadata API to implement your own serializers, editors, validators, or other integrations.

## Quick start

Copy `reflection/single_include/reflection/reflection.h` into your project, preserving the `reflection/` directory name:

```text
my_project/
├── vendor/
│   └── reflection/
│       └── reflection.h
└── src/
    └── main.cpp
```

Enable C++17, add `vendor/` to the compiler's include paths, and include:

```cpp
#include <reflection/reflection.h>
```

### Intrusive registration

Declare and register fields directly inside a class:

```cpp
#include <string>
#include <reflection/reflection.h>

struct Player
{
    REFLECTION_BEGIN(Player)
    REFLECTION_PROPERTY(int, level, 1)
    REFLECTION_PROPERTY(std::string, name)
    REFLECTION_END()
};

inline constexpr char levelKey[] = "level";
using PlayerMeta = reflection::StaticPropertyMeta<Player>;

static_assert(PlayerMeta::size() == 2);
static_assert(PlayerMeta::contains<levelKey>());

int main()
{
    Player player;
    constexpr auto levelProperty = PlayerMeta::at<levelKey>();
    static_assert(levelProperty != nullptr);

    levelProperty->set(player, 5);

    PlayerMeta::forEach([&](auto entry)
    {
        using Entry = decltype(entry);
        (void)Entry::value.get(player);
    });

    return player.level == 5 ? 0 : 1;
}
```

`REFLECTION_PROPERTY(Type, Name, ...)` declares a member with optional initialization. Use `REFLECTION_PROPERTY_CUSTOM(Type, Name, "externalName", ...)` to specify its reflected name.

**Intrusive registration uses source-line scanning.** Put each `REFLECTION_PROPERTY` and `REFLECTION_BASE_CLASS` invocation on its own physical source line between `REFLECTION_BEGIN` and `REFLECTION_END`. Multiple registrations using the same `__LINE__` value are not supported.

### Non-intrusive registration

Register members without modifying their class:

```cpp
#include <reflection/reflection.h>

struct Settings
{
    int retries = 3;
    bool enabled = true;
};

REFLECTION_NONINTRUSIVE(Settings,
    &Settings::retries,
    &Settings::enabled)

static_assert(reflection::StaticPropertyMeta<Settings>::size() == 2);
```

Declare the registration after the complete type definition, in the type's enclosing namespace. Only listed fields are registered. Empty registration is supported:

```cpp
struct EmptySettings {};
REFLECTION_NONINTRUSIVE(EmptySettings)

static_assert(reflection::StaticPropertyMeta<EmptySettings>::empty());
```

Use exactly one registration style per type. Duplicate names within a metadata tuple are rejected at compile time.

### Inheritance

Base classes must be registered explicitly. Intrusive classes use `REFLECTION_BASE_CLASS(Base)` inside the registration block. For non-intrusive classes, use a separate base-class declaration:

```cpp
struct Base
{
    int id = 0;
};
REFLECTION_NONINTRUSIVE(Base, &Base::id)

struct Derived : Base
{
    int score = 0;
};
REFLECTION_NONINTRUSIVE(Derived, &Derived::score)
REFLECTION_BASE_CLASSES_NONINTRUSIVE(Derived, Base)

static_assert(reflection::StaticPropertyMeta<Derived>::size() == 2);
```

The size above counts one base-class entry and one local property. Use `reflection::StaticPropertyTraversal` to visit properties in registered base classes recursively.

Base-class traversal requires accessible, unambiguous conversions. Ambiguous repeated bases and some complex inheritance arrangements are not supported.

### Class templates

Class templates can be reflected by either registration style. For non-intrusive template registration, enclose the template declaration and arguments in parentheses:

```cpp
template <typename T>
struct Box
{
    T value{};
};

REFLECTION_TEMPLATE_NONINTRUSIVE((typename T), (T), Box,
    &Box<T>::value)

static_assert(reflection::StaticPropertyMeta<Box<int>>::size() == 1);
```

See [`tests/src/test/puretypes.h`](tests/src/test/puretypes.h) and [`tests/src/test/reflectedtypes.h`](tests/src/test/reflectedtypes.h) for additional examples.

## Metadata API

`reflection::StaticPropertyMeta<T>` describes entries registered directly on `T`, including base-class entries:

| API | Purpose |
| --- | --- |
| `getStaticTypeInfo()` | Retrieve compile-time type-name metadata |
| `size<TypeChecker>()`, `empty<TypeChecker>()` | Count matching entries or check whether a registration is empty |
| `contains<key>()`, `at<key>()` | Find a property by a compile-time key; `at()` returns `nullptr` if absent |
| `forEach(func)` | Visit registered entries in order |
| `forEachIf(func, predicate)` | Visit entries accepted by a predicate |
| `doForKey(func, key)` | Find an entry by a runtime key |
| `doForKey<key>(func)` | Find an entry by a compile-time key |

Compile-time lookup keys must have suitable static storage duration, for example:

```cpp
inline constexpr char nameKey[] = "name";
```

Visitors receive an entry tag. `decltype(entry)::key` contains the reflected name; `decltype(entry)::value` references either property metadata or base-class metadata. Property metadata offers `get(object)` and `set(object, value)`. `get` preserves constness, and `set` is constrained to assignable values.

Visitors return either `void` or `bool`; returning `false` stops the traversal. Compile-time entry filtering is available through a custom `TypeChecker`.

### Recursive base-class traversal

`reflection::StaticPropertyTraversal` visits registered base classes recursively. It does not automatically traverse nested member objects.

```cpp
Player player;
reflection::StaticPropertyTraversal::forEach(player,
    [](auto entry, auto& currentObject)
    {
        using Entry = decltype(entry);
        (void)Entry::value.get(currentObject);
    });
```

This example has no registered bases. For types with bases, the visitor also receives base-class entries, so distinguish entry types before calling property-only methods. See [`tests/src/test/reflection_traversal_tests.cpp`](tests/src/test/reflection_traversal_tests.cpp).

Traversal supports `forEachIf` and `doForKey`, both with and without an object reference. Rejecting a base-class entry in a predicate skips that base's subtree; returning `false` from a visitor stops the entire traversal.

## Registration requirements

- **Register before use.** The complete type and registration must be visible before the first instantiation of `StaticPropertyMeta<T>`.
- **Keep translation units consistent.** Every translation unit using reflection for a type must see its complete registration. Inconsistent definitions can violate the One Definition Rule (ODR).
- **Register types explicitly.** An unregistered type is not treated as an empty reflected type; `StaticPropertyMeta<T>` produces a compile-time error.
- **Use unique names.** Duplicate property names within a metadata tuple are rejected at compile time.
- **Observe the line rule.** Each intrusive field or base registration must use its own source line.
- **Consider compile-time cost.** Very large classes can be expensive to reflect and may encounter compiler/STL template-instantiation limits.

## Optional extensions

[`extensions/`](extensions/) provides examples built on the core metadata API:

| Header | API | Purpose |
| --- | --- | --- |
| `extensions/tojsonextension.h` | `extensions::convertToJson` | Serialize to `nlohmann::json` |
| `extensions/fromjsonextension.h` | `extensions::convertFromJson` | Deserialize from `nlohmann::json` |
| `extensions/tostringextension.h` | `extensions::convertToString` | Generate a readable string representation |
| `extensions/comparisonextension.h` | `extensions::compare`, `less`, `lessEqual`, `greater`, `greaterEqual`, `equal`, `notEqual` | Compare reflected objects |

The JSON extensions use **nlohmann/json 3.12.0**, supplied by an installed package or the pinned `third_party/json` submodule. The core does not require it.

Deserialization is not transactional: some fields may change before a failure is reported. Missing fields remain unchanged; unknown JSON keys are ignored. Const fields are not assigned, although supplied values may be validated in non-`NDEBUG` builds. Raw pointers are not supported by JSON conversion, and deserialization does not populate `std::string_view`.

These extensions are optional examples; consult their source and tests for supported types and edge cases.

## Building and testing

### CMake integration

The modular core provides the `reflection::reflection` interface target:

```cmake
add_subdirectory(path/to/Marseflection/reflection reflection_build)
target_link_libraries(my_target PRIVATE reflection::reflection)
```

Alternatively, include the generated single header directly; it requires neither a compiled library nor a link dependency.

### Run tests

Initialize the pinned Git submodules, then configure and build:

```sh
git submodule update --init --recursive
cmake -S tests -B build/tests -DBUILD_TESTING=ON
cmake --build build/tests --config Debug
ctest --test-dir build/tests -C Debug --output-on-failure
```

CTest runs GoogleTest cases, expected compile-failure tests, single-header verification, and compile-time benchmarks.

To compile the core headers independently:

```sh
cmake -S reflection -B build/reflection -DREFLECTION_BUILD_HEADER_CHECK=ON
cmake --build build/reflection --config Debug
```

The extensions provide a similar standalone check with `-DEXTENSIONS_BUILD_HEADER_CHECK=ON`.

### Compile-time benchmarks

The default benchmarks measure compilation of separate translation units containing a class with **10, 50, or 100 properties**, using both registration styles. They instantiate metadata queries and traversal, not just the registration macros.

Run them from an existing test build:

```sh
ctest --test-dir build/tests -C Debug -L compile-benchmark --output-on-failure
```

Or independently:

```sh
cmake -S benchmarks -B build/benchmarks -DCMAKE_BUILD_TYPE=Release
ctest --test-dir build/benchmarks -C Release -L compile-benchmark --output-on-failure
```

Optional 250- and 500-property stress cases can be enabled with `-DREFLECTION_BENCHMARK_STRESS=ON`. They are excluded from the default CI matrix because some compiler/STL versions cannot compile them. Timings include build-tool startup and are informational, not performance thresholds.

See [`benchmarks/README.md`](benchmarks/README.md).

### Compiler matrix

The [GitHub Actions workflow](.github/workflows/compiler-matrix.yml) runs strict C++17 checks in Debug and Release on:

| Compiler | Environment |
| --- | --- |
| GCC 11 | Ubuntu 22.04 |
| GCC 14 | Ubuntu 24.04 |
| GCC 15 | Ubuntu 26.04 |
| Clang 16 | Ubuntu 24.04 |
| Clang 22 | Ubuntu 26.04 |
| MSVC 2022 | Windows Server 2022 |
| MSVC 2026 | Windows VS 2026 runner |
| Apple Clang | macOS 26, ARM64 |

The workflow is started manually and uploads per-compiler Markdown reports, including benchmark measurements. [Successful matrix run](https://github.com/MarsRool/Marseflection/actions/runs/40220639453): all eight configurations passed.

The matrix can also be run locally:

```sh
python tools/run_compiler_matrix.py
```

On Windows, select the Visual Studio CMake generator with `--generator`.

## Regenerate the single header

From the repository root:

```sh
python tools/amalgamate.py
python tools/amalgamate.py --check
```

The generator consolidates the core headers deterministically, checks for missing includes, reports unused headers, and embeds the MIT license. `--check` verifies that the generated header matches the current modular sources.

**Do not edit the generated header manually.** Update `reflection/include/` and regenerate it.

## License

[MIT License](LICENSE). Copyright © 2026 **Mars Rooll**.

Keep the license notice when redistributing the library or the single-header distribution.
