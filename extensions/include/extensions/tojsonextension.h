#pragma once

#include <cmath>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>

#include "reflection/reflection.h"
#include "extensions/extensionstypetraits.h"
#include "extensions/detail/qstringutils.h"

namespace extensions
{

template <typename Outer, typename = std::enable_if_t<reflection::IsObject<Outer>::value>>
bool convertToJson(const Outer& value, QJsonObject& parentJsonObject, TypeTag<Outer> = {})
{
    using Meta = reflection::StaticPropertyMeta<Outer>;
    constexpr auto staticPropertyMapPtr = Meta::getStaticPropertyMap();

    return propertyMapToJson(value, parentJsonObject,
        PointerTag<staticPropertyMapPtr>{});
}

template <typename Outer, typename StaticPropertyT, const reflection::StaticPropertyProxy<Outer, StaticPropertyT>* staticPropertyProxyPtr>
bool propertyProxyToJson(const typename reflection::StaticPropertyProxy<Outer, StaticPropertyT>::TargetOuterClass& outer,
    QJsonObject& parentJsonObject,
    PointerTag<staticPropertyProxyPtr>)
{
    static_assert(staticPropertyProxyPtr != nullptr);

    using ProxyClass = reflection::StaticPropertyProxy<Outer, StaticPropertyT>;
    using TargetStaticPropertyClass = typename ProxyClass::TargetStaticPropertyClass;
    using TargetOuterClass = typename reflection::StaticPropertyProxy<Outer, StaticPropertyT>::TargetOuterClass;

    constexpr const auto& targetStaticProperty = staticPropertyProxyPtr->staticProperty;

    if constexpr (reflection::IsSpecialization<TargetStaticPropertyClass, reflection::StaticProperty>::value)
    {
        return propertyToJson(staticPropertyProxyPtr->name, outer, parentJsonObject,
            PointerToMemberTag<targetStaticProperty->valuePtr>{});
    }
    else if constexpr (reflection::IsSpecialization<TargetStaticPropertyClass, reflection::StaticPropertyMap>::value)
    {
        return propertyMapToJson(staticPropertyProxyPtr->name, outer, parentJsonObject,
            TypeTag<TargetOuterClass>{});
    }
    else
    {
        static_assert(false, "propertyProxyToJson: unexpected static property type");
        return false;
    }
}

template <typename Outer, const reflection::StaticPropertyMap<Outer>* staticPropertyMapPtr>
bool propertyMapToJson(const Outer& outer,
    QJsonObject& parentJsonObject,
    PointerTag<staticPropertyMapPtr>)
{
    static_assert(staticPropertyMapPtr != nullptr);

    return propertyMapToJson(staticPropertyMapPtr->name, outer, parentJsonObject, TypeTag<Outer>{});
}

template <typename Outer>
bool propertyMapToJson(std::string_view propertyName,
    const Outer& outer,
    QJsonObject& parentJsonObject,
    TypeTag<Outer>)
{
    using Meta = reflection::StaticPropertyMeta<Outer>;

    QJsonObject jsonObject;
    bool result = true;

    Meta::forEach(
        [&outer, &result, &jsonObject](auto, auto constValue)
    {
        constexpr auto staticPropertyPtr = decltype(constValue)::value;
        if constexpr (staticPropertyPtr == nullptr)
        {
            return;
        }

        using StaticPropertyClass = std::remove_cv_t<std::remove_pointer_t<decltype(staticPropertyPtr)>>;
        using PointerTagClass = PointerTag<staticPropertyPtr>;

        if constexpr (reflection::IsSpecialization<StaticPropertyClass, reflection::StaticProperty>::value)
        {
            if (!propertyToJson(outer, jsonObject, PointerTagClass{}))
                result = false;
        }
        else if constexpr (reflection::IsSpecialization<StaticPropertyClass, reflection::StaticPropertyMap>::value)
        {
            if (!propertyMapToJson(outer, jsonObject, PointerTagClass{}))
                result = false;
        }
        else if constexpr (reflection::IsSpecialization<StaticPropertyClass, reflection::StaticPropertyProxy>::value)
        {
            if (!propertyProxyToJson(outer, jsonObject, PointerTagClass{}))
                result = false;
        }
        else
        {
            static_assert(false, "propertyMapToJson: unexpected static property type");
        }
    });

    parentJsonObject[impl::toQString(propertyName)] = jsonObject;
    return result;
}

template <typename Outer, typename T, const reflection::StaticProperty<Outer, T>* staticPropertyPtr>
bool propertyToJson(const Outer& outer,
    QJsonObject& parentJsonObject,
    PointerTag<staticPropertyPtr>)
{
    static_assert(staticPropertyPtr != nullptr);

    return propertyToJson(staticPropertyPtr->name, outer, parentJsonObject,
        PointerToMemberTag<staticPropertyPtr->valuePtr>{});
}

template <typename Outer, typename T, T Outer::* valuePtr>
bool propertyToJson(std::string_view propertyName,
    const Outer& outer,
    QJsonObject& parentJsonObject,
    PointerToMemberTag<valuePtr>)
{
    static_assert(valuePtr != nullptr);

    return namedValueToJson(propertyName, outer.*valuePtr, parentJsonObject, TypeTag<const T>{});
}

template <typename T>
bool namedValueToJson(std::string_view propertyName,
    const T& value,
    QJsonObject& parentJsonObject,
    TypeTag<const T>)
{
    QJsonValue jsonValue;
    if (!valueToJson(value, jsonValue, TypeTag<const T>{}))
        return false;
    parentJsonObject[impl::toQString(propertyName)] = jsonValue;
    return true;
}

template <typename T>
bool valueToJson(const T& value, QJsonValue& jsonValue, TypeTag<const T>)
{
    using Type = std::remove_reference_t<T>;

    if constexpr (reflection::IsObject<Type>::value)
    {
        using Meta = reflection::StaticPropertyMeta<Type>;
        constexpr auto staticPropertyMapPtr = Meta::getStaticPropertyMap();
        static_assert(staticPropertyMapPtr != nullptr);
        constexpr auto propertyName = staticPropertyMapPtr->name;

        QJsonObject parentJsonObject;
        if (!convertToJson(value, parentJsonObject, TypeTag<Type>{}))
            return false;
        jsonValue = parentJsonObject[impl::toQString(propertyName)];
    }
    else if constexpr (std::is_pointer_v<Type>
                       && reflection::IsObject<std::remove_pointer_t<Type>>::value)
    {
        using PlainType = std::remove_pointer_t<Type>;

        using Meta = reflection::StaticPropertyMeta<PlainType>;
        constexpr auto staticPropertyMapPtr = Meta::getStaticPropertyMap();
        static_assert(staticPropertyMapPtr != nullptr);
        constexpr auto propertyName = staticPropertyMapPtr->name;

        if (value != nullptr)
        {
            QJsonObject parentJsonObject;
            if (!convertToJson(*value, parentJsonObject, TypeTag<PlainType>{}))
                return false;
            jsonValue = parentJsonObject[impl::toQString(propertyName)];
        }
        else
        {
            jsonValue = QJsonValue::Null;
        }
    }
    else if constexpr (reflection::IsString<Type>::value)
        jsonValue = QString::fromStdString(std::string(value));
    else if constexpr (std::is_same_v<Type, bool>)
        jsonValue = value;
    else if constexpr (std::is_integral_v<Type>)
    {
        // QJsonValue stores exact integral values as signed 64-bit integers.
        if constexpr (std::is_unsigned_v<Type>)
        {
            if (static_cast<quint64>(value)
                > static_cast<quint64>(std::numeric_limits<qint64>::max()))
                return false;
        }
        jsonValue = static_cast<qint64>(value);
    }
    else if constexpr (std::is_floating_point_v<Type>)
    {
        const double number = static_cast<double>(value);
        if (!std::isfinite(number))
            return false;
        jsonValue = number;
    }
    else if constexpr (reflection::IsIterable<Type>::value)
    {
        QJsonArray jsonArray;
        bool result = true;

        for (const auto& item : value)
        {
            using ItemType = std::remove_reference_t<decltype(item)>;

            QJsonValue iterJsonValue;
            if (!valueToJson(item, iterJsonValue, TypeTag<ItemType>{}))
            {
                result = false;
                continue;
            }
            jsonArray.append(iterJsonValue);
        }

        jsonValue = std::move(jsonArray);
        return result;
    }
    else
    {
        static_assert(false, "valueToJson: unexpected type");
        return false;
    }

    return true;
}

} // namespace extensions
