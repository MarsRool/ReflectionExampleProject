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
std::string convertToString(const Outer& value)
{
    constexpr const auto* staticPropertyMapPtr = &Outer::staticPropertyMap;
    return propertyMapToString(value, PointerHolderTag<StaticPropertyMap<Outer>, staticPropertyMapPtr>{});
}

template <typename Outer, typename StaticPropertyT, const StaticPropertyProxy<Outer, StaticPropertyT>* staticPropertyProxyPtr>
std::string propertyProxyToString(const typename StaticPropertyProxy<Outer, StaticPropertyT>::TargetOuterClass& outer,
                                  PointerHolderTag<StaticPropertyProxy<Outer, StaticPropertyT>, staticPropertyProxyPtr>)
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

        return propertyToString(propertyName, outer, PointerToMemberHolderTag<Outer, ValueT, valuePtr>{});
    }
    else if constexpr (IsSpecialization<TargetStaticPropertyClass, reflection::StaticPropertyMap>::value)
    {
        constexpr const auto propertyName = staticPropertyProxyPtr->getName();

        return propertyMapToString(propertyName, outer, PointerHolderTag<StaticPropertyMap<TargetOuterClass>, &targetStaticProperty>{});
    }
    else
    {
        static_assert(false, "propertyProxyToString proxy: unexpected static property type");
        Q_UNUSED(outer)
        return "unknown-type";
    }
}

template <typename Outer, const StaticPropertyMap<Outer>* staticPropertyMapPtr>
std::string propertyMapToString(const Outer& outer, PointerHolderTag<StaticPropertyMap<Outer>, staticPropertyMapPtr>)
{
    static_assert(staticPropertyMapPtr != nullptr);

    constexpr const auto propertyName = staticPropertyMapPtr->getName();

    return propertyMapToString(propertyName, outer, PointerHolderTag<StaticPropertyMap<Outer>, staticPropertyMapPtr>{});
}

template <typename Outer, const StaticPropertyMap<Outer>* staticPropertyMapPtr>
std::string propertyMapToString(std::string_view propertyName, const Outer& outer, PointerHolderTag<StaticPropertyMap<Outer>, staticPropertyMapPtr>)
{
    // Note, staticPropertyMapPtr is not used directly here
    // it's necessary to avoid usage of this overload by mistake
    // static_assert(staticPropertyMapPtr != nullptr);

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
            result += propertyToString(outer, PointerHolderTag<StaticPropertyClass, staticPropertyPtr>{});
        }
        else if constexpr (IsSpecialization<StaticPropertyClass, reflection::StaticPropertyMap>::value)
        {
            result += propertyMapToString(outer, PointerHolderTag<StaticPropertyClass, staticPropertyPtr>{});
        }
        else if constexpr (IsSpecialization<StaticPropertyClass, reflection::StaticPropertyProxy>::value)
        {
            result += propertyProxyToString(outer, PointerHolderTag<StaticPropertyClass, staticPropertyPtr>{});
        }
        else
        {
            static_assert(false, "propertyMapToString map: unexpected static property type");
        }

        if (i != uniqueStaticHeterogeneousMapKeysCount<Outer, KeyType>([]{}))
            result += ",\n";
        i++;
    });

    result += " }";
    return result;
}

template <typename Outer, typename T, const StaticProperty<Outer, T>* staticPropertyPtr>
std::string propertyToString(const Outer& outer, PointerHolderTag<StaticProperty<Outer, T>, staticPropertyPtr>)
{
    static_assert(staticPropertyPtr != nullptr);

    constexpr const auto propertyName = staticPropertyPtr->getName();
    constexpr const auto valuePtr = staticPropertyPtr->getRaw();

    return propertyToString(propertyName, outer, PointerToMemberHolderTag<Outer, T, valuePtr>{});
}

template <typename Outer, typename T, const T Outer::* valuePtr>
std::string propertyToString(std::string_view propertyName, const Outer& outer, PointerToMemberHolderTag<Outer, T, valuePtr>)
{
    static_assert(valuePtr != nullptr);

    return namedValueToString(propertyName, outer.*valuePtr, TypeTag<T>{});
}

template <typename T, typename U>
std::string namedValueToString(std::string_view propertyName, const T& value, TypeTag<U>)
{
    return '\"' + std::string(propertyName) + "\": " + valueToString(value, TypeTag<U>{});
}

template <typename T, typename U>
std::string valueToString(const T& value, TypeTag<U>)
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
            result += valueToString(item, TypeTag<Type>{});
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
