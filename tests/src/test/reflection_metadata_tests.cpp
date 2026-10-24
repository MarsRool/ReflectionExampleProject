#include <gtest/gtest.h>

#include <cstddef>
#include <set>
#include <string_view>
#include <type_traits>

#include "test/testfixtures.h"

using namespace reflection;

namespace
{

constexpr char nonexistentKey[] = "nonexistent";
constexpr char pureKey[] = "pureTestObject";
constexpr char scoreKey[] = "score";
constexpr char idKey[] = "id";
constexpr char nestedKey[] = "nested";
constexpr char baseFlagsKey[] = "_base_Flags";

template <typename Object>
void checkRegisteredFields()
{
    using Meta = StaticPropertyMeta<Object>;

    static_assert(Meta::size() == 23);
    static_assert(Meta::template size<ExcludeBase>() == 21);
    static_assert(Meta::template contains<nestedKey>());
    static_assert(Meta::template contains<baseFlagsKey>());
    static_assert(!Meta::template contains<nonexistentKey>());
    static_assert(Meta::template at<nonexistentKey>() == nullptr);

    EXPECT_EQ(Meta::getStaticTypeInfo().name, std::string_view("TestObject"));

    std::set<std::string_view> names;
    Meta::forEach([&](auto entry) { names.emplace(decltype(entry)::key); });
    EXPECT_EQ(names.size(), Meta::size());
    EXPECT_EQ(names.count("nested"), 1u);
    EXPECT_EQ(names.count("_base_Flags"), 1u);

    Object object = makeTestObject<Object>();
    const auto property = Meta::template at<nestedKey>();
    ASSERT_NE(property, nullptr);
    EXPECT_EQ(property->get(object).count, 9);
    property->get(object).count = 42;
    EXPECT_EQ(object.nested.count, 42);

    std::size_t filtered = 0;
    Meta::forEachIf([&](auto) { ++filtered; }, [](auto entry)
    {
        return std::string_view(decltype(entry)::key) == "nested";
    });
    EXPECT_EQ(filtered, 1u);

    std::size_t stoppedAfter = 0;
    Meta::forEach([&](auto) -> bool
    {
        ++stoppedAfter;
        return false;
    });
    EXPECT_EQ(stoppedAfter, 1u);

    std::size_t found = 0;
    Meta::doForKey([&](auto) { ++found; }, "nested");
    EXPECT_EQ(found, 1u);

    found = 0;
    Meta::doForKey([&](auto) { ++found; }, "notRegistered");
    EXPECT_EQ(found, 0u);

    found = 0;
    Meta::template doForKey<nestedKey>([&](auto) { ++found; });
    EXPECT_EQ(found, 1u);
}

using PureStats = StaticPropertyMeta<pure::Stats<int>>;
using ReflectedStats = StaticPropertyMeta<reflected::Stats<int>>;

static_assert(PureStats::size() == 3);
static_assert(ReflectedStats::size() == 3);
static_assert(PureStats::contains<scoreKey>());
static_assert(ReflectedStats::contains<scoreKey>());
static_assert(StaticPropertyMeta<pure::Identity>::contains<idKey>());
static_assert(StaticPropertyMeta<reflected::Identity>::contains<idKey>());

} // namespace

TEST(Reflection, StaticPropertyMap)
{
    using Meta = StaticPropertyMeta<compound::CompoundTestObject>;
    static constexpr auto canonicalKey = CanonicalStaticString<pureKey>::value;

    static_assert(!Meta::empty());
    static_assert(Meta::size() == 2);
    static_assert(Meta::contains<pureKey>());
    static_assert(!Meta::contains<nonexistentKey>());
    static_assert(Meta::at<nonexistentKey>() == nullptr);
    static_assert(Meta::at<pureKey>() == Meta::at<canonicalKey>());
}

TEST(ReflectionMetadata, NonintrusiveFieldTraversal)
{
    checkRegisteredFields<pure::TestObject>();
}

TEST(ReflectionMetadata, IntrusiveFieldTraversal)
{
    checkRegisteredFields<reflected::TestObject>();
}
