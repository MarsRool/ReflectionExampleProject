#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "reflection/utils/typetraits.h"
#include "reflection/property/intrusivelinescanner.h"
#include "reflection/property/staticproperty.h"
#include "reflection/property/statictypeinfo.h"

namespace reflection
{

namespace impl
{

template <typename F>
constexpr bool isValidCallable(F& func)
{
    if constexpr (IsNullableCallable<F>::value)
        return !(func == nullptr);
    else
        return true;
}

} // namespace impl

struct TrueTypeChecker
{
    template <typename...>
    using Filter = std::true_type;
};

struct PropertyNameEqual
{
    constexpr bool operator()(const char* left, const char* right) const noexcept
    {
        return left && right && std::string_view(left) == std::string_view(right);
    }
};

template <typename Outer>
struct StaticPropertyMeta
{
    using OuterType = Outer;
    using KeyType = const char*;
    using DefaultTypeChecker = TrueTypeChecker;

    static constexpr bool hasIntrusive = impl::HasIntrusive<Outer>::value;
    static constexpr bool hasNonIntrusive = impl::HasNonIntrusive<Outer>::value;

    static_assert(!(hasIntrusive && hasNonIntrusive),
        "A type cannot have both intrusive and non-intrusive reflection");
    static_assert(hasIntrusive || hasNonIntrusive,
        "No reflection registration found for this type");

    static constexpr auto getStaticTypeInfo()
    {
        if constexpr (hasIntrusive)
            return z_reflection_type_info(TypeTag<Outer>{});
        else
            return z_reflection_type_info_nonintrusive(TypeTag<Outer>{});
    }

    static constexpr auto makeProperties()
    {
        if constexpr (hasIntrusive)
        {
            return IntrusiveLineScanner<Outer>::makeTuple();
        }
        else if constexpr (impl::HasNonIntrusiveBases<Outer>::value)
        {
            return std::tuple_cat(
                z_reflection_base_class_properties_nonintrusive(TypeTag<Outer>{}),
                z_reflection_properties_nonintrusive(TypeTag<Outer>{}));
        }
        else
        {
            return z_reflection_properties_nonintrusive(TypeTag<Outer>{});
        }
    }

    static constexpr auto properties = makeProperties();

    template <typename TypeChecker = DefaultTypeChecker>
    static constexpr std::size_t size() noexcept
    {
        return countMatches<TypeChecker>(
            std::make_index_sequence<std::tuple_size_v<decltype(properties)>>{});
    }

    template <typename TypeChecker = DefaultTypeChecker>
    static constexpr bool empty() noexcept
    {
        return size<TypeChecker>() == 0;
    }

    template <KeyType name>
    static constexpr auto at()
    {
        return atIndex<name, 0>();
    }

    template <KeyType name>
    static constexpr bool contains()
    {
        return at<name>() != nullptr;
    }

    template <typename TypeChecker = DefaultTypeChecker, typename F>
    static constexpr void forEach(F&& func)
    {
        if (!impl::isValidCallable(func))
            return;

        visit<TypeChecker, 0>(func);
    }

    template <typename TypeChecker = DefaultTypeChecker, typename F, typename P>
    static constexpr void forEachIf(F&& func, P&& pred)
    {
        if (!impl::isValidCallable(func) || !impl::isValidCallable(pred))
            return;

        visit<TypeChecker, 0>(func, std::addressof(pred));
    }

    template <typename Comparator = PropertyNameEqual,
              typename TypeChecker = DefaultTypeChecker, typename F>
    static constexpr void doForKey(F&& func, KeyType key,
                         Comparator comparator = Comparator{})
    {
        if (!key || !impl::isValidCallable(func) || !impl::isValidCallable(comparator))
            return;

        forEach<TypeChecker>([&](auto entryTag)
        {
            if (comparator(decltype(entryTag)::key, key))
            {
                func(entryTag);
                return false;
            }
            return true;
        });
    }

