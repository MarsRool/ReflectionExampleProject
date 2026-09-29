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
#pragma GCC diagnostic ignored "-Wunused-function"
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
    static constexpr void define(T)
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
        UniqueStaticMapElement<Outer, T, U, key>::template define<value>(key);
    }
    return value;
}

namespace impl
{

struct UniqueStaticMapElementTrueChecker
{
    template <typename...>
    using Filter = std::true_type;
};

template <typename TInKey, typename TTestKey>
struct UniqueStaticMapElementKeyChecker
{
    template <typename /* Outer */, typename T, typename /* U */, typename TKey, typename /* TValue */>
    using Filter = std::bool_constant<TKey::value == TTestKey::value>;
};

template <typename Outer, typename T, typename U, typename TypeChecker,
    typename F, std::size_t index, typename Tag>
void uniqueStaticMapForEachImpl(Tag tag, F&& func)
{
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
        using Filter = typename TypeChecker::template Filter<Outer, T, U, Key, Value>;

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

        uniqueStaticMapForEachImpl<Outer, T, U, TypeChecker, F, index + 1, Tag>(tag, std::forward<F>(func));
    }
}

template <typename Outer, typename T, typename U, typename TypeChecker,
    typename F, typename P, std::size_t index, typename Tag>
void uniqueStaticMapForEachIfImpl(Tag tag, F&& func, P&& pred)
{
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
        using Filter = typename TypeChecker::template Filter<Outer, T, U, Key, Value>;

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

        uniqueStaticMapForEachIfImpl<Outer, T, U, TypeChecker, F, P, index + 1, Tag>(tag, std::forward<F>(func), std::forward<P>(pred));
    }
}

} // namespace impl

/**
 * @brief perform a function for each element
 * @param tag empty lambda
 * @param func delegate to iterate over elements, can return void or bool, in such case false means break
 */
template <typename Outer, typename T, typename U,
    typename TypeChecker = impl::UniqueStaticMapElementTrueChecker,
    typename F, typename Tag>
void uniqueStaticMapForEach(Tag tag, F&& func)
{
    using CleanF = std::decay_t<F>;

    if constexpr (std::is_convertible_v<CleanF, std::nullptr_t>)
    {
        if (func == nullptr)
        {
            qCritical() << "uniqueStaticMapForEach: empty func";
            return;
        }
    }

    return impl::uniqueStaticMapForEachImpl<Outer, T, U, TypeChecker, F, 0>(tag, std::forward<F>(func));
}

/**
 * @brief perform a function for each element that satisfies predicate condition
 * @param tag empty lambda
 * @param func delegate to iterate over elements, can return void or bool, in such case false means break
 * @param pred predicate, that defines which elements are passed to func (true) or skipped (false)
 */
template <typename Outer, typename T, typename U,
    typename TypeChecker = impl::UniqueStaticMapElementTrueChecker,
    typename F, typename P, typename Tag>
void uniqueStaticMapForEachIf(Tag tag, F&& func, P&& pred)
{
    using CleanF = std::decay_t<F>;

    if constexpr (std::is_convertible_v<CleanF, std::nullptr_t>)
    {
        if (func == nullptr)
        {
            qCritical() << "uniqueStaticMapForEachIf: empty func";
            return;
        }
    }

    using CleanP = std::decay_t<P>;

    if constexpr (std::is_convertible_v<CleanP, std::nullptr_t>)
    {
        if (pred == nullptr)
        {
            qCritical() << "uniqueStaticMapForEachIf: empty pred";
            return;
        }
    }

    impl::uniqueStaticMapForEachIfImpl<Outer, T, U, TypeChecker, F, P, 0>(tag, std::forward<F>(func), std::forward<P>(pred));
}

template <typename Outer, typename T, typename U, typename F, typename Comparator = std::equal_to<void>, typename Tag>
void uniqueStaticMapDoForKey(Tag tag, F&& func, T key, Comparator comparator = Comparator())
{
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

template <typename Outer, typename T, typename U, T key, typename F, typename Tag>
void uniqueStaticMapDoForKey(Tag tag, F&& func)
{
    using KeyChecker = impl::UniqueStaticMapElementKeyChecker<
        T, std::integral_constant<ArrayReturnTypeT<T>, key>>;
    uniqueStaticMapForEach<Outer, T, U, KeyChecker>(tag, func);
}
