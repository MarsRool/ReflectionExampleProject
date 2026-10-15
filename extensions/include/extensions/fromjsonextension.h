#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <iterator>
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

// Deserialize known properties present in JSON. Unknown properties are ignored,
// and missing properties are left unchanged. Failure may leave partial updates.
// Const properties are validated in QT_DEBUG and skipped in release builds.
// Pointer targets must already exist; this extension does not manage ownership.
template <typename Outer, typename = std::enable_if_t<reflection::IsObject<Outer>::value>>
bool convertFromJson(Outer& value, const QJsonObject& parentJsonObject, TypeTag<Outer> = {})
{
    using Meta = reflection::StaticPropertyMeta<Outer>;
    constexpr auto staticPropertyMapPtr = Meta::getStaticPropertyMap();

    return propertyMapFromJson(value, parentJsonObject,
        PointerTag<staticPropertyMapPtr>{});
}

template <typename Outer, typename StaticPropertyT, const reflection::StaticPropertyProxy<Outer, StaticPropertyT>* staticPropertyProxyPtr>
bool propertyProxyFromJson(typename reflection::StaticPropertyProxy<Outer, StaticPropertyT>::TargetOuterClass& outer,
    const QJsonObject& parentJsonObject,
    PointerTag<staticPropertyProxyPtr>)
{
    static_assert(staticPropertyProxyPtr != nullptr);

    using ProxyClass = reflection::StaticPropertyProxy<Outer, StaticPropertyT>;
    using TargetStaticPropertyClass = typename ProxyClass::TargetStaticPropertyClass;
    using TargetOuterClass = typename reflection::StaticPropertyProxy<Outer, StaticPropertyT>::TargetOuterClass;

    constexpr const auto& targetStaticProperty = staticPropertyProxyPtr->staticProperty;

    if constexpr (reflection::IsSpecialization<TargetStaticPropertyClass, reflection::StaticProperty>::value)
    {
        return propertyFromJson(staticPropertyProxyPtr->name, outer, parentJsonObject,
            PointerToMemberTag<targetStaticProperty->valuePtr>{});
    }
    else if constexpr (reflection::IsSpecialization<TargetStaticPropertyClass, reflection::StaticPropertyMap>::value)
    {
        return propertyMapFromJson(staticPropertyProxyPtr->name, outer, parentJsonObject,
            TypeTag<TargetOuterClass>{});
    }
    else
    {
        static_assert(false, "propertyProxyFromJson: unexpected static property type");
        return false;
    }
}

template <typename Outer, const reflection::StaticPropertyMap<Outer>* staticPropertyMapPtr>
bool propertyMapFromJson(Outer& outer,
    const QJsonObject& parentJsonObject,
    PointerTag<staticPropertyMapPtr>)
{
    static_assert(staticPropertyMapPtr != nullptr);

    return propertyMapFromJson(staticPropertyMapPtr->name, outer, parentJsonObject,
        TypeTag<Outer>{});
}

template <typename Outer>
bool propertyMapFromJson(std::string_view propertyName,
    Outer& outer,
    const QJsonObject& parentJsonObject,
    TypeTag<Outer>)
{
    using Meta = reflection::StaticPropertyMeta<Outer>;

    const auto jsonValue = parentJsonObject[impl::toQString(propertyName)];
    if (!jsonValue.isObject())
        return false;
    const auto jsonObject = jsonValue.toObject();

    bool result = true;

    for (auto iter = jsonObject.constBegin(); iter != jsonObject.constEnd(); ++iter)
    {
        const auto name = iter.key().toStdString();

        Meta::doForKey(
            [&outer, &result, &jsonObject](auto, auto constValue)
        {
            constexpr auto staticPropertyPtr = decltype(constValue)::value;
            if constexpr (staticPropertyPtr == nullptr)
            {
                result = false;
                return;
            }

            using StaticPropertyClass = std::remove_cv_t<std::remove_pointer_t<decltype(staticPropertyPtr)>>;
            using PointerTagClass = PointerTag<staticPropertyPtr>;

            if constexpr (reflection::IsSpecialization<StaticPropertyClass, reflection::StaticProperty>::value)
            {
                if (!propertyFromJson(outer, jsonObject, PointerTagClass{}))
                    result = false;
            }
            else if constexpr (reflection::IsSpecialization<StaticPropertyClass, reflection::StaticPropertyMap>::value)
            {
                if (!propertyMapFromJson(outer, jsonObject, PointerTagClass{}))
                    result = false;
            }
            else if constexpr (reflection::IsSpecialization<StaticPropertyClass, reflection::StaticPropertyProxy>::value)
            {
                if (!propertyProxyFromJson(outer, jsonObject, PointerTagClass{}))
                    result = false;
            }
            else
            {
                static_assert(false, "propertyMapFromJson: unexpected static property type");
            }
        }, name.c_str());
    }

    return result;
}

