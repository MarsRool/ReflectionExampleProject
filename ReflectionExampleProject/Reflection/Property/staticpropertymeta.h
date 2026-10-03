#pragma once

#include "Reflection/Property/staticproperty.h"
#include "Reflection/Property/staticpropertymap.h"

namespace reflection
{

template <typename Outer, typename T, T Outer::* valuePtr, const char rawName[]>
struct StaticPropertyHolder
{
    using StaticPropertyType = reflection::StaticProperty<Outer, T>;
    static constexpr StaticPropertyType staticProperty
    {
        rawName,
        valuePtr
    };
};

template <typename Outer>
struct StaticPropertyMeta
{
    using ThisClass = StaticPropertyMeta<Outer>;
    template <typename T>
    using KeyType = T Outer::*;

    static constexpr auto size() noexcept
    {
        return uniqueStaticHeterogeneousMapKeysCount<ThisClass, KeyType>([]{});
    }

    static constexpr bool empty() noexcept
    {
        return size() == 0;
    }

    template <typename T, T Outer::* valuePtr>
    static constexpr auto exists()
    {
        return uniqueStaticHeterogeneousMapExists<ThisClass, KeyType, valuePtr>([]{});
    }

    template <auto valuePtr, typename = std::void_t<extensions::impl::MemberPointerTraits<decltype(valuePtr)>>>
    static constexpr auto get()
    {
        using Traits = extensions::impl::MemberPointerTraits<decltype(valuePtr)>;
        using ValueType = typename Traits::T;
        constexpr auto staticPropertyPtr = uniqueStaticHeterogeneousMapGetValue<
            ThisClass, KeyType<ValueType>, valuePtr>([]{});
        return staticPropertyPtr;
    }

    template <typename T, T Outer::* valuePtr, const char rawName[]>
    static constexpr auto define()
    {
        using CurrentMeta = StaticPropertyHolder<Outer, T, valuePtr, rawName>;
        using StaticPropertyType = typename CurrentMeta::StaticPropertyType;
        constexpr auto staticPropertyPtr = &CurrentMeta::staticProperty;
        return uniqueStaticHeterogeneousMapAdd<
            ThisClass, KeyType<T>, const StaticPropertyType*, valuePtr, staticPropertyPtr>([]{});
    }
};

} // namespace reflection
