#pragma once
#include "Reflection/Property/Static/basestaticproperty.h"

namespace reflection
{

template <class Outer>
class BaseProperty
{
public:
    using ThisClass = BaseProperty<Outer>;
    using ThisStaticProperty = BaseStaticProperty<Outer>;

    BaseProperty(const ThisStaticProperty& baseStaticProperty)
        : baseStaticProperty(baseStaticProperty)
    {}
    BaseProperty(const ThisClass&) = default;
    BaseProperty(ThisClass&&) noexcept = default;
	virtual ~BaseProperty() = default;

    FORCEINLINE const ThisStaticProperty& getBaseStaticProperty() const noexcept { return baseStaticProperty; }
    FORCEINLINE std::string_view getName() const noexcept { return baseStaticProperty.getName(); }

    virtual bool equals(const ThisClass& other) const noexcept
    {
        return baseStaticProperty.equalsName(other.baseStaticProperty);
    }

    virtual std::string toString() const = 0;
	virtual StatusCode toJson(QJsonObject& json) const = 0;
    virtual StatusCode fromJson(const QJsonObject& json) = 0;

    template <class Other>
    FORCEINLINE constexpr bool operator==(const Other& other) const noexcept { return equals(other); }

    template <class Other>
    FORCEINLINE constexpr BaseProperty& operator=(const Other&) = delete;
    template <class Other>
    FORCEINLINE constexpr BaseProperty& operator=(Other&&) noexcept = delete;

protected:
    const ThisStaticProperty& baseStaticProperty;
};

} // namespace reflection
