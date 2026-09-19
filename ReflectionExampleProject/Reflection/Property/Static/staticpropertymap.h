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
    using StaticPropertyPtr = const BaseStaticProperty<Outer>* const;
    using StaticPropertyDoublePtr = StaticPropertyPtr*;

    template <class ProxyOuter, class StaticPropertyT>
    friend class StaticPropertyProxy;

    constexpr StaticPropertyMap(std::string_view name) noexcept
        : BaseClass(name)
    {}

    constexpr bool empty() const noexcept
    {
        return uniqueStaticMapKeysCount<Outer, KeyType, StaticPropertyDoublePtr>([]{}) == 0;
    }
    template <KeyType propertyName>
    constexpr bool contains() const
    {
        return uniqueStaticMapExists<Outer, KeyType, StaticPropertyDoublePtr, propertyName>([]{});
    }
    template <KeyType propertyName>
    constexpr StaticPropertyPtr at() const
    {
        constexpr StaticPropertyDoublePtr staticPropertyDoublePtr = uniqueStaticMapGetValue<Outer, KeyType, StaticPropertyDoublePtr, propertyName>([]{});
        if constexpr (staticPropertyDoublePtr == nullptr)
        {
            return nullptr;
        }
        else
        {
            return *staticPropertyDoublePtr;
        }
    }

    template <KeyType propertyName, StaticPropertyDoublePtr staticPropertyDoublePtr>
    constexpr StatusCode add() const
    {
        constexpr auto value = uniqueStaticMapAdd<Outer, KeyType, StaticPropertyDoublePtr, propertyName, staticPropertyDoublePtr>([]{});
        (void)value;
        return StatusCode::Good;
    }

    bool equals(const Outer& outer, const Outer& otherOuter) const noexcept override;

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
bool StaticPropertyMap<Outer>::equals(const Outer& outer, const Outer& otherOuter) const noexcept
{
    bool equals = true;
    uniqueStaticMapForEach<Outer, KeyType, StaticPropertyDoublePtr>([]{},
        [&outer, &otherOuter, &equals](const char*, StaticPropertyDoublePtr staticPropertyDoublePtr)
    {
        CHECK_POINTER_D(staticPropertyDoublePtr, equals = false; return;);
        CHECK_POINTER_D(*staticPropertyDoublePtr, equals = false; return;);
        StaticPropertyRef staticPropertyRef = **staticPropertyDoublePtr;
        equals = equals && staticPropertyRef.equals(outer, otherOuter);
    });

    return equals;
}

template <class Outer>
std::string StaticPropertyMap<Outer>::toString(std::string_view propertyName, const Outer& outer)
{
    std::string result{ '\"' + std::string(propertyName) + "\":\n{ " };
    std::size_t i = 0;

    uniqueStaticMapForEach<Outer, KeyType, StaticPropertyDoublePtr>([]{},
        [&outer, &result, &i](const char*, StaticPropertyDoublePtr staticPropertyDoublePtr)
    {
        CHECK_POINTER_R0(staticPropertyDoublePtr);
        CHECK_POINTER_R0(*staticPropertyDoublePtr);
        StaticPropertyRef staticPropertyRef = **staticPropertyDoublePtr;
        result += staticPropertyRef.toString(outer);
        if (i != uniqueStaticMapKeysCount<Outer, KeyType, StaticPropertyDoublePtr>([]{}))
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

    uniqueStaticMapForEach<Outer, KeyType, StaticPropertyDoublePtr>([]{},
        [&outer, &statusCode, &jsonObject](const char*, StaticPropertyDoublePtr staticPropertyDoublePtr)
    {
        CHECK_POINTER_D(staticPropertyDoublePtr, statusCode = StatusCode::BadPointer; return;);
        CHECK_POINTER_D(*staticPropertyDoublePtr, statusCode = StatusCode::BadPointer; return;);
        StaticPropertyRef staticPropertyRef = **staticPropertyDoublePtr;
        CHECK_SC_D(staticPropertyRef.toJson(outer, jsonObject), statusCode = sc;)
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
        const auto staticPropertyDoublePtr = uniqueStaticMapFindValue<Outer, KeyType, StaticPropertyDoublePtr>(
            []{}, name.c_str(), stringComparator);
        if (!(staticPropertyDoublePtr && *staticPropertyDoublePtr))
        {
            statusCode = StatusCode::NotFound;
            continue;
        }
        StaticPropertyRef staticPropertyRef = **staticPropertyDoublePtr;
        CHECK_SC_D(staticPropertyRef.fromJson(outer, jsonObject), statusCode = sc;)
    }

    return statusCode;
}

} // namespace reflection
