#pragma once
#include "Reflection/Property/Static/staticproperty.h"
#include "Reflection/Property/Instance/baseproperty.h"

namespace reflection
{

template <class Outer, class T>
class Property : public BaseProperty<Outer>
{
public:
    using BaseClass = BaseProperty<Outer>;
    using ThisClass = Property<Outer, T>;
    using ThisStaticProperty = StaticProperty<Outer, T>;

    static constexpr bool isPlain = ThisStaticProperty::isPlain;
    static constexpr bool isString = ThisStaticProperty::isString;
    static constexpr bool isArray = ThisStaticProperty::isArray;
    static constexpr bool isObject = ThisStaticProperty::isObject;
    static constexpr bool isConst = ThisStaticProperty::isConst;

    using ValueT = typename ThisStaticProperty::ValueT;

    Property(Outer& outer, const ThisStaticProperty& staticProperty)
        : BaseClass(staticProperty), outer(outer) {}

    FORCEINLINE const ThisStaticProperty& getStaticProperty() const noexcept
    {
        return static_cast<const ThisStaticProperty&>(BaseClass::baseStaticProperty);
    }

    FORCEINLINE ValueT get() const noexcept
    {
        return getStaticProperty().get(outer);
    }

    FORCEINLINE ThisClass& set(T&& value) noexcept
    {
        getStaticProperty().set(outer, std::move(value));
        return *this;
    }
    FORCEINLINE ThisClass& set(ValueT value) noexcept
    {
        getStaticProperty().set(outer, value);
        return *this;
    }

    FORCEINLINE ThisClass& set(ThisClass&& other) noexcept
    {
        if (&other == this)
            return *this;
        getStaticProperty().set(std::move(other.outer.*(other.getStaticProperty().getRaw())));
        return *this;
    }
    template <class Other>
    FORCEINLINE ThisClass& set(const Other& other)
    {
        if (&other == this)
            return *this;
        getStaticProperty().set(*other);
        return *this;
    }

    template <class Other>
    FORCEINLINE bool equalsValue(const Other& other) const noexcept
    {
        return *this == other.operator*();
    }
    template <class Other>
    bool equals(const Other& other) const noexcept
    {
        return BaseClass::equals(other) && equalsValue(other);
    }
    bool equals(const BaseClass& other) const noexcept override
    {
        if (const auto* otherProperty = dynamic_cast<const ThisClass*>(&other))
        {
            return getStaticProperty().equals(outer, otherProperty->getStaticProperty(), otherProperty->outer);
        }
        return false;
    }

    std::string toString() const override
    {
        return getStaticProperty().toString(outer);
    }
    StatusCode toJson(QJsonObject& parentJsonObject) const override
    {
        return getStaticProperty().toJson(outer, parentJsonObject);
    }
    StatusCode fromJson(const QJsonObject& parentJsonObject) override
    {
        return getStaticProperty().fromJson(outer, parentJsonObject);
    }

    template <class Other>
    FORCEINLINE constexpr bool operator==(const Other& other) const noexcept { return equals(other); }

    FORCEINLINE ValueT operator*() const noexcept { return get(); }

protected:
    Outer& outer;
};

template <class Outer, class T>
Property(StaticProperty<Outer, T>&, Outer&) -> Property<Outer, T>;

template <class Outer>
using NullProperty = Property<Outer, std::nullptr_t>;
template <class Outer>
using BoolProperty = Property<Outer, bool>;
template <class Outer>
using IntProperty = Property<Outer, std::int32_t>;
template <class Outer>
using RealProperty = Property<Outer, real>;
template <class Outer>
using StringProperty = Property<Outer, std::string>;

} // namespace reflection
