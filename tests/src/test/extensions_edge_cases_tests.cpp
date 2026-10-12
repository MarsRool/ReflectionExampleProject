#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

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
    std::array<int, 2> fixed{{ 4, 5 }};
    std::vector<int> dynamic{ 1, 2 };
    std::string ownedString = "original";
    std::string_view borrowedString = "borrowed";
    float smallFloat = 3.5f;
    const int immutable = 7;
};

DECL_REFLECTION_NONINTRUSIVE(JsonRecord,
    &JsonRecord::smallSigned,
    &JsonRecord::smallUnsigned,
    &JsonRecord::wideUnsigned,
    &JsonRecord::fixed,
    &JsonRecord::dynamic,
    &JsonRecord::ownedString,
    &JsonRecord::borrowedString,
    &JsonRecord::smallFloat,
    &JsonRecord::immutable)

struct Child
{
    int number = 0;
};

DECL_REFLECTION_NONINTRUSIVE(Child, &Child::number)

struct PointerRecord
{
    Child* child = nullptr;
};

DECL_REFLECTION_NONINTRUSIVE(PointerRecord, &PointerRecord::child)

struct NumberRecord
{
    double value = 0;
};

DECL_REFLECTION_NONINTRUSIVE(NumberRecord, &NumberRecord::value)

QJsonObject wrap(const char* typeName, const QJsonObject& fields)
{
    QJsonObject result;
    result.insert(QString::fromUtf8(typeName), fields);
    return result;
}

QJsonArray makeArray(std::initializer_list<int> values)
{
    QJsonArray result;
    for (int value : values)
        result.append(value);
    return result;
}

TEST(ExtensionsEdgeCases, UnknownFieldsAreIgnoredAndMissingFieldsRemainUnchanged)
{
    JsonRecord record;
    QJsonObject fields;
    fields.insert("smallSigned", 42);
    fields.insert("unrecognized", "ignored");

    EXPECT_TRUE(extensions::convertFromJson(record, wrap("JsonRecord", fields)));
    EXPECT_EQ(record.smallSigned, 42);
    EXPECT_EQ(record.smallUnsigned, 8);
    EXPECT_EQ(record.ownedString, "original");
}

TEST(ExtensionsEdgeCases, StringViewCannotBindTemporaryString)
{
    JsonRecord record;
    QJsonObject fields;
    fields.insert("borrowedString", "new text");

    EXPECT_FALSE(extensions::convertFromJson(record, wrap("JsonRecord", fields)));
    EXPECT_EQ(record.borrowedString, "borrowed");
}

TEST(ExtensionsEdgeCases, IntegralOverflowNegativeUnsignedAndFractionAreRejected)
{
    JsonRecord record;
    QJsonObject fields;

    fields.insert("smallUnsigned", 300);
    EXPECT_FALSE(extensions::convertFromJson(record, wrap("JsonRecord", fields)));
    EXPECT_EQ(record.smallUnsigned, 8);

    fields.remove("smallUnsigned");
    fields.insert("smallUnsigned", -1);
    EXPECT_FALSE(extensions::convertFromJson(record, wrap("JsonRecord", fields)));
    EXPECT_EQ(record.smallUnsigned, 8);

    fields.remove("smallUnsigned");
    fields.insert("smallSigned", 3.25);
    EXPECT_FALSE(extensions::convertFromJson(record, wrap("JsonRecord", fields)));
    EXPECT_EQ(record.smallSigned, 7);

    fields.remove("smallSigned");
    fields.insert("smallSigned", -128);
    EXPECT_TRUE(extensions::convertFromJson(record, wrap("JsonRecord", fields)));
    EXPECT_EQ(record.smallSigned, -128);
}

TEST(ExtensionsEdgeCases, FullSigned64BitRangeCanRoundTrip)
{
    // Direct low-level helpers test exact qint64 conversion without registration.
    QJsonValue json;
    const auto source = std::numeric_limits<std::int64_t>::min();
    EXPECT_TRUE(extensions::valueToJson(source, json, extensions::TypeTag<const std::int64_t>{}));
    std::int64_t restored = 0;
    EXPECT_TRUE(extensions::valueFromJson(restored, json, extensions::TypeTag<std::int64_t>{}));
    EXPECT_EQ(restored, source);
}

TEST(ExtensionsEdgeCases, RejectsUnsignedValuesBeyondQtSignedRange)
{
    JsonRecord record;
    record.wideUnsigned = std::numeric_limits<std::uint64_t>::max();
    QJsonObject json;
    EXPECT_FALSE(extensions::convertToJson(record, json));
}

