#pragma once

#include <utility>

template <typename T, T... chars>
struct CanonicalStaticString
{
    static constexpr T value[] = {chars..., '\0'};
};

template <typename T>
constexpr std::size_t canonicalStaticStringLength(const T* str)
{
    std::size_t size = 0;
    while (str[size])
        ++size;

    return size;
}

template <typename T, const T* str, std::size_t... indices>
auto makeCanonicalStaticString(std::index_sequence<indices...>)
    -> CanonicalStaticString<T, str[indices]...>;

template <const char* str>
using CanonicalStaticStringT =
    decltype(makeCanonicalStaticString<char, str>(
        std::make_index_sequence<canonicalStaticStringLength(str)>{}));
