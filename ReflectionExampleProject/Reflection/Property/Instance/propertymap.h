#pragma once
#include "Reflection/Property/Static/staticpropertymap.h"
#include "Reflection/Property/Instance/baseproperty.h"

namespace reflection
{

template <class Outer>
class PropertyMap : public BaseProperty<Outer>
{
public:
    using BaseClass = BaseProperty<Outer>;
    using ThisClass = PropertyMap<Outer>;
    using ThisStaticPropertyMap = StaticPropertyMap<Outer>;

    PropertyMap(Outer& outer, const ThisStaticPropertyMap& staticPropertyMap)
        : BaseClass(staticPropertyMap),
        outer(outer)
    {}

    FORCEINLINE Outer& getOuter() noexcept { return outer; }
    FORCEINLINE const ThisStaticPropertyMap& getStaticPropertyMap() const noexcept
    {
        return static_cast<const ThisStaticPropertyMap&>(BaseClass::baseStaticProperty);
    }
    FORCEINLINE const auto& getStorage() const noexcept { return getStaticPropertyMap().getStorage(); }
    FORCEINLINE bool empty() const noexcept { return getStaticPropertyMap().empty(); }
    FORCEINLINE bool contains(std::string_view propertyName) const noexcept { return getStaticPropertyMap().contains(propertyName); }
    FORCEINLINE const auto* at(std::string_view propertyName) const noexcept
    {
        return getStaticPropertyMap().at(propertyName);
    }

    bool equals(const BaseClass& other) const noexcept override
    {
        if (const auto* otherProperty = dynamic_cast<const ThisClass*>(&other))
        {
            return getStaticPropertyMap().equals(outer, otherProperty->getStaticPropertyMap(), otherProperty->outer);
        }
        return false;
    }

    std::string toString() const override
    {
        return getStaticPropertyMap().toString(outer);
    }
    StatusCode toJson(QJsonObject& parentJsonObject) const override
    {
        return getStaticPropertyMap().toJson(outer, parentJsonObject);
    }
    StatusCode fromJson(const QJsonObject& parentJsonObject) override
    {
        return getStaticPropertyMap().fromJson(outer, parentJsonObject);
    }

private:
    Outer& outer;
};

} // namespace reflection