TEST(ExtensionsEdgeCases, RejectsFloatingPointOverflow)
{
    JsonRecord record;
    QJsonObject fields;
    fields.insert("smallFloat", 1.0e100);
    EXPECT_FALSE(extensions::convertFromJson(record, wrap("JsonRecord", fields)));
    EXPECT_FLOAT_EQ(record.smallFloat, 3.5f);
}

TEST(ExtensionsEdgeCases, FixedArraysRequireMatchingLength)
{
    JsonRecord record;
    QJsonObject fields;

    fields.insert("fixed", makeArray({ 1 }));
    EXPECT_FALSE(extensions::convertFromJson(record, wrap("JsonRecord", fields)));
    EXPECT_EQ(record.fixed[0], 4);
    EXPECT_EQ(record.fixed[1], 5);

    fields.insert("fixed", makeArray({ 1, 2, 3 }));
    EXPECT_FALSE(extensions::convertFromJson(record, wrap("JsonRecord", fields)));
    EXPECT_EQ(record.fixed[0], 4);
    EXPECT_EQ(record.fixed[1], 5);

    fields.insert("fixed", makeArray({ 6, 7 }));
    EXPECT_TRUE(extensions::convertFromJson(record, wrap("JsonRecord", fields)));
    EXPECT_EQ(record.fixed[0], 6);
    EXPECT_EQ(record.fixed[1], 7);
}

TEST(ExtensionsEdgeCases, VectorRemainsResizable)
{
    JsonRecord record;
    QJsonObject fields;
    fields.insert("dynamic", makeArray({ 1, 3, 5, 7 }));
    EXPECT_TRUE(extensions::convertFromJson(record, wrap("JsonRecord", fields)));
    EXPECT_EQ(record.dynamic, (std::vector<int>{ 1, 3, 5, 7 }));
}

TEST(ExtensionsEdgeCases, ConstFieldsKeepDebugValidationAndReleaseSkip)
{
    JsonRecord record;
    QJsonObject fields;
    fields.insert("immutable", 999);
#ifdef QT_DEBUG
    EXPECT_FALSE(extensions::convertFromJson(record, wrap("JsonRecord", fields)));
#else
    EXPECT_TRUE(extensions::convertFromJson(record, wrap("JsonRecord", fields)));
#endif
    EXPECT_EQ(record.immutable, 7);
}

TEST(ExtensionsEdgeCases, AlreadyNullPointerAcceptsJsonNull)
{
    PointerRecord record;
    QJsonObject fields;
    fields.insert("child", QJsonValue(QJsonValue::Null));
    EXPECT_TRUE(extensions::convertFromJson(record, wrap("PointerRecord", fields)));
    EXPECT_EQ(record.child, nullptr);

    Child child{};
    record.child = &child;
    EXPECT_FALSE(extensions::convertFromJson(record, wrap("PointerRecord", fields)));
    EXPECT_EQ(record.child, &child);

    QJsonObject childFields;
    childFields.insert("number", 17);
    fields.insert("child", childFields);
    EXPECT_TRUE(extensions::convertFromJson(record, wrap("PointerRecord", fields)));
    EXPECT_EQ(child.number, 17);
}

TEST(ExtensionsEdgeCases, NullablePointersHaveStableValueBasedOrdering)
{
    Child small{ 1 };
    Child otherSmall{ 1 };
    Child big{ 2 };
    PointerRecord empty{};
    PointerRecord left{ &small };
    PointerRecord sameValue{ &otherSmall };
    PointerRecord higher{ &big };

    EXPECT_TRUE(extensions::equal(empty, empty));
    EXPECT_LT(extensions::compare(empty, left), 0);
    EXPECT_GT(extensions::compare(left, empty), 0);
    EXPECT_TRUE(extensions::equal(left, sameValue));
    EXPECT_LT(extensions::compare(left, higher), 0);
}

TEST(ExtensionsEdgeCases, NaNHasDefinedOrdering)
{
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();
    NumberRecord left{ nan };
    NumberRecord right{ nan };
    NumberRecord finite{ 42.0 };
    NumberRecord infinite{ infinity };

    EXPECT_TRUE(extensions::equal(left, right));
    EXPECT_GT(extensions::compare(left, finite), 0);
    EXPECT_GT(extensions::compare(left, infinite), 0);
    EXPECT_LT(extensions::compare(finite, left), 0);

    QJsonObject output;
    EXPECT_FALSE(extensions::convertToJson(left, output));
}

TEST(ExtensionsEdgeCases, ToStringDoesNotInsertTrailingCommas)
{
    NumberRecord record{ 1.25 };
    const auto text = extensions::convertToString(record);
    EXPECT_NE(text.find("\"value\": 1.250000"), std::string::npos);
    EXPECT_EQ(text.find(",\n }"), std::string::npos);
}

} // namespace extensions_edge_case_tests
