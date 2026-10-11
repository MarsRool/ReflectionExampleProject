#pragma once
#include <string_view>
#include <QString>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QCborMap>

#include "reflection/reflection.h"
#include "extensions/extensionstypetraits.h"
#include "extensions/detail/qstringutils.h"

namespace extensions
{

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

        if (value == nullptr)
            return false;
        if (!jsonValue.isObject())
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
        value = jsonValue.toString().toStdString();
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
        value = static_cast<Type>(jsonValue.toInteger());
    }
    else if constexpr (std::is_floating_point_v<Type>)
    {
        if (!jsonValue.isDouble())
            return false;
        value = static_cast<Type>(jsonValue.toDouble());
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
        bool result = true;

        auto iter = jsonArray.constBegin();

        auto valueIter = std::begin(value);
        auto valueEndIter = std::end(value);

        using ValueType = std::remove_reference_t<decltype(*valueIter)>;

        for (; iter != jsonArray.constEnd() && valueIter != valueEndIter; ++iter, ++valueIter)
        {
            ValueType arrayElement;
            if (!valueFromJson(arrayElement, *iter, TypeTag<ValueType>{}))
            {
                result = false;
                continue;
            }
            *valueIter = std::move(arrayElement);
        }

        for (; valueIter != valueEndIter; ++valueIter)
        {
            *valueIter = ValueType{};
        }

        return result;
    }
}

} // namespace extensions
