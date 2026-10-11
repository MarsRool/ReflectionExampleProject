#include <gtest/gtest.h>

#include <string>

#include "extensions/tostringextension.h"
#include "test/test.h"

using namespace extensions;

TEST(Extensions, StringConversion)
{
    const auto test = createCompoundTestObject();
    const auto testString = convertToString(test);

    static constexpr char expectedString[] = "\"CompoundTestObject\":\n{ \"pureTestObject\": \"TestObject\":\n{ \"age\": 157,\n\"nested\": \"NestedTestObject\":\n{ \"isValid\": true,\n\"_base_BaseTestObject\":\n{ \"name\": \"Nested test\",\n },\n },\n\"templateNested1\": \"TemplateNestedTestObject\":\n{ \"value\": -137,\n\"_base_BaseTestObject\":\n{ \"name\": \"Template nested 1st\",\n },\n },\n\"templateNested2\": \"TemplateNestedTestObject\":\n{ \"value\": \"some value\",\n\"_base_BaseTestObject\":\n{ \"name\": \"Template nested 2nd\",\n },\n },\n\"nestedPtr\": \"NestedTestObject\":\n{ \"isValid\": false,\n\"_base_BaseTestObject\":\n{ \"name\": \"Nested value\",\n },\n },\n\"templateNestedPtr\": \"TemplateNestedTestObject\":\n{ \"value\": 123.456001,\n\"_base_BaseTestObject\":\n{ \"name\": \"Template Nested Value\",\n },\n },\n\"stringArr\": [ \"asdf\", \"fdsa\" ],\n\"realArr\": [ 1.750000, -1651.130000, 179.000000 ],\n\"_base_BaseTestObject\":\n{ \"name\": \"Name\",\n },\n\"_base_AnotherBaseTestObject\":\n{ \"anotherName\": \"Another name\",\n },\n },\n\"reflectedTestObject\": \"TestObject\":\n{ \"_base_BaseTestObject\":\n{ \"name\": \"Name\",\n },\n\"_base_AnotherBaseTestObject\":\n{ \"anotherName\": \"Another name\",\n },\n\"age\": 157,\n\"nested\": \"NestedTestObject\":\n{ \"_base_BaseTestObject\":\n{ \"name\": \"Nested test\",\n },\n\"isValid\": true,\n },\n\"templateNested1\": \"TemplateNestedTestObject\":\n{ \"_base_BaseTestObject\":\n{ \"name\": \"Template nested 1st\",\n },\n\"value\": -137,\n },\n\"templateNested2\": \"TemplateNestedTestObject\":\n{ \"_base_BaseTestObject\":\n{ \"name\": \"Template nested 2nd\",\n },\n\"value\": \"some value\",\n },\n\"nestedPtr\": \"NestedTestObject\":\n{ \"_base_BaseTestObject\":\n{ \"name\": \"Nested value\",\n },\n\"isValid\": false,\n },\n\"templateNestedPtr\": \"TemplateNestedTestObject\":\n{ \"_base_BaseTestObject\":\n{ \"name\": \"Template Nested Value\",\n },\n\"value\": 123.456001,\n },\n\"stringArr\": [ \"asdf\", \"fdsa\" ],\n\"realArr\": [ 1.750000, -1651.130000, 179.000000 ],\n },\n }";

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
