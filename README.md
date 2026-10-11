# Reflection

**Header-only static reflection for C++17.**

Reflection provides compile-time registration and inspection of C++ object properties, without modifying the language or requiring a runtime reflection registry. It supports both intrusive and non-intrusive registration, including inheritance and class templates.

The core library depends only on the C++17 standard library. Qt is not required.

> **Recommended distribution:** [`reflection/single_include/reflection/reflection.h`](reflection/single_include/reflection/reflection.h)  
> One public header, no compiled library, no build system required.

## What is in this repository?

| Component | Purpose | Dependencies | Distribution |
| --- | --- | --- | --- |
| [`reflection/`](reflection/) | Core compile-time reflection library | C++17 standard library | **Recommended:** generated single-header |
| [`extensions/`](extensions/) | Example mechanics built on reflection: JSON conversion, string conversion, comparison | Reflection; Qt Core for JSON support | Optional examples, not part of the core API |
| [`tests/`](tests/) | Compile-time, runtime, and cross-translation-unit regression tests | Reflection, extensions, GoogleTest, Qt Core | Internal development, not intended for consumers |

The repository is organized so consumers can use the core independently and write their own extensions for their own data formats and application frameworks.

```text
ReflectionExampleProject/
├── reflection/
│   ├── include/reflection/                   # Modular core headers
│   ├── single_include/reflection/reflection.h # Distributable header
│   ├── CMakeLists.txt
│   └── reflection.pri
├── extensions/
│   ├── includes/extensions/                  # Optional example extensions
│   ├── CMakeLists.txt
│   └── extensions.pri
├── tests/                                    # GoogleTest suite
│   ├── src/
│   ├── CMakeLists.txt
│   └── tests.pro
├── third_party/googletest/                   # Git submodule, for tests only
├── tools/amalgamate.py                      # Single-header generator
└── LICENSE
```

## Use the library

Copy **only** `reflection/single_include/reflection/reflection.h` into your project, keeping the `reflection/` directory prefix. Add its parent directory to your compiler's include paths:

```text
my_project/
├── vendor/reflection/reflection.h
└── src/main.cpp
```

Compile with C++17 and `vendor/` on the include path. No linking step is needed for the library.

### Intrusive registration

```cpp
#include <string>
#include <reflection/reflection.h>

struct Player
{
    DECL_REFLECTION_BODY(Player)

    DECL_PROPERTY_INIT(int, level, 1)
    DECL_PROPERTY_DEFAULT(std::string, name)
};

inline constexpr char levelKey[] = "level";
using PlayerMeta = reflection::StaticPropertyMeta<Player>;

static_assert(reflection::IsObject<Player>::value);
static_assert(PlayerMeta::size() == 2);
static_assert(PlayerMeta::contains<levelKey>());

int main()
{
    Player player;
    constexpr auto levelProperty = PlayerMeta::at<levelKey>();

    int newLevel = 5;
    levelProperty->set(player, newLevel);

    return levelProperty->get(player) == newLevel ? 0 : 1;
}
```

`DECL_REFLECTION_BODY` registers the type's property map; `DECL_PROPERTY_INIT` and `DECL_PROPERTY_DEFAULT` declare and register members. `StaticPropertyMeta<T>` exposes compile-time queries such as `size()`, `empty()`, `contains<key>()`, and `at<key>()`, as well as traversal methods including `forEach`, `forEachIf`, and `doForKey`.

### Non-intrusive registration

Existing types can be registered without changing their definitions:

```cpp
#include <reflection/reflection.h>

struct Settings
{
    int retries = 3;
    bool enabled = true;
};

DECL_REFLECTION_NONINTRUSIVE(Settings,
    &Settings::retries,
    &Settings::enabled)

static_assert(reflection::IsObject<Settings>::value);
static_assert(reflection::StaticPropertyMeta<Settings>::size() == 2);
```

Non-intrusive registration must be declared in the appropriate namespace after the full type definition. It may live in a separate header, provided every translation unit using that type's reflection includes the completed registration before using it.

