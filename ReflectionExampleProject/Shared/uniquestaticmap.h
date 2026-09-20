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
void uniqueStaticMapForEach(Tag tag, F&& func)
{
    // TODO: decompose and add compile-time versions for func
    // TODO: split on wrapper and add nullptr check for func
    if constexpr (index >= uniqueStaticMapKeysCount<Outer, T, U>(tag))
    {
        return;
    }
    else
    {
        constexpr auto key = uniqueStaticArrayGetValue<UniqueStaticMap<Outer, T, U>, T, index>(tag);
        constexpr auto value = uniqueStaticMapGetValue<Outer, T, U, key>(tag);
        using Key = std::integral_constant<decltype(key), key>;
        using Value = std::integral_constant<decltype(value), value>;
        using FuncRet = decltype(func(Key{}, Value{}));

        if constexpr (std::is_same_v<FuncRet, bool>)
        {
            if (!func(Key{}, Value{}))
            {
                return;
            }
        }
        else if constexpr (std::is_void_v<FuncRet>)
        {
            func(Key{}, Value{});
        }
        else
        {
            static_assert(false, "Unexpected func return type");
        }

        uniqueStaticMapForEach<Outer, T, U, F, index + 1, Tag>(tag, std::forward<F>(func));
    }
}

template <typename Outer, typename T, typename U, typename F, typename P, std::size_t index = 0, typename Tag>
void uniqueStaticMapForEachIf(Tag tag, F&& func, P&& pred)
{
    // TODO: decompose and add compile-time versions for func and pred
    // TODO: split on wrapper and add nullptr check for func and pred
    if constexpr (index >= uniqueStaticMapKeysCount<Outer, T, U>(tag))
    {
        return;
    }
    else
    {
        constexpr auto key = uniqueStaticArrayGetValue<UniqueStaticMap<Outer, T, U>, T, index>(tag);
        constexpr auto value = uniqueStaticMapGetValue<Outer, T, U, key>(tag);
        using Key = std::integral_constant<decltype(key), key>;
        using Value = std::integral_constant<decltype(value), value>;
        using FuncRet = decltype(func(Key{}, Value{}));

        if (pred(Key{}, Value{}))
        {
            if constexpr (std::is_same_v<FuncRet, bool>)
            {
                if (!func(Key{}, Value{}))
                {
                    return;
                }
            }
            else if constexpr (std::is_void_v<FuncRet>)
            {
                func(Key{}, Value{});
            }
            else
            {
                static_assert(false, "Unexpected func return type");
            }
        }

        uniqueStaticMapForEachIf<Outer, T, U, F, P, index + 1, Tag>(tag, std::forward<F>(func), std::forward<P>(pred));
    }
}

template <typename Outer, typename T, typename U, typename F, typename Comparator = std::equal_to<void>, std::size_t index = 0, typename Tag>
void uniqueStaticMapDoForKey(Tag tag, F&& func, T key, Comparator comparator = Comparator())
{
    // TODO: decompose and add compile-time version for key as NTTP
    uniqueStaticMapForEachIf<Outer, T, U>(tag, [&func](auto constKey, auto constValue)
    {
        func(constKey, constValue);
        return false;
    }, [&key, &comparator](auto constKey, auto)
    {
        constexpr auto currentKey = decltype(constKey)::value;
        return comparator(key, currentKey);
    });
}
