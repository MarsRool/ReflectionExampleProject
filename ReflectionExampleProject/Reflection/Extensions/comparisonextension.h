#pragma once
#include "Reflection/Utils/typetraits.h"
#include "Reflection/reflection.h"

// TODO: move out all extensions from namespace reflection and make everything outside of reflection require include reflection.h

namespace reflection::extensions
{

template <typename Outer,
          typename = std::enable_if_t<IsObject<Outer>::value>>
constexpr bool less(const Outer& value, const Outer& otherValue, TypeTag<Outer> = {})
{
    return compare(value, otherValue, TypeTag<Outer>{}) < 0;
}

template <typename Outer,
          typename = std::enable_if_t<IsObject<Outer>::value>>
constexpr bool lessEqual(const Outer& value, const Outer& otherValue, TypeTag<Outer> = {})
{
    return compare(value, otherValue, TypeTag<Outer>{}) <= 0;
}

template <typename Outer,
          typename = std::enable_if_t<IsObject<Outer>::value>>
constexpr bool greater(const Outer& value, const Outer& otherValue, TypeTag<Outer> = {})
{
    return compare(value, otherValue, TypeTag<Outer>{}) > 0;
}

template <typename Outer,
          typename = std::enable_if_t<IsObject<Outer>::value>>
constexpr bool greaterEqual(const Outer& value, const Outer& otherValue, TypeTag<Outer> = {})
{
    return compare(value, otherValue, TypeTag<Outer>{}) >= 0;
}

template <typename Outer,
          typename = std::enable_if_t<IsObject<Outer>::value>>
constexpr bool equal(const Outer& value, const Outer& otherValue, TypeTag<Outer> = {})
{
    return compare(value, otherValue, TypeTag<Outer>{}) == 0;
}

template <typename Outer,
          typename = std::enable_if_t<IsObject<Outer>::value>>
constexpr bool notEqual(const Outer& value, const Outer& otherValue, TypeTag<Outer> = {})
{
    return compare(value, otherValue, TypeTag<Outer>{}) != 0;
}


template <typename Outer,
          typename = std::enable_if_t<IsObject<Outer>::value>>
int compare(const Outer& value, const Outer& otherValue, TypeTag<Outer> = {})
{
    return comparePropertyMap(value, otherValue,
        TypeTag<Outer>{});
}

template <typename Outer, typename StaticPropertyT, const StaticPropertyProxy<Outer, StaticPropertyT>* staticPropertyProxyPtr>
int comparePropertyProxy(const typename StaticPropertyProxy<Outer, StaticPropertyT>::TargetOuterClass& outer,
    const typename StaticPropertyProxy<Outer, StaticPropertyT>::TargetOuterClass& otherOuter,
    PointerTag<staticPropertyProxyPtr>)
{
    static_assert(staticPropertyProxyPtr != nullptr);

    using ProxyClass = StaticPropertyProxy<Outer, StaticPropertyT>;
    using TargetStaticPropertyClass = typename ProxyClass::TargetStaticPropertyClass;
    using TargetOuterClass = typename StaticPropertyProxy<Outer, StaticPropertyT>::TargetOuterClass;

    constexpr const auto& targetStaticProperty = staticPropertyProxyPtr->staticProperty;

    if constexpr (IsSpecialization<TargetStaticPropertyClass, StaticProperty>::value)
    {
        return compareProperty(outer, otherOuter, PointerTag<&targetStaticProperty>{});
    }
    else if constexpr (IsSpecialization<TargetStaticPropertyClass, StaticPropertyMap>::value)
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
    using Meta = StaticPropertyMeta<Outer>;

    int result = 0;

    Meta::forEach(
        [&outer, &otherOuter, &result](auto, auto constValue)
    {
        constexpr const auto staticPropertyPtr = decltype(constValue)::value;
        if constexpr (staticPropertyPtr == nullptr)
        {
            return true;
        }

        using StaticPropertyClass = std::remove_cv_t<std::remove_pointer_t<decltype(staticPropertyPtr)>>;
        using PointerTagClass = PointerTag<staticPropertyPtr>;

        int tempResult;
        if constexpr (IsSpecialization<StaticPropertyClass, StaticProperty>::value)
        {
            tempResult = compareProperty(outer, otherOuter, PointerTagClass{});
        }
        else if constexpr (IsSpecialization<StaticPropertyClass, StaticPropertyMap>::value)
        {
            tempResult = comparePropertyMap(outer, otherOuter, TypeTag<Outer>{});
        }
        else if constexpr (IsSpecialization<StaticPropertyClass, StaticPropertyProxy>::value)
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

template <typename Outer, typename T, const StaticProperty<Outer, T>* staticPropertyPtr>
int compareProperty(const Outer& outer,
    const Outer& otherOuter,
    PointerTag<staticPropertyPtr>)
{
    static_assert(staticPropertyPtr != nullptr);

    constexpr const auto valuePtr = staticPropertyPtr->valuePtr;

    return compareValue(outer.*valuePtr, otherOuter.*valuePtr, TypeTag<const T>{});
}

template <typename T>
int compareValue(const T& value, const T& otherValue, TypeTag<const T>)
{
    using Type = std::remove_reference_t<T>;

    if constexpr (IsObject<Type>::value)
    {
        return compare(value, otherValue, TypeTag<Type>{});
    }
    else if constexpr (std::is_pointer_v<Type>
                       && IsObject<std::remove_pointer_t<Type>>::value)
    {
        using PlainType = std::remove_pointer_t<Type>;

        if (value != nullptr && otherValue != nullptr)
        {
            return compare(*value, *otherValue, TypeTag<PlainType>{});
        }

        return (value > otherValue) - (value < otherValue);
    }
    else if constexpr (IsString<Type>::value)
        return value.compare(otherValue);
    else if constexpr (std::disjunction_v<std::is_arithmetic<Type>, std::is_enum<Type>>)
        return (value > otherValue) - (value < otherValue);
    else if constexpr (IsIterable<Type>::value)
    {
        const std::size_t size = std::size(value);
        const std::size_t otherSize = std::size(otherValue);
        const std::size_t minSize = std::min(size, otherSize);

        auto iter = value.begin();
        auto otherIter = otherValue.begin();

        for (std::size_t index = 0;
             index < minSize;
             ++index, ++iter, ++otherIter)
        {
            using ItemType = std::remove_reference_t<decltype(*iter)>;

            int tempResult = compareValue(*iter, *otherIter, TypeTag<ItemType>{});
            if (tempResult != 0)
            {
                return tempResult;
            }
        }

        return (size > otherSize) - (size < otherSize);
    }
    else
    {
        static_assert(false, "compareValue: unexpected type");
        return 0;
    }
}

} // namespace reflection::extensions
