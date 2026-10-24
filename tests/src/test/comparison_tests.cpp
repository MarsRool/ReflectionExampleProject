#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "extensions/comparisonextension.h"
#include "test/testfixtures.h"

using namespace extensions;

TEST(Extensions, Equality)
{
    const auto original = makeCompoundTestObject();
    auto copy = original;

    ASSERT_TRUE(equal<ExcludeUnordered>(original, copy));
    copy.pureTestObject.score += 15;
    EXPECT_FALSE(equal<ExcludeUnordered>(original, copy));
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

    const auto original = makeTestObject<Object>();
    const std::vector<Mutation> mutations
    {
        { "inherited.id", [](Object& v) { ++v.id; } },
        { "inherited.name", [](Object& v) { v.name += " changed"; } },
        { "inherited.score", [](Object& v) { ++v.score; } },
        { "inherited.history[0]", [](Object& v) { ++v.history[0]; } },
        { "inherited.history[1]", [](Object& v) { ++v.history[1]; } },
        { "inherited.enabled", [](Object& v) { v.enabled = !v.enabled; } },
        { "inherited.ratio", [](Object& v) { v.ratio += 1.0; } },
        { "nested.label", [](Object& v) { v.nested.label += " changed"; } },
        { "nested.count", [](Object& v) { ++v.nested.count; } },
        { "nested.weight", [](Object& v) { v.nested.weight += 1.0f; } },
        { "templateNested.id", [](Object& v) { ++v.templateNested.id; } },
        { "templateNested.name", [](Object& v) { v.templateNested.name += " changed"; } },
        { "templateNested.score", [](Object& v) { v.templateNested.score += " changed"; } },
        { "templateNested.history[0]", [](Object& v) { ++v.templateNested.history[0]; } },
        { "templateNested.history[1]", [](Object& v) { ++v.templateNested.history[1]; } },
        { "children[0].label", [](Object& v) { v.children[0].label += " changed"; } },
        { "children[0].count", [](Object& v) { ++v.children[0].count; } },
        { "children[0].weight", [](Object& v) { v.children[0].weight += 1.0f; } },
        { "children[1].label", [](Object& v) { v.children[1].label += " changed"; } },
        { "children[1].count", [](Object& v) { ++v.children[1].count; } },
        { "children[1].weight", [](Object& v) { v.children[1].weight += 1.0f; } },
        { "children.size", [](Object& v) { v.children.emplace_back(); } },
        { "numbers[0]", [](Object& v) { ++v.numbers[0]; } },
        { "numbers[1]", [](Object& v) { ++v.numbers[1]; } },
        { "numbers[2]", [](Object& v) { ++v.numbers[2]; } },
        { "numbers.size", [](Object& v) { v.numbers.push_back(99); } },
        { "switches[0]", [](Object& v) { v.switches[0] = !v.switches[0]; } },
        { "switches[1]", [](Object& v) { v.switches[1] = !v.switches[1]; } },
        { "switches[2]", [](Object& v) { v.switches[2] = !v.switches[2]; } },
        { "dequeValues[0]", [](Object& v) { ++v.dequeValues[0]; } },
        { "dequeValues[1]", [](Object& v) { ++v.dequeValues[1]; } },
        { "listValues.front", [](Object& v) { ++v.listValues.front(); } },
        { "listValues.back", [](Object& v) { ++v.listValues.back(); } },
        { "sorted", [](Object& v) { v.sorted.insert(9); } },
        { "repeated", [](Object& v) { v.repeated.insert(9); } },
        { "fixed[0]", [](Object& v) { ++v.fixed[0]; } },
        { "fixed[1]", [](Object& v) { ++v.fixed[1]; } },
        { "fixed[2]", [](Object& v) { ++v.fixed[2]; } },
        { "fixedNested[0].label", [](Object& v) { v.fixedNested[0].label += " changed"; } },
        { "fixedNested[0].count", [](Object& v) { ++v.fixedNested[0].count; } },
        { "fixedNested[1].label", [](Object& v) { v.fixedNested[1].label += " changed"; } },
        { "fixedNested[1].count", [](Object& v) { ++v.fixedNested[1].count; } },
        { "raw[0]", [](Object& v) { ++v.raw[0]; } },
        { "raw[1]", [](Object& v) { ++v.raw[1]; } },
        { "matrix[0][0]", [](Object& v) { ++v.matrix[0][0]; } },
        { "matrix[0][1]", [](Object& v) { ++v.matrix[0][1]; } },
        { "matrix[0][2]", [](Object& v) { ++v.matrix[0][2]; } },
        { "matrix[1][0]", [](Object& v) { ++v.matrix[1][0]; } },
        { "matrix[1][1]", [](Object& v) { ++v.matrix[1][1]; } },
        { "matrix[1][2]", [](Object& v) { ++v.matrix[1][2]; } },
        { "valarrayValues[0]", [](Object& v) { ++v.valarrayValues[0]; } },
        { "valarrayValues[1]", [](Object& v) { ++v.valarrayValues[1]; } },
        { "valarrayValues[2]", [](Object& v) { ++v.valarrayValues[2]; } },
        { "text", [](Object& v) { v.text += " changed"; } },
        { "mode", [](Object& v) { v.mode = decltype(v.mode)::idle; } },
        { "signedWide", [](Object& v) { ++v.signedWide; } },
        { "unsignedWide", [](Object& v) { --v.unsignedWide; } }
    };

    EXPECT_TRUE(equal<ExcludeUnordered>(original, original));

    for (const auto& mutation : mutations)
    {
        SCOPED_TRACE(mutation.name);
        auto modified = makeTestObject<Object>();
        ASSERT_TRUE(equal<ExcludeUnordered>(original, modified));
        mutation.apply(modified);
        EXPECT_FALSE(equal<ExcludeUnordered>(original, modified));
        EXPECT_TRUE(notEqual<ExcludeUnordered>(original, modified));
    }
}

