#pragma once
#include "Reflection/Property/Static/staticproperty.h"
#include "Reflection/Property/Static/staticpropertymap.h"

namespace reflection
{

template <class Outer, class StaticPropertyT>
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

    FORCEINLINE bool equals(const Outer& outer, const ThisClass& other, const Outer& otherOuter) const noexcept
    {
        return staticProperty.equals(
            static_cast<const TargetOuterClass&>(outer),
            other.staticProperty,
            static_cast<const TargetOuterClass&>(otherOuter));
    }
    bool equals(const Outer& outer, const BaseClass& other, const Outer& otherOuter) const noexcept override
    {
        if (const auto* otherStaticProperty = dynamic_cast<const ThisClass*>(&other))
        {
            return equals(outer, *otherStaticProperty, otherOuter);
        }
        return false;
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

    std::string toString(const Outer& outer) const override
    {
        return toString(static_cast<const TargetOuterClass&>(outer));
    }
    StatusCode toJson(const Outer& outer, QJsonObject& parentJsonObject) const override
    {
        return toJson(static_cast<const TargetOuterClass&>(outer), parentJsonObject);
    }
    StatusCode fromJson(Outer& outer, const QJsonObject& parentJsonObject) const override
    {
        return fromJson(static_cast<TargetOuterClass&>(outer), parentJsonObject);
    }

private:
    const TargetStaticPropertyClass& staticProperty;
};

} // namespace reflection
