#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "reflection/reflection.h"

namespace static_property_api_tests
{

struct PotentiallyThrowing
{
    int value = 0;

    PotentiallyThrowing& operator=(const PotentiallyThrowing& other) noexcept(false)
    {
        value = other.value;
        return *this;
    }
};

struct Record
{
    int number = 0;
    std::string name;
    std::unique_ptr<int> owned;
    PotentiallyThrowing potentiallyThrowing;
    const int immutable = 7;
};

DECL_REFLECTION_NONINTRUSIVE(Record,
    &Record::number,
    &Record::name,
    &Record::owned,
    &Record::potentiallyThrowing,
    &Record::immutable)

inline constexpr char numberKey[] = "number";
inline constexpr char nameKey[] = "name";
inline constexpr char ownedKey[] = "owned";
inline constexpr char throwingKey[] = "potentiallyThrowing";
inline constexpr char immutableKey[] = "immutable";

using Meta = reflection::StaticPropertyMeta<Record>;
using NumberProperty = std::remove_pointer_t<decltype(Meta::at<numberKey>())>;
using ThrowingProperty = std::remove_pointer_t<decltype(Meta::at<throwingKey>())>;
using ImmutableProperty = std::remove_pointer_t<decltype(Meta::at<immutableKey>())>;

template <typename Property, typename Value, typename = void>
struct CanSet : std::false_type {};

template <typename Property, typename Value>
struct CanSet<Property, Value, std::void_t<decltype(
    std::declval<const Property&>().set(std::declval<Record&>(), std::declval<Value>()))>>
    : std::true_type {};

static_assert(reflection::IsObject<Record>::value);
static_assert(reflection::IsObject<const Record&>::value);
static_assert(reflection::IsObject<volatile Record&&>::value);
static_assert(reflection::IsString<const std::string&>::value);
static_assert(!CanSet<ImmutableProperty, int>::value);
static_assert(CanSet<NumberProperty, int>::value);
static_assert(noexcept(std::declval<const NumberProperty&>().set(
    std::declval<Record&>(), 42)));
static_assert(!noexcept(std::declval<const ThrowingProperty&>().set(
    std::declval<Record&>(), std::declval<const PotentiallyThrowing&>())));

// Verify that getters preserve references and respect object/field constness.
static_assert(std::is_same_v<decltype(std::declval<const NumberProperty&>().get(
    std::declval<Record&>())), int&>);
static_assert(std::is_same_v<decltype(std::declval<const NumberProperty&>().get(
    std::declval<const Record&>())), const int&>);
static_assert(std::is_same_v<decltype(std::declval<const ImmutableProperty&>().get(
    std::declval<Record&>())), const int&>);

struct LiteralRecord
{
    int value = 11;
};

constexpr reflection::StaticProperty<LiteralRecord, int> literalProperty
{
    { "value" },
    &LiteralRecord::value
};
constexpr LiteralRecord literalObject{};
static_assert(literalProperty.get(literalObject) == 11);
static_assert(&literalProperty.get(literalObject) == &literalObject.value);

TEST(StaticPropertyApi, SetterSupportsRvaluesLvaluesAndMoveOnlyValues)
{
    Record record;
    auto number = Meta::at<numberKey>();
    auto name = Meta::at<nameKey>();
    auto owned = Meta::at<ownedKey>();

    number->set(record, 42);
    EXPECT_EQ(record.number, 42);

    int other = 77;
    number->set(record, other);
    EXPECT_EQ(record.number, 77);

    name->set(record, std::string("hello"));
    EXPECT_EQ(record.name, "hello");

    auto unique = std::make_unique<int>(101);
    owned->set(record, std::move(unique));
    EXPECT_EQ(unique, nullptr);
    ASSERT_NE(record.owned, nullptr);
    EXPECT_EQ(*record.owned, 101);
}

TEST(StaticPropertyApi, GetReturnsMutableAndConstReferences)
{
    Record record;
    auto number = Meta::at<numberKey>();
    auto name = Meta::at<nameKey>();
    auto immutable = Meta::at<immutableKey>();

    number->get(record) = 6;
    EXPECT_EQ(record.number, 6);
    EXPECT_EQ(number->get(record), 6);

    name->get(record) = "changed";
    EXPECT_EQ(name->get(record), std::string_view("changed"));

    const Record& constant = record;
    EXPECT_EQ(&number->get(constant), &record.number);
    EXPECT_EQ(immutable->get(constant), 7);
}

TEST(StaticPropertyApi, RuntimeDoForKeyComparesStringContents)
{
    std::string dynamicKey = "number";
    int visited = 0;
    Meta::doForKey([&](auto, auto)
    {
        ++visited;
    }, dynamicKey.c_str());
    EXPECT_EQ(visited, 1);

    visited = 0;
    Meta::doForKey([&](auto, auto)
    {
        ++visited;
    }, static_cast<const char*>(nullptr));
    EXPECT_EQ(visited, 0);
}

} // namespace static_property_api_tests