template <typename Object>
void checkUnorderedAreExcluded()
{
    using Meta = reflection::StaticPropertyMeta<Object>;
    static_assert(Meta::template size<ExcludeUnordered>() == 21);

    const auto original = makeTestObject<Object>();
    auto modified = original;
    modified.unordered.insert(123);
    modified.unorderedRepeated.insert(456);

    ASSERT_NE(original.unordered, modified.unordered);
    ASSERT_NE(original.unorderedRepeated, modified.unorderedRepeated);
    EXPECT_TRUE(equal<ExcludeUnordered>(original, modified));
    EXPECT_FALSE(notEqual<ExcludeUnordered>(original, modified));
}

} // namespace

TEST(ExtensionsComparison, UnorderedFieldsAreExcluded)
{
    checkUnorderedAreExcluded<pure::TestObject>();
    checkUnorderedAreExcluded<reflected::TestObject>();
}

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
    const auto original = makeCompoundTestObject();

    auto modifiedPure = original;
    modifiedPure.pureTestObject.score += 1;
    EXPECT_FALSE(equal<ExcludeUnordered>(original, modifiedPure));

    auto modifiedReflected = original;
    modifiedReflected.reflectedTestObject.score += 1;
    EXPECT_FALSE(equal<ExcludeUnordered>(original, modifiedReflected));
}

TEST(ExtensionsComparison, OrderingAndInverseRelations)
{
    auto lower = makeTestObject<pure::TestObject>();
    auto higher = lower;
    ++higher.id;

    EXPECT_LT(compare<ExcludeUnordered>(lower, higher), 0);
    EXPECT_GT(compare<ExcludeUnordered>(higher, lower), 0);
    EXPECT_EQ(compare<ExcludeUnordered>(lower, lower), 0);
    EXPECT_TRUE(less<ExcludeUnordered>(lower, higher));
    EXPECT_TRUE(lessEqual<ExcludeUnordered>(lower, higher));
    EXPECT_TRUE(greater<ExcludeUnordered>(higher, lower));
    EXPECT_TRUE(greaterEqual<ExcludeUnordered>(higher, lower));
    EXPECT_TRUE(lessEqual<ExcludeUnordered>(lower, lower));
    EXPECT_TRUE(greaterEqual<ExcludeUnordered>(lower, lower));
    EXPECT_FALSE(less<ExcludeUnordered>(higher, lower));
    EXPECT_FALSE(greater<ExcludeUnordered>(lower, higher));
}
