#pragma once

#include <cstddef>
#include <memory>
#include <string_view>
#include <type_traits>

#include "reflection/property/staticpropertymeta.h"

namespace reflection
{

struct StaticPropertyTraversal
{
    template <typename Outer, typename TypeChecker = TrueTypeChecker, typename F>
    static constexpr void forEach(F&& func)
    {
        if (!impl::isValidCallable(func))
            return;

        visit<Outer, TypeChecker>(func);
    }

    template <typename TypeChecker = TrueTypeChecker, typename Outer, typename F>
    static constexpr void forEach(Outer& object, F&& func)
    {
        if (!impl::isValidCallable(func))
            return;

        visit<std::remove_cv_t<Outer>, TypeChecker>(func, nullptr, std::addressof(object));
    }

    template <typename Outer, typename TypeChecker = TrueTypeChecker, typename F, typename P>
    static constexpr void forEachIf(F&& func, P&& pred)
    {
        if (!impl::isValidCallable(func) || !impl::isValidCallable(pred))
            return;

        visit<Outer, TypeChecker>(func, std::addressof(pred));
    }

    template <typename TypeChecker = TrueTypeChecker, typename Outer, typename F, typename P>
    static constexpr void forEachIf(Outer& object, F&& func, P&& pred)
    {
        if (!impl::isValidCallable(func) || !impl::isValidCallable(pred))
            return;

        visit<std::remove_cv_t<Outer>, TypeChecker>(func, std::addressof(pred), std::addressof(object));
    }

    template <typename Outer, typename Comparator = PropertyNameEqual,
              typename TypeChecker = TrueTypeChecker, typename F>
    static constexpr void doForKey(F&& func, const char* key,
                                   Comparator comparator = Comparator{})
    {
        if (!key || !impl::isValidCallable(func) || !impl::isValidCallable(comparator))
            return;

        forEach<Outer, TypeChecker>([&](auto entryTag) -> bool
        {
            if (!comparator(decltype(entryTag)::key, key))
                return true;

            func(entryTag);
            return false;
        });
    }

    template <typename Comparator = PropertyNameEqual,
              typename TypeChecker = TrueTypeChecker, typename Outer, typename F>
    static constexpr void doForKey(Outer& object, F&& func, const char* key,
                                   Comparator comparator = Comparator{})
    {
        if (!key || !impl::isValidCallable(func) || !impl::isValidCallable(comparator))
            return;

        forEach<TypeChecker>(object, [&](auto entryTag, auto& currentObject) -> bool
        {
            if (!comparator(decltype(entryTag)::key, key))
                return true;

            func(entryTag, currentObject);
            return false;
        });
    }

    template <typename Outer, const char* key,
              typename TypeChecker = TrueTypeChecker, typename F>
    static constexpr void doForKey(F&& func)
    {
        if (!impl::isValidCallable(func))
            return;

        forEach<Outer, TypeChecker>([&](auto entryTag) -> bool
        {
            if constexpr (std::string_view(decltype(entryTag)::key) == std::string_view(key))
            {
                func(entryTag);
                return false;
            }

            return true;
        });
    }

    template <const char* key, typename TypeChecker = TrueTypeChecker,
              typename Outer, typename F>
    static constexpr void doForKey(Outer& object, F&& func)
    {
        if (!impl::isValidCallable(func))
            return;

        forEach<TypeChecker>(object, [&](auto entryTag, auto& currentObject) -> bool
        {
            if constexpr (std::string_view(decltype(entryTag)::key) == std::string_view(key))
            {
                func(entryTag, currentObject);
                return false;
            }

            return true;
        });
    }

private:
    template <typename Outer, typename TypeChecker, typename F, typename P = std::nullptr_t,
              typename Object = std::nullptr_t>
    static constexpr bool visit(F& func, P pred = nullptr, Object* object = nullptr)
    {
        bool proceed = true;

        StaticPropertyMeta<Outer>::template forEach<TypeChecker>(
            [&](auto entryTag) -> bool
        {
            using Entry = std::remove_cv_t<std::remove_reference_t<decltype(entryTag)>>;

            if constexpr (std::is_same_v<Object, std::nullptr_t>)
            {
                if constexpr (!std::is_same_v<P, std::nullptr_t>)
                {
                    if (!(*pred)(Entry{}))
                        return true;
                }

                using ReturnType = std::invoke_result_t<F&, Entry>;
                static_assert(std::is_void_v<ReturnType> || std::is_same_v<ReturnType, bool>,
                    "Reflection visitor func must return void or bool");

                if constexpr (std::is_same_v<ReturnType, bool>)
                {
                    if (!func(Entry{}))
                    {
                        proceed = false;
                        return false;
                    }
                }
                else
                {
                    func(Entry{});
                }
            }
            else
            {
                if constexpr (!std::is_same_v<P, std::nullptr_t>)
                {
                    if (!(*pred)(Entry{}, *object))
                        return true;
                }

                using ReturnType = std::invoke_result_t<F&, Entry, Object&>;
                static_assert(std::is_void_v<ReturnType> || std::is_same_v<ReturnType, bool>,
                    "Reflection visitor func must return void or bool");

                if constexpr (std::is_same_v<ReturnType, bool>)
                {
                    if (!func(Entry{}, *object))
                    {
                        proceed = false;
                        return false;
                    }
                }
                else
                {
                    func(Entry{}, *object);
                }
            }

            if constexpr (impl::IsBaseClassEntry<Entry>::value)
            {
                using ValueType = std::remove_cv_t<std::remove_reference_t<decltype(Entry::value)>>;
                using Base = typename ValueType::OuterType;

                if constexpr (std::is_same_v<Object, std::nullptr_t>)
                {
                    proceed = visit<Base, TypeChecker>(func, pred);
                }
                else
                {
                    using BaseRef = std::conditional_t<std::is_const_v<Object>, const Base&, Base&>;
                    auto& baseObject = static_cast<BaseRef>(*object);
                    proceed = visit<Base, TypeChecker>(func, pred, std::addressof(baseObject));
                }
            }

            return proceed;
        });

        return proceed;
    }
};

} // namespace reflection
