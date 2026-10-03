#pragma once
#include "Reflection/Utils/typetraits.h"
#include "Reflection/Utils/uniquestaticheterogeneousmap.h"
#include "Reflection/Property/basestaticproperty.h"

namespace reflection
{

template <typename Outer, typename T>
class StaticProperty : public BaseStaticProperty<Outer>
{
public:
    using BaseClass = BaseStaticProperty<Outer>;
    using ThisClass = StaticProperty<Outer, T>;
    using OuterClass = Outer;
    using ValueType = T;
    using ValueTransferType = typename reflection::ValueTransferType<T>;
    using ValuePtr = T Outer::*;

    constexpr StaticProperty(std::string_view name, ValuePtr valuePtr)
        : BaseClass(name), valuePtr(valuePtr)
    {
#ifdef QT_DEBUG
        CHECK_POINTER_THROW(valuePtr)
#endif // #ifdef QT_DEBUG
    }

    FORCEINLINE constexpr ValuePtr getRaw() const noexcept
    {
        return valuePtr;
    }
    FORCEINLINE ValueTransferType get(const Outer& outer) const noexcept
    {
        return outer.*valuePtr;
    }
    FORCEINLINE const ThisClass& set(Outer& outer, T&& value) const noexcept
    {
        outer.*valuePtr = std::move(value);
        return *this;
    }
    FORCEINLINE const ThisClass& set(Outer& outer, ValueTransferType value) const noexcept
    {
        outer.*valuePtr = value;
        return *this;
    }

private:
    ValuePtr valuePtr;
};

} // namespace reflection
