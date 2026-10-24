#pragma once

#include <cstddef>
#include <type_traits>
#include <tuple>
#include <utility>

#include "reflection/utils/canonicalpropertynameparser.h"
#include "reflection/utils/typetraits.h"
#include "reflection/property/staticproperty.h"
#include "reflection/property/statictypeinfo.h"

namespace reflection
{

template <
    typename Outer,
    const auto* propertyNames,
    bool includesSentinelName,
    auto... valuePtrs>
struct NonIntrusivePropertyParser
{
    using OuterType = Outer;
    using Parser = CanonicalPropertyNameParser<propertyNames>;

    // The last value is an internal sentinel for optional macro arguments.
    static_assert(sizeof...(valuePtrs) > 0);
    static constexpr std::size_t propertyCount = sizeof...(valuePtrs) - 1;
    static_assert(Parser::namesCount == propertyCount + (includesSentinelName ? 1 : 0),
        "property names count doesn't match value pointers count");

    template <std::size_t index>
    using ValueAt = std::tuple_element_t<index,
        std::tuple<std::integral_constant<decltype(valuePtrs), valuePtrs>...>>;

    template <std::size_t... indices>
    static constexpr auto makeStaticProperties(std::index_sequence<indices...>)
    {
        return std::make_tuple(
            makeStaticProperty<indices, ValueAt<indices>::value>()...);
    }

    template <std::size_t propertyIndex, auto valuePtr>
    static constexpr auto makeStaticProperty()
    {
        static_assert(
            propertyIndex < Parser::namesCount,
            "makeStaticProperties: propertyIndex is out of range");

        using PropertyName =
            typename Parser::template CanonicalName<propertyIndex>;
        using ValueType = typename impl::MemberPointerTraits<
            std::remove_cv_t<decltype(valuePtr)>>::T;

        return StaticProperty<OuterType, ValueType>
        {
            PropertyName::value, valuePtr
        };
    }

    static constexpr auto staticProperties = makeStaticProperties(
        std::make_index_sequence<propertyCount>());
};

template <
    typename Outer,
    const auto* propertyNamesPrefix,
    const auto* propertyNames,
    bool includesSentinelName,
    typename... BaseClasses>
struct NonIntrusiveBaseClassParser
{
    using OuterType = Outer;
    using Parser = CanonicalPropertyNameParser<propertyNames>;

    // The last type is an internal sentinel for optional macro arguments.
    static_assert(sizeof...(BaseClasses) > 0);
    static constexpr std::size_t baseCount = sizeof...(BaseClasses) - 1;
    static_assert(Parser::namesCount == baseCount + (includesSentinelName ? 1 : 0),
        "base class names count doesn't match base classes count");

    template <std::size_t index>
    using ValueAt = std::tuple_element_t<index, std::tuple<BaseClasses...>>;

    template <std::size_t... indices>
    static constexpr auto makeStaticProperties(std::index_sequence<indices...>)
    {
        return std::make_tuple(
            makeStaticProperty<indices, ValueAt<indices>>()...);
    }

    template <std::size_t propertyIndex, typename BaseClass>
    static constexpr auto makeStaticProperty()
    {
        static_assert(
            propertyIndex < Parser::namesCount,
            "makeStaticProperties: propertyIndex is out of range");

        using BaseClassName =
            CanonicalStaticStringConcat<
                propertyNamesPrefix,
                Parser::template CanonicalName<propertyIndex>::value>;

        return StaticTypeInfo<BaseClass>
        {
            BaseClassName::value
        };
    }

    static constexpr auto baseClassTypeInfos = makeStaticProperties(
        std::make_index_sequence<baseCount>());
};

} // namespace reflection
