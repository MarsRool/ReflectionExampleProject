#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>

#include "extensions/comparisonextension.h"
#include "extensions/fromjsonextension.h"
#include "extensions/tojsonextension.h"
#include "extensions/tostringextension.h"

namespace extensions_edge_case_tests
{

struct JsonRecord
{
    std::int8_t smallSigned = 7;
    std::uint8_t smallUnsigned = 8;
    std::uint64_t wideUnsigned = 12;
    std::array<int, 2> fixed{ { 4, 5 } };
    std::vector<int> dynamic{ 1, 2 };
    std::string ownedString = "original";
    std::string_view borrowedString = "borrowed";
    float smallFloat = 3.5f;
    const int immutable = 7;
};

REFLECTION_NONINTRUSIVE(JsonRecord,
    &JsonRecord::smallSigned,
    &JsonRecord::smallUnsigned,
    &JsonRecord::wideUnsigned,
    &JsonRecord::fixed,
    &JsonRecord::dynamic,
    &JsonRecord::ownedString,
    &JsonRecord::borrowedString,
    &JsonRecord::smallFloat,
    &JsonRecord::immutable)

struct NumberRecord
{
    double value = 0;
};
REFLECTION_NONINTRUSIVE(NumberRecord, &NumberRecord::value)

TEST(ExtensionsEdgeCases, UnknownFieldsAreIgnoredAndMissingFieldsRemainUnchanged)
{
    JsonRecord record;
    const nlohmann::json input = { { "smallSigned", 42 }, { "unrecognized", "ignored" } };
    EXPECT_TRUE(extensions::convertFromJson(record, input));
    EXPECT_EQ(record.smallSigned, 42);
    EXPECT_EQ(record.smallUnsigned, 8);
    EXPECT_EQ(record.ownedString, "original");
}

TEST(ExtensionsEdgeCases, StringViewCannotBindTemporaryString)
{
    JsonRecord record;
    const nlohmann::json input = { { "borrowedString", "new text" } };
    EXPECT_FALSE(extensions::convertFromJson(record, input));
    EXPECT_EQ(record.borrowedString, "borrowed");
}

TEST(ExtensionsEdgeCases, IntegralOverflowNegativeUnsignedAndFractionAreRejected)
{
    JsonRecord record;
    EXPECT_FALSE(extensions::convertFromJson(record, { { "smallUnsigned", 300 } }));
    EXPECT_EQ(record.smallUnsigned, 8);
    EXPECT_FALSE(extensions::convertFromJson(record, { { "smallUnsigned", -1 } }));
    EXPECT_EQ(record.smallUnsigned, 8);
    EXPECT_FALSE(extensions::convertFromJson(record, { { "smallSigned", 3.25 } }));
    EXPECT_EQ(record.smallSigned, 7);
    EXPECT_TRUE(extensions::convertFromJson(record, { { "smallSigned", -128 } }));
    EXPECT_EQ(record.smallSigned, -128);
}

TEST(ExtensionsEdgeCases, FullSignedAndUnsigned64BitRangeCanRoundTrip)
{
    JsonRecord record;
    record.wideUnsigned = std::numeric_limits<std::uint64_t>::max();
    nlohmann::json output;
    EXPECT_TRUE(extensions::convertToJson(record, output));
    EXPECT_TRUE(output.at("wideUnsigned").is_number_unsigned());

    std::uint64_t unsignedValue = 0;
    EXPECT_TRUE(extensions::impl::valueFromJson<reflection::TrueTypeChecker>(
        unsignedValue, output.at("wideUnsigned")));
    EXPECT_EQ(unsignedValue, record.wideUnsigned);

    const auto signedValue = std::numeric_limits<std::int64_t>::min();
    EXPECT_TRUE(extensions::impl::valueToJson<reflection::TrueTypeChecker>(signedValue, output));
    std::int64_t restored = 0;
    EXPECT_TRUE(extensions::impl::valueFromJson<reflection::TrueTypeChecker>(restored, output));
    EXPECT_EQ(restored, signedValue);
}

TEST(ExtensionsEdgeCases, RejectsFloatingPointOverflow)
{
    JsonRecord record;
    EXPECT_FALSE(extensions::convertFromJson(record, { { "smallFloat", 1.0e100 } }));
    EXPECT_FLOAT_EQ(record.smallFloat, 3.5f);
}

TEST(ExtensionsEdgeCases, FixedArraysRequireMatchingLength)
{
    JsonRecord record;
    EXPECT_FALSE(extensions::convertFromJson(record, { { "fixed", { 1 } } }));
    EXPECT_FALSE(extensions::convertFromJson(record, { { "fixed", { 1, 2, 3 } } }));
    EXPECT_EQ(record.fixed[0], 4);
    EXPECT_EQ(record.fixed[1], 5);

    EXPECT_TRUE(extensions::convertFromJson(record, { { "fixed", { 6, 7 } } }));
    EXPECT_EQ(record.fixed[0], 6);
    EXPECT_EQ(record.fixed[1], 7);
}

TEST(ExtensionsEdgeCases, VectorRemainsResizable)
{
    JsonRecord record;
    EXPECT_TRUE(extensions::convertFromJson(record, { { "dynamic", { 1, 3, 5, 7 } } }));
    EXPECT_EQ(record.dynamic, (std::vector<int>{ 1, 3, 5, 7 }));
}

TEST(ExtensionsEdgeCases, ConstFieldsKeepDebugValidationAndReleaseSkip)
{
    JsonRecord record;
    EXPECT_TRUE(extensions::convertFromJson(record, { { "immutable", 999 } }));
#ifndef NDEBUG
    EXPECT_FALSE(extensions::convertFromJson(record, { { "immutable", "invalid value" } }));
#else
    EXPECT_TRUE(extensions::convertFromJson(record, { { "immutable", "invalid value" } }));
#endif
    EXPECT_EQ(record.immutable, 7);
}

TEST(ExtensionsEdgeCases, NaNHasDefinedOrdering)
{
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();
    const NumberRecord left{ nan };
    const NumberRecord right{ nan };
    const NumberRecord finite{ 42.0 };
    const NumberRecord infinite{ infinity };

    EXPECT_TRUE(extensions::equal(left, right));
    EXPECT_GT(extensions::compare(left, finite), 0);
    EXPECT_GT(extensions::compare(left, infinite), 0);
    EXPECT_LT(extensions::compare(finite, left), 0);

    nlohmann::json output;
    EXPECT_FALSE(extensions::convertToJson(left, output));
}

TEST(ExtensionsEdgeCases, ToStringDoesNotInsertTrailingCommas)
{
    const NumberRecord record{ 1.25 };
    const auto text = extensions::convertToString(record);
    EXPECT_NE(text.find("\"value\": 1.250000"), std::string::npos);
    EXPECT_EQ(text.find(", }"), std::string::npos);
}

} // namespace extensions_edge_case_tests
