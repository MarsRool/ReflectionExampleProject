#include <gtest/gtest.h>

#include <string>

#include "extensions/tostringextension.h"
#include "test/test.h"

using namespace extensions;

TEST(Extensions, StringConversion)
{
    const auto test = createCompoundTestObject();
    const auto testString = convertToString(test);

    static constexpr char expectedString[] = "\"CompoundTestObject\":\n{ \"pureTestObject\": \"TestObject\":\n{ \"age\": 157,\n\"nested\": \"NestedTestObject\":\n{ \"isValid\": true,\n\"_base_BaseTestObject\":\n{ \"name\": \"Nested test\" } },\n\"templateNested1\": \"TemplateNestedTestObject\":\n{ \"value\": -137,\n\"_base_BaseTestObject\":\n{ \"name\": \"Template nested 1st\" } },\n\"templateNested2\": \"TemplateNestedTestObject\":\n{ \"value\": \"some value\",\n\"_base_BaseTestObject\":\n{ \"name\": \"Template nested 2nd\" } },\n\"nestedPtr\": \"NestedTestObject\":\n{ \"isValid\": false,\n\"_base_BaseTestObject\":\n{ \"name\": \"Nested value\" } },\n\"templateNestedPtr\": \"TemplateNestedTestObject\":\n{ \"value\": 123.456001,\n\"_base_BaseTestObject\":\n{ \"name\": \"Template Nested Value\" } },\n\"stringArr\": [ \"asdf\", \"fdsa\" ],\n\"realArr\": [ 1.750000, -1651.130000, 179.000000 ],\n\"_base_BaseTestObject\":\n{ \"name\": \"Name\" },\n\"_base_AnotherBaseTestObject\":\n{ \"anotherName\": \"Another name\" } },\n\"reflectedTestObject\": \"TestObject\":\n{ \"_base_BaseTestObject\":\n{ \"name\": \"Name\" },\n\"_base_AnotherBaseTestObject\":\n{ \"anotherName\": \"Another name\" },\n\"age\": 157,\n\"nested\": \"NestedTestObject\":\n{ \"_base_BaseTestObject\":\n{ \"name\": \"Nested test\" },\n\"isValid\": true },\n\"templateNested1\": \"TemplateNestedTestObject\":\n{ \"_base_BaseTestObject\":\n{ \"name\": \"Template nested 1st\" },\n\"value\": -137 },\n\"templateNested2\": \"TemplateNestedTestObject\":\n{ \"_base_BaseTestObject\":\n{ \"name\": \"Template nested 2nd\" },\n\"value\": \"some value\" },\n\"nestedPtr\": \"NestedTestObject\":\n{ \"_base_BaseTestObject\":\n{ \"name\": \"Nested value\" },\n\"isValid\": false },\n\"templateNestedPtr\": \"TemplateNestedTestObject\":\n{ \"_base_BaseTestObject\":\n{ \"name\": \"Template Nested Value\" },\n\"value\": 123.456001 },\n\"stringArr\": [ \"asdf\", \"fdsa\" ],\n\"realArr\": [ 1.750000, -1651.130000, 179.000000 ] } }";

    EXPECT_EQ(testString, expectedString);

    const auto pureTestString = convertToString(test.pureTestObject);
    EXPECT_NE(testString.find(pureTestString), std::string::npos);

    const auto reflectedTestString = convertToString(test.reflectedTestObject);
    EXPECT_NE(testString.find(reflectedTestString), std::string::npos);
}


TEST(ExtensionsString, NullPointerHasReadableRepresentation)
{
    auto source = createTestObject<pure::TestObject>();
    source.nestedPtr = nullptr;
    const auto testString = convertToString(source);
    EXPECT_NE(testString.find("\"nestedPtr\": null"), std::string::npos);
}
