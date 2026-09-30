#pragma once

#include <utility>

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

} // namespace impl

template <const char* str>
using CanonicalStaticString =
    decltype(impl::makeCanonicalStaticString<char, str>(
        std::make_index_sequence<impl::canonicalStaticStringLength(str)>{}));
