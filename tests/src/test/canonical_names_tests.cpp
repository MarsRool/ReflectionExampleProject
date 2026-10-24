#include <gtest/gtest.h>

#include <cstddef>
#include <string_view>
#include <type_traits>

#include "reflection/reflection.h"

namespace
{

constexpr char firstName[] = "example";
constexpr char sameFirstName[] = "example";
constexpr char suffix[] = "_field";
constexpr char multiNames[] =
    "(&Example::first), &ns::Object::second, &ns::Example<std::pair<int, int>>::third";

} // namespace

TEST(Reflection, CanonicalStringsShareStorageForIdenticalText)
{
    using First = reflection::CanonicalStaticString<firstName>;
    using Second = reflection::CanonicalStaticString<sameFirstName>;
    using Prefix = reflection::CanonicalStaticStringRange<firstName, 0, 3>;
    using Joined = reflection::CanonicalStaticStringConcat<firstName, suffix>;

    static_assert(std::is_same_v<First, Second>);
    static_assert(std::string_view(Prefix::value) == "exa");
    static_assert(std::string_view(Joined::value) == "example_field");
    EXPECT_EQ(std::string_view(First::value), "example");
}

TEST(Reflection, CanonicalPropertyParserHandlesQualifiedAndTemplateNames)
{
    using Parser = reflection::CanonicalPropertyNameParser<multiNames>;
    static_assert(Parser::namesCount == 3);
    static_assert(std::string_view(Parser::CanonicalName<0>::value) == "first");
    static_assert(std::string_view(Parser::CanonicalName<1>::value) == "second");
    static_assert(std::string_view(Parser::CanonicalName<2>::value) == "third");
    EXPECT_EQ(Parser::namesCount, 3u);
}
