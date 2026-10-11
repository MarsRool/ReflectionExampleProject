#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <vector>

#include "reflection/reflection.h"
#include "test/compoundtypes.h"

using namespace reflection;

TEST(Reflection, UniqueStaticHeterogeneousMapProperties)
{
    struct OuterT{};
    using StaticKey = const char[];
    using Meta = StaticPropertyMeta<compound::CompoundTestObject>;

    static constexpr StaticKey key1{ "keyTest1" };
    static constexpr StaticKey key2{ "keyTest2" };
    static constexpr StaticKey key3{ "keyTest3" };
    static constexpr StaticKey idPureTestObject{ "pureTestObject" };
    static constexpr StaticKey idReflectedTestObject{ "reflectedTestObject" };
    static constexpr auto valuePtr1{ Meta::at<idPureTestObject>() };
    static constexpr auto valuePtr2{ Meta::at<idReflectedTestObject>() };
    using ValueT1 = decltype(valuePtr1);
    using ValueT2 = decltype(valuePtr2);

    static_assert(valuePtr1 != nullptr);
    static_assert(valuePtr2 != nullptr);

    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, StaticKey>([]{}) == 0);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key1>([]{}) == false);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key1>([]{}) == nullptr);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key2>([]{}) == false);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key2>([]{}) == nullptr);

    uniqueStaticHeterogeneousMapAdd<OuterT, StaticKey, ValueT1, key1, valuePtr1>([]{});

    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, StaticKey>([]{}) == 1);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key1>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key1>([]{}) == valuePtr1);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key2>([]{}) == false);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key2>([]{}) == nullptr);

    uniqueStaticHeterogeneousMapAdd<OuterT, StaticKey, ValueT2, key1, valuePtr2>([]{});

    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, StaticKey>([]{}) == 1);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key1>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key1>([]{}) == valuePtr1);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key2>([]{}) == false);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key2>([]{}) == nullptr);

    uniqueStaticHeterogeneousMapAdd<OuterT, StaticKey, ValueT2, key2, valuePtr2>([]{});

    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, StaticKey>([]{}) == 2);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key1>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key1>([]{}) == valuePtr1);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key2>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key2>([]{}) == valuePtr2);

    std::size_t foundCount1 = 0;
    std::size_t foundCount2 = 0;
    std::size_t foundCount3 = 0;

    uniqueStaticHeterogeneousMapDoForKey<OuterT, StaticKey, key1>([]{},
        [&foundCount1](auto constKey, auto constValue)
    {
        static_assert(decltype(constKey)::value == key1);
        static_assert(decltype(constValue)::value == valuePtr1);
        ++foundCount1;
    });
    uniqueStaticHeterogeneousMapDoForKey<OuterT, StaticKey, key2>([]{},
        [&foundCount2](auto constKey, auto constValue)
    {
        static_assert(decltype(constKey)::value == key2);
        static_assert(decltype(constValue)::value == valuePtr2);
        ++foundCount2;
    });
    uniqueStaticHeterogeneousMapDoForKey<OuterT, StaticKey, key3>([]{},
        [&foundCount3](auto, auto)
    {
        static_assert(false);
        ++foundCount3;
    });

    EXPECT_EQ(foundCount1, std::size_t{ 1 });
    EXPECT_EQ(foundCount2, std::size_t{ 1 });
    EXPECT_EQ(foundCount3, std::size_t{ 0 });
}


TEST(Reflection, UniqueStaticHeterogeneousMapValues)
{
    struct OuterT{};
    using StaticKey = const char[];
    using StaticValue1 = const char[];
    using StaticValue2 = std::size_t;

    static constexpr StaticKey key1{ "keyChar1" };
    static constexpr StaticValue1 value1{ "value1" };
    static constexpr StaticValue2 value2{ 42 };

    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, StaticKey>([]{}) == 0);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key1>([]{}) == false);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key1>([]{}) == nullptr);

    uniqueStaticHeterogeneousMapAdd<OuterT, StaticKey, StaticValue1, key1, value1>([]{});

    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, StaticKey>([]{}) == 1);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key1>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key1>([]{}) == value1);

    uniqueStaticHeterogeneousMapAdd<OuterT, StaticKey, StaticValue2, key1, value2>([]{});

    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, StaticKey>([]{}) == 1);
    static_assert(uniqueStaticHeterogeneousMapExists<OuterT, StaticKey, key1>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousMapGetValue<OuterT, StaticKey, key1>([]{}) == value1);
}


