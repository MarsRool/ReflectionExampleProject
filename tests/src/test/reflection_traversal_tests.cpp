#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <string_view>
#include <type_traits>
#include <vector>

#include "reflection/reflection.h"

namespace reflection_traversal_tests
{

using Traversal = reflection::StaticPropertyTraversal;
using Names = std::vector<std::string_view>;

struct Root
{
    int root = 1;
};
REFLECTION_NONINTRUSIVE(Root, &Root::root)

struct Middle : Root
{
    int middle = 2;
};
REFLECTION_NONINTRUSIVE(Middle, &Middle::middle)
REFLECTION_BASE_CLASSES_NONINTRUSIVE(Middle, Root)

struct Side
{
    int side = 3;
};
REFLECTION_NONINTRUSIVE(Side, &Side::side)

struct Nested
{
    int deep = 4;
};
REFLECTION_NONINTRUSIVE(Nested, &Nested::deep)

struct NonIntrusive : Middle, Side
{
    Nested nested;
    int own = 5;
};
REFLECTION_NONINTRUSIVE(NonIntrusive, &NonIntrusive::nested, &NonIntrusive::own)
REFLECTION_BASE_CLASSES_NONINTRUSIVE(NonIntrusive, Middle, Side)

struct Intrusive : Middle, Side
{
    REFLECTION_BEGIN(Intrusive)
    REFLECTION_BASE_CLASS(Middle)
    REFLECTION_BASE_CLASS(Side)
    REFLECTION_PROPERTY(Nested, nested)
    REFLECTION_PROPERTY(int, own, 5)
    REFLECTION_END()
};

struct Empty
{
    REFLECTION_BEGIN(Empty)
    REFLECTION_END()
};

struct EmptyNonIntrusive {};
REFLECTION_NONINTRUSIVE(EmptyNonIntrusive)

struct ExcludeMiddleAndNested
{
    template <typename, typename Entry>
    using Filter = std::bool_constant<
        std::string_view(Entry::key) != "_base_Middle" &&
        std::string_view(Entry::key) != "nested">;
};

struct ExcludeRootField
{
    template <typename, typename Entry>
    using Filter = std::bool_constant<std::string_view(Entry::key) != "root">;
};

struct RootAliasComparator
{
    bool operator()(const char* candidate, const char* requested) const
    {
        return std::string_view(candidate) == "root" &&
               std::string_view(requested) == "ROOT_ALIAS";
    }
};

struct NullableCallback
{
    int* calls;
    bool isNull;

    bool operator==(std::nullptr_t) const noexcept
    {
        return isNull;
    }

    explicit operator bool() const noexcept
    {
        return !isNull;
    }

    template <typename... Args>
    void operator()(Args&&...) const
    {
        ++*calls;
    }
};

struct NullablePredicate
{
    bool operator==(std::nullptr_t) const noexcept
    {
        return true;
    }

    explicit operator bool() const noexcept
    {
        return false;
    }

