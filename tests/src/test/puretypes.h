#pragma once

#include <array>
#include <cstdint>
#include <deque>
#include <list>
#include <set>
#include <string>
#include <unordered_set>
#include <valarray>
#include <vector>

#include "reflection/reflection.h"

namespace pure
{

enum class Mode : std::uint8_t
{
    idle,
    active
};

struct Identity
{
    int id = 0;
    std::string name;
};

REFLECTION_NONINTRUSIVE(Identity, &Identity::id, &Identity::name)

struct Flags
{
    bool enabled = false;
    double ratio = 0.0;
};

REFLECTION_NONINTRUSIVE(Flags, &Flags::enabled, &Flags::ratio)

template <typename T>
struct Stats : Identity
{
    T score{};
    std::array<int, 2> history{};
};

REFLECTION_TEMPLATE_NONINTRUSIVE((typename T), (T), Stats,
    &Stats<T>::score, &Stats<T>::history)
REFLECTION_BASE_CLASSES_TEMPLATE_NONINTRUSIVE((typename T), (T), Stats, Identity)

struct Nested
{
    std::string label;
    int count = 0;
    float weight = 0.0f;
};

REFLECTION_NONINTRUSIVE(Nested, &Nested::label, &Nested::count, &Nested::weight)

struct Empty {};

REFLECTION_NONINTRUSIVE(Empty)

struct TestObject : Stats<int>, Flags
{
    Nested nested;
    Stats<std::string> templateNested;
    Empty empty;

    std::vector<Nested> children;
    std::vector<int> numbers;
    std::vector<bool> switches;
    std::deque<int> dequeValues;
    std::list<int> listValues;
    std::set<int> sorted;
    std::multiset<int> repeated;
    std::unordered_set<int> unordered;
    std::unordered_multiset<int> unorderedRepeated;
    std::array<int, 3> fixed{};
    std::array<Nested, 2> fixedNested{};
    int raw[2]{};
    int matrix[2][3]{};
    std::valarray<int> valarrayValues{ 0, 0, 0 };

    std::string text;
    Mode mode = Mode::idle;
    std::int64_t signedWide = 0;
    std::uint64_t unsignedWide = 0;
};

REFLECTION_NONINTRUSIVE(TestObject,
    &TestObject::nested,
    &TestObject::templateNested,
    &TestObject::empty,
    &TestObject::children,
    &TestObject::numbers,
    &TestObject::switches,
    &TestObject::dequeValues,
    &TestObject::listValues,
    &TestObject::sorted,
    &TestObject::repeated,
    &TestObject::unordered,
    &TestObject::unorderedRepeated,
    &TestObject::fixed,
    &TestObject::fixedNested,
    &TestObject::raw,
    &TestObject::matrix,
    &TestObject::valarrayValues,
    &TestObject::text,
    &TestObject::mode,
    &TestObject::signedWide,
    &TestObject::unsignedWide)

REFLECTION_BASE_CLASSES_NONINTRUSIVE(TestObject, Stats<int>, Flags)

} // namespace pure
