#pragma once
#include "Reflection/Utils/canonicalstaticstring.h"
#include "Reflection/Utils/uniquestaticheterogeneousmap.h"
#include "Reflection/Property/basestaticproperty.h"

namespace reflection
{

// TODO: maybe remove StaticPropertyMap at all :D

template <typename Outer>
class StaticPropertyMap : public BaseStaticProperty<Outer>
{
    using DefaultTypeChecker = impl::UniqueStaticHeterogeneousMapElementTrueChecker;
public:
    using BaseClass = BaseStaticProperty<Outer>;
    using ThisClass = StaticPropertyMap<Outer>;
    using OuterClass = Outer;
    using KeyType = const char[];

    constexpr StaticPropertyMap(std::string_view name) noexcept
        : BaseClass(name)
    {}

    constexpr auto size() const noexcept
    {
        return uniqueStaticHeterogeneousMapKeysCount<ThisClass, KeyType>([]{});
    }
    constexpr bool empty() const noexcept
    {
        return size() == 0;
    }
    template <KeyType propertyName>
    constexpr bool contains() const
    {
        constexpr auto canonicalPropertyName = CanonicalStaticString<propertyName>::value;
        return uniqueStaticHeterogeneousMapExists<ThisClass, KeyType, canonicalPropertyName>([]{});
    }
    template <KeyType propertyName>
    constexpr auto at() const
    {
        constexpr auto canonicalPropertyName = CanonicalStaticString<propertyName>::value;
        constexpr auto staticPropertyPtr = uniqueStaticHeterogeneousMapGetValue<ThisClass, KeyType, canonicalPropertyName>([]{});
        return staticPropertyPtr;
    }

    template <typename PropertyPtrT, KeyType propertyName, PropertyPtrT propertyPtr>
    constexpr auto add() const
    {
        constexpr auto canonicalPropertyName = CanonicalStaticString<propertyName>::value;
        return uniqueStaticHeterogeneousMapAdd<
            ThisClass, KeyType, PropertyPtrT, canonicalPropertyName, propertyPtr>([]{});
    }

    template <typename TypeChecker = DefaultTypeChecker,
        typename F>
    void forEach(F&& func) const
    {
        uniqueStaticHeterogeneousMapForEach<ThisClass, KeyType, TypeChecker>(
            []{}, std::forward<F>(func));
    }

    template <typename TypeChecker = DefaultTypeChecker,
        typename F, typename P>
    void forEachIf(F&& func, P&& pred) const
    {
        uniqueStaticHeterogeneousMapForEachIf<ThisClass, KeyType, TypeChecker>(
            []{}, std::forward<F>(func), std::forward<P>(pred));
    }

    template <typename Comparator = std::equal_to<void>,
        typename TypeChecker = DefaultTypeChecker,
        typename F>
    void doForKey(F&& func, KeyType key, Comparator comparator = Comparator()) const
    {
        uniqueStaticHeterogeneousMapDoForKey<ThisClass, KeyType, Comparator, TypeChecker>(
            []{}, std::forward<F>(func), key, comparator);
    }

    template <KeyType key, typename F>
    void doForKey(F&& func) const
    {
        uniqueStaticHeterogeneousMapDoForKey<ThisClass, KeyType, key>(
            []{}, std::forward<F>(func));
    }
};

} // namespace reflection