The core also supports registering base classes and template types. See [`tests/src/test/puretypes.h`](tests/src/test/puretypes.h) and [`tests/src/test/reflectedtypes.h`](tests/src/test/reflectedtypes.h) for working examples of both registration styles.

## Reflection usage contract

Reflection uses compile-time registration techniques, including friend injection and argument-dependent lookup (ADL). **The registration's visibility and point of first use matter.**

1. **Register each type in one place.** Registration may be *intrusive* or *non-intrusive* (optionally, in a separate header), but the complete registration for a given type must be defined together in one location.
2. **Complete registration before use.** The relevant type definitions and their registration must be available and fully processed before the first access to that type's reflection.
3. **Guard reflection-dependent APIs.** Access metadata through `reflection::StaticPropertyMeta<T>` and require `reflection::IsObject<T>` for APIs that expect registered reflection.  
   Unregistered types should be rejected at compile time rather than silently treated as reflected objects without properties.  
   This check is for consumers after registration, not for the registration process itself.
4. **Keep translation units consistent.** Every translation unit using reflection for the same type must see its completed registration.  
   *Inconsistent reflection registration across translation units* can lead to an **ODR violation** and a program that is *ill-formed, no diagnostic required*.

`IsObject<T>` checks whether the type has a registered static property map. It is an availability check, not an independent proof that every intended member was registered: completeness is a requirement of the contract above.

The test suite includes cross-translation-unit checks for consistent metadata and static-object identity when this contract is followed.

## Extensions: examples, not requirements

[`extensions/`](extensions/) demonstrates use cases of the reflection metadata API:

- **JSON:** `convertToJson` and `convertFromJson` using Qt's `QJsonObject`.
- **Text:** `convertToString` for a readable property representation.
- **Comparison:** `compare`, `equal`, `notEqual`, and ordering helpers.

These are **optional example implementations**, not dependencies of `reflection`. They are intentionally separate so applications can implement their own serializers, inspectors, validators, UI bindings, or other reflection-based mechanisms. JSON conversion requires Qt Core; the reflection core does not.

The extensions are not a promise of a general-purpose serialization format or a stable, separately distributed library. Review their current behavior and limitations before using them in production.

## Development and testing

To work with the complete repository, initialize the GoogleTest submodule:

```sh
git submodule update --init --recursive
```

### CMake

The core can be checked independently, without Qt:

```sh
cmake -S reflection -B build/reflection
cmake --build build/reflection
```

The standalone `reflection` build compiles every modular header in a separate translation unit. This is a header-compilation check, not a substitute for behavior tests.

To build and run the GoogleTest suite, provide a suitable Qt installation when configuring:

```sh
cmake -S tests -B build/tests -DCMAKE_PREFIX_PATH=/path/to/Qt
cmake --build build/tests
ctest --test-dir build/tests --output-on-failure
```

CMake discovers individual GoogleTest tests through CTest. On Windows with dynamic Qt/MinGW installations, running the test executable may also require the corresponding runtime DLL directories on `PATH`.

### qmake

The repository also contains qmake configurations:

- [`reflection/reflection.pri`](reflection/reflection.pri) exposes the **modular** core headers to a qmake consumer.
- [`extensions/extensions.pri`](extensions/extensions.pri) exposes the optional extensions and their dependencies.
- [`tests/tests.pro`](tests/tests.pro) builds the test executable using GoogleTest.

The generated single-header is the recommended **third-party distribution**; the `.pri` and CMake project files mainly support development within a source checkout.

### Regenerating the single-header

From the repository root:

```sh
python tools/amalgamate.py
python tools/amalgamate.py --check
```

The generator starts from `reflection/include/reflection/reflection.h`, consolidates reachable core headers, and writes `reflection/single_include/reflection/reflection.h`. It preserves deterministic dependency order, checks for missing includes, warns about unused core headers, and embeds the MIT license.

**Do not manually edit the generated header.** Make changes in `reflection/include/` and regenerate it.

## License

[MIT License](LICENSE). Copyright © 2026 **Mars Rooll**.

The generated single-header includes the license notice. Keep that notice when redistributing the file.
