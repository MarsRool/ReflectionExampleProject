#pragma once
#include "Reflection/Utils/typetraits.h"
#include "Reflection/Property/basestaticproperty.h"

namespace reflection
{

template <typename Outer, typename T>
struct StaticProperty : public BaseStaticProperty<Outer>
{
    using BaseClass = BaseStaticProperty<Outer>;
    using ThisClass = StaticProperty<Outer, T>;
    using OuterClass = Outer;
    using ValueType = T;
    using ValueTransferType = typename reflection::ValueTransferType<T>;
    using ValuePtr = T Outer::*;

    ValueTransferType get(const Outer& outer) const noexcept
    {
        return outer.*valuePtr;
    }
    const ThisClass& set(Outer& outer, T&& value) const noexcept
    {
        outer.*valuePtr = std::move(value);
        return *this;
    }
    const ThisClass& set(Outer& outer, ValueTransferType value) const noexcept
    {
        outer.*valuePtr = value;
        return *this;
    }

    const ValuePtr valuePtr;
};

} // namespace reflection
