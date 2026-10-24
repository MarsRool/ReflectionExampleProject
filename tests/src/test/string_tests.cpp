#include <gtest/gtest.h>

#include <string>

#include "extensions/tostringextension.h"
#include "test/testfixtures.h"

using namespace extensions;

TEST(Extensions, StringConversion)
{
    const auto value = makeCompoundTestObject();
    const auto result = convertToString(value);

    EXPECT_NE(result.find("\"pureTestObject\": {"), std::string::npos);
    EXPECT_NE(result.find("\"reflectedTestObject\": {"), std::string::npos);
    EXPECT_NE(result.find("\"_base_Stats\": {"), std::string::npos);
    EXPECT_NE(result.find("\"_base_Stats<int>\": {"), std::string::npos);
    EXPECT_NE(result.find("\"_base_Identity\": {"), std::string::npos);
    EXPECT_NE(result.find("\"numbers\": [ 3, 5, 8 ]"), std::string::npos);
    EXPECT_NE(result.find("\"label\": \"nested\""), std::string::npos);
    EXPECT_EQ(result, convertToString(value));
}

TEST(ExtensionsString, StringValuesRemainUnescaped)
{
    auto value = makeCompoundTestObject();
    value.pureTestObject.text = "a\"b";
    const auto result = convertToString(value);
    EXPECT_NE(result.find("\"text\": \"a\"b\""), std::string::npos);
}

TEST(ExtensionsString, NoTrailingCommasInEmptyAndNonemptyObjects)
{
    const auto result = convertToString(makeCompoundTestObject());
    EXPECT_EQ(result.find(", }"), std::string::npos);
    EXPECT_NE(result.find("\"empty\": {  }"), std::string::npos);
}
