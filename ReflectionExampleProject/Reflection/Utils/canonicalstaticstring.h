#pragma once

#include <utility>

namespace reflection
{

template <typename T, T... chars>
struct CanonicalStaticStringHolder
{
    static constexpr T value[] = {chars..., '\0'};
};

namespace impl
{

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
    -> CanonicalStaticStringHolder<T, str[indices]...>;

template <
    typename T,
    const T* str,
    std::size_t begin,
    std::size_t... indices>
auto makeCanonicalStaticStringRange(
    std::index_sequence<indices...>)
    -> CanonicalStaticStringHolder<
        T,
        str[begin + indices]...>;

template <
    typename T,
    const T* str1,
    const T* str2,
    std::size_t... indices1,
    std::size_t... indices2>
auto concatCanonicalStaticString(
    std::index_sequence<indices1...>,
    std::index_sequence<indices2...>)
    -> CanonicalStaticStringHolder<
        T,
        str1[indices1]...,
        str2[indices2]...>;

} // namespace impl

template <const char* str>
using CanonicalStaticString =
    decltype(impl::makeCanonicalStaticString<char, str>(
        std::make_index_sequence<impl::canonicalStaticStringLength(str)>{}));

template <
    const char* str,
    std::size_t begin,
    std::size_t length>
using CanonicalStaticStringRange =
    decltype(
        impl::makeCanonicalStaticStringRange<
            char,
            str,
            begin>(
            std::make_index_sequence<length>{}));

template <const char* str1, const char* str2>
using CanonicalStaticStringConcat =
    decltype(impl::concatCanonicalStaticString<char, str1, str2>(
    std::make_index_sequence<impl::canonicalStaticStringLength(str1)>{},
    std::make_index_sequence<impl::canonicalStaticStringLength(str2)>{}));

} // namespace reflection