    template <KeyType key, typename TypeChecker = DefaultTypeChecker, typename F>
    static constexpr void doForKey(F&& func)
    {
        if (!impl::isValidCallable(func))
            return;

        forEach<TypeChecker>([&](auto entryTag)
        {
            if constexpr (std::string_view(decltype(entryTag)::key)
                          == std::string_view(key))
            {
                func(entryTag);
            }
        });
    }

private:
    template <std::size_t... indices>
    static constexpr bool validateBaseClasses(std::index_sequence<indices...>)
    {
        return (isValidBaseClass<std::tuple_element_t<indices, decltype(properties)>>() && ...);
    }

    template <typename Property>
    static constexpr bool isValidBaseClass()
    {
        using CleanProperty = std::remove_cv_t<Property>;
        if constexpr (std::is_same_v<CleanProperty, StaticTypeInfo<typename CleanProperty::OuterType>>)
        {
            using Base = typename CleanProperty::OuterType;
            return !std::is_same_v<Base, Outer> && std::is_base_of_v<Base, Outer>;
        }
        else
        {
            return true;
        }
    }

    template <std::size_t... indices>
    static constexpr bool validateUniqueNames(std::index_sequence<indices...>)
    {
        constexpr std::array<std::string_view, sizeof...(indices)> names
        {
            std::get<indices>(properties).name...
        };

        for (std::size_t first = 0; first < names.size(); ++first)
        {
            for (std::size_t second = first + 1; second < names.size(); ++second)
            {
                if (names[first] == names[second])
                    return false;
            }
        }

        return true;
    }

    template <std::size_t index>
    struct EntryTag
    {
        static constexpr KeyType key = std::get<index>(properties).name.data();
        static constexpr const auto& value = std::get<index>(properties);
    };

    template <typename TypeChecker, std::size_t index>
    using EntryFilter = typename TypeChecker::template Filter<Outer, EntryTag<index>>;

    template <typename TypeChecker, std::size_t... indices>
    static constexpr std::size_t countMatches(std::index_sequence<indices...>)
    {
        return (std::size_t{ 0 } + ...
            + static_cast<std::size_t>(EntryFilter<TypeChecker, indices>::value));
    }

    template <KeyType name, std::size_t index>
    static constexpr auto atIndex()
    {
        if constexpr (index == std::tuple_size_v<decltype(properties)>)
            return nullptr;
        else if constexpr (std::get<index>(properties).name == std::string_view(name))
            return &std::get<index>(properties);
        else
            return atIndex<name, index + 1>();
    }

    template <typename TypeChecker, std::size_t index,
              typename F, typename P = std::nullptr_t>
    static constexpr void visit(F& func, P pred = nullptr)
    {
        if constexpr (index < std::tuple_size_v<decltype(properties)>)
        {
            using Entry = EntryTag<index>;

            bool proceed = true;

            if constexpr (EntryFilter<TypeChecker, index>::value)
            {
                bool selected = true;

                if constexpr (!std::is_same_v<P, std::nullptr_t>)
                    selected = (*pred)(Entry{});

                if (selected)
                {
                    using ReturnType = std::invoke_result_t<F&, Entry>;

                    static_assert(std::is_void_v<ReturnType> || std::is_same_v<ReturnType, bool>,
                        "Reflection visitor must return void or bool");

                    if constexpr (std::is_same_v<ReturnType, bool>)
                        proceed = func(Entry{});
                    else
                        func(Entry{});
                }
            }

            if (proceed)
                visit<TypeChecker, index + 1>(func, pred);
        }
    }

    static_assert(validateBaseClasses(
        std::make_index_sequence<std::tuple_size_v<decltype(properties)>>{}),
        "Invalid registered base class");

    static_assert(validateUniqueNames(
        std::make_index_sequence<std::tuple_size_v<decltype(properties)>>{}),
        "Duplicate reflection property name");
};

} // namespace reflection
