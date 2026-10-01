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
    using ValueT = T;
    using ValueTransferT = typename ValueTransfer<T>::type;
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

template <typename Outer, typename T, T Outer::* valuePtr, const char rawName[]>
struct StaticPropertyHolder
{
    using StaticPropertyType = reflection::StaticProperty<Outer, T>;
    static constexpr StaticPropertyType staticProperty
    {
        rawName,
        valuePtr
    };
};

// TODO: probably rename

template <typename Outer>
struct StaticPropertyClassMeta
{
    using ThisClass = StaticPropertyClassMeta<Outer>;
    template <typename T>
    using KeyType = T Outer::*;

    static constexpr auto size() noexcept
    {
        return uniqueStaticHeterogeneousMapKeysCount<ThisClass, KeyType>([]{});
    }

    static constexpr bool empty() noexcept
    {
        return size() == 0;
    }

    template <typename T, T Outer::* valuePtr>
    static constexpr auto exists()
    {
        return uniqueStaticHeterogeneousMapExists<ThisClass, KeyType, valuePtr>([]{});
    }

    template <auto valuePtr, typename = std::void_t<extensions::impl::MemberPointerTraits<decltype(valuePtr)>>>
    static constexpr auto get()
    {
        using Traits = extensions::impl::MemberPointerTraits<decltype(valuePtr)>;
        using ValueT = typename Traits::T;
        constexpr auto staticPropertyPtr = uniqueStaticHeterogeneousMapGetValue<
            ThisClass, KeyType<ValueT>, valuePtr>([]{});
        return staticPropertyPtr;
    }

    template <typename T, T Outer::* valuePtr, const char rawName[]>
    static constexpr auto define()
    {
        using CurrentMeta = StaticPropertyHolder<Outer, T, valuePtr, rawName>;
        using StaticPropertyType = typename CurrentMeta::StaticPropertyType;
        constexpr auto staticPropertyPtr = &CurrentMeta::staticProperty;
        return uniqueStaticHeterogeneousMapAdd<
            ThisClass, KeyType<T>, const StaticPropertyType*, valuePtr, staticPropertyPtr>([]{});
    }
};

} // namespace reflection
