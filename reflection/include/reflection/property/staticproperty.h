#pragma once

#include <cstddef>
#include <string_view>
#include <type_traits>
#include <utility>

namespace reflection
{

template <typename Outer, typename T>
struct StaticProperty
{
    using OuterType = Outer;
    using ValueType = T;
    using ValuePtr = T Outer::*;

    constexpr T& get(Outer& outer) const noexcept
    {
        return outer.*valuePtr;
    }

    constexpr const T& get(const Outer& outer) const noexcept
    {
        return outer.*valuePtr;
    }

    template <typename U, std::enable_if_t<std::is_assignable_v<T&, U&&>, std::size_t> = 0>
    const StaticProperty& set(Outer& outer, U&& value) const
        noexcept(std::is_nothrow_assignable_v<T&, U&&>)
    {
        outer.*valuePtr = std::forward<U>(value);
        return *this;
    }

    const std::string_view name;

    const ValuePtr valuePtr;
};

} // namespace reflection
