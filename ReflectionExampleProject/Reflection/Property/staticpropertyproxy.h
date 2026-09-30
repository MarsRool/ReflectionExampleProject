#pragma once
#include "Reflection/Property/basestaticproperty.h"

namespace reflection
{

template <typename Outer, typename StaticPropertyT>
class StaticPropertyProxy : public BaseStaticProperty<Outer>
{
public:
    using BaseClass = BaseStaticProperty<Outer>;
    using ThisClass = StaticPropertyProxy<Outer, StaticPropertyT>;
    using OuterClass = Outer;
    using TargetStaticPropertyClass = StaticPropertyT;
    using TargetOuterClass = typename TargetStaticPropertyClass::OuterClass;

    constexpr StaticPropertyProxy(std::string_view name, const TargetStaticPropertyClass& staticProperty)
        : BaseClass(name), staticProperty(staticProperty)
        {}

    FORCEINLINE constexpr const TargetStaticPropertyClass& get() const noexcept
    {
        return staticProperty;
    }

private:
    const TargetStaticPropertyClass& staticProperty;
};

} // namespace reflection
