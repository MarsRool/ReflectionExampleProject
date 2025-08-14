#pragma once
#include "Reflection/Utils/valueutils.h"
#include "Reflection/Property/Static/basestaticproperty.h"
#include "Reflection/Property/Instance/baseproperty.h"

namespace reflection
{

template <class Outer, class T>
class CustomProperty : public BaseProperty<Outer>
{
public:
    using BaseClass = BaseProperty<Outer>;
    using ThisClass = CustomProperty<Outer, T>;
    using ThisStaticProperty = BaseStaticProperty<Outer>;

    static constexpr bool isPlain = IsPlain<T>::value;
    static constexpr bool isString = IsString<T>::value;
    static constexpr bool isArray = IsArray<T>::value;
    static constexpr bool isObject = IsObject<T>::value;
    static constexpr bool isConst = std::is_const<T>::value;

    using ValueT = ValueTransfer<T>::type;
    using Getter = std::function<T()>;
    using Setter = std::function<void(ValueT)>;

    CustomProperty(auto&& getter, auto&& setter,
                   const ThisStaticProperty& staticProperty)
        : BaseClass(staticProperty),
        getter(std::move(getter)),
        setter(std::move(setter))
	{
#ifdef QT_DEBUG
		CHECK_POINTER_THROW(this->getter)
		CHECK_POINTER_THROW(this->setter)
#endif // #ifdef QT_DEBUG
    }

    FORCEINLINE ValueT get() const noexcept
    {
        return getter();
    }

    FORCEINLINE ThisClass& set(ValueT value) noexcept
    {
        setter(value);
        return *this;
    }

    template <class Other>
    FORCEINLINE bool equalsValue(const Other& other) const noexcept
    {
        return *this == other.operator*();
    }
    template <class Other>
    FORCEINLINE bool equals(const Other& other) const noexcept
    {
        return BaseClass::equals(other) && equalsValue(other);
    }
    bool equals(const BaseClass& other) const noexcept override
    {
        if (const auto* otherProperty = dynamic_cast<const ThisClass*>(&other))
        {
            return BaseClass::equals(other) && equalsValue(*otherProperty);
        }
        return false;
    }

    std::string toString() const override
    {
        return propertyToString(BaseClass::getName(), get());
    }
    StatusCode toJson(QJsonObject& parentJsonObject) const override
    {
        return propertyToJson(BaseClass::getName(), get(), parentJsonObject);
    }
    StatusCode fromJson(const QJsonObject&) override
    {
        return StatusCode::GoodNothingTodo;
    }

    template <class Other>
    FORCEINLINE constexpr bool operator==(const Other& other) const noexcept { return equals(other); }

    FORCEINLINE ValueT operator*() const noexcept { return get(); }

private:
	Getter getter;
	Setter setter;
};

} // namespace reflection
