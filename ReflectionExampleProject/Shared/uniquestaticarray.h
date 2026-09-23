#pragma once
#include "Shared/typetester.h"

template <typename Outer, typename T, std::size_t index>
struct UniqueStaticArrayElement
{
    using ValueType = ArrayReturnTypeT<T>;
    template <ValueType value>
    struct Generator
    {
        friend constexpr auto getDefinedValue(UniqueStaticArrayElement)
        { return value; }
    };

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

    friend constexpr auto getDefinedValue(UniqueStaticArrayElement);

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

    template <typename Tag = UniqueStaticArrayElement, auto = getDefinedValue(Tag{})>
    static constexpr auto exists(std::size_t)
    { return true; }

    static constexpr auto exists(...)
    { return false; }

    template <T value, typename Tag = UniqueStaticArrayElement, auto = getDefinedValue(Tag{})>
    static constexpr void define(std::size_t)
    {}

    template <T value>
    static constexpr void define(...)
    {
        Generator<value>();
    }

    template <typename Tag = UniqueStaticArrayElement, auto = getDefinedValue(Tag{})>
    static constexpr ValueType getValue(std::size_t)
    {
        return getDefinedValue(Tag{});
    }

    static constexpr ValueType getValue(...)
    {
        return ValueType{};
    }
};

template <typename Outer, typename T, std::size_t index, typename Tag>
constexpr auto uniqueStaticArrayExists(Tag)
{
    return UniqueStaticArrayElement<Outer, T, index>::exists(index);
}

template <typename Outer, typename T, std::size_t index = 0, typename Tag>
constexpr auto uniqueStaticArrayLength(Tag tag)
{
    if constexpr (!UniqueStaticArrayElement<Outer, T, index>::exists(index))
        return index;
    else
        return uniqueStaticArrayLength<Outer, T, index + 1, Tag>(tag);
}

template <typename Outer, typename T, std::size_t index, typename Tag>
constexpr auto uniqueStaticArrayGetValue(Tag)
{
    return UniqueStaticArrayElement<Outer, T, index>::getValue(index);
}

template <typename Outer, typename T, T value, typename Tag>
constexpr auto uniqueStaticArrayPushBack(Tag tag)
{
    constexpr std::size_t insertIndex = uniqueStaticArrayLength<Outer, T>(tag);
    UniqueStaticArrayElement<Outer, T, insertIndex>::
        template define<value>(insertIndex);
    return value;
}
