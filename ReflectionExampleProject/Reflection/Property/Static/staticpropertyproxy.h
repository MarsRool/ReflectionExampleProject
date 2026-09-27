#pragma once
#include "Reflection/Property/Static/staticproperty.h"
#include "Reflection/Property/Static/staticpropertymap.h"

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

    bool equals(const Outer& outer, const Outer& otherOuter) const noexcept
    {
        return staticProperty.equals(
            static_cast<const TargetOuterClass&>(outer),
            static_cast<const TargetOuterClass&>(otherOuter));
    }

private:
    const TargetStaticPropertyClass& staticProperty;
};

} // namespace reflection