TEST(Reflection, UniqueStaticHeterogeneousArray)
{
    struct OuterT{};
    using Value1Type = const char[];
    using Value2Type = std::int32_t;
    using Value3Type = const std::int32_t*;

    static constexpr Value1Type value1{ "test value" };
    static constexpr Value2Type value2{ 42 };
    static constexpr Value3Type value3{ &value2 };

    static_assert(uniqueStaticHeterogeneousArrayExists<OuterT, 0>([]{}) == false);
    static_assert(uniqueStaticHeterogeneousArrayLength<OuterT>([]{}) == 0);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 0>([]{}) == nullptr);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 1>([]{}) == nullptr);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 2>([]{}) == nullptr);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 3>([]{}) == nullptr);

    uniqueStaticHeterogeneousArrayPushBack<OuterT, Value1Type, value1>([]{});

    static_assert(uniqueStaticHeterogeneousArrayExists<OuterT, 0>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousArrayLength<OuterT>([]{}) == 1);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 0>([]{}) == value1);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 1>([]{}) == nullptr);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 2>([]{}) == nullptr);

    uniqueStaticHeterogeneousArrayPushBack<OuterT, Value2Type, value2>([]{});

    static_assert(uniqueStaticHeterogeneousArrayExists<OuterT, 0>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousArrayLength<OuterT>([]{}) == 2);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 0>([]{}) == value1);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 1>([]{}) == value2);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 2>([]{}) == nullptr);

    uniqueStaticHeterogeneousArrayPushBack<OuterT, Value3Type, value3>([]{});

    static_assert(uniqueStaticHeterogeneousArrayExists<OuterT, 0>([]{}) == true);
    static_assert(uniqueStaticHeterogeneousArrayLength<OuterT>([]{}) == 3);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 0>([]{}) == value1);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 1>([]{}) == value2);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<OuterT, 2>([]{}) == value3);

    static constexpr auto valueRead1 = uniqueStaticHeterogeneousArrayGetValue<OuterT, 0>([]{});
    static constexpr auto valueRead2 = uniqueStaticHeterogeneousArrayGetValue<OuterT, 1>([]{});
    static constexpr auto valueRead3 = uniqueStaticHeterogeneousArrayGetValue<OuterT, 2>([]{});

    static_assert(std::is_same_v<decltype(valueRead1), std::add_const_t<std::decay_t<Value1Type>>>);
    static_assert(std::is_same_v<decltype(valueRead2), std::add_const_t<Value2Type>>);
    static_assert(std::is_same_v<decltype(valueRead3), std::add_const_t<Value3Type>>);
}

