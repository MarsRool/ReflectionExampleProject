#pragma once
#include "Shared/canonicalstaticstring.h"
#include "Shared/uniquestaticheterogeneousmap.h"
#include "Reflection/Property/Static/basestaticproperty.h"

namespace reflection
{

template <typename Outer>
class StaticPropertyMap : public BaseStaticProperty<Outer>
{
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
        return uniqueStaticHeterogeneousMapKeysCount<Outer, KeyType>([]{});
    }
    constexpr bool empty() const noexcept
    {
        return size() == 0;
    }
    template <KeyType propertyName>
    constexpr bool contains() const
    {
        constexpr auto canonicalPropertyName = CanonicalStaticStringT<propertyName>::value;
        return uniqueStaticHeterogeneousMapExists<Outer, KeyType, canonicalPropertyName>([]{});
    }
    template <KeyType propertyName>
    constexpr auto at() const
    {
        constexpr auto canonicalPropertyName = CanonicalStaticStringT<propertyName>::value;
        constexpr auto staticPropertyPtr = uniqueStaticHeterogeneousMapGetValue<Outer, KeyType, canonicalPropertyName>([]{});
        return staticPropertyPtr;
    }

    template <typename PropertyPtrT, KeyType propertyName, PropertyPtrT propertyPtr>
    constexpr StatusCode add() const
    {
        constexpr auto canonicalPropertyName = CanonicalStaticStringT<propertyName>::value;
        constexpr auto value = uniqueStaticHeterogeneousMapAdd<Outer, KeyType, PropertyPtrT, canonicalPropertyName, propertyPtr>([]{});
        (void)value;
        return StatusCode::Good;
    }

    bool equals(const Outer& outer, const Outer& otherOuter) const noexcept;
};

template <typename Outer>
bool StaticPropertyMap<Outer>::equals(const Outer& outer, const Outer& otherOuter) const noexcept
{
    bool equals = true;
    uniqueStaticHeterogeneousMapForEach<Outer, KeyType>([]{},
        [&outer, &otherOuter, &equals](auto, auto constValue)
    {
        constexpr auto staticPropertyPtr = decltype(constValue)::value;
        if constexpr (staticPropertyPtr == nullptr)
        {
            return;
        }
        equals = equals && staticPropertyPtr->equals(outer, otherOuter);
    });

    return equals;
}

} // namespace reflection
