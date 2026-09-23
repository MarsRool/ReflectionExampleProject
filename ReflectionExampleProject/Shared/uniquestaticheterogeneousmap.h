#pragma once
#include "Shared/uniquestaticheterogeneousarray.h"

template <typename Outer, typename KeyT, KeyT key>
struct UniqueStaticHeterogeneousMapElement
{
    using KeyType = ArrayReturnTypeT<KeyT>;
    template <typename ValueT, ValueT value>
    struct Generator
    {
        friend constexpr auto getDefinedValue(UniqueStaticHeterogeneousMapElement)
        { return value; }
    };

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

    friend constexpr auto getDefinedValue(UniqueStaticHeterogeneousMapElement);

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

    template <typename Tag = UniqueStaticHeterogeneousMapElement, auto = getDefinedValue(Tag{})>
    static constexpr auto exists(KeyT)
    { return true; }

    static constexpr auto exists(...)
    { return false; }

    template <typename ValueT, ValueT value, typename Tag = UniqueStaticHeterogeneousMapElement, auto = getDefinedValue(Tag{})>
    static constexpr void define(KeyT)
    {}

    template <typename ValueT, ValueT value>
    static constexpr void define(...)
    {
        Generator<ValueT, value>();
    }

    static constexpr KeyType getKey()
    {
        return key;
    }

    template <typename Tag = UniqueStaticHeterogeneousMapElement, auto = getDefinedValue(Tag{})>
    static constexpr auto getValue(KeyT)
    {
        return getDefinedValue(Tag{});
    }

    static constexpr auto getValue(...)
    {
        return nullptr;
    }
};

template <typename Outer, typename T>
struct UniqueStaticHeterogeneousMap
{};

template <typename Outer, typename T, typename Tag>
constexpr auto uniqueStaticHeterogeneousMapKeysCount(Tag tag)
{
    return uniqueStaticHeterogeneousArrayLength<UniqueStaticHeterogeneousMap<Outer, T>>(tag);
}

template <typename Outer, typename T, T key, typename Tag>
constexpr auto uniqueStaticHeterogeneousMapExists(Tag)
{
    return UniqueStaticHeterogeneousMapElement<Outer, T, key>::exists(key);
}

template <typename Outer, typename T, T key, typename Tag>
constexpr auto uniqueStaticHeterogeneousMapGetValue(Tag)
{
    return UniqueStaticHeterogeneousMapElement<Outer, T, key>::getValue(key);
}

template <typename Outer, typename T, typename U, T key, U value, typename Tag>
constexpr auto uniqueStaticHeterogeneousMapAdd(Tag tag)
{
    if constexpr (!UniqueStaticHeterogeneousMapElement<Outer, T, key>::exists(key))
    {
        uniqueStaticHeterogeneousArrayPushBack<UniqueStaticHeterogeneousMap<Outer, T>, T, key>(tag);
        UniqueStaticHeterogeneousMapElement<Outer, T, key>::template define<U, value>();
    }
    return value;
}

template <typename Outer, typename T, typename F, std::size_t index = 0, typename Tag>
void uniqueStaticHeterogeneousMapForEach(Tag tag, F&& func)
{
    // TODO: decompose and add compile-time versions for func
    // TODO: split on wrapper and add nullptr check for func
    if constexpr (index >= uniqueStaticHeterogeneousMapKeysCount<Outer, T>(tag))
    {
        return;
    }
    else
    {
        constexpr auto key = uniqueStaticHeterogeneousArrayGetValue<UniqueStaticHeterogeneousMap<Outer, T>, index>(tag);
        constexpr auto value = uniqueStaticHeterogeneousMapGetValue<Outer, T, key>(tag);
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

        uniqueStaticHeterogeneousMapForEach<Outer, T, F, index + 1, Tag>(tag, std::forward<F>(func));
    }
}

template <typename Outer, typename T, typename F, typename P, std::size_t index = 0, typename Tag>
void uniqueStaticHeterogeneousMapForEachIf(Tag tag, F&& func, P&& pred)
{
    // TODO: decompose and add compile-time versions for func and pred
    // TODO: split on wrapper and add nullptr check for func and pred
    if constexpr (index >= uniqueStaticHeterogeneousMapKeysCount<Outer, T>(tag))
    {
        return;
    }
    else
    {
        constexpr auto key = uniqueStaticHeterogeneousArrayGetValue<UniqueStaticHeterogeneousMap<Outer, T>, index>(tag);
        constexpr auto value = uniqueStaticHeterogeneousMapGetValue<Outer, T, key>(tag);
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

        uniqueStaticHeterogeneousMapForEachIf<Outer, T, F, P, index + 1, Tag>(tag, std::forward<F>(func), std::forward<P>(pred));
    }
}

template <typename Outer, typename T, typename F, typename Comparator = std::equal_to<void>, std::size_t index = 0, typename Tag>
void uniqueStaticHeterogeneousMapDoForKey(Tag tag, F&& func, T key, Comparator comparator = Comparator())
{
    // TODO: decompose and add compile-time version for key as NTTP
    uniqueStaticHeterogeneousMapForEachIf<Outer, T>(tag, [&func](auto constKey, auto constValue)
    {
        func(constKey, constValue);
        return false;
    }, [&key, &comparator](auto constKey, auto)
    {
        constexpr auto currentKey = decltype(constKey)::value;
        return comparator(key, currentKey);
    });
}
