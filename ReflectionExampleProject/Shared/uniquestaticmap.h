#pragma once
#include "Shared/uniquestaticarray.h"

template <typename Outer, typename T, typename U, T key>
struct UniqueStaticMapElement
{
    using KeyType = ArrayReturnTypeT<T>;
    using ValueType = ArrayReturnTypeT<U>;
    template <ValueType value>
    struct Generator
    {
        friend constexpr auto getDefinedValue(UniqueStaticMapElement)
        { return value; }
    };

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"
#endif

    friend constexpr auto getDefinedValue(UniqueStaticMapElement);

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

    template <typename Tag = UniqueStaticMapElement, auto = getDefinedValue(Tag{})>
    static constexpr auto exists(T)
    { return true; }

    static constexpr auto exists(...)
    { return false; }

    template <U value, typename Tag = UniqueStaticMapElement, auto = getDefinedValue(Tag{})>
    static constexpr void define()
    {}

    template <U value>
    static constexpr void define(...)
    {
        Generator<value>();
    }

    static constexpr KeyType getKey()
    {
        return key;
    }

    template <typename Tag = UniqueStaticMapElement, auto = getDefinedValue(Tag{})>
    static constexpr ValueType getValue(T)
    {
        return getDefinedValue(Tag{});
    }

    static constexpr ValueType getValue(...)
    {
        return ValueType{};
    }
};

template <typename Outer, typename T, typename U>
struct UniqueStaticMap
{};

template <typename Outer, typename T, typename U, typename Tag>
constexpr auto uniqueStaticMapKeysCount(Tag tag)
{
    return uniqueStaticArrayLength<UniqueStaticMap<Outer, T, U>, T>(tag);
}

template <typename Outer, typename T, typename U, T key, typename Tag>
constexpr auto uniqueStaticMapExists(Tag)
{
    return UniqueStaticMapElement<Outer, T, U, key>::exists(key);
}

template <typename Outer, typename T, typename U, T key, typename Tag>
constexpr auto uniqueStaticMapGetValue(Tag)
{
    return UniqueStaticMapElement<Outer, T, U, key>::getValue(key);
}

template <typename Outer, typename T, typename U, T key, U value, typename Tag>
constexpr auto uniqueStaticMapAdd(Tag tag)
{
    if constexpr (!UniqueStaticMapElement<Outer, T, U, key>::exists(key))
    {
        uniqueStaticArrayPushBack<UniqueStaticMap<Outer, T, U>, T, key>(tag);
        UniqueStaticMapElement<Outer, T, U, key>::template define<value>();
    }
    return value;
}

template <typename Outer, typename T, typename U, typename F, std::size_t index = 0, typename Tag>
inline void uniqueStaticMapForEach(Tag tag, F&& func)
{
    if constexpr (index >= uniqueStaticMapKeysCount<Outer, T, U>(tag))
    {
        return;
    }
    else
    {
        constexpr auto key = uniqueStaticArrayGetValue<UniqueStaticMap<Outer, T, U>, T, index>(tag);
        constexpr auto value = uniqueStaticMapGetValue<Outer, T, U, key>(tag);
        func(key, value);
        uniqueStaticMapForEach<Outer, T, U, F, index + 1, Tag>(tag, std::forward<F>(func));
    }
}

template <typename Outer, typename T, typename U, typename Comparator = std::equal_to<void>, std::size_t index = 0, typename Tag>
inline auto uniqueStaticMapFindValue(Tag tag, T key, Comparator comparator = Comparator())
{
    if constexpr (index >= uniqueStaticMapKeysCount<Outer, T, U>(tag))
    {
        return U{};
    }
    else
    {
        constexpr auto currentKey = uniqueStaticArrayGetValue<UniqueStaticMap<Outer, T, U>, T, index>(tag);
        if (comparator(key, currentKey))
        {
            return uniqueStaticMapGetValue<Outer, T, U, currentKey>(tag);
        }
        else
        {
            return uniqueStaticMapFindValue<Outer, T, U, Comparator, index + 1, Tag>(tag, key, comparator);
        }
    }
}
