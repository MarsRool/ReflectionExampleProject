#pragma once
#include "Shared/typetester.h"

template <typename Outer, std::size_t index>
struct UniqueStaticHeterogeneousArrayElement
{
    template <typename ValueT, ValueT value>
    struct Generator
    {
        friend constexpr auto getDefinedValue(UniqueStaticHeterogeneousArrayElement)
        { return value; }
    };

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

    friend constexpr auto getDefinedValue(UniqueStaticHeterogeneousArrayElement);

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

    template <typename Tag = UniqueStaticHeterogeneousArrayElement, auto = getDefinedValue(Tag{})>
    static constexpr auto exists(std::size_t)
    { return true; }

    static constexpr auto exists(...)
    { return false; }

    template <typename ValueT, ValueT value, typename Tag = UniqueStaticHeterogeneousArrayElement, auto = getDefinedValue(Tag{})>
    static constexpr void define(std::size_t)
    {}

    template <typename ValueT, ValueT value>
    static constexpr void define(...)
    {
        Generator<ValueT, value>();
    }

    template <typename Tag = UniqueStaticHeterogeneousArrayElement, auto = getDefinedValue(Tag{})>
    static constexpr auto getValue(std::size_t)
    {
        return getDefinedValue(Tag{});
    }

    static constexpr auto getValue(...)
    {
        return nullptr;
    }
};

template <typename Outer, std::size_t index, typename Tag>
constexpr auto uniqueStaticHeterogeneousArrayExists(Tag)
{
    return UniqueStaticHeterogeneousArrayElement<Outer, index>::exists(index);
}

template <typename Outer, std::size_t index = 0, typename Tag>
constexpr auto uniqueStaticHeterogeneousArrayLength(Tag tag)
{
    if constexpr (!UniqueStaticHeterogeneousArrayElement<Outer, index>::exists(index))
        return index;
    else
        return uniqueStaticHeterogeneousArrayLength<Outer, index + 1, Tag>(tag);
}

template <typename Outer, std::size_t index, typename Tag>
constexpr auto uniqueStaticHeterogeneousArrayGetValue(Tag)
{
    return UniqueStaticHeterogeneousArrayElement<Outer, index>::getValue(index);
}

template <typename Outer, typename T, T value, typename Tag>
constexpr auto uniqueStaticHeterogeneousArrayPushBack(Tag tag)
{
    constexpr std::size_t insertIndex = uniqueStaticHeterogeneousArrayLength<Outer>(tag);
    UniqueStaticHeterogeneousArrayElement<Outer, insertIndex>::
        template define<T, value>(insertIndex);
    return value;
}