template <typename Outer, typename T, const reflection::StaticProperty<Outer, T>* staticPropertyPtr>
bool propertyFromJson(Outer& outer,
    const QJsonObject& parentJsonObject,
    PointerTag<staticPropertyPtr>)
{
    static_assert(staticPropertyPtr != nullptr);

    return propertyFromJson(staticPropertyPtr->name, outer, parentJsonObject,
        PointerToMemberTag<staticPropertyPtr->valuePtr>{});
}

template <typename Outer, typename T, T Outer::* valuePtr>
bool propertyFromJson(std::string_view propertyName,
    Outer& outer,
    const QJsonObject& parentJsonObject,
    PointerToMemberTag<valuePtr>)
{
    static_assert(valuePtr != nullptr);

    if constexpr (std::is_const_v<T>)
    {
#ifdef QT_DEBUG
        using NonConstValueType = std::remove_cv_t<T>;

        NonConstValueType value;
        if (!namedValueFromJson(propertyName, value, parentJsonObject,
            TypeTag<NonConstValueType>{}))
            return false;
        return outer.*valuePtr == value;
#else
        return true;
#endif
    }
    else
    {
        return namedValueFromJson(propertyName, outer.*valuePtr, parentJsonObject, TypeTag<T>{});
    }
}

template <typename T>
bool namedValueFromJson(std::string_view propertyName,
    T& value,
    const QJsonObject& parentJsonObject,
    TypeTag<T>)
{
    const auto jsonValue = parentJsonObject[impl::toQString(propertyName)];
    return valueFromJson(value, jsonValue, TypeTag<T>{});
}

