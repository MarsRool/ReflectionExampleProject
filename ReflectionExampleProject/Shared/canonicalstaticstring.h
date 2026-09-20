#pragma once

#include <utility>

template <char... chars>
struct CanonicalStaticString
{
    static constexpr char value[] = {chars..., '\0'};
};

constexpr std::size_t canonicalStaticStringLength(const char* str)
{
    std::size_t size = 0;
    while (str[size])
        ++size;

    return size;
}

template <const char* str, std::size_t... indices>
auto makeStaticString(std::index_sequence<indices...>)
    -> CanonicalStaticString<str[indices]...>;

template <const char* str>
using CanonicalStaticStringT =
    decltype(makeStaticString<str>(
        std::make_index_sequence<canonicalStaticStringLength(str)>{}));
