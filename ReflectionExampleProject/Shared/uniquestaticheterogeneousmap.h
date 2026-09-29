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

namespace impl
{

struct UniqueStaticHeterogeneousMapElementTrueChecker
{
    template <typename...>
    using Filter = std::true_type;
};

template <typename TInKey, typename TTestKey>
struct UniqueStaticHeterogeneousMapElementKeyChecker
{
    template <typename /* Outer */, typename T, typename TKey, typename /* TValue */>
    using Filter = std::bool_constant<TKey::value == TTestKey::value>;
};

template <typename Outer, typename T, typename TypeChecker,
    typename F, std::size_t index, typename Tag>
void uniqueStaticHeterogeneousMapForEachImpl(Tag tag, F&& func)
{
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
        using Filter = typename TypeChecker::template Filter<Outer, T, Key, Value>;

        if constexpr (Filter::value)
        {
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
        }

        uniqueStaticHeterogeneousMapForEachImpl<Outer, T, TypeChecker, F, index + 1, Tag>(tag, std::forward<F>(func));
    }
}

template <typename Outer, typename T, typename TypeChecker,
    typename F, typename P, std::size_t index, typename Tag>
void uniqueStaticHeterogeneousMapForEachIfImpl(Tag tag, F&& func, P&& pred)
{
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
        using Filter = typename TypeChecker::template Filter<Outer, T, Key, Value>;

        if constexpr (Filter::value)
        {
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
        }

        uniqueStaticHeterogeneousMapForEachIfImpl<Outer, T, TypeChecker, F, P, index + 1, Tag>(tag, std::forward<F>(func), std::forward<P>(pred));
    }
}

} // namespace impl

/**
 * @brief perform a function for each element
 * @param tag empty lambda
 * @param func delegate to iterate over elements, can return void or bool, in such case false means break
 */
template <typename Outer, typename T,
    typename TypeChecker = impl::UniqueStaticHeterogeneousMapElementTrueChecker,
    typename F, typename Tag>
void uniqueStaticHeterogeneousMapForEach(Tag tag, F&& func)
{
    using CleanF = std::decay_t<F>;

    if constexpr (std::is_convertible_v<CleanF, std::nullptr_t>)
    {
        if (func == nullptr)
        {
            qCritical() << "uniqueStaticHeterogeneousMapForEach: empty func";
            return;
        }
    }

    impl::uniqueStaticHeterogeneousMapForEachImpl<Outer, T, TypeChecker, F, 0>(
        tag, std::forward<F>(func));
}

/**
 * @brief perform a function for each element that satisfies predicate condition
 * @param tag empty lambda
 * @param func delegate to iterate over elements, can return void or bool, in such case false means break
 * @param pred predicate, that defines which elements are passed to func (true) or skipped (false)
 */
template <typename Outer, typename T,
    typename TypeChecker = impl::UniqueStaticHeterogeneousMapElementTrueChecker,
    typename F, typename P, typename Tag>
void uniqueStaticHeterogeneousMapForEachIf(Tag tag, F&& func, P&& pred)
{
    using CleanF = std::decay_t<F>;

    if constexpr (std::is_convertible_v<CleanF, std::nullptr_t>)
    {
        if (func == nullptr)
        {
            qCritical() << "uniqueStaticHeterogeneousMapForEachIf: empty func";
            return;
        }
    }

    using CleanP = std::decay_t<P>;

    if constexpr (std::is_convertible_v<CleanP, std::nullptr_t>)
    {
        if (pred == nullptr)
        {
            qCritical() << "uniqueStaticHeterogeneousMapForEachIf: empty pred";
            return;
        }
    }

    impl::uniqueStaticHeterogeneousMapForEachIfImpl<Outer, T, TypeChecker, F, P, 0>(
        tag, std::forward<F>(func), std::forward<P>(pred));
}

template <typename Outer, typename T, typename F, typename Comparator = std::equal_to<void>, typename Tag>
void uniqueStaticHeterogeneousMapDoForKey(Tag tag, F&& func, T key, Comparator comparator = Comparator())
{
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

template <typename Outer, typename T, T key, typename F, typename Tag>
void uniqueStaticHeterogeneousMapDoForKey(Tag tag, F&& func)
{
    using KeyChecker = impl::UniqueStaticHeterogeneousMapElementKeyChecker<
        T, std::integral_constant<ArrayReturnTypeT<T>, key>>;
    uniqueStaticHeterogeneousMapForEach<Outer, T, KeyChecker>(tag, func);
}