template <typename T>
bool valueFromJson(T& value, const QJsonValue& jsonValue, TypeTag<T>)
{
    using Type = std::remove_reference_t<T>;

    if constexpr (reflection::IsObject<Type>::value)
    {
        using Meta = reflection::StaticPropertyMeta<Type>;
        constexpr auto staticPropertyMapPtr = Meta::getStaticPropertyMap();
        static_assert(staticPropertyMapPtr != nullptr);
        constexpr auto propertyName = staticPropertyMapPtr->name;

        if (!jsonValue.isObject())
            return false;

        const auto jsonObject = jsonValue.toObject();
        QJsonObject parentJsonObject;
        parentJsonObject[impl::toQString(propertyName)] = jsonObject;

        return convertFromJson(value, parentJsonObject, TypeTag<Type>{});
    }
    else if constexpr (std::is_pointer_v<Type>
                       && reflection::IsObject<std::remove_pointer_t<Type>>::value)
    {
        using PlainType = std::remove_pointer_t<Type>;

        using Meta = reflection::StaticPropertyMeta<PlainType>;
        constexpr auto staticPropertyMapPtr = Meta::getStaticPropertyMap();
        static_assert(staticPropertyMapPtr != nullptr);
        constexpr auto propertyName = staticPropertyMapPtr->name;

        // A null JSON value matches an already-null pointer. No allocation or
        // deletion is performed by this extension.
        if (jsonValue.isNull())
            return value == nullptr;
        if (value == nullptr || !jsonValue.isObject())
            return false;

        const auto jsonObject = jsonValue.toObject();
        QJsonObject parentJsonObject;
        parentJsonObject[impl::toQString(propertyName)] = jsonObject;

        return convertFromJson(*value, parentJsonObject, TypeTag<PlainType>{});
    }
    else if constexpr (reflection::IsString<Type>::value)
    {
        if (!jsonValue.isString())
            return false;
        if constexpr (std::is_same_v<Type, std::string_view>)
        {
            // A temporary std::string cannot safely back a string_view.
            return false;
        }
        else
        {
            value = jsonValue.toString().toStdString();
        }
    }
    else if constexpr (std::is_same_v<Type, bool>)
    {
        if (!jsonValue.isBool())
            return false;
        value = jsonValue.toBool();
    }
    else if constexpr (std::is_integral_v<Type>)
    {
        if (!jsonValue.isDouble())
            return false;

        // Two distinct defaults distinguish a valid qint64 from a conversion
        // failure without sacrificing the exact minimum/maximum values.
        const qint64 first = jsonValue.toInteger(std::numeric_limits<qint64>::min());
        const qint64 second = jsonValue.toInteger(std::numeric_limits<qint64>::max());
        if (first != second)
            return false;

        if constexpr (std::is_signed_v<Type>)
        {
            if (first < static_cast<qint64>(std::numeric_limits<Type>::min())
                || first > static_cast<qint64>(std::numeric_limits<Type>::max()))
                return false;
        }
        else
        {
            if (first < 0 || static_cast<quint64>(first)
                > static_cast<quint64>(std::numeric_limits<Type>::max()))
                return false;
        }
        value = static_cast<Type>(first);
    }
    else if constexpr (std::is_floating_point_v<Type>)
    {
        if (!jsonValue.isDouble())
            return false;
        const double number = jsonValue.toDouble();
        const double maxValue = static_cast<double>(std::numeric_limits<Type>::max());
        if (!std::isfinite(number) || number < -maxValue || number > maxValue)
            return false;
        value = static_cast<Type>(number);
    }
    else if constexpr (reflection::IsIterable<Type>::value)
    {
        if (!jsonValue.isArray())
            return false;
        const auto jsonArray = jsonValue.toArray();

        return valueFromJsonArray(value, jsonArray, TypeTag<Type>{});
    }
    else
    {
        static_assert(false, "valueFromJson: unexpected type");
        return false;
    }
    return true;
}

template <typename T, typename = std::enable_if_t<reflection::IsIterable<T>::value>>
bool valueFromJsonArray(T& value, const QJsonArray& jsonArray, TypeTag<T>)
{
    if constexpr (reflection::IsInsertable<T>::value)
    {
        using ValueType = typename T::value_type;

        bool result = true;

        value.clear();

        if constexpr (reflection::HasReserve<T>::value)
        {
            value.reserve(jsonArray.size());
        }

        auto inserter = std::inserter(value, value.end());

        for (auto iter = jsonArray.constBegin(); iter != jsonArray.constEnd(); ++iter)
        {
            ValueType arrayElement;
            if (!valueFromJson(arrayElement, *iter, TypeTag<ValueType>{}))
            {
                result = false;
                continue;
            }
            *inserter = std::move(arrayElement);
        }

        return result;
    }
    else
    {
        static_assert(std::is_array_v<T>
                || reflection::IsSpecializationSized<T, std::array>::value,
            "valueFromJsonArray: expected an insertable container or fixed-size array");

        // Fixed-size arrays must match exactly; do not truncate or pad.
        if (static_cast<std::size_t>(jsonArray.size()) != std::size(value))
            return false;

        bool result = true;
        auto valueIter = std::begin(value);
        using ValueType = std::remove_cv_t<std::remove_reference_t<decltype(*valueIter)>>;

        for (const auto& item : jsonArray)
        {
            ValueType arrayElement{};
            if (!valueFromJson(arrayElement, item, TypeTag<ValueType>{}))
            {
                result = false;
                ++valueIter;
                continue;
            }
            *valueIter = std::move(arrayElement);
            ++valueIter;
        }
        return result;
    }
}

} // namespace extensions
