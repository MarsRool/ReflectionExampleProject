#include <gtest/gtest.h>

#include <QJsonObject>
#include <QJsonValue>

#include "extensions/comparisonextension.h"
#include "extensions/fromjsonextension.h"
#include "extensions/tojsonextension.h"
#include "test/test.h"

using namespace extensions;

TEST(Extensions, JsonConversion)
{
    const auto test = createCompoundTestObject();

    QJsonObject parentJsonObject;
    ASSERT_TRUE(convertToJson(test, parentJsonObject));

    compound::CompoundTestObject loadedTest;

    ASSERT_TRUE(convertFromJson(loadedTest, parentJsonObject));

    EXPECT_TRUE(equal(test, loadedTest));
}


TEST(ExtensionsJson, ContainsBothNestedObjectKinds)
{
    const auto source = createCompoundTestObject();
    QJsonObject json;
    ASSERT_TRUE(convertToJson(source, json));
    ASSERT_TRUE(json.value("CompoundTestObject").isObject());

    const auto fields = json.value("CompoundTestObject").toObject();
    ASSERT_TRUE(fields.value("pureTestObject").isObject());
    ASSERT_TRUE(fields.value("reflectedTestObject").isObject());

    const auto pureFields = fields.value("pureTestObject").toObject();
    EXPECT_EQ(pureFields.value("age").toInteger(), 157);
    EXPECT_TRUE(pureFields.value("nested").isObject());
    EXPECT_TRUE(pureFields.value("stringArr").isArray());
    EXPECT_TRUE(pureFields.value("realArr").isArray());

    const auto reflectedFields = fields.value("reflectedTestObject").toObject();
    EXPECT_EQ(reflectedFields.value("age").toInteger(), 157);
    EXPECT_TRUE(reflectedFields.value("nested").isObject());
    EXPECT_TRUE(reflectedFields.value("stringArr").isArray());
    EXPECT_TRUE(reflectedFields.value("realArr").isArray());
}

TEST(ExtensionsJson, RoundTripAfterChangingValuesAndContainers)
{
    auto source = createCompoundTestObject();
    source.pureTestObject.name = "Updated name";
    source.pureTestObject.nested.isValid = false;
    source.pureTestObject.stringArr.emplace_back("extra");
    source.pureTestObject.realArr[2] = 9.25;
    source.reflectedTestObject.templateNested1.value = 17;
    source.reflectedTestObject.nestedPtr->name = "Changed pointer target";

    QJsonObject json;
    ASSERT_TRUE(convertToJson(source, json));

    auto restored = createCompoundTestObject();

    ASSERT_TRUE(convertFromJson(restored, json));
    EXPECT_TRUE(equal(source, restored));
}

TEST(ExtensionsJson, RejectsNonObjectRoot)
{
    QJsonObject malformed;
    malformed.insert("CompoundTestObject", 42);
    auto output = createCompoundTestObject();
    EXPECT_FALSE(convertFromJson(output, malformed));
}

TEST(ExtensionsJson, RejectsWrongFieldType)
{
    QJsonObject json;
    ASSERT_TRUE(convertToJson(createCompoundTestObject(), json));

    auto top = json.value("CompoundTestObject").toObject();
    auto pureFields = top.value("pureTestObject").toObject();
    pureFields.insert("age", "not an integer");
    top.insert("pureTestObject", pureFields);
    json.insert("CompoundTestObject", top);

    auto output = createCompoundTestObject();
    EXPECT_FALSE(convertFromJson(output, json));
}

TEST(ExtensionsJson, SerializesNullPointerAsNull)
{
    auto source = createCompoundTestObject();
    source.pureTestObject.nestedPtr = nullptr;

    QJsonObject json;
    ASSERT_TRUE(convertToJson(source, json));
    const auto root = json.value("CompoundTestObject").toObject();
    const auto pureFields = root.value("pureTestObject").toObject();
    EXPECT_TRUE(pureFields.contains("nestedPtr"));
    EXPECT_TRUE(pureFields.value("nestedPtr").isNull());
}
