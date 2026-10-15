#pragma once

#include <cmath>
#include <iterator>
#include <string>
#include <type_traits>

#include "reflection/reflection.h"
#include "extensions/extensionstypetraits.h"

namespace extensions
{

template <typename Outer,
          typename = std::enable_if_t<reflection::IsObject<Outer>::value>>
bool less(const Outer& value, const Outer& otherValue, TypeTag<Outer> = {})
{
    return compare(value, otherValue, TypeTag<Outer>{}) < 0;
}

template <typename Outer,
          typename = std::enable_if_t<reflection::IsObject<Outer>::value>>
bool lessEqual(const Outer& value, const Outer& otherValue, TypeTag<Outer> = {})
{
    return compare(value, otherValue, TypeTag<Outer>{}) <= 0;
}

template <typename Outer,
          typename = std::enable_if_t<reflection::IsObject<Outer>::value>>
bool greater(const Outer& value, const Outer& otherValue, TypeTag<Outer> = {})
{
    return compare(value, otherValue, TypeTag<Outer>{}) > 0;
}

template <typename Outer,
          typename = std::enable_if_t<reflection::IsObject<Outer>::value>>
bool greaterEqual(const Outer& value, const Outer& otherValue, TypeTag<Outer> = {})
{
    return compare(value, otherValue, TypeTag<Outer>{}) >= 0;
}

template <typename Outer,
          typename = std::enable_if_t<reflection::IsObject<Outer>::value>>
bool equal(const Outer& value, const Outer& otherValue, TypeTag<Outer> = {})
{
    return compare(value, otherValue, TypeTag<Outer>{}) == 0;
}

template <typename Outer,
          typename = std::enable_if_t<reflection::IsObject<Outer>::value>>
bool notEqual(const Outer& value, const Outer& otherValue, TypeTag<Outer> = {})
{
    return compare(value, otherValue, TypeTag<Outer>{}) != 0;
}


template <typename Outer,
          typename = std::enable_if_t<reflection::IsObject<Outer>::value>>
int compare(const Outer& value, const Outer& otherValue, TypeTag<Outer> = {})
{
    return comparePropertyMap(value, otherValue,
        TypeTag<Outer>{});
}

template <typename Outer, typename StaticPropertyT, const reflection::StaticPropertyProxy<Outer, StaticPropertyT>* staticPropertyProxyPtr>
int comparePropertyProxy(const typename reflection::StaticPropertyProxy<Outer, StaticPropertyT>::TargetOuterClass& outer,
    const typename reflection::StaticPropertyProxy<Outer, StaticPropertyT>::TargetOuterClass& otherOuter,
    PointerTag<staticPropertyProxyPtr>)
{
    static_assert(staticPropertyProxyPtr != nullptr);

    using ProxyClass = reflection::StaticPropertyProxy<Outer, StaticPropertyT>;
    using TargetStaticPropertyClass = typename ProxyClass::TargetStaticPropertyClass;
    using TargetOuterClass = typename reflection::StaticPropertyProxy<Outer, StaticPropertyT>::TargetOuterClass;

    constexpr const auto& targetStaticProperty = staticPropertyProxyPtr->staticProperty;

    if constexpr (reflection::IsSpecialization<TargetStaticPropertyClass, reflection::StaticProperty>::value)
    {
        return compareProperty(outer, otherOuter, PointerTag<&targetStaticProperty>{});
    }
    else if constexpr (reflection::IsSpecialization<TargetStaticPropertyClass, reflection::StaticPropertyMap>::value)
    {
        return comparePropertyMap(outer, otherOuter, TypeTag<TargetOuterClass>{});
    }
    else
    {
        static_assert(false, "comparePropertyProxy: unexpected static property type");
        return 0;
    }
}

template <typename Outer>
int comparePropertyMap(const Outer& outer,
    const Outer& otherOuter,
    TypeTag<Outer>)
{
    using Meta = reflection::StaticPropertyMeta<Outer>;

    int result = 0;

    Meta::forEach(
        [&outer, &otherOuter, &result](auto, auto constValue)
    {
        constexpr auto staticPropertyPtr = decltype(constValue)::value;
        if constexpr (staticPropertyPtr == nullptr)
        {
            return true;
        }

        using StaticPropertyClass = std::remove_cv_t<std::remove_pointer_t<decltype(staticPropertyPtr)>>;
        using PointerTagClass = PointerTag<staticPropertyPtr>;

        int tempResult;
        if constexpr (reflection::IsSpecialization<StaticPropertyClass, reflection::StaticProperty>::value)
        {
            tempResult = compareProperty(outer, otherOuter, PointerTagClass{});
        }
        else if constexpr (reflection::IsSpecialization<StaticPropertyClass, reflection::StaticPropertyMap>::value)
        {
            tempResult = comparePropertyMap(outer, otherOuter, TypeTag<Outer>{});
        }
        else if constexpr (reflection::IsSpecialization<StaticPropertyClass, reflection::StaticPropertyProxy>::value)
        {
            tempResult = comparePropertyProxy(outer, otherOuter, PointerTagClass{});
        }
        else
        {
            static_assert(false, "comparePropertyMap: unexpected static property type");
            return true;
        }

        if (tempResult != 0)
        {
            result = tempResult;
            return false;
        }

        return true;
    });

    return result;
}

template <typename Outer, typename T, const reflection::StaticProperty<Outer, T>* staticPropertyPtr>
int compareProperty(const Outer& outer,
    const Outer& otherOuter,
    PointerTag<staticPropertyPtr>)
{
    static_assert(staticPropertyPtr != nullptr);

    constexpr auto valuePtr = staticPropertyPtr->valuePtr;

    return compareValue(outer.*valuePtr, otherOuter.*valuePtr, TypeTag<const T>{});
}

template <typename T>
int compareValue(const T& value, const T& otherValue, TypeTag<const T>)
{
    using Type = std::remove_reference_t<T>;

    if constexpr (reflection::IsObject<Type>::value)
    {
        return compare(value, otherValue, TypeTag<Type>{});
    }
    else if constexpr (std::is_pointer_v<Type>
                       && reflection::IsObject<std::remove_pointer_t<Type>>::value)
    {
        using PlainType = std::remove_cv_t<std::remove_pointer_t<Type>>;

        if (value == nullptr)
            return otherValue == nullptr ? 0 : -1;
        if (otherValue == nullptr)
            return 1;
        return compare(*value, *otherValue, TypeTag<PlainType>{});
    }
    else if constexpr (reflection::IsString<Type>::value)
        return value.compare(otherValue);
    else if constexpr (std::is_floating_point_v<Type>)
    {
        // Total ordering: NaN compares equal to NaN and greater than infinity.
        const bool leftNan = std::isnan(value);
        const bool rightNan = std::isnan(otherValue);
        if (leftNan || rightNan)
            return static_cast<int>(leftNan) - static_cast<int>(rightNan);
        return (value > otherValue) - (value < otherValue);
    }
    else if constexpr (std::disjunction_v<std::is_integral<Type>, std::is_enum<Type>>)
        return (value > otherValue) - (value < otherValue);
    else if constexpr (reflection::IsIterable<Type>::value)
    {
        auto iter = std::begin(value);
        auto otherIter = std::begin(otherValue);
        const auto end = std::end(value);
        const auto otherEnd = std::end(otherValue);

        while (iter != end && otherIter != otherEnd)
        {
            using ItemType = std::remove_reference_t<decltype(*iter)>;
            const int tempResult = compareValue(*iter, *otherIter, TypeTag<ItemType>{});
            if (tempResult != 0)
                return tempResult;
            ++iter;
            ++otherIter;
        }

        return static_cast<int>(iter != end) - static_cast<int>(otherIter != otherEnd);
    }
    else
    {
        static_assert(false, "compareValue: unexpected type");
        return 0;
    }
}

} // namespace extensions
