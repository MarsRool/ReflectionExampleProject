#pragma once
#include "Shared/uniquestaticmap.h"
#include "Reflection/Property/Static/basestaticproperty.h"

namespace reflection
{

template <class Outer>
class StaticPropertyMap : public BaseStaticProperty<Outer>
{
public:
    using BaseClass = BaseStaticProperty<Outer>;
    using ThisClass = StaticPropertyMap<Outer>;
    using OuterClass = Outer;
    using KeyType = const char[];
    using StaticPropertyRef = const BaseStaticProperty<Outer>&;
    using StaticPropertyPtr = const BaseStaticProperty<Outer>*;

    template <class ProxyOuter, class StaticPropertyT>
    friend class StaticPropertyProxy;

    constexpr StaticPropertyMap(std::string_view name) noexcept
        : BaseClass(name)
    {}

    constexpr bool empty() const noexcept
    {
        return uniqueStaticMapKeysCount<Outer, KeyType, StaticPropertyPtr>() == 0;
    }
    template <KeyType propertyName>
    constexpr bool contains() const
    {
        return uniqueStaticMapExists<Outer, KeyType, StaticPropertyPtr, propertyName>();
    }
    template <KeyType propertyName>
    constexpr StaticPropertyPtr at() const
    {
        return uniqueStaticMapGetValue<Outer, KeyType, StaticPropertyPtr, propertyName>();
    }

    template <KeyType propertyName, StaticPropertyPtr staticPropertyPtr>
    constexpr StatusCode add() const
    {
        constexpr auto value = uniqueStaticMapAdd<Outer, KeyType, StaticPropertyPtr, propertyName, staticPropertyPtr>();
        (void)value;
        return StatusCode::Good;
    }

    bool equals(const Outer& outer, const BaseClass& other, const Outer& otherOuter) const noexcept override;

    std::string toString(const Outer& outer) const override
    {
        return toString(BaseClass::getName(), outer);
    }
    StatusCode toJson(const Outer& outer, QJsonObject& parentJsonObject) const override
    {
        return toJson(BaseClass::getName(), outer, parentJsonObject);
    }
    StatusCode fromJson(Outer& outer, const QJsonObject& parentJsonObject) const override
    {
        return fromJson(BaseClass::getName(), outer, parentJsonObject);
    }

    static std::string toString(std::string_view propertyName, const Outer& outer);
    static StatusCode toJson(std::string_view propertyName, const Outer& outer, QJsonObject& parentJsonObject);
    static StatusCode fromJson(std::string_view propertyName, Outer& outer, const QJsonObject& parentJsonObject);
};

template <class Outer>
bool StaticPropertyMap<Outer>::equals(const Outer& outer, const BaseClass& other, const Outer& otherOuter) const noexcept
{
    const auto* otherStaticPropertyMap = dynamic_cast<const ThisClass*>(&other);
    if (!otherStaticPropertyMap)
    {
        return false;
    }

    bool equals = true;
    uniqueStaticMapForEach<Outer, KeyType, StaticPropertyPtr>(
        [&outer, &otherOuter, &equals](const char*, StaticPropertyPtr staticPropertyPtr)
    {
        CHECK_POINTER_D(staticPropertyPtr, equals = false; return;);
        equals = equals && staticPropertyPtr->equals(outer, *staticPropertyPtr, otherOuter);
    });

    return equals;
}

template <class Outer>
std::string StaticPropertyMap<Outer>::toString(std::string_view propertyName, const Outer& outer)
{
    std::string result{ '\"' + std::string(propertyName) + "\":\n{ " };
    std::size_t i = 0;

    uniqueStaticMapForEach<Outer, KeyType, StaticPropertyPtr>(
        [&outer, &result, &i](const char*, StaticPropertyPtr staticPropertyPtr)
    {
        CHECK_POINTER_R0(staticPropertyPtr);
        result += staticPropertyPtr->toString(outer);
        if (i != uniqueStaticMapKeysCount<Outer, KeyType, StaticPropertyPtr>())
            result += ",\n";
        i++;
    });

    result += " }";
    return result;
}

template <class Outer>
StatusCode StaticPropertyMap<Outer>::toJson(std::string_view propertyName, const Outer& outer, QJsonObject& parentJsonObject)
{
    StatusCode statusCode = StatusCode::Good;
    QJsonObject jsonObject;

    uniqueStaticMapForEach<Outer, KeyType, StaticPropertyPtr>(
        [&outer, &statusCode, &jsonObject](const char*, StaticPropertyPtr staticPropertyPtr)
    {
        CHECK_POINTER_D(staticPropertyPtr, statusCode = StatusCode::BadPointer; return;);
        CHECK_SC_D(staticPropertyPtr->toJson(outer, jsonObject), statusCode = sc;)
    });

    parentJsonObject[propertyName.data()] = jsonObject;
    return statusCode;
}

template <class Outer>
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
        const auto staticPropertyPtr = uniqueStaticMapGetValue<Outer, KeyType, StaticPropertyPtr>(
            name.c_str(), stringComparator);
        if (!staticPropertyPtr)
        {
            statusCode = StatusCode::NotFound;
            continue;
        }
        CHECK_SC_D(staticPropertyPtr->fromJson(outer, jsonObject), statusCode = sc;)
    }

    return statusCode;
}

} // namespace reflection
