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

    StatusCode fromJson(Outer& outer, const QJsonObject& parentJsonObject) const
    {
        return fromJson(BaseClass::getName(), outer, parentJsonObject);
    }

    static StatusCode fromJson(std::string_view propertyName, Outer& outer, const QJsonObject& parentJsonObject);
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

template <typename Outer>
StatusCode StaticPropertyMap<Outer>::fromJson(std::string_view propertyName, Outer& outer, const QJsonObject& parentJsonObject)
{
    StatusCode statusCode = StatusCode::Good;
    const auto jsonValue = parentJsonObject[propertyName.data()];
    CHECK_R2(jsonValue.isObject(), StatusCode::Bad)
    const auto jsonObject = jsonValue.toObject();

    const auto stringComparator = [](std::string_view s1, std::string_view s2)
    {
        return s1.compare(s2) == 0;
    };

    for (auto iter = jsonObject.constBegin(); iter != jsonObject.constEnd(); iter++)
    {
        const auto name = iter.key().toStdString();
        uniqueStaticHeterogeneousMapDoForKey<Outer, KeyType>([]{},
            [&outer, &statusCode, &jsonObject](auto, auto constValue)
        {
            constexpr auto staticPropertyPtr = decltype(constValue)::value;
            if constexpr (staticPropertyPtr == nullptr)
            {
                statusCode = StatusCode::NotFound;
                return;
            }
            CHECK_SC_D(staticPropertyPtr->fromJson(outer, jsonObject), statusCode = sc;)
        }, name.c_str(), stringComparator);
    }

    return statusCode;
}

} // namespace reflection
