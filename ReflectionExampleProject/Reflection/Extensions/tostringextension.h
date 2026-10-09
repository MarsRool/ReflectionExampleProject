#pragma once
#include "Reflection/Extensions/extensionstypetraits.h"
#include "Reflection/reflection.h"

namespace extensions
{

template <typename Outer, typename = std::enable_if_t<reflection::IsObject<Outer>::value>>
std::string convertToString(const Outer& value, TypeTag<Outer> = {})
{
    using Meta = reflection::StaticPropertyMeta<Outer>;
    constexpr auto staticPropertyMapPtr = Meta::getStaticPropertyMap();

    return propertyMapToString(value,
        PointerTag<staticPropertyMapPtr>{});
}

template <typename Outer, typename StaticPropertyT, const reflection::StaticPropertyProxy<Outer, StaticPropertyT>* staticPropertyProxyPtr>
std::string propertyProxyToString(const typename reflection::StaticPropertyProxy<Outer, StaticPropertyT>::TargetOuterClass& outer,
    PointerTag<staticPropertyProxyPtr>)
{
    static_assert(staticPropertyProxyPtr != nullptr);

    using ProxyClass = reflection::StaticPropertyProxy<Outer, StaticPropertyT>;
    using TargetStaticPropertyClass = typename ProxyClass::TargetStaticPropertyClass;
    using TargetOuterClass = typename reflection::StaticPropertyProxy<Outer, StaticPropertyT>::TargetOuterClass;

    constexpr const auto& targetStaticProperty = staticPropertyProxyPtr->staticProperty;

    if constexpr (reflection::IsSpecialization<TargetStaticPropertyClass, reflection::StaticProperty>::value)
    {
        return propertyToString(staticPropertyProxyPtr->name, outer,
            PointerToMemberTag<targetStaticProperty->valuePtr>{});
    }
    else if constexpr (reflection::IsSpecialization<TargetStaticPropertyClass, reflection::StaticPropertyMap>::value)
    {
        return propertyMapToString(staticPropertyProxyPtr->name, outer,
            TypeTag<TargetOuterClass>{});
    }
    else
    {
        static_assert(false, "propertyProxyToString: unexpected static property type");
        return "unknown-type";
    }
}

template <typename Outer, const reflection::StaticPropertyMap<Outer>* staticPropertyMapPtr>
std::string propertyMapToString(const Outer& outer,
    PointerTag<staticPropertyMapPtr>)
{
    static_assert(staticPropertyMapPtr != nullptr);

    return propertyMapToString(staticPropertyMapPtr->name, outer, TypeTag<Outer>{});
}

template <typename Outer>
std::string propertyMapToString(std::string_view propertyName,
    const Outer& outer,
    TypeTag<Outer>)
{
    using Meta = reflection::StaticPropertyMeta<Outer>;

    std::string result{ '\"' + std::string(propertyName) + "\":\n{ " };
    std::size_t i = 0;

    Meta::forEach(
        [&outer, &result, &i](auto, auto constValue)
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
            result += propertyToString(outer, PointerTagClass{});
        }
        else if constexpr (reflection::IsSpecialization<StaticPropertyClass, reflection::StaticPropertyMap>::value)
        {
            result += propertyMapToString(outer, PointerTagClass{});
        }
        else if constexpr (reflection::IsSpecialization<StaticPropertyClass, reflection::StaticPropertyProxy>::value)
        {
            result += propertyProxyToString(outer, PointerTagClass{});
        }
        else
        {
            static_assert(false, "propertyMapToString: unexpected static property type");
        }

        if (i != Meta::size())
            result += ",\n";
        i++;
    });

    result += " }";
    return result;
}

template <typename Outer, typename T, const reflection::StaticProperty<Outer, T>* staticPropertyPtr>
std::string propertyToString(const Outer& outer,
    PointerTag<staticPropertyPtr>)
{
    static_assert(staticPropertyPtr != nullptr);

    return propertyToString(staticPropertyPtr->name, outer, PointerToMemberTag<staticPropertyPtr->valuePtr>{});
}

template <typename Outer, typename T, T Outer::* valuePtr>
std::string propertyToString(std::string_view propertyName,
    const Outer& outer,
    PointerToMemberTag<valuePtr>)
{
    static_assert(valuePtr != nullptr);

    return namedValueToString(propertyName, outer.*valuePtr, TypeTag<const T>{});
}

template <typename T>
std::string namedValueToString(std::string_view propertyName,
    const T& value,
    TypeTag<const T>)
{
    return '\"' + std::string(propertyName) + "\": " + valueToString(value, TypeTag<const T>{});
}

template <typename T>
std::string valueToString(const T& value, TypeTag<const T>)
{
    using Type = std::remove_reference_t<T>;

    if constexpr (reflection::IsObject<Type>::value)
    {
        return convertToString(value, TypeTag<Type>{});
    }
    else if constexpr (std::is_pointer_v<Type>
                       && reflection::IsObject<std::remove_pointer_t<Type>>::value)
    {
        using PlainType = std::remove_pointer_t<Type>;

        if (value != nullptr)
        {
            return convertToString(*value, TypeTag<PlainType>{});
        }

        return "null";
    }
    else if constexpr (reflection::IsString<Type>::value)
        return '\"' + std::string(value) + '\"';
    else if constexpr (std::is_same_v<Type, bool>)
        return value ? "true" : "false";
    else if constexpr (reflection::IsToStringAvailable<Type>::value)
        return std::to_string(value);
    else if constexpr (reflection::IsIterable<Type>::value)
    {
        std::string result{ "[ " };
        std::size_t index = 0;
        const std::size_t size = std::size(value);

        for (const auto& item : value)
        {
            using ItemType = std::remove_reference_t<decltype(item)>;

            result += valueToString(item, TypeTag<ItemType>{});
            if (index != size - 1)
                result += ", ";
            ++index;
        }

        result += " ]";
        return result;
    }
    else
    {
        static_assert(false, "valueToString: unexpected type");
        return "unknown-type";
    }
}

} // namespace extensions
