#pragma once

#include <string>
#include <string_view>
#include <type_traits>

#include "reflection/reflection.h"
#include "extensions/extensionstypetraits.h"

namespace extensions
{

template <typename TypeChecker = reflection::TrueTypeChecker, typename Outer>
std::string convertToString(const Outer& object);

namespace impl
{

template <typename TypeChecker, typename T>
std::string valueToString(const T& value)
{
    using Type = CleanType<T>;

    if constexpr (reflection::impl::IsReflected<Type>::value)
    {
        return convertToString<TypeChecker>(value);
    }
    else if constexpr (IsString<Type>::value)
    {
        return '"' + std::string(value) + '"';
    }
    else if constexpr (std::is_same_v<Type, bool>)
    {
        return value ? "true" : "false";
    }
    else if constexpr (std::is_enum_v<Type>)
    {
        return valueToString<TypeChecker>(static_cast<std::underlying_type_t<Type>>(value));
    }
    else if constexpr (IsToStringAvailable<Type>::value)
    {
        return std::to_string(value);
    }
    else if constexpr (std::is_pointer_v<Type>)
    {
        static_assert(AlwaysFalse<Type>::value, "String conversion does not support pointers");
    }
    else if constexpr (IsIterable<Type>::value)
    {
        std::string result = "[ ";
        bool first = true;
        for (const auto& item : value)
        {
            if (!first)
                result += ", ";
            first = false;
            result += valueToString<TypeChecker>(item);
        }
        return result + " ]";
    }
    else
    {
        static_assert(AlwaysFalse<Type>::value, "Unsupported type for string conversion");
    }
}

template <typename TypeChecker, typename T>
std::string namedValueToString(std::string_view name, const T& value)
{
    return '"' + std::string(name) + "\": " + valueToString<TypeChecker>(value);
}

} // namespace impl

template <typename TypeChecker, typename Outer>
std::string convertToString(const Outer& object)
{
    static_assert(reflection::impl::IsReflected<Outer>::value,
        "Type must be registered for reflection");

    std::string result = "{ ";
    bool first = true;

    reflection::StaticPropertyMeta<Outer>::template forEach<TypeChecker>([&](auto entry)
    {
        using Entry = decltype(entry);
        if (!first)
            result += ", ";
        first = false;

        if constexpr (reflection::impl::IsBaseClassEntry<Entry>::value)
        {
            using Base = typename impl::CleanType<decltype(Entry::value)>::OuterType;
            result += impl::namedValueToString<TypeChecker>(Entry::key, static_cast<const Base&>(object));
        }
        else
        {
            result += impl::namedValueToString<TypeChecker>(Entry::key, Entry::value.get(object));
        }
    });

    return result + " }";
}

} // namespace extensions
