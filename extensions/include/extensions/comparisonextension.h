#pragma once

#include <cmath>
#include <iterator>
#include <type_traits>

#include "reflection/reflection.h"
#include "extensions/extensionstypetraits.h"

namespace extensions
{

template <typename TypeChecker = reflection::TrueTypeChecker, typename Outer>
int compare(const Outer& value, const Outer& otherValue);

namespace impl
{

template <typename TypeChecker, typename T>
int compareValue(const T& left, const T& right)
{
    using Type = CleanType<T>;

    if constexpr (reflection::impl::IsReflected<Type>::value)
    {
        return compare<TypeChecker>(left, right);
    }
    else if constexpr (IsString<Type>::value ||
                       std::is_integral_v<Type> ||
                       std::is_enum_v<Type>)
    {
        return (left > right) - (left < right);
    }
    else if constexpr (std::is_floating_point_v<Type>)
    {
        const bool leftNan = std::isnan(left);
        const bool rightNan = std::isnan(right);
        if (leftNan || rightNan)
            return static_cast<int>(leftNan) - static_cast<int>(rightNan);
        return (left > right) - (left < right);
    }
    else if constexpr (std::is_pointer_v<Type>)
    {
        static_assert(AlwaysFalse<Type>::value, "Comparison extension does not support pointers");
    }
    else if constexpr (IsIterable<Type>::value)
    {
        auto first = begin(left);
        auto second = begin(right);
        const auto firstEnd = end(left);
        const auto secondEnd = end(right);

        while (first != firstEnd && second != secondEnd)
        {
            const int result = compareValue<TypeChecker>(*first, *second);
            if (result != 0)
                return result;
            ++first;
            ++second;
        }
        return static_cast<int>(first != firstEnd) - static_cast<int>(second != secondEnd);
    }
    else
    {
        static_assert(AlwaysFalse<Type>::value, "Unsupported type for comparison");
    }
}

} // namespace impl

template <typename TypeChecker, typename Outer>
int compare(const Outer& value, const Outer& otherValue)
{
    static_assert(reflection::impl::IsReflected<Outer>::value, "Type must be registered for reflection");

    int result = 0;
    reflection::StaticPropertyTraversal::forEach<TypeChecker>(value,
        [&](auto entry, const auto& currentObject) -> bool
    {
        using Entry = decltype(entry);
        if constexpr (!reflection::impl::IsBaseClassEntry<Entry>::value)
        {
            const auto& otherObject = static_cast<decltype(currentObject)>(otherValue);
            result = impl::compareValue<TypeChecker>(Entry::value.get(currentObject), Entry::value.get(otherObject));
        }
        return result == 0;
    });
    return result;
}

template <typename TypeChecker = reflection::TrueTypeChecker, typename Outer>
bool less(const Outer& value, const Outer& otherValue)
{
    return compare<TypeChecker>(value, otherValue) < 0;
}

template <typename TypeChecker = reflection::TrueTypeChecker, typename Outer>
bool lessEqual(const Outer& value, const Outer& otherValue)
{
    return compare<TypeChecker>(value, otherValue) <= 0;
}

template <typename TypeChecker = reflection::TrueTypeChecker, typename Outer>
bool greater(const Outer& value, const Outer& otherValue)
{
    return compare<TypeChecker>(value, otherValue) > 0;
}

template <typename TypeChecker = reflection::TrueTypeChecker, typename Outer>
bool greaterEqual(const Outer& value, const Outer& otherValue)
{
    return compare<TypeChecker>(value, otherValue) >= 0;
}

template <typename TypeChecker = reflection::TrueTypeChecker, typename Outer>
bool equal(const Outer& value, const Outer& otherValue)
{
    return compare<TypeChecker>(value, otherValue) == 0;
}

template <typename TypeChecker = reflection::TrueTypeChecker, typename Outer>
bool notEqual(const Outer& value, const Outer& otherValue)
{
    return compare<TypeChecker>(value, otherValue) != 0;
}

} // namespace extensions
