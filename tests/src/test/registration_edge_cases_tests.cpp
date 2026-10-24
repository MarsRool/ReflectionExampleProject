#include <gtest/gtest.h>

#include <cstddef>
#include <string_view>

#include "reflection/reflection.h"

namespace registration_edge_cases_tests
{

struct EmptyIntrusive
{
    REFLECTION_BEGIN(EmptyIntrusive)
    REFLECTION_END()
};

struct EmptyNonIntrusive {};
REFLECTION_NONINTRUSIVE(EmptyNonIntrusive)

struct NotReflected
{
    int field = 1;
};

struct EmptyDerived : EmptyIntrusive
{
    REFLECTION_BEGIN(EmptyDerived)
    REFLECTION_BASE_CLASS(EmptyIntrusive)
    REFLECTION_END()
};

struct PartiallyReflected
{
    int registeredField = 7;
    int ignoredField = 9;
};
REFLECTION_NONINTRUSIVE(PartiallyReflected, &PartiallyReflected::registeredField)

constexpr char missingKey[] = "notRegistered";
constexpr char registeredKey[] = "registeredField";
constexpr char ignoredKey[] = "ignoredField";
constexpr char baseKey[] = "_base_EmptyIntrusive";

using EmptyMeta = reflection::StaticPropertyMeta<EmptyIntrusive>;
using DerivedMeta = reflection::StaticPropertyMeta<EmptyDerived>;
using PartialMeta = reflection::StaticPropertyMeta<PartiallyReflected>;

static_assert(reflection::impl::IsReflected<EmptyIntrusive>::value);
static_assert(reflection::impl::IsReflected<EmptyNonIntrusive>::value);
static_assert(!reflection::impl::IsReflected<NotReflected>::value);
static_assert(EmptyMeta::getStaticTypeInfo().name == "EmptyIntrusive");
static_assert(EmptyMeta::empty());
static_assert(EmptyMeta::size() == 0);
static_assert(!EmptyMeta::contains<missingKey>());
static_assert(EmptyMeta::at<missingKey>() == nullptr);

static_assert(DerivedMeta::size() == 1);
static_assert(DerivedMeta::contains<baseKey>());

static_assert(PartialMeta::size() == 1);
static_assert(PartialMeta::contains<registeredKey>());
static_assert(!PartialMeta::contains<ignoredKey>());
static_assert(PartialMeta::at<ignoredKey>() == nullptr);

TEST(ReflectionRegistrationCornerCases, EmptyIntrusiveTypeHasNoProperties)
{
    EXPECT_EQ(EmptyMeta::getStaticTypeInfo().name, "EmptyIntrusive");
    std::size_t visited = 0;
    EmptyMeta::forEach([&](auto) { ++visited; });
    EXPECT_EQ(visited, 0u);
    EmptyMeta::doForKey([&](auto) { ++visited; }, "missing");
    EXPECT_EQ(visited, 0u);
}

TEST(ReflectionRegistrationCornerCases, EmptyNonintrusiveRegistration)
{
    EXPECT_TRUE(reflection::StaticPropertyMeta<EmptyNonIntrusive>::empty());
}

TEST(ReflectionRegistrationCornerCases, UnregisteredTypeIsNotReflected)
{
    EXPECT_FALSE(reflection::impl::IsReflected<NotReflected>::value);
}

TEST(ReflectionRegistrationCornerCases, DerivedTypeCanContainOnlyBaseEntry)
{
    constexpr auto base = DerivedMeta::at<baseKey>();
    ASSERT_NE(base, nullptr);
    EXPECT_EQ(base->name, std::string_view("_base_EmptyIntrusive"));
    EXPECT_EQ(reflection::StaticPropertyMeta<EmptyIntrusive>::size(), 0u);
}

TEST(ReflectionRegistrationCornerCases, NonintrusiveRegistrationCanBePartial)
{
    constexpr auto registered = PartialMeta::at<registeredKey>();
    ASSERT_NE(registered, nullptr);
    PartiallyReflected value{};
    EXPECT_EQ(registered->get(value), 7);
    registered->set(value, 12);
    EXPECT_EQ(value.registeredField, 12);
    EXPECT_EQ(value.ignoredField, 9);
}

} // namespace registration_edge_cases_tests
