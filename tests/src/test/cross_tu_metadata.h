#pragma once

#include <cstddef>
#include <string_view>

#include "reflection/reflection.h"

namespace cross_tu_test
{

struct TypeMetadata
{
    std::size_t size;
    const void* propertyMap;
    const void* primaryProperty;
    const void* inheritedProperty;
    std::string_view typeName;
    std::string_view primaryName;
    std::string_view inheritedName;
    bool hasPrimary;
    bool hasInherited;
    bool hasMissing;
    bool isEmpty;
};

struct MetadataSnapshot
{
    TypeMetadata nonintrusiveObject;
    TypeMetadata intrusiveObject;
    TypeMetadata nonintrusiveTemplate;
    TypeMetadata intrusiveTemplate;
};

MetadataSnapshot captureFromFirstTu();
MetadataSnapshot captureFromSecondTu();

namespace detail
{

inline constexpr char ageKey[] = "age";
inline constexpr char valueKey[] = "value";
inline constexpr char baseKey[] = "_base_BaseTestObject";
inline constexpr char missingKey[] = "property_that_does_not_exist";

// This function is instantiated and constant-evaluated separately in each TU.
template <typename T, const char primaryKey[], std::size_t expectedSize>
constexpr TypeMetadata captureType()
{
    using Meta = reflection::StaticPropertyMeta<T>;

    static_assert(Meta::size() == expectedSize);
    static_assert(!Meta::empty());
    static_assert(Meta::template contains<primaryKey>());
    static_assert(Meta::template contains<baseKey>());
    static_assert(!Meta::template contains<missingKey>());
    static_assert(Meta::template at<missingKey>() == nullptr);

    constexpr auto map = Meta::getStaticPropertyMap();
    constexpr auto primary = Meta::template at<primaryKey>();
    constexpr auto inherited = Meta::template at<baseKey>();
    static_assert(map != nullptr);
    static_assert(primary != nullptr);
    static_assert(inherited != nullptr);

    return
    {
        Meta::size(),
        map,
        primary,
        inherited,
        map->name,
        primary->name,
        inherited->name,
        Meta::template contains<primaryKey>(),
        Meta::template contains<baseKey>(),
        Meta::template contains<missingKey>(),
        Meta::empty()
    };
}

} // namespace detail

} // namespace cross_tu_test
