#pragma once

#include <array>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "reflection/utils/typetraits.h"

namespace reflection
{

template <typename Outer>
struct IntrusiveLineScanner
{
    static constexpr auto makeTuple()
    {
        return tupleFromLines(std::make_index_sequence<count>{});
    }

private:
    static_assert(impl::HasIntrusive<Outer>::value,
        "IntrusiveLineScanner expects intrusive reflection");

    static constexpr std::size_t begin =
        decltype(z_reflection_begin(TypeTag<Outer>{}))::value + 1;
    static constexpr std::size_t end =
        decltype(z_reflection_end(TypeTag<Outer>{}))::value;

    static_assert(begin <= end, "Invalid reflection line interval");

    template <std::size_t line, typename U = Outer>
    static auto detectEntry(std::size_t) -> decltype(z_reflection_entry(
        typename U::template z_reflection_slot<line, U>{}), std::true_type{});

    template <std::size_t line>
    static auto detectEntry(...) -> std::false_type;

    template <std::size_t line>
    static constexpr bool hasLine()
    {
        return decltype(detectEntry<line>(0))::value;
    }

    template <std::size_t... indices>
    static constexpr auto makeFlags(std::index_sequence<indices...>)
    {
        return std::array<bool, sizeof...(indices)>
        {
            hasLine<begin + indices>()...
        };
    }

    static constexpr auto flags = makeFlags(
        std::make_index_sequence<end - begin>{});

    template <std::size_t n>
    static constexpr std::size_t getFlagsCount(
        const std::array<bool, n>& flagValues)
    {
        std::size_t result = 0;
        for (bool present : flagValues)
            result += present;
        return result;
    }

    static constexpr std::size_t count = getFlagsCount(flags);

    template <std::size_t count, std::size_t n>
    static constexpr auto collectLines(const std::array<bool, n>& flagValues)
    {
        std::array<std::size_t, count> result{};
        std::size_t outputIndex = 0;
        for (std::size_t index = 0; index < n; ++index)
            if (flagValues[index])
                result[outputIndex++] = begin + index;
        return result;
    }

    static constexpr auto lines = collectLines<count>(flags);

    template <std::size_t... indices>
    static constexpr auto tupleFromLines(std::index_sequence<indices...>)
    {
        return std::make_tuple(
            z_reflection_entry(typename Outer::template z_reflection_slot<
                lines[indices], Outer>{})...);
    }
};

} // namespace reflection
