#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <nlohmann/json.hpp>

#include "extensions/comparisonextension.h"
#include "extensions/fromjsonextension.h"
#include "extensions/tojsonextension.h"
#include "extensions/tostringextension.h"
#include "test/testfixtures.h"

namespace extensions_type_checker_tests
{

struct Base
{
    int inherited = 10;
    int excluded = 20;
};

REFLECTION_NONINTRUSIVE(Base, &Base::inherited, &Base::excluded)

struct Nested
{
    REFLECTION_BEGIN(Nested)
    REFLECTION_PROPERTY(int, included, 30)
    REFLECTION_PROPERTY(int, excluded, 40)
    REFLECTION_END()
};

struct NonIntrusiveRoot : Base
{
    Nested nested;
    std::vector<Nested> children{ Nested{} };
    int excluded = 50;
};

REFLECTION_NONINTRUSIVE(NonIntrusiveRoot,
    &NonIntrusiveRoot::nested, &NonIntrusiveRoot::children, &NonIntrusiveRoot::excluded)
REFLECTION_BASE_CLASSES_NONINTRUSIVE(NonIntrusiveRoot, Base)

struct IntrusiveRoot : Base
{
    REFLECTION_BEGIN(IntrusiveRoot)
    REFLECTION_BASE_CLASS(Base)
    REFLECTION_PROPERTY(Nested, nested)
    REFLECTION_PROPERTY(std::vector<Nested>, children, std::vector<Nested>{ Nested{} })
    REFLECTION_PROPERTY(int, excluded, 50)
    REFLECTION_END()
};

struct ConstNestedRoot
{
    const Nested immutable{};
};

REFLECTION_NONINTRUSIVE(ConstNestedRoot, &ConstNestedRoot::immutable)

template <typename Root>
void checkFilteringAcrossExtensions()
{
    const Root original{};
    Root different = original;
    different.Base::excluded = 81;
    different.nested.excluded = 82;
    different.children.front().excluded = 83;
    different.excluded = 84;

    EXPECT_TRUE(extensions::equal<ExcludeFields>(original, different));
    EXPECT_FALSE(extensions::notEqual<ExcludeFields>(original, different));
    EXPECT_NE(extensions::compare(original, different), 0);

    nlohmann::json output;
    ASSERT_TRUE(extensions::convertToJson<ExcludeFields>(original, output));
    EXPECT_FALSE(output.contains("excluded"));
    EXPECT_FALSE(output.at("_base_Base").contains("excluded"));
    EXPECT_FALSE(output.at("nested").contains("excluded"));
    EXPECT_FALSE(output.at("children").at(0).contains("excluded"));
    EXPECT_EQ(output.at("nested").at("included"), 30);

    const auto rendered = extensions::convertToString<ExcludeFields>(original);
    EXPECT_EQ(rendered.find("\"excluded\""), std::string::npos);
    EXPECT_NE(rendered.find("\"included\""), std::string::npos);
    EXPECT_NE(rendered.find("\"inherited\""), std::string::npos);

    different.Base::inherited = -1;
    different.nested.included = -2;
    different.children.front().included = -3;
    ASSERT_TRUE(extensions::convertFromJson<ExcludeFields>(different, output));
    EXPECT_TRUE(extensions::equal<ExcludeFields>(original, different));
    EXPECT_EQ(different.Base::inherited, 10);
    EXPECT_EQ(different.nested.included, 30);
    EXPECT_EQ(different.children.front().included, 30);
    EXPECT_EQ(different.Base::excluded, 81);
    EXPECT_EQ(different.nested.excluded, 82);
    EXPECT_EQ(different.children.front().excluded, 40);
    EXPECT_EQ(different.excluded, 84);
}

template <typename Root>
void checkBaseFiltering()
{
    const Root original{};
    Root different = original;
    different.Base::inherited = 100;
    different.Base::excluded = 200;

    EXPECT_TRUE(extensions::equal<ExcludeBase>(original, different));
    EXPECT_FALSE(extensions::equal(original, different));

    nlohmann::json output;
    ASSERT_TRUE(extensions::convertToJson<ExcludeBase>(original, output));
    EXPECT_FALSE(output.contains("_base_Base"));
    EXPECT_EQ(extensions::convertToString<ExcludeBase>(original).find("_base_Base"),
        std::string::npos);

    ASSERT_TRUE(extensions::convertFromJson<ExcludeBase>(different, output));
    EXPECT_EQ(different.Base::inherited, 100);
    EXPECT_EQ(different.Base::excluded, 200);
}

TEST(ExtensionsTypeChecker, FiltersNestedFieldsAndContainersForBothRegistrations)
{
    checkFilteringAcrossExtensions<NonIntrusiveRoot>();
    checkFilteringAcrossExtensions<IntrusiveRoot>();
}

TEST(ExtensionsTypeChecker, FiltersAnEntireBaseClassForBothRegistrations)
{
    checkBaseFiltering<NonIntrusiveRoot>();
    checkBaseFiltering<IntrusiveRoot>();
}

TEST(ExtensionsTypeChecker, ConstReflectedFieldUsesTheSameFilterInDebugValidation)
{
    const ConstNestedRoot original{};
    nlohmann::json output;
    ASSERT_TRUE(extensions::convertToJson<ExcludeFields>(original, output));
    EXPECT_FALSE(output.at("immutable").contains("excluded"));

    ConstNestedRoot restored{};
    EXPECT_TRUE(extensions::convertFromJson<ExcludeFields>(restored, output));
}

TEST(ExtensionsTypeChecker, OrderingWrappersUseTheSameChecker)
{
    NonIntrusiveRoot lower;
    NonIntrusiveRoot higher;
    higher.excluded = 0;

    EXPECT_TRUE(extensions::equal<ExcludeFields>(lower, higher));
    EXPECT_TRUE(extensions::lessEqual<ExcludeFields>(lower, higher));
    EXPECT_TRUE(extensions::greaterEqual<ExcludeFields>(lower, higher));
    EXPECT_FALSE(extensions::less<ExcludeFields>(lower, higher));
    EXPECT_FALSE(extensions::greater<ExcludeFields>(lower, higher));

    higher.nested.included = lower.nested.included + 1;
    EXPECT_TRUE(extensions::less<ExcludeFields>(lower, higher));
    EXPECT_TRUE(extensions::lessEqual<ExcludeFields>(lower, higher));
    EXPECT_TRUE(extensions::greater<ExcludeFields>(higher, lower));
    EXPECT_TRUE(extensions::greaterEqual<ExcludeFields>(higher, lower));
    EXPECT_TRUE(extensions::notEqual<ExcludeFields>(lower, higher));
}

} // namespace extensions_type_checker_tests
