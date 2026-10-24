#pragma once

#include <cstdint>
#include <limits>
#include <string_view>
#include <type_traits>
#include <unordered_set>

#include "reflection/reflection.h"
#include "test/compoundtypes.h"

// Both registration styles expose the same value fields, so one initializer
// provides equivalent fixtures without maintaining two sets of test data.
template <typename T>
void initializeTestObject(T& object)
{
    object.id = 42;
    object.name = "identity";
    object.score = 17;
    object.history = { 4, 5 };
    object.enabled = true;
    object.ratio = 1.25;

    object.nested.label = "nested";
    object.nested.count = 9;
    object.nested.weight = 3.5f;

    object.templateNested.id = 7;
    object.templateNested.name = "template identity";
    object.templateNested.score = "template score";
    object.templateNested.history = { 6, 8 };

    object.children.resize(2);
    object.children[0].label = "first";
    object.children[0].count = 1;
    object.children[0].weight = 0.5f;
    object.children[1].label = "second";
    object.children[1].count = 2;
    object.children[1].weight = 1.5f;

    object.numbers = { 3, 5, 8 };
    object.switches = { true, false, true };
    object.dequeValues = { 2, 4 };
    object.listValues = { 7, 11 };
    object.sorted = { 1, 2, 4 };
    object.repeated = { 1, 1, 2 };
    object.unordered = { 99 };
    object.unorderedRepeated = { 5, 5 };
    object.fixed = { 6, 7, 8 };
    object.fixedNested[0].label = "fixed first";
    object.fixedNested[0].count = 10;
    object.fixedNested[1].label = "fixed second";
    object.fixedNested[1].count = 20;
    object.raw[0] = 12;
    object.raw[1] = 13;
    object.matrix[0][0] = 1;
    object.matrix[0][1] = 2;
    object.matrix[0][2] = 3;
    object.matrix[1][0] = 4;
    object.matrix[1][1] = 5;
    object.matrix[1][2] = 6;
    object.valarrayValues[0] = 21;
    object.valarrayValues[1] = 22;
    object.valarrayValues[2] = 23;
    object.text = "compound test";
    object.mode = decltype(object.mode)::active;
    object.signedWide = std::numeric_limits<std::int64_t>::min();
    object.unsignedWide = std::numeric_limits<std::uint64_t>::max();
}

template <typename T>
T makeTestObject()
{
    T test;
    initializeTestObject(test);
    return test;
}

inline compound::CompoundTestObject makeCompoundTestObject()
{
    compound::CompoundTestObject object;
    initializeTestObject(object.pureTestObject);
    initializeTestObject(object.reflectedTestObject);
    return object;
}

struct ExcludeBase
{
    template <typename, typename Entry>
    using Filter = std::negation<reflection::impl::IsBaseClassEntry<Entry>>;
};

struct ExcludeFields
{
    template <typename, typename Entry>
    using Filter = std::bool_constant<std::string_view(Entry::key) != "excluded">;
};

struct ExcludeUnordered
{
    template <typename T>
    struct IsUnordered : std::false_type {};

    template <typename T, typename Hash, typename Equal, typename Alloc>
    struct IsUnordered<std::unordered_set<T, Hash, Equal, Alloc>>
        : std::true_type {};

    template <typename T, typename Hash, typename Equal, typename Alloc>
    struct IsUnordered<std::unordered_multiset<T, Hash, Equal, Alloc>>
        : std::true_type {};

    template <typename Outer, typename T>
    struct IsUnordered<reflection::StaticProperty<Outer, T>>
        : IsUnordered<std::remove_cv_t<std::remove_reference_t<T>>> {};

    template <typename, typename Entry>
    using Filter = std::negation<
        IsUnordered<reflection::impl::EntryValueType<Entry>>>;
};
