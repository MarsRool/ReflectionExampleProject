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

namespace reflected
{

enum class Mode : std::uint8_t
{
    idle,
    active
};

struct Identity
{
    REFLECTION_BEGIN(Identity)
    REFLECTION_PROPERTY(int, id)
    REFLECTION_PROPERTY(std::string, name)
    REFLECTION_END()
};

struct Flags
{
    REFLECTION_BEGIN(Flags)
    REFLECTION_PROPERTY(bool, enabled)
    REFLECTION_PROPERTY(double, ratio)
    REFLECTION_END()
};

template <typename T>
struct Stats : Identity
{
    REFLECTION_BEGIN(Stats)
    REFLECTION_BASE_CLASS(Identity)
    REFLECTION_PROPERTY(T, score)
    using History = std::array<int, 2>;
    REFLECTION_PROPERTY(History, history)
    REFLECTION_END()
};

struct Nested
{
    REFLECTION_BEGIN(Nested)
    REFLECTION_PROPERTY(std::string, label)
    REFLECTION_PROPERTY(int, count)
    REFLECTION_PROPERTY(float, weight)
    REFLECTION_END()
};

struct Empty
{
    REFLECTION_BEGIN(Empty)
    REFLECTION_END()
};

struct TestObject : Stats<int>, Flags
{
    using Fixed = std::array<int, 3>;
    using FixedNested = std::array<Nested, 2>;
    using Raw = int[2];
    using Matrix = int[2][3];

    REFLECTION_BEGIN(TestObject)
    REFLECTION_BASE_CLASS(Stats<int>)
    REFLECTION_BASE_CLASS(Flags)
    REFLECTION_PROPERTY(Nested, nested)
    REFLECTION_PROPERTY(Stats<std::string>, templateNested)
    REFLECTION_PROPERTY(Empty, empty)
    REFLECTION_PROPERTY(std::vector<Nested>, children)
    REFLECTION_PROPERTY(std::vector<int>, numbers)
    REFLECTION_PROPERTY(std::vector<bool>, switches)
    REFLECTION_PROPERTY(std::deque<int>, dequeValues)
    REFLECTION_PROPERTY(std::list<int>, listValues)
    REFLECTION_PROPERTY(std::set<int>, sorted)
    REFLECTION_PROPERTY(std::multiset<int>, repeated)
    REFLECTION_PROPERTY(std::unordered_set<int>, unordered)
    REFLECTION_PROPERTY(std::unordered_multiset<int>, unorderedRepeated)
    REFLECTION_PROPERTY(Fixed, fixed)
    REFLECTION_PROPERTY(FixedNested, fixedNested)
    REFLECTION_PROPERTY(Raw, raw)
    REFLECTION_PROPERTY(Matrix, matrix)
    REFLECTION_PROPERTY(std::valarray<int>, valarrayValues, 0, 0, 0)
    REFLECTION_PROPERTY(std::string, text)
    REFLECTION_PROPERTY(Mode, mode, Mode::idle)
    REFLECTION_PROPERTY(std::int64_t, signedWide)
    REFLECTION_PROPERTY(std::uint64_t, unsignedWide)
    REFLECTION_END()
};

} // namespace reflected
