#pragma once

#include "Shared/typetester.h"
#include "Shared/uniquestaticheterogeneousmap.h"
#include "Reflection/Utils/valueutils.h"

namespace reflection
{

template <typename Outer, typename T>
class StaticProperty;

template <typename Outer>
class StaticPropertyMap;

template <typename Outer, typename StaticPropertyT>
class StaticPropertyProxy;


template <typename Outer, typename = std::enable_if_t<IsObject<Outer>::value, void>>
auto convertToString(const Outer& value);

template <typename Outer, typename StaticPropertyT, const StaticPropertyProxy<Outer, StaticPropertyT>* staticPropertyProxyPtr>
std::string propertyToString(const typename StaticPropertyProxy<Outer, StaticPropertyT>::TargetOuterClass& outer);

template <typename Outer, const StaticPropertyMap<Outer>* staticPropertyMap>
std::string propertyToString(const Outer& outer);

template <typename Outer, const StaticPropertyMap<Outer>* staticPropertyMap>
std::string propertyToString(std::string_view propertyName, const Outer& outer);

template <typename Outer, typename T, const StaticProperty<Outer, T>* staticPropertyPtr>
std::string propertyToString(const Outer& outer);

template <typename Outer, typename T, const typename StaticProperty<Outer, T>::ValuePtr valuePtr>
std::string propertyToString(std::string_view propertyName, const Outer& outer);

template <typename T>
std::string propertyToString(std::string_view propertyName, const T& value);

template <typename T>
std::string valueToString(const T& value);


template <typename Outer, typename>
auto convertToString(const Outer& value)
{
    constexpr const auto* staticPropertyMapPtr = &Outer::staticPropertyMap;
    return propertyToString<Outer, staticPropertyMapPtr>(value);
}

template <typename Outer, typename StaticPropertyT, const StaticPropertyProxy<Outer, StaticPropertyT>* staticPropertyProxyPtr>
std::string propertyToString(const typename StaticPropertyProxy<Outer, StaticPropertyT>::TargetOuterClass& outer)
{
    static_assert(staticPropertyProxyPtr != nullptr);

    using ProxyClass = StaticPropertyProxy<Outer, StaticPropertyT>;
    using TargetStaticPropertyClass = typename ProxyClass::TargetStaticPropertyClass;
    using TargetOuterClass = typename ProxyClass::TargetOuterClass;

    constexpr const auto& targetStaticProperty = staticPropertyProxyPtr->get();

    if constexpr (IsSpecialization<TargetStaticPropertyClass, reflection::StaticProperty>::value)
    {
        using ValueT = typename TargetStaticPropertyClass::ValueT;

        constexpr const auto propertyName = staticPropertyProxyPtr->getName();
        constexpr const auto valuePtr = targetStaticProperty.getRaw();

        return propertyToString<TargetOuterClass, ValueT, valuePtr>(propertyName, outer);
    }
    else if constexpr (IsSpecialization<TargetStaticPropertyClass, reflection::StaticPropertyMap>::value)
    {
        constexpr const auto propertyName = staticPropertyProxyPtr->getName();

        return propertyToString<TargetOuterClass, &targetStaticProperty>(propertyName, outer);
    }
    else
    {
        static_assert(false, "propertyToString proxy: unexpected static property type");
        Q_UNUSED(outer)
        return "unknown-type";
    }
}

template <typename Outer, const StaticPropertyMap<Outer>* staticPropertyMap>
std::string propertyToString(const Outer& outer)
{
    static_assert(staticPropertyMap != nullptr);

    constexpr const auto propertyName = staticPropertyMap->getName();

    return propertyToString<Outer, staticPropertyMap>(propertyName, outer);
}

template <typename Outer, const StaticPropertyMap<Outer>* staticPropertyMap>
std::string propertyToString(std::string_view propertyName, const Outer& outer)
{
    // Note, staticPropertyMap is not used directly here
    // it's necessary to avoid usage of this overload by mistake
    // static_assert(staticPropertyMap != nullptr);

    using StaticPropertyMapClass = StaticPropertyMap<Outer>;
    using KeyType = typename StaticPropertyMapClass::KeyType;

    std::string result{ '\"' + std::string(propertyName) + "\":\n{ " };
    std::size_t i = 0;

    uniqueStaticHeterogeneousMapForEach<Outer, KeyType>([]{},
    [&outer, &result, &i](auto, auto constValue)
    {
        constexpr const auto staticPropertyPtr = decltype(constValue)::value;
        if constexpr (staticPropertyPtr == nullptr)
        {
            return;
        }

        using StaticPropertyClass = std::remove_cv_t<std::remove_pointer_t<decltype(staticPropertyPtr)>>;

        if constexpr (IsSpecialization<StaticPropertyClass, reflection::StaticProperty>::value)
        {
            using ValueT = typename StaticPropertyClass::ValueT;
            result += propertyToString<Outer, ValueT, staticPropertyPtr>(outer);
        }
        else if constexpr (IsSpecialization<StaticPropertyClass, reflection::StaticPropertyMap>::value)
        {
            result += propertyToString<Outer, staticPropertyPtr>(outer);
        }
        else if constexpr (IsSpecialization<StaticPropertyClass, reflection::StaticPropertyProxy>::value)
        {
            using TargetStaticPropertyClass = typename StaticPropertyClass::TargetStaticPropertyClass;
            using TargetOuterClass = typename StaticPropertyClass::TargetOuterClass;
            result += propertyToString<Outer, TargetStaticPropertyClass, staticPropertyPtr>(
                static_cast<const TargetOuterClass&>(outer));
        }
        else
        {
            static_assert(false, "propertyToString map: unexpected static property type");
        }

        if (i != uniqueStaticHeterogeneousMapKeysCount<Outer, KeyType>([]{}))
            result += ",\n";
        i++;
    });

    result += " }";
    return result;
}

template <typename Outer, typename T, const StaticProperty<Outer, T>* staticPropertyPtr>
std::string propertyToString(const Outer& outer)
{
    static_assert(staticPropertyPtr != nullptr);

    constexpr const auto propertyName = staticPropertyPtr->getName();
    constexpr const auto valuePtr = staticPropertyPtr->getRaw();

    return propertyToString<Outer, T, valuePtr>(propertyName, outer);
}

template <typename Outer, typename T, const typename StaticProperty<Outer, T>::ValuePtr valuePtr>
std::string propertyToString(std::string_view propertyName, const Outer& outer)
{
    static_assert(valuePtr != nullptr);

    return propertyToString(propertyName, outer.*valuePtr);
}

template <typename T>
std::string propertyToString(std::string_view propertyName, const T& value)
{
    return '\"' + std::string(propertyName) + "\": " + valueToString(value);
}

template <typename T>
std::string valueToString(const T& value)
{
    using Type = std::remove_reference_t<T>;

    if constexpr (IsObject<Type>::value)
    {
        return convertToString<Type>(value);
    }
    else if constexpr (IsArray<Type>::value)
    {
        std::string result{ "[ " };
        std::size_t index = 0;
        const std::size_t size = std::size(value);

        for (const auto& item : value)
        {
            result += valueToString(item);
            if (index != size - 1)
                result += ", ";
            ++index;
        }

        result += " ]";
        return result;
    }
    else if constexpr (IsString<Type>::value)
        return '\"' + std::string(value) + '\"';
    else if constexpr (std::is_same_v<Type, bool>)
        return value ? "true" : "false";
    else if constexpr (std::is_null_pointer_v<Type>)
        return "null";
    else if constexpr (ToStringDetect<Type>::value)
        return std::to_string(value);
    else
    {
        static_assert(false, "valueToString: unexpected type");
        Q_UNUSED(value)
        return "unknown-type";
    }
}

} // namespace reflection
