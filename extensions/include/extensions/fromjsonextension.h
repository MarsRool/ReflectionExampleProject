#pragma once

#include <cmath>
#include <cstdint>
#include <iterator>
#include <limits>
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
bool convertFromJson(Outer& object, const nlohmann::json& input);

namespace impl
{

template <typename TypeChecker, typename T>
bool valueFromJson(T& value, const nlohmann::json& input)
{
    using Type = CleanType<T>;

    if constexpr (reflection::impl::IsReflected<Type>::value)
    {
        return convertFromJson<TypeChecker>(value, input);
    }
    else if constexpr (std::is_same_v<Type, std::string_view>)
    {
        // A temporary JSON string cannot safely store value to string_view.
        return false;
    }
    else if constexpr (std::is_same_v<Type, std::string>)
    {
        if (!input.is_string())
            return false;
        value = input.get<std::string>();
        return true;
    }
    else if constexpr (std::is_same_v<Type, bool>)
    {
        if (!input.is_boolean())
            return false;
        value = input.get<bool>();
        return true;
    }
    else if constexpr (std::is_enum_v<Type>)
    {
        using Underlying = std::underlying_type_t<Type>;
        Underlying converted{};
        if (!valueFromJson<TypeChecker>(converted, input))
            return false;
        value = static_cast<Type>(converted);
        return true;
    }
    else if constexpr (std::is_integral_v<Type>)
    {
        if (!input.is_number_integer())
            return false;

        if (input.is_number_unsigned())
        {
            const auto number = input.get<std::uint64_t>();
            if (number > static_cast<std::uint64_t>(std::numeric_limits<Type>::max()))
                return false;
            value = static_cast<Type>(number);
        }
        else
        {
            const auto number = input.get<std::int64_t>();
            if constexpr (std::is_signed_v<Type>)
            {
                if (number < static_cast<std::int64_t>(std::numeric_limits<Type>::min()) ||
                    number > static_cast<std::int64_t>(std::numeric_limits<Type>::max()))
                    return false;
            }
            else
            {
                if (number < 0 ||
                    static_cast<std::uint64_t>(number) > std::numeric_limits<Type>::max())
                    return false;
            }
            value = static_cast<Type>(number);
        }
        return true;
    }
    else if constexpr (std::is_floating_point_v<Type>)
    {
        if (!input.is_number())
            return false;

        const double number = input.get<double>();
        const long double limit = std::numeric_limits<Type>::max();
        if (!std::isfinite(number) ||
            static_cast<long double>(number) < -limit ||
            static_cast<long double>(number) > limit)
            return false;

        value = static_cast<Type>(number);
        return true;
    }
    else if constexpr (std::is_pointer_v<Type>)
    {
        static_assert(AlwaysFalse<Type>::value, "JSON conversion does not support pointers");
    }
    else if constexpr (IsIterable<Type>::value)
    {
        if (!input.is_array())
            return false;

        if constexpr (IsInsertable<Type>::value)
        {
            using Element = typename Type::value_type;
            value.clear();

            if constexpr (HasReserve<Type>::value)
                value.reserve(input.size());

            auto output = std::inserter(value, value.end());
            bool success = true;

            for (const auto& item : input)
            {
                Element element{};
                if (!valueFromJson<TypeChecker>(element, item))
                {
                    success = false;
                    continue;
                }
                *output = std::move(element);
                ++output;
            }
            return success;
        }
        else if constexpr (IsWritableSizedRange<Type>::value)
        {
            if (input.size() != std::size(value))
                return false;

            bool success = true;
            auto output = begin(value);

            for (const auto& item : input)
            {
                if (!valueFromJson<TypeChecker>(*output, item))
                    success = false;
                ++output;
            }
            return success;
        }
        else
        {
            static_assert(AlwaysFalse<Type>::value,
                "JSON deserialization requires insertable containers or writable sized ranges");
        }
    }
    else
    {
        static_assert(AlwaysFalse<Type>::value, "Unsupported type for JSON deserialization");
    }
}

template <typename TypeChecker, typename T>
bool namedValueFromJson(std::string_view name, T& value, const nlohmann::json& input)
{
    const auto found = input.find(std::string(name));
    if (found == input.end())
        return true;

    if constexpr (std::is_const_v<T>)
    {
#ifndef NDEBUG
        using MutableType = std::remove_const_t<T>;

        // Debug checks that deserialization works for mutable type T
        if constexpr (std::is_default_constructible_v<MutableType>)
        {
            MutableType temporary{};
            return valueFromJson<TypeChecker>(temporary, *found);
        }
        else if constexpr (std::is_copy_constructible_v<MutableType>)
        {
            MutableType temporary(value);
            return valueFromJson<TypeChecker>(temporary, *found);
        }
        else
        {
            return true;
        }
#else
        return true;
#endif
    }
    else
    {
        return valueFromJson<TypeChecker>(value, *found);
    }
}

} // namespace impl

template <typename TypeChecker, typename Outer>
bool convertFromJson(Outer& object, const nlohmann::json& input)
{
    static_assert(reflection::impl::IsReflected<Outer>::value, "Type must be registered for reflection");

    if (!input.is_object())
        return false;

    bool success = true;
    reflection::StaticPropertyMeta<Outer>::template forEach<TypeChecker>([&](auto entry)
    {
        using Entry = decltype(entry);
        if constexpr (reflection::impl::IsBaseClassEntry<Entry>::value)
        {
            using Base = typename impl::CleanType<decltype(Entry::value)>::OuterType;
            if (!impl::namedValueFromJson<TypeChecker>(Entry::key, static_cast<Base&>(object), input))
                success = false;
        }
        else
        {
            if (!impl::namedValueFromJson<TypeChecker>(Entry::key, Entry::value.get(object), input))
                success = false;
        }
    });
    return success;
}

} // namespace extensions
