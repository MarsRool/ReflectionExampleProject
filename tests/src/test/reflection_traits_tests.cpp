#include <gtest/gtest.h>

#include <cstddef>
#include <tuple>
#include <type_traits>

#include "reflection/reflection.h"

namespace reflection_traits_tests
{

struct Base
{
    int inherited = 1;
};

REFLECTION_NONINTRUSIVE(Base, &Base::inherited)

struct NonIntrusive : Base
{
    int field = 2;
    const int immutable = 3;
};

REFLECTION_NONINTRUSIVE(NonIntrusive, &NonIntrusive::field, &NonIntrusive::immutable)
REFLECTION_BASE_CLASSES_NONINTRUSIVE(NonIntrusive, Base)

struct Intrusive : Base
{
    REFLECTION_BEGIN(Intrusive)
    REFLECTION_BASE_CLASS(Base)
    REFLECTION_PROPERTY(int, field, 2)
    REFLECTION_PROPERTY(const int, immutable, 3)
    REFLECTION_END()
};

template <typename Meta, std::size_t index>
struct Entry
{
    static constexpr const auto& value = std::get<index>(Meta::properties);
};

template <typename T>
struct CvReferenceEntry
{
    static const volatile T& value;
};

struct UnrelatedEntry
{
    static constexpr int value = 42;
};

using Property = reflection::StaticProperty<NonIntrusive, int>;
using ConstProperty = reflection::StaticProperty<NonIntrusive, const int>;
using BaseInfo = reflection::StaticTypeInfo<Base>;

using NonIntrusiveMeta = reflection::StaticPropertyMeta<NonIntrusive>;
using IntrusiveMeta = reflection::StaticPropertyMeta<Intrusive>;

using NonIntrusiveBaseEntry = Entry<NonIntrusiveMeta, 0>;
using NonIntrusivePropertyEntry = Entry<NonIntrusiveMeta, 1>;
using NonIntrusiveConstEntry = Entry<NonIntrusiveMeta, 2>;
using IntrusiveBaseEntry = Entry<IntrusiveMeta, 0>;
using IntrusivePropertyEntry = Entry<IntrusiveMeta, 1>;
using IntrusiveConstEntry = Entry<IntrusiveMeta, 2>;

// Direct trait classification and preservation of the reflected value type.
static_assert(reflection::impl::StaticPropertyTraits<Property>::value);
static_assert(reflection::impl::StaticPropertyTraits<ConstProperty>::value);
static_assert(!reflection::impl::StaticPropertyTraits<BaseInfo>::value);
static_assert(!reflection::impl::StaticPropertyTraits<int>::value);
static_assert(std::is_same_v<
    reflection::impl::StaticPropertyTraits<Property>::ValueType, int>);
static_assert(std::is_same_v<
    reflection::impl::StaticPropertyTraits<ConstProperty>::ValueType, const int>);

static_assert(reflection::impl::BaseClassTraits<BaseInfo>::value);
static_assert(!reflection::impl::BaseClassTraits<Property>::value);
static_assert(!reflection::impl::BaseClassTraits<int>::value);
static_assert(std::is_same_v<
    reflection::impl::BaseClassTraits<BaseInfo>::OuterType, Base>);

// EntryValueType removes top-level cv/ref, not the field's own const qualifier.
static_assert(std::is_same_v<
    reflection::impl::EntryValueType<CvReferenceEntry<Property>>, Property>);
static_assert(std::is_same_v<
    reflection::impl::EntryValueType<CvReferenceEntry<BaseInfo>>, BaseInfo>);
static_assert(std::is_same_v<
    reflection::impl::EntryValueType<UnrelatedEntry>, int>);
static_assert(std::is_same_v<
    reflection::impl::EntryValueType<NonIntrusiveConstEntry>, ConstProperty>);

// Classify actual metadata entries from both registration styles.
static_assert(NonIntrusiveMeta::size() == 3);
static_assert(IntrusiveMeta::size() == 3);
static_assert(std::is_same_v<
    reflection::impl::EntryValueType<NonIntrusiveBaseEntry>, BaseInfo>);
static_assert(std::is_same_v<
    reflection::impl::EntryValueType<NonIntrusivePropertyEntry>, Property>);
static_assert(std::is_same_v<
    reflection::impl::EntryValueType<IntrusiveBaseEntry>, BaseInfo>);
static_assert(std::is_same_v<
    reflection::impl::EntryValueType<IntrusivePropertyEntry>,
    reflection::StaticProperty<Intrusive, int>>);
static_assert(std::is_same_v<
    reflection::impl::EntryValueType<IntrusiveConstEntry>,
    reflection::StaticProperty<Intrusive, const int>>);

static_assert(reflection::impl::IsBaseClassEntry<NonIntrusiveBaseEntry>::value);
static_assert(reflection::impl::IsBaseClassEntry<IntrusiveBaseEntry>::value);
static_assert(!reflection::impl::IsBaseClassEntry<NonIntrusivePropertyEntry>::value);
static_assert(!reflection::impl::IsBaseClassEntry<IntrusiveConstEntry>::value);
static_assert(!reflection::impl::IsBaseClassEntry<UnrelatedEntry>::value);
static_assert(std::is_same_v<
    reflection::impl::IsBaseClassEntry<NonIntrusiveBaseEntry>::OuterType, Base>);

static_assert(reflection::impl::IsStaticPropertyEntry<NonIntrusivePropertyEntry>::value);
static_assert(reflection::impl::IsStaticPropertyEntry<NonIntrusiveConstEntry>::value);
static_assert(reflection::impl::IsStaticPropertyEntry<IntrusivePropertyEntry>::value);
static_assert(reflection::impl::IsStaticPropertyEntry<IntrusiveConstEntry>::value);
static_assert(!reflection::impl::IsStaticPropertyEntry<NonIntrusiveBaseEntry>::value);
static_assert(!reflection::impl::IsStaticPropertyEntry<IntrusiveBaseEntry>::value);
static_assert(!reflection::impl::IsStaticPropertyEntry<UnrelatedEntry>::value);
static_assert(std::is_same_v<
    reflection::impl::IsStaticPropertyEntry<NonIntrusiveConstEntry>::ValueType, const int>);

template <typename Object>
void checkRealEntryTags()
{
    std::size_t baseCount = 0;
    std::size_t propertyCount = 0;

    reflection::StaticPropertyMeta<Object>::forEach([&](auto entry)
    {
        using EntryType = decltype(entry);
        constexpr bool isBase = reflection::impl::IsBaseClassEntry<EntryType>::value;
        constexpr bool isProperty = reflection::impl::IsStaticPropertyEntry<EntryType>::value;
        static_assert(isBase != isProperty);

        baseCount += isBase ? 1 : 0;
        propertyCount += isProperty ? 1 : 0;
    });

    EXPECT_EQ(baseCount, 1u);
    EXPECT_EQ(propertyCount, 2u);
}

TEST(ReflectionTraits, ClassifyActualEntryTags)
{
    checkRealEntryTags<NonIntrusive>();
    checkRealEntryTags<Intrusive>();
}

} // namespace reflection_traits_tests
