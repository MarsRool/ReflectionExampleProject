#pragma once
#include "Shared/typetester.h"

template <typename Outer, typename T, std::size_t index>
struct UniqueStaticArrayElement
{
    using ValueType = ArrayReturnTypeT<T>;
    template <ValueType value>
    struct Generator
    {
        friend consteval ValueType getDefinedValue(UniqueStaticArrayElement)
        { return value; }
    };
    friend consteval ValueType getDefinedValue(UniqueStaticArrayElement);

    template <typename Tag = UniqueStaticArrayElement, auto = getDefinedValue(Tag{})>
    static consteval auto exists(auto)
    { return true; }

    static consteval auto exists(...)
    { return false; }

    template <T value, typename Tag = UniqueStaticArrayElement, auto = getDefinedValue(Tag{})>
    static consteval void define()
    {}

    template <T value>
    static consteval void define(...)
    {
        Generator<value>();
    }

    template <typename Tag = UniqueStaticArrayElement, auto = getDefinedValue(Tag{})>
    static consteval ValueType getValue(auto)
    {
        return getDefinedValue(Tag{});
    }
    // TODO: maybe remove to make compile-time check of getting only defined value
    static consteval ValueType getValue(...)
    {
        return ValueType{};
    }
};

template <typename Outer, typename T, std::size_t index, typename = decltype([]{})>
consteval auto uniqueStaticArrayExists()
{
    return UniqueStaticArrayElement<Outer, T, index>::exists(index);
}

template <typename Outer, typename T, std::size_t index = 0, typename = decltype([]{})>
consteval auto uniqueStaticArrayLength()
{
    if constexpr (!UniqueStaticArrayElement<Outer, T, index>::exists(index))
        return index;
    else
        return uniqueStaticArrayLength<Outer, T, index + 1>();
}

template <typename Outer, typename T, std::size_t index, typename = decltype([]{})>
consteval auto uniqueStaticArrayGetValue()
{
    return UniqueStaticArrayElement<Outer, T, index>::getValue(index);
}

template <typename Outer, typename T, T value, typename = decltype([]{})>
consteval auto uniqueStaticArrayPushBack()
{
    UniqueStaticArrayElement<Outer, T, uniqueStaticArrayLength<Outer, T>()>::
        template define<value>();
    return value;
}
