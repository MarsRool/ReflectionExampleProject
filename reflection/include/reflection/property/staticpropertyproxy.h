#pragma once
#include "reflection/property/basestaticproperty.h"

namespace reflection
{

template <typename Outer, typename StaticPropertyT>
struct StaticPropertyProxy : public BaseStaticProperty<Outer>
{
    using BaseClass = BaseStaticProperty<Outer>;
    using ThisClass = StaticPropertyProxy<Outer, StaticPropertyT>;
    using OuterClass = Outer;
    using TargetStaticPropertyClass = StaticPropertyT;
    using TargetOuterClass = typename TargetStaticPropertyClass::OuterClass;

    const TargetStaticPropertyClass& staticProperty;
};

} // namespace reflection
