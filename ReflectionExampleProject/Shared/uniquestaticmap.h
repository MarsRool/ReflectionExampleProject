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
        friend consteval ValueType getDefinedValue(UniqueStaticMapElement)
        { return value; }
    };
    friend consteval ValueType getDefinedValue(UniqueStaticMapElement);

    template <typename Tag = UniqueStaticMapElement, auto = getDefinedValue(Tag{})>
    static consteval auto exists(auto)
    { return true; }

    static consteval auto exists(...)
    { return false; }

    template <U value, typename Tag = UniqueStaticMapElement, auto = getDefinedValue(Tag{})>
    static consteval void define()
    {}

    template <U value>
    static consteval void define(...)
    {
        Generator<value>();
    }

    static constexpr KeyType getKey()
    {
        return key;
    }

    template <typename Tag = UniqueStaticMapElement, auto = getDefinedValue(Tag{})>
    static constexpr ValueType getValue(auto)
    {
        return getDefinedValue(Tag{});
    }
    // TODO: maybe remove to make compile-time check of getting only defined value
    static constexpr ValueType getValue(...)
    {
        return ValueType{};
    }
};

template <typename Outer, typename T, typename U>
struct UniqueStaticMap
{};

template <typename Outer, typename T, typename U, typename = decltype([]{})>
consteval auto uniqueStaticMapKeysCount()
{
    return uniqueStaticArrayLength<UniqueStaticMap<Outer, T, U>, T>();
}

template <typename Outer, typename T, typename U, T key, typename = decltype([]{})>
consteval auto uniqueStaticMapExists()
{
    return UniqueStaticMapElement<Outer, T, U, key>::exists(key);
}

template <typename Outer, typename T, typename U, T key, typename = decltype([]{})>
consteval auto uniqueStaticMapGetValue()
{
    return UniqueStaticMapElement<Outer, T, U, key>::getValue(key);
}

template <typename Outer, typename T, typename U, T key, U value, typename = decltype([]{})>
consteval auto uniqueStaticMapAdd()
{
    if constexpr (!UniqueStaticMapElement<Outer, T, U, key>::exists(key))
    {
        uniqueStaticArrayPushBack<UniqueStaticMap<Outer, T, U>, T, key>();
        UniqueStaticMapElement<Outer, T, U, key>::template define<value>();
    }
    return value;
}

template <typename Outer, typename T, typename U, typename F, std::size_t index = 0>
inline void uniqueStaticMapForEach(F&& func)
{
    if constexpr (index >= uniqueStaticMapKeysCount<Outer, T, U>())
    {
        return;
    }
    else
    {
        constexpr auto key = uniqueStaticArrayGetValue<UniqueStaticMap<Outer, T, U>, T, index>();
        constexpr auto value = uniqueStaticMapGetValue<Outer, T, U, key>();
        func(key, value);
        uniqueStaticMapForEach<Outer, T, U, F, index + 1>(std::forward<F>(func));
    }
}

template <typename Outer, typename T, typename U, typename Comparator = std::equal_to<void>, std::size_t index = 0>
inline auto uniqueStaticMapGetValue(T key, Comparator comparator = Comparator())
{
    if constexpr (index >= uniqueStaticMapKeysCount<Outer, T, U>())
    {
        return U{};
    }
    else
    {
        constexpr auto currentKey = uniqueStaticArrayGetValue<UniqueStaticMap<Outer, T, U>, T, index>();
        if (comparator(key, currentKey))
        {
            return uniqueStaticMapGetValue<Outer, T, U, currentKey>();
        }
        else
        {
            return uniqueStaticMapGetValue<Outer, T, U, Comparator, index + 1>(key, comparator);
        }
    }
}