TEST(Reflection, UniqueStaticHeterogeneousMapTraversal)
{
    struct OuterT {};
    using KeyType = const char[];

    static constexpr KeyType alpha{ "alpha" };
    static constexpr KeyType beta{ "beta" };
    static constexpr KeyType missing{ "missing" };
    static constexpr int firstValue = 21;
    static constexpr std::size_t secondValue = 34;

    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, KeyType>([]{}) == 0);
    uniqueStaticHeterogeneousMapAdd<OuterT, KeyType, int, alpha, firstValue>([]{});
    uniqueStaticHeterogeneousMapAdd<OuterT, KeyType, std::size_t, beta, secondValue>([]{});
    static_assert(uniqueStaticHeterogeneousMapKeysCount<OuterT, KeyType>([]{}) == 2);

    std::vector<std::string_view> visited;
    uniqueStaticHeterogeneousMapForEach<OuterT, KeyType>([]{}, [&](auto constKey, auto)
    {
        visited.emplace_back(decltype(constKey)::value);
    });
    ASSERT_EQ(visited.size(), std::size_t{ 2 });
    EXPECT_EQ(visited[0], "alpha");
    EXPECT_EQ(visited[1], "beta");

    std::size_t filtered = 0;
    uniqueStaticHeterogeneousMapForEachIf<OuterT, KeyType>([]{},
        [&](auto, auto) { ++filtered; },
        [](auto constKey, auto)
    {
        return std::string_view(decltype(constKey)::value) == "beta";
    });
    EXPECT_EQ(filtered, std::size_t{ 1 });

    std::size_t stopped = 0;
    uniqueStaticHeterogeneousMapForEach<OuterT, KeyType>([]{}, [&](auto, auto) -> bool
    {
        ++stopped;
        return false;
    });
    EXPECT_EQ(stopped, std::size_t{ 1 });

    std::size_t found = 0;
    uniqueStaticHeterogeneousMapDoForKey<OuterT, KeyType, beta>([]{},
        [&](auto constKey, auto constValue)
    {
        static_assert(decltype(constKey)::value == beta);
        static_assert(decltype(constValue)::value == secondValue);
        ++found;
    });
    EXPECT_EQ(found, std::size_t{ 1 });

    const auto comparator = [](const char* left, const char* right)
    {
        return std::string_view(left) == std::string_view(right);
    };

    found = 0;
    uniqueStaticHeterogeneousMapDoForKey<OuterT, KeyType>([]{},
        [&](auto, auto) { ++found; }, "alpha", comparator);
    EXPECT_EQ(found, std::size_t{ 1 });
    found = 0;
    uniqueStaticHeterogeneousMapDoForKey<OuterT, KeyType>([]{},
        [&](auto, auto) { ++found; }, missing, comparator);
    EXPECT_EQ(found, std::size_t{ 0 });
}

TEST(Reflection, UniqueStaticCollectionsAreIsolatedByOwner)
{
    struct FirstOwner {};
    struct SecondOwner {};
    using KeyType = const char[];

    static constexpr KeyType key{ "sharedKey" };
    static constexpr int first = 12;
    static constexpr int second = 28;

    static_assert(uniqueStaticHeterogeneousMapKeysCount<FirstOwner, KeyType>([]{}) == 0);
    static_assert(uniqueStaticHeterogeneousMapKeysCount<SecondOwner, KeyType>([]{}) == 0);
    uniqueStaticHeterogeneousMapAdd<FirstOwner, KeyType, int, key, first>([]{});
    static_assert(uniqueStaticHeterogeneousMapKeysCount<FirstOwner, KeyType>([]{}) == 1);
    static_assert(uniqueStaticHeterogeneousMapKeysCount<SecondOwner, KeyType>([]{}) == 0);
    uniqueStaticHeterogeneousMapAdd<SecondOwner, KeyType, int, key, second>([]{});
    static_assert(uniqueStaticHeterogeneousMapGetValue<FirstOwner, KeyType, key>([]{}) == first);
    static_assert(uniqueStaticHeterogeneousMapGetValue<SecondOwner, KeyType, key>([]{}) == second);

    struct FirstArray {};
    struct SecondArray {};
    static_assert(uniqueStaticHeterogeneousArrayLength<FirstArray>([]{}) == 0);
    static_assert(uniqueStaticHeterogeneousArrayLength<SecondArray>([]{}) == 0);
    uniqueStaticHeterogeneousArrayPushBack<FirstArray, int, first>([]{});
    static_assert(uniqueStaticHeterogeneousArrayLength<FirstArray>([]{}) == 1);
    static_assert(uniqueStaticHeterogeneousArrayLength<SecondArray>([]{}) == 0);
    uniqueStaticHeterogeneousArrayPushBack<SecondArray, int, second>([]{});
    static_assert(uniqueStaticHeterogeneousArrayGetValue<FirstArray, 0>([]{}) == first);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<SecondArray, 0>([]{}) == second);
    static_assert(uniqueStaticHeterogeneousArrayGetValue<FirstArray, 1>([]{}) == nullptr);
}
