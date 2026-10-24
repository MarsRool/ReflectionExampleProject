#pragma once

#include <cstddef>
#include <string_view>
#include <tuple>
#include <vector>

#include "reflection/reflection.h"

namespace cross_tu_test
{

struct TypeSnapshot
{
    std::size_t propertyCount = 0;
    std::vector<std::string_view> propertyNames;
    const void* propertiesAddress = nullptr;
    const void* nestedPropertyAddress = nullptr;
};

struct MetadataSnapshot
{
    TypeSnapshot pure;
    TypeSnapshot reflected;
};

MetadataSnapshot captureFromFirstTu();
MetadataSnapshot captureFromSecondTu();

namespace detail
{

inline constexpr char nestedKey[] = "nested";

// Internal linkage ensures a separate instantiation in each translation unit.
template <typename Outer>
static TypeSnapshot captureType()
{
    using Meta = reflection::StaticPropertyMeta<Outer>;

    static_assert(Meta::template contains<nestedKey>());
    constexpr auto nestedProperty = Meta::template at<nestedKey>();
    static_assert(nestedProperty != nullptr);

    TypeSnapshot snapshot;
    snapshot.propertyCount = Meta::size();
    snapshot.propertiesAddress = &Meta::properties;
    snapshot.nestedPropertyAddress = nestedProperty;
    snapshot.propertyNames.reserve(snapshot.propertyCount);

    std::apply([&](const auto&... property)
    {
        (snapshot.propertyNames.emplace_back(property.name), ...);
    }, Meta::properties);

    return snapshot;
}

} // namespace detail

} // namespace cross_tu_test
