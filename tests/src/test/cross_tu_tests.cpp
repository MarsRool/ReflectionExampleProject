#include <gtest/gtest.h>

#include <cstddef>
#include <string_view>

#include "test/cross_tu_metadata.h"

namespace
{

void expectSameMetadata(const cross_tu_test::TypeMetadata& first,
                        const cross_tu_test::TypeMetadata& second,
                        std::size_t expectedSize,
                        std::string_view expectedTypeName,
                        std::string_view expectedPrimaryName)
{
    EXPECT_EQ(first.size, expectedSize);
    EXPECT_EQ(second.size, expectedSize);
    EXPECT_EQ(first.size, second.size);
    EXPECT_FALSE(first.isEmpty);
    EXPECT_FALSE(second.isEmpty);

    EXPECT_TRUE(first.hasPrimary);
    EXPECT_TRUE(second.hasPrimary);
    EXPECT_TRUE(first.hasInherited);
    EXPECT_TRUE(second.hasInherited);
    EXPECT_FALSE(first.hasMissing);
    EXPECT_FALSE(second.hasMissing);

    EXPECT_EQ(first.typeName, expectedTypeName);
    EXPECT_EQ(second.typeName, expectedTypeName);
    EXPECT_EQ(first.primaryName, expectedPrimaryName);
    EXPECT_EQ(second.primaryName, expectedPrimaryName);
    EXPECT_EQ(first.inheritedName, "_base_BaseTestObject");
    EXPECT_EQ(second.inheritedName, "_base_BaseTestObject");

    // Property objects are static constexpr template members (inline in C++17).
    // They should resolve to the same address in both translation units.
    ASSERT_NE(first.propertyMap, nullptr);
    ASSERT_NE(first.primaryProperty, nullptr);
    ASSERT_NE(first.inheritedProperty, nullptr);
    EXPECT_EQ(first.propertyMap, second.propertyMap);
    EXPECT_EQ(first.primaryProperty, second.primaryProperty);
    EXPECT_EQ(first.inheritedProperty, second.inheritedProperty);
}

} // namespace

TEST(ReflectionCrossTU, NonintrusiveObjectMetadata)
{
    const auto first = cross_tu_test::captureFromFirstTu();
    const auto second = cross_tu_test::captureFromSecondTu();
    expectSameMetadata(first.nonintrusiveObject, second.nonintrusiveObject,
        10, "TestObject", "age");
}

TEST(ReflectionCrossTU, IntrusiveObjectMetadata)
{
    const auto first = cross_tu_test::captureFromFirstTu();
    const auto second = cross_tu_test::captureFromSecondTu();
    expectSameMetadata(first.intrusiveObject, second.intrusiveObject,
        10, "TestObject", "age");
}

TEST(ReflectionCrossTU, NonintrusiveTemplateMetadata)
{
    const auto first = cross_tu_test::captureFromFirstTu();
    const auto second = cross_tu_test::captureFromSecondTu();
    expectSameMetadata(first.nonintrusiveTemplate, second.nonintrusiveTemplate,
        2, "TemplateNestedTestObject", "value");
}

TEST(ReflectionCrossTU, IntrusiveTemplateMetadata)
{
    const auto first = cross_tu_test::captureFromFirstTu();
    const auto second = cross_tu_test::captureFromSecondTu();
    expectSameMetadata(first.intrusiveTemplate, second.intrusiveTemplate,
        2, "TemplateNestedTestObject", "value");
}
