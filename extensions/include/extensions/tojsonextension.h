#pragma once

#include <cmath>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <nlohmann/json.hpp>

#include "reflection/reflection.h"
#include "extensions/extensionstypetraits.h"

namespace extensions
{

template <typename TypeChecker = reflection::TrueTypeChecker, typename Outer>
bool convertToJson(const Outer& object, nlohmann::json& output);

namespace impl
{

template <typename TypeChecker, typename T>
bool valueToJson(const T& value, nlohmann::json& output)
{
    using Type = CleanType<T>;

    if constexpr (reflection::impl::IsReflected<Type>::value)
    {
        return convertToJson<TypeChecker>(value, output);
    }
    else if constexpr (IsString<Type>::value)
    {
        output = std::string(value);
        return true;
    }
    else if constexpr (std::is_same_v<Type, bool> || std::is_integral_v<Type>)
    {
        output = value;
        return true;
    }
    else if constexpr (std::is_enum_v<Type>)
    {
        return valueToJson<TypeChecker>(static_cast<std::underlying_type_t<Type>>(value), output);
    }
    else if constexpr (std::is_floating_point_v<Type>)
    {
        if (!std::isfinite(value))
            return false;

        output = value;
        return true;
    }
    else if constexpr (std::is_pointer_v<Type>)
    {
        static_assert(AlwaysFalse<Type>::value, "JSON conversion does not support pointers");
    }
    else if constexpr (IsIterable<Type>::value)
    {
        nlohmann::json array = nlohmann::json::array();
        for (const auto& item : value)
        {
            nlohmann::json element;
            if (!valueToJson<TypeChecker>(item, element))
                return false;
            array.push_back(std::move(element));
        }
        output = std::move(array);
        return true;
    }
    else
    {
        static_assert(AlwaysFalse<Type>::value, "Unsupported type for JSON serialization");
    }
}

template <typename TypeChecker, typename T>
bool namedValueToJson(std::string_view name, const T& value, nlohmann::json& output)
{
    nlohmann::json converted;
    if (!valueToJson<TypeChecker>(value, converted))
        return false;

    output[std::string(name)] = std::move(converted);
    return true;
}

} // namespace impl

template <typename TypeChecker, typename Outer>
bool convertToJson(const Outer& object, nlohmann::json& output)
{
    static_assert(reflection::impl::IsReflected<Outer>::value, "Type must be registered for reflection");

    nlohmann::json result = nlohmann::json::object();
    bool success = true;

    reflection::StaticPropertyMeta<Outer>::template forEach<TypeChecker>([&](auto entry)
    {
        using Entry = decltype(entry);
        if constexpr (reflection::impl::IsBaseClassEntry<Entry>::value)
        {
            using Base = typename impl::CleanType<decltype(Entry::value)>::OuterType;
            if (!impl::namedValueToJson<TypeChecker>(Entry::key, static_cast<const Base&>(object), result))
                success = false;
        }
        else
        {
            if (!impl::namedValueToJson<TypeChecker>(Entry::key, Entry::value.get(object), result))
                success = false;
        }
    });

    if (success)
        output = std::move(result);
    return success;
}

} // namespace extensions
