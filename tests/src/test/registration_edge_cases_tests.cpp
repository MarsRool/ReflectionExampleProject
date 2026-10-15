#include <gtest/gtest.h>

#include <cstddef>
#include <string_view>

#include "reflection/reflection.h"

namespace registration_edge_cases_tests
{

// Empty types may still have an intrusive reflection map.
struct EmptyIntrusive
{
    DECL_REFLECTION_BODY(EmptyIntrusive)
};

struct NotReflected
{
    int field = 1;
};

// A derived type can register an empty reflected base without own data fields.
struct EmptyDerived : EmptyIntrusive
{
    DECL_REFLECTION_BODY(EmptyDerived)
    DECL_BASE_CLASS(EmptyIntrusive)
};

// Non-intrusive registration may intentionally expose only a subset.
struct PartiallyReflected
{
    int registeredField = 7;
    int ignoredField = 9;
};

DECL_REFLECTION_NONINTRUSIVE(PartiallyReflected,
    &PartiallyReflected::registeredField)

inline constexpr char missingKey[] = "notRegistered";
inline constexpr char registeredKey[] = "registeredField";
inline constexpr char ignoredKey[] = "ignoredField";
inline constexpr char baseKey[] = "_base_EmptyIntrusive";

using EmptyMeta = reflection::StaticPropertyMeta<EmptyIntrusive>;
using DerivedMeta = reflection::StaticPropertyMeta<EmptyDerived>;
using PartialMeta = reflection::StaticPropertyMeta<PartiallyReflected>;

static_assert(reflection::IsObject<EmptyIntrusive>::value);
static_assert(!reflection::IsObject<NotReflected>::value);
static_assert(EmptyMeta::getStaticPropertyMap() != nullptr);
static_assert(EmptyMeta::empty());
static_assert(EmptyMeta::size() == 0);
static_assert(!EmptyMeta::contains<missingKey>());
static_assert(EmptyMeta::at<missingKey>() == nullptr);

static_assert(reflection::IsObject<EmptyDerived>::value);
static_assert(DerivedMeta::size() == 1);
static_assert(DerivedMeta::contains<baseKey>());
static_assert(DerivedMeta::at<baseKey>() != nullptr);

static_assert(reflection::IsObject<PartiallyReflected>::value);
static_assert(PartialMeta::size() == 1);
static_assert(PartialMeta::contains<registeredKey>());
static_assert(!PartialMeta::contains<ignoredKey>());
static_assert(PartialMeta::at<ignoredKey>() == nullptr);

TEST(ReflectionRegistrationCornerCases, EmptyIntrusiveTypeHasMapAndNoProperties)
{
    EXPECT_EQ(EmptyMeta::getStaticPropertyMap()->name, std::string_view("EmptyIntrusive"));
    std::size_t visited = 0;
    EmptyMeta::forEach([&](auto, auto) { ++visited; });
    EXPECT_EQ(visited, std::size_t{ 0 });

    EmptyMeta::doForKey([&](auto, auto) { ++visited; }, "missing");
    EXPECT_EQ(visited, std::size_t{ 0 });
}

TEST(ReflectionRegistrationCornerCases, UnregisteredTypeHasNoMap)
{
    using Meta = reflection::StaticPropertyMeta<NotReflected>;
    EXPECT_EQ(Meta::getStaticPropertyMap(), nullptr);
    EXPECT_EQ(Meta::size(), std::size_t{ 0 });
}

TEST(ReflectionRegistrationCornerCases, DerivedTypeCanContainOnlyBaseProxy)
{
    auto base = DerivedMeta::at<baseKey>();
    ASSERT_NE(base, nullptr);
    EXPECT_EQ(base->name, std::string_view("_base_EmptyIntrusive"));
    EXPECT_EQ(base->staticProperty.name, std::string_view("EmptyIntrusive"));
}

TEST(ReflectionRegistrationCornerCases, NonIntrusiveRegistrationCanBePartial)
{
    auto registered = PartialMeta::at<registeredKey>();
    ASSERT_NE(registered, nullptr);
    PartiallyReflected obj{};
    EXPECT_EQ(registered->get(obj), 7);
    registered->set(obj, 12);
    EXPECT_EQ(obj.registeredField, 12);
    EXPECT_EQ(obj.ignoredField, 9);
}

} // namespace registration_edge_cases_tests
