#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include "extensions/comparisonextension.h"
#include "extensions/fromjsonextension.h"
#include "extensions/tojsonextension.h"
#include "test/testfixtures.h"

using namespace extensions;

TEST(Extensions, JsonConversion)
{
    const auto original = makeCompoundTestObject();
    nlohmann::json output;
    ASSERT_TRUE(convertToJson(original, output));
    ASSERT_TRUE(output.is_object());

    compound::CompoundTestObject restored;
    ASSERT_TRUE(convertFromJson(restored, output));
    EXPECT_TRUE(equal<ExcludeUnordered>(original, restored));
    EXPECT_EQ(original.pureTestObject.unordered,
              restored.pureTestObject.unordered);
    EXPECT_EQ(original.pureTestObject.unorderedRepeated,
              restored.pureTestObject.unorderedRepeated);
    EXPECT_EQ(original.reflectedTestObject.unordered,
              restored.reflectedTestObject.unordered);
    EXPECT_EQ(original.reflectedTestObject.unorderedRepeated,
              restored.reflectedTestObject.unorderedRepeated);
}

TEST(ExtensionsJson, ContainsBothNestedObjectKinds)
{
    const auto original = makeCompoundTestObject();
    nlohmann::json output;
    ASSERT_TRUE(convertToJson(original, output));

    const auto& pure = output.at("pureTestObject");
    const auto& reflected = output.at("reflectedTestObject");
    EXPECT_TRUE(pure.is_object());
    EXPECT_TRUE(reflected.is_object());
    EXPECT_EQ(pure.at("_base_Stats").at("_base_Identity").at("id").get<int>(), 42);
    EXPECT_EQ(reflected.at("_base_Stats<int>").at("_base_Identity").at("id").get<int>(), 42);
    EXPECT_TRUE(pure.at("nested").is_object());
    EXPECT_TRUE(reflected.at("templateNested").is_object());
    EXPECT_TRUE(pure.at("numbers").is_array());
    EXPECT_TRUE(reflected.at("fixedNested").is_array());
    EXPECT_TRUE(pure.at("matrix").is_array());
}

TEST(ExtensionsJson, RoundTripAfterChangingValuesAndContainers)
{
    auto original = makeCompoundTestObject();
    original.pureTestObject.name = "Updated name";
    original.pureTestObject.nested.count += 1;
    original.pureTestObject.numbers.push_back(99);
    original.pureTestObject.fixedNested[0].label = "Changed";
    original.reflectedTestObject.templateNested.score = "Updated template";
    original.reflectedTestObject.children[0].label = "Updated child";
    original.pureTestObject.unordered = { 11, 33, 99 };
    original.pureTestObject.unorderedRepeated = { 3, 3, 5, 5 };
    original.reflectedTestObject.unordered = { 44, 77, 99 };
    original.reflectedTestObject.unorderedRepeated = { 5, 5, 8, 8 };

    nlohmann::json output;
    ASSERT_TRUE(convertToJson(original, output));

    auto restored = makeCompoundTestObject();
    ASSERT_TRUE(convertFromJson(restored, output));
    EXPECT_TRUE(equal<ExcludeUnordered>(original, restored));
    EXPECT_EQ(original.pureTestObject.unordered,
              restored.pureTestObject.unordered);
    EXPECT_EQ(original.pureTestObject.unorderedRepeated,
              restored.pureTestObject.unorderedRepeated);
    EXPECT_EQ(original.reflectedTestObject.unordered,
              restored.reflectedTestObject.unordered);
    EXPECT_EQ(original.reflectedTestObject.unorderedRepeated,
              restored.reflectedTestObject.unorderedRepeated);
}

TEST(ExtensionsJson, RejectsNonObjectRoot)
{
    auto output = makeCompoundTestObject();
    EXPECT_FALSE(convertFromJson(output, nlohmann::json::array()));
    EXPECT_FALSE(convertFromJson(output, nlohmann::json(42)));
}

TEST(ExtensionsJson, RejectsWrongFieldType)
{
    nlohmann::json output;
    ASSERT_TRUE(convertToJson(makeCompoundTestObject(), output));
    output["pureTestObject"]["_base_Stats"]["_base_Identity"]["id"] = "not an integer";

    auto restored = makeCompoundTestObject();
    EXPECT_FALSE(convertFromJson(restored, output));
}
