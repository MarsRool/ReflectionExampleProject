#pragma once
#include "Reflection/Utils/canonicalstaticstring.h"
#include "Reflection/Utils/uniquestaticheterogeneousmap.h"
#include "Reflection/Property/staticproperty.h"
#include "Reflection/Property/staticpropertymap.h"
#include "Reflection/Property/staticpropertyproxy.h"

namespace reflection
{

namespace impl
{

template <typename Outer, typename T,
    const char rawName[], T Outer::* valuePtr>
struct StaticPropertyHolder
{
    using StaticPropertyType = reflection::StaticProperty<Outer, T>;
    static constexpr StaticPropertyType staticProperty
    {
        rawName,
        valuePtr
    };
};

template <typename Outer, const char rawName[]>
struct StaticPropertyMapHolder
{
    using StaticPropertyType = reflection::StaticPropertyMap<Outer>;
    static constexpr StaticPropertyType staticProperty
    {
        rawName
    };
};

template <typename Outer, typename StaticPropertyT,
    const char rawName[], const StaticPropertyT& targetStaticProperty>
struct StaticPropertyProxyHolder
{
    using StaticPropertyType = reflection::StaticPropertyProxy<Outer, StaticPropertyT>;
    static constexpr StaticPropertyType staticProperty
    {
        rawName,
        targetStaticProperty
    };
};

template <const char rawName[], auto valuePtr>
using DefineStaticProperty = StaticPropertyHolder<
    typename MemberPointerTraits<decltype(valuePtr)>::Outer,
    typename MemberPointerTraits<decltype(valuePtr)>::T,
    rawName,
    valuePtr>;

template <typename Outer, const char rawName[]>
using DefineStaticPropertyMap = StaticPropertyMapHolder<
    Outer, rawName>;

template <typename Outer, const char rawName[], const auto& targetStaticProperty>
using DefineStaticPropertyProxy = StaticPropertyProxyHolder<
    Outer,
    std::remove_cv_t<std::remove_reference_t<decltype(targetStaticProperty)>>,
    rawName,
    targetStaticProperty>;

} // namespace impl

template <typename Outer>
struct StaticPropertyMeta
{
    using ThisClass = StaticPropertyMeta<Outer>;
    using OuterClass = Outer;
    using KeyType = const char[];
    using DefaultTypeChecker = impl::UniqueStaticHeterogeneousMapElementTrueChecker;

    static constexpr KeyType staticStorageKey{};

    static constexpr auto size() noexcept
    {
        return uniqueStaticHeterogeneousMapKeysCount<ThisClass, KeyType>([]{});
    }

    static constexpr bool empty() noexcept
    {
        return size() == 0;
    }

    template <KeyType propertyName>
    static constexpr auto contains()
    {
        constexpr auto canonicalPropertyName = CanonicalStaticString<propertyName>::value;
        return uniqueStaticHeterogeneousMapExists<ThisClass, KeyType, canonicalPropertyName>([]{});
    }

    static constexpr auto getStaticPropertyMap()
    {
        return uniqueStaticHeterogeneousMapGetValue<StaticPropertyMap<Outer>, KeyType, staticStorageKey>([]{});
    }

    template <KeyType propertyName>
    static constexpr auto at()
    {
        constexpr auto canonicalPropertyName = CanonicalStaticString<propertyName>::value;
        return uniqueStaticHeterogeneousMapGetValue<ThisClass, KeyType, canonicalPropertyName>([]{});
    }

    template <KeyType propertyName>
    static constexpr auto defineStaticPropertyMap()
    {
        using Holder = impl::DefineStaticPropertyMap<Outer, propertyName>;
        using StaticPropertyType = typename Holder::StaticPropertyType;
        constexpr auto staticPropertyPtr = &Holder::staticProperty;
        return uniqueStaticHeterogeneousMapAdd<
            StaticPropertyMap<Outer>, KeyType, const StaticPropertyType*, staticStorageKey, staticPropertyPtr>([]{});
    }

    template <KeyType propertyName, auto valuePtr>
    static constexpr auto defineStaticProperty()
    {
        using Holder = impl::DefineStaticProperty<propertyName, valuePtr>;
        using StaticPropertyType = typename Holder::StaticPropertyType;
        constexpr auto canonicalPropertyName = CanonicalStaticString<propertyName>::value;
        constexpr auto staticPropertyPtr = &Holder::staticProperty;
        return uniqueStaticHeterogeneousMapAdd<
            ThisClass, KeyType, const StaticPropertyType*, canonicalPropertyName, staticPropertyPtr>([]{});
    }

    template <KeyType propertyName, const auto& targetStaticProperty>
    static constexpr auto defineStaticPropertyProxy()
    {
        using Holder = impl::DefineStaticPropertyProxy<Outer, propertyName, targetStaticProperty>;
        using StaticPropertyType = typename Holder::StaticPropertyType;
        constexpr auto canonicalPropertyName = CanonicalStaticString<propertyName>::value;
        constexpr auto staticPropertyPtr = &Holder::staticProperty;
        return uniqueStaticHeterogeneousMapAdd<
            ThisClass, KeyType, const StaticPropertyType*, canonicalPropertyName, staticPropertyPtr>([]{});
    }

    template <typename TypeChecker = DefaultTypeChecker,
             typename F>
    static void forEach(F&& func)
    {
        uniqueStaticHeterogeneousMapForEach<ThisClass, KeyType, TypeChecker>(
            []{}, std::forward<F>(func));
    }

    template <typename TypeChecker = DefaultTypeChecker,
             typename F, typename P>
    static void forEachIf(F&& func, P&& pred)
    {
        uniqueStaticHeterogeneousMapForEachIf<ThisClass, KeyType, TypeChecker>(
            []{}, std::forward<F>(func), std::forward<P>(pred));
    }

    template <typename Comparator = std::equal_to<void>,
             typename TypeChecker = DefaultTypeChecker,
             typename F>
    static void doForKey(F&& func, KeyType key, Comparator comparator = Comparator())
    {
        uniqueStaticHeterogeneousMapDoForKey<ThisClass, KeyType, Comparator, TypeChecker>(
            []{}, std::forward<F>(func), key, comparator);
    }

    template <KeyType key, typename F>
    static void doForKey(F&& func)
    {
        uniqueStaticHeterogeneousMapDoForKey<ThisClass, KeyType, key>(
            []{}, std::forward<F>(func));
    }
};

} // namespace reflection