    template <typename... Args>
    bool operator()(Args&&...) const
    {
        return true;
    }
};

constexpr char rootKey[] = "root";
constexpr char absentKey[] = "notRegistered";

const Names expectedOrder
{
    "_base_Middle", "_base_Root", "root", "middle",
    "_base_Side", "side", "nested", "own"
};

const Names expectedAfterPruning
{
    "_base_Side", "side", "own"
};

template <typename Object>
Names getTraversalOrder()
{
    Names names;
    Traversal::forEach<Object>([&](auto entry)
    {
        names.emplace_back(decltype(entry)::key);
    });
    return names;
}

template <typename Object>
void checkStopInsideBaseClass()
{
    const Names expected{ "_base_Middle", "_base_Root", "root" };

    Names withoutObject;
    Traversal::forEach<Object>([&](auto entry) -> bool
    {
        const auto key = std::string_view(decltype(entry)::key);
        withoutObject.emplace_back(key);
        return key != "root";
    });
    EXPECT_EQ(withoutObject, expected);

    Object object{};
    Names withObject;
    Traversal::forEach(object, [&](auto entry, auto&) -> bool
    {
        const auto key = std::string_view(decltype(entry)::key);
        withObject.emplace_back(key);
        return key != "root";
    });
    EXPECT_EQ(withObject, expected);
}

template <typename Object>
void checkPredicatesPruneBranches()
{
    const auto allow = [](auto entry)
    {
        const auto key = std::string_view(decltype(entry)::key);
        return key != "_base_Middle" && key != "nested";
    };

    Names withoutObject;
    Traversal::forEachIf<Object>([&](auto entry)
    {
        withoutObject.emplace_back(decltype(entry)::key);
    }, allow);
    EXPECT_EQ(withoutObject, expectedAfterPruning);

    Object object{};
    Names withObject;
    Traversal::forEachIf(object, [&](auto entry, auto&)
    {
        withObject.emplace_back(decltype(entry)::key);
    }, [&](auto entry, auto&) { return allow(entry); });
    EXPECT_EQ(withObject, expectedAfterPruning);
}

template <typename Object>
void checkStaticFiltering()
{
    using Meta = reflection::StaticPropertyMeta<Object>;
    static_assert(Meta::template size<ExcludeMiddleAndNested>() == 2);

    Names withoutObject;
    Traversal::forEach<Object, ExcludeMiddleAndNested>([&](auto entry)
    {
        withoutObject.emplace_back(decltype(entry)::key);
    });
    EXPECT_EQ(withoutObject, expectedAfterPruning);

    Object object{};
    Names withObject;
    Traversal::forEach<ExcludeMiddleAndNested>(object, [&](auto entry, auto&)
    {
        withObject.emplace_back(decltype(entry)::key);
    });
    EXPECT_EQ(withObject, expectedAfterPruning);

    const Names expectedWithoutRoot
    {
        "_base_Middle", "_base_Root", "middle",
        "_base_Side", "side", "nested", "own"
    };

    Names withoutRoot;
    Traversal::forEach<Object, ExcludeRootField>([&](auto entry)
    {
        withoutRoot.emplace_back(decltype(entry)::key);
    });
    EXPECT_EQ(withoutRoot, expectedWithoutRoot);
}

template <typename Object>
void checkRuntimeKeyLookup()
{
    int hits = 0;
    Traversal::doForKey<Object>([&](auto entry)
    {
        EXPECT_EQ(std::string_view(decltype(entry)::key), "root");
        ++hits;
    }, "root");
    EXPECT_EQ(hits, 1);

    Object object{};
    hits = 0;
    Traversal::doForKey(object, [&](auto entry, auto& current)
    {
        using Entry = decltype(entry);
        if constexpr (std::string_view(Entry::key) == "root")
        {
            static_assert(std::is_same_v<std::remove_reference_t<decltype(current)>, Root>);
            Entry::value.get(current) = 42;
            ++hits;
        }
    }, "root");
    EXPECT_EQ(hits, 1);
    EXPECT_EQ(object.root, 42);

    hits = 0;
    Traversal::doForKey<Object, RootAliasComparator>([&](auto)
    {
        ++hits;
    }, "ROOT_ALIAS");
    EXPECT_EQ(hits, 1);

    hits = 0;
    Traversal::doForKey<Object>([&](auto) { ++hits; }, "missing");
    Traversal::doForKey<Object>([&](auto) { ++hits; }, nullptr);
    Traversal::doForKey(object, [&](auto, auto&) { ++hits; }, "missing");
    Traversal::doForKey(object, [&](auto, auto&) { ++hits; }, nullptr);
    Traversal::doForKey<Object, reflection::PropertyNameEqual, ExcludeRootField>(
        [&](auto) { ++hits; }, "root");
    Traversal::doForKey<reflection::PropertyNameEqual, ExcludeRootField>(
        object, [&](auto, auto&) { ++hits; }, "root");
    EXPECT_EQ(hits, 0);
}

template <typename Object>
void checkCompileTimeKeyLookup()
{
    int hits = 0;
    Traversal::doForKey<Object, rootKey>([&](auto entry)
    {
        using Entry = decltype(entry);
        static_assert(std::string_view(Entry::key) == "root");
        ++hits;
    });
    EXPECT_EQ(hits, 1);

    Object object{};
    hits = 0;
    Traversal::doForKey<rootKey>(object, [&](auto entry, auto& current)
    {
        using Entry = decltype(entry);
        static_assert(std::string_view(Entry::key) == "root");
        Entry::value.get(current) = 73;
        ++hits;
    });
    EXPECT_EQ(hits, 1);
    EXPECT_EQ(object.root, 73);

    hits = 0;
    Traversal::doForKey<Object, absentKey>([&](auto) { ++hits; });
    Traversal::doForKey<absentKey>(object, [&](auto, auto&) { ++hits; });
    Traversal::doForKey<Object, rootKey, ExcludeRootField>([&](auto) { ++hits; });
    Traversal::doForKey<rootKey, ExcludeRootField>(object, [&](auto, auto&) { ++hits; });
    EXPECT_EQ(hits, 0);
}

template <typename Object>
void checkObjectConstness()
{
    Object mutableObject{};
    std::size_t mutableVisited = 0;
    Traversal::forEach(mutableObject, [&](auto entry, auto& current)
    {
        using Entry = decltype(entry);
        using ObjectType = std::remove_reference_t<decltype(current)>;
        static_assert(!std::is_const_v<ObjectType>);
        if constexpr (reflection::impl::IsStaticPropertyEntry<Entry>::value)
        {
            using Owner = typename reflection::impl::EntryValueType<Entry>::OuterType;
            static_assert(std::is_same_v<ObjectType, Owner>);
            (void)Entry::value.get(current);
        }
        ++mutableVisited;
    });
    EXPECT_EQ(mutableVisited, expectedOrder.size());

    const Object& constantObject = mutableObject;
    std::size_t constVisited = 0;
    Traversal::forEach(constantObject, [&](auto entry, auto& current)
    {
        using Entry = decltype(entry);
        using ObjectType = std::remove_reference_t<decltype(current)>;
        static_assert(std::is_const_v<ObjectType>);
        if constexpr (reflection::impl::IsStaticPropertyEntry<Entry>::value)
        {
            using Owner = typename reflection::impl::EntryValueType<Entry>::OuterType;
            static_assert(std::is_same_v<std::remove_const_t<ObjectType>, Owner>);
            static_assert(std::is_const_v<
                std::remove_reference_t<decltype(Entry::value.get(current))>>);
        }
        ++constVisited;
    });
    EXPECT_EQ(constVisited, expectedOrder.size());
}

constexpr bool constexprTraversalWorks()
{
    std::size_t count = 0;
    Traversal::forEach<NonIntrusive>([&](auto)
    {
        ++count;
    });
    return count == 8;
}

constexpr bool constexprTraversalCanStop()
{
    std::size_t count = 0;
    Traversal::forEach<Intrusive>([&](auto) -> bool
    {
        ++count;
        return count < 3;
    });
    return count == 3;
}

constexpr bool constexprFilteredLookupWorks()
{
    std::size_t count = 0;
    Traversal::forEachIf<NonIntrusive>([&](auto)
    {
        ++count;
    }, [](auto entry)
    {
        return std::string_view(decltype(entry)::key) == "own";
    });

    Traversal::doForKey<NonIntrusive, rootKey>([&](auto)
    {
        ++count;
    });

    return count == 2;
}

constexpr bool constexprObjectTraversalWorks()
{
    NonIntrusive object{};
    std::size_t count = 0;
    Traversal::forEach(object, [&](auto, auto&)
    {
        ++count;
    });
    return count == 8;
}

static_assert(constexprTraversalWorks());
static_assert(constexprTraversalCanStop());
static_assert(constexprFilteredLookupWorks());
static_assert(constexprObjectTraversalWorks());

TEST(ReflectionTraversal, OrderAndOnlyInheritanceIsRecursive)
{
    const auto nonIntrusiveNames = getTraversalOrder<NonIntrusive>();
    const auto intrusiveNames = getTraversalOrder<Intrusive>();
    EXPECT_EQ(nonIntrusiveNames, expectedOrder);
    EXPECT_EQ(intrusiveNames, expectedOrder);

    // Nested is reflected, but traversal only expands registered base classes.
    EXPECT_EQ(reflection::StaticPropertyMeta<Nested>::size(), 1u);
    EXPECT_EQ(std::count(nonIntrusiveNames.begin(), nonIntrusiveNames.end(), "deep"), 0);
    EXPECT_EQ(std::count(intrusiveNames.begin(), intrusiveNames.end(), "deep"), 0);
}

TEST(ReflectionTraversal, StopsTheEntireTraversalFromInsideAnInheritedClass)
{
    checkStopInsideBaseClass<NonIntrusive>();
    checkStopInsideBaseClass<Intrusive>();
}

TEST(ReflectionTraversal, PredicatesSkipBaseSubtreesAndContinueToLaterProperties)
{
    checkPredicatesPruneBranches<NonIntrusive>();
    checkPredicatesPruneBranches<Intrusive>();
}

TEST(ReflectionTraversal, TypeCheckerFiltersEntriesAtEveryInheritanceLevel)
{
    checkStaticFiltering<NonIntrusive>();
    checkStaticFiltering<Intrusive>();
}

TEST(ReflectionTraversal, RuntimeKeyLookupOverloadsAndFiltering)
{
    checkRuntimeKeyLookup<NonIntrusive>();
    checkRuntimeKeyLookup<Intrusive>();
}

TEST(ReflectionTraversal, CompileTimeKeyLookupOverloadsAndFiltering)
{
    checkCompileTimeKeyLookup<NonIntrusive>();
    checkCompileTimeKeyLookup<Intrusive>();
}

TEST(ReflectionTraversal, ObjectReferencesPreserveConstnessAndOwningTypes)
{
    checkObjectConstness<NonIntrusive>();
    checkObjectConstness<Intrusive>();
}

TEST(ReflectionTraversal, EmptyRegistrationsAndNullableCallables)
{
    Empty object{};
    int calls = 0;
    Traversal::forEach<Empty>([&](auto) { ++calls; });
    Traversal::forEach(object, [&](auto, auto&) { ++calls; });
    Traversal::forEachIf<Empty>([&](auto) { ++calls; }, [](auto) { return true; });
    Traversal::doForKey<Empty>([&](auto) { ++calls; }, "missing");

    EmptyNonIntrusive other{};
    Traversal::forEach<EmptyNonIntrusive>([&](auto) { ++calls; });
    Traversal::forEach(other, [&](auto, auto&) { ++calls; });
    EXPECT_EQ(calls, 0);

    NonIntrusive nonIntrusive{};
    Traversal::forEach<NonIntrusive>(NullableCallback{ &calls, true });
    Traversal::forEach(nonIntrusive, NullableCallback{ &calls, true });
    Traversal::forEachIf<NonIntrusive>([&](auto) { ++calls; }, NullablePredicate{});
    Traversal::forEachIf(nonIntrusive, [&](auto, auto&) { ++calls; }, NullablePredicate{});
    Traversal::doForKey<NonIntrusive>(NullableCallback{ &calls, true }, "root");
    Traversal::doForKey(nonIntrusive, NullableCallback{ &calls, true }, "root");

    std::function<bool(const char*, const char*)> emptyComparator;
    Traversal::doForKey<NonIntrusive>([&](auto) { ++calls; }, "root", emptyComparator);
    EXPECT_EQ(calls, 0);
}

} // namespace reflection_traversal_tests
