#pragma once
#include "Reflection/Utils/valueutils.h"
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
    using ValueT = T;
    using ValueTransferT = typename ValueTransfer<T>::type;
    using ValuePtr = T Outer::*;

    static constexpr bool isPlain = IsPlain<T>::value;
    static constexpr bool isString = IsString<T>::value;
    static constexpr bool isArray = IsArray<T>::value;
    static constexpr bool isObject = IsObject<T>::value;
    static constexpr bool isConst = std::is_const<T>::value;

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
    FORCEINLINE ValueTransferT get(const Outer& outer) const noexcept
    {
        return outer.*valuePtr;
    }
    FORCEINLINE const ThisClass& set(Outer& outer, T&& value) const noexcept
    {
        outer.*valuePtr = std::move(value);
        return *this;
    }
    FORCEINLINE const ThisClass& set(Outer& outer, ValueTransferT value) const noexcept
    {
        outer.*valuePtr = value;
        return *this;
    }

private:
    ValuePtr valuePtr;
};


} // namespace reflection
