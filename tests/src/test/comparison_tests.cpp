#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "extensions/comparisonextension.h"
#include "test/test.h"

using namespace extensions;

TEST(Extensions, Equality)
{
    const auto test = createCompoundTestObject();
    auto test2{ test };

    ASSERT_TRUE(equal(test, test2)) << "Copied object must equal original";

    test2.pureTestObject.age += 15;
    EXPECT_FALSE(equal(test, test2)) << "Objects must differ after property modification";
}


namespace
{

template <typename Object>
void checkEveryFieldAffectsComparison()
{
    struct Mutation
    {
        const char* name;
        void (*apply)(Object&);
    };

    // Independent instances are essential: a copy would share pointer targets.
    const auto original = createTestObject<Object>();
    const std::vector<Mutation> mutations
    {
        { "inherited.name", [](Object& v) { v.name += " changed"; } },
        { "inherited.anotherName", [](Object& v) { v.anotherName += " changed"; } },
        { "age", [](Object& v) { ++v.age; } },
        { "nested.name", [](Object& v) { v.nested.name += " changed"; } },
        { "nested.isValid", [](Object& v) { v.nested.isValid = !v.nested.isValid; } },
        { "templateNested1.name", [](Object& v) { v.templateNested1.name += " changed"; } },
        { "templateNested1.value", [](Object& v) { ++v.templateNested1.value; } },
        { "templateNested2.name", [](Object& v) { v.templateNested2.name += " changed"; } },
        { "templateNested2.value", [](Object& v) { v.templateNested2.value += " changed"; } },
        { "nestedPtr.name", [](Object& v) { v.nestedPtr->name += " changed"; } },
        { "nestedPtr.isValid", [](Object& v) { v.nestedPtr->isValid = !v.nestedPtr->isValid; } },
        { "templateNestedPtr.name", [](Object& v) { v.templateNestedPtr->name += " changed"; } },
        { "templateNestedPtr.value", [](Object& v) { v.templateNestedPtr->value += 1.0f; } },
        { "stringArr.element", [](Object& v) { v.stringArr[0] += " changed"; } },
        { "stringArr.size", [](Object& v) { v.stringArr.emplace_back("another"); } },
        { "realArr.element", [](Object& v) { v.realArr[1] += 1.0; } }
    };

    EXPECT_TRUE(equal(original, original));

    for (const auto& mutation : mutations)
    {
        SCOPED_TRACE(mutation.name);
        auto modified = createTestObject<Object>();
        ASSERT_TRUE(equal(original, modified));
        mutation.apply(modified);
        EXPECT_FALSE(equal(original, modified));
        EXPECT_TRUE(notEqual(original, modified));
    }
}

} // namespace

TEST(ExtensionsComparison, EveryNonintrusiveFieldAffectsEquality)
{
    checkEveryFieldAffectsComparison<pure::TestObject>();
}

TEST(ExtensionsComparison, EveryIntrusiveFieldAffectsEquality)
{
    checkEveryFieldAffectsComparison<reflected::TestObject>();
}

TEST(ExtensionsComparison, BothCompoundMembersAffectEquality)
{
    const auto original = createCompoundTestObject();

    auto modifiedPure = createCompoundTestObject();
    modifiedPure.pureTestObject.age += 1;
    EXPECT_FALSE(equal(original, modifiedPure));

    auto modifiedReflected = createCompoundTestObject();
    modifiedReflected.reflectedTestObject.age += 1;
    EXPECT_FALSE(equal(original, modifiedReflected));
}

TEST(ExtensionsComparison, OrderingAndInverseRelations)
{
    auto lower = createTestObject<pure::TestObject>();
    auto higher = createTestObject<pure::TestObject>();
    higher.age += 1;

    EXPECT_LT(compare(lower, higher), 0);
    EXPECT_GT(compare(higher, lower), 0);
    EXPECT_EQ(compare(lower, lower), 0);
    EXPECT_TRUE(less(lower, higher));
    EXPECT_TRUE(lessEqual(lower, higher));
    EXPECT_TRUE(greater(higher, lower));
    EXPECT_TRUE(greaterEqual(higher, lower));
    EXPECT_TRUE(lessEqual(lower, lower));
    EXPECT_TRUE(greaterEqual(lower, lower));
    EXPECT_FALSE(less(higher, lower));
    EXPECT_FALSE(greater(lower, higher));
}
