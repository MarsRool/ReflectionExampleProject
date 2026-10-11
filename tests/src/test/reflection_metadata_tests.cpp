#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <set>
#include <string_view>

#include "test/test.h"

using namespace reflection;

TEST(Reflection, StaticPropertyMap)
{
    using StaticKey = const char[];
    using Meta = StaticPropertyMeta<compound::CompoundTestObject>;

    static constexpr StaticKey nonexistentKey{ "nonexistent" };
    static constexpr StaticKey existentKey1{ "pureTestObject" };
    static constexpr auto existentKey2{ CanonicalStaticString<existentKey1>::value };

    static_assert(existentKey1 != existentKey2);

    static_assert(Meta::empty() == false);
    static_assert(Meta::size() == 2);

    static_assert(Meta::contains<nonexistentKey>() == false);
    static_assert(Meta::at<nonexistentKey>() == nullptr);

    static_assert(Meta::contains<existentKey1>() == true);
    static_assert(Meta::at<existentKey1>() != nullptr);

    static_assert(Meta::contains<existentKey2>() == true);
    static_assert(Meta::at<existentKey2>() != nullptr);

    static_assert(Meta::at<existentKey1>()
        == Meta::at<existentKey2>());
}


namespace
{

template <typename Object>
void checkRegisteredFields()
{
    using Meta = StaticPropertyMeta<Object>;
    static constexpr char ageName[] = "age";
    static constexpr char missingName[] = "notRegistered";
    static constexpr char baseName[] = "_base_BaseTestObject";
    static constexpr char anotherBaseName[] = "_base_AnotherBaseTestObject";

    static_assert(Meta::size() == 10);
    static_assert(Meta::template contains<ageName>());
    static_assert(Meta::template contains<baseName>());
    static_assert(Meta::template contains<anotherBaseName>());
    static_assert(!Meta::template contains<missingName>());
    static_assert(Meta::template at<missingName>() == nullptr);

    constexpr auto propertyMap = Meta::getStaticPropertyMap();
    static_assert(propertyMap != nullptr);
    EXPECT_EQ(propertyMap->name, std::string_view("TestObject"));

    const auto ageProperty = Meta::template at<ageName>();
    ASSERT_NE(ageProperty, nullptr);
    EXPECT_EQ(ageProperty->name, std::string_view("age"));

    Object object = createTestObject<Object>();
    EXPECT_EQ(ageProperty->get(object), std::size_t{ 157 });
    const std::size_t newAge = 42;
    ageProperty->set(object, newAge);
    EXPECT_EQ(object.age, newAge);
    EXPECT_EQ(ageProperty->get(object), newAge);

    std::set<std::string_view> names;
    std::size_t visited = 0;
    Meta::forEach([&](auto constKey, auto constValue)
    {
        constexpr auto propertyPtr = decltype(constValue)::value;
        EXPECT_NE(propertyPtr, nullptr);
        names.emplace(decltype(constKey)::value);
        ++visited;
    });
    EXPECT_EQ(visited, Meta::size());
    EXPECT_EQ(names.size(), Meta::size());
    EXPECT_NE(names.count("age"), std::size_t{ 0 });
    EXPECT_NE(names.count("_base_BaseTestObject"), std::size_t{ 0 });
    EXPECT_NE(names.count("_base_AnotherBaseTestObject"), std::size_t{ 0 });

    std::size_t filtered = 0;
    Meta::forEachIf([&](auto, auto) { ++filtered; },
        [](auto constKey, auto)
    {
        return std::string_view(decltype(constKey)::value) == "age";
    });
    EXPECT_EQ(filtered, std::size_t{ 1 });

    std::size_t stoppedAfter = 0;
    Meta::forEach([&](auto, auto) -> bool
    {
        ++stoppedAfter;
        return false;
    });
    EXPECT_EQ(stoppedAfter, std::size_t{ 1 });

    const auto nameComparator = [](const char* left, const char* right)
    {
        return std::string_view(left) == std::string_view(right);
    };

    std::size_t matchingKeys = 0;
    Meta::doForKey([&](auto constKey, auto)
    {
        EXPECT_EQ(std::string_view(decltype(constKey)::value), "age");
        ++matchingKeys;
    }, "age", nameComparator);
    EXPECT_EQ(matchingKeys, std::size_t{ 1 });

    matchingKeys = 0;
    Meta::doForKey([&](auto, auto) { ++matchingKeys; }, "notRegistered", nameComparator);
    EXPECT_EQ(matchingKeys, std::size_t{ 0 });
}

} // namespace

TEST(ReflectionMetadata, NonintrusiveFieldTraversal)
{
    checkRegisteredFields<pure::TestObject>();
}

TEST(ReflectionMetadata, IntrusiveFieldTraversal)
{
    checkRegisteredFields<reflected::TestObject>();
}

TEST(ReflectionMetadata, TemplateAndInheritedProperties)
{
    using PureNestedMeta = StaticPropertyMeta<pure::NestedTestObject>;
    using ReflectedNestedMeta = StaticPropertyMeta<reflected::NestedTestObject>;
    using PureTemplateMeta = StaticPropertyMeta<pure::TemplateNestedTestObject<int>>;
    using ReflectedTemplateMeta = StaticPropertyMeta<reflected::TemplateNestedTestObject<std::string>>;

    static constexpr char valueName[] = "value";
    static constexpr char validName[] = "isValid";
    static constexpr char baseName[] = "_base_BaseTestObject";

    static_assert(PureNestedMeta::size() == 2);
    static_assert(ReflectedNestedMeta::size() == 2);
    static_assert(PureTemplateMeta::size() == 2);
    static_assert(ReflectedTemplateMeta::size() == 2);

    static_assert(PureNestedMeta::contains<validName>());
    static_assert(ReflectedNestedMeta::contains<baseName>());
    static_assert(PureTemplateMeta::contains<valueName>());
    static_assert(ReflectedTemplateMeta::contains<baseName>());

    EXPECT_EQ(PureTemplateMeta::getStaticPropertyMap()->name,
        std::string_view("TemplateNestedTestObject"));
}
