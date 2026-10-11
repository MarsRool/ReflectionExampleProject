#pragma once

#include <type_traits>
#include <utility>

#include "reflection/utils/typetraits.h"
#include "reflection/property/basestaticproperty.h"

namespace reflection
{

template <typename Outer, typename T>
struct StaticProperty : public BaseStaticProperty<Outer>
{
    using BaseClass = BaseStaticProperty<Outer>;
    using ThisClass = StaticProperty<Outer, T>;
    using OuterClass = Outer;
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

    template <typename U, std::enable_if_t<std::is_assignable_v<T&, U&&>, int> = 0>
    const ThisClass& set(Outer& outer, U&& value) const
        noexcept(std::is_nothrow_assignable_v<T&, U&&>)
    {
        outer.*valuePtr = std::forward<U>(value);
        return *this;
    }

    const ValuePtr valuePtr;
};

} // namespace reflection
