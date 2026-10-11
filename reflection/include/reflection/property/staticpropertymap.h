#pragma once
#include "reflection/property/basestaticproperty.h"

namespace reflection
{

template <typename Outer>
struct StaticPropertyMap : public BaseStaticProperty<Outer>
{
    using BaseClass = BaseStaticProperty<Outer>;
    using ThisClass = StaticPropertyMap<Outer>;
    using OuterClass = Outer;
};

} // namespace reflection
