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

    bool equals(const Outer& outer, const Outer& otherOuter) const noexcept
    {
        return staticProperty.equals(
            static_cast<const TargetOuterClass&>(outer),
            static_cast<const TargetOuterClass&>(otherOuter));
    }

    std::string toString(const TargetOuterClass& outer) const
    {
        if constexpr (IsSpecialization<TargetStaticPropertyClass, reflection::StaticProperty>::value)
        {
            return TargetStaticPropertyClass::toString(BaseClass::getName(), outer, staticProperty.valuePtr);
        }
        else if constexpr (IsSpecialization<TargetStaticPropertyClass, reflection::StaticPropertyMap>::value)
        {
            return TargetStaticPropertyClass::toString(BaseClass::getName(), outer);
        }
        else
        {
            return staticProperty.toString(outer);
        }
    }
    StatusCode toJson(const TargetOuterClass& outer, QJsonObject& parentJsonObject) const
    {
        if constexpr (IsSpecialization<TargetStaticPropertyClass, reflection::StaticProperty>::value)
        {
            return TargetStaticPropertyClass::toJson(BaseClass::getName(), outer, staticProperty.valuePtr, parentJsonObject);
        }
        else if constexpr (IsSpecialization<TargetStaticPropertyClass, reflection::StaticPropertyMap>::value)
        {
            return TargetStaticPropertyClass::toJson(BaseClass::getName(), outer, parentJsonObject);
        }
        else
        {
            return staticProperty.toJson(outer);
        }
    }
    StatusCode fromJson(TargetOuterClass& outer, const QJsonObject& parentJsonObject) const
    {
        if constexpr (IsSpecialization<TargetStaticPropertyClass, reflection::StaticProperty>::value)
        {
            return TargetStaticPropertyClass::fromJson(BaseClass::getName(), outer, staticProperty.valuePtr, parentJsonObject);
        }
        else if constexpr (IsSpecialization<TargetStaticPropertyClass, reflection::StaticPropertyMap>::value)
        {
            return TargetStaticPropertyClass::fromJson(BaseClass::getName(), outer, parentJsonObject);
        }
        else
        {
            return staticProperty.fromJson(outer);
        }
    }

private:
    const TargetStaticPropertyClass& staticProperty;
};

} // namespace reflection
