#pragma once
#include "Reflection/Utils/valueutils.h"
#include "Reflection/Property/Static/basestaticproperty.h"

namespace reflection
{

template <typename Outer, typename T>
class StaticProperty : public BaseStaticProperty<Outer>
{
public:
    using BaseClass = BaseStaticProperty<Outer>;
    using ThisClass = StaticProperty<Outer, T>;
    using OuterClass = Outer;
    using ValuePtr = T Outer::*;

    static constexpr bool isPlain = IsPlain<T>::value;
    static constexpr bool isString = IsString<T>::value;
    static constexpr bool isArray = IsArray<T>::value;
    static constexpr bool isObject = IsObject<T>::value;
    static constexpr bool isConst = std::is_const<T>::value;

    template <typename ProxyOuter, typename StaticPropertyT>
    friend class StaticPropertyProxy;

    using ValueT = typename ValueTransfer<T>::type;

    constexpr StaticProperty(std::string_view name, ValuePtr valuePtr)
        : BaseClass(name), valuePtr(valuePtr)
    {
#ifdef QT_DEBUG
        CHECK_POINTER_THROW(valuePtr)
#endif // #ifdef QT_DEBUG
    }

    FORCEINLINE ValuePtr getRaw() const noexcept
    {
        return valuePtr;
    }
    FORCEINLINE ValueT get(const Outer& outer) const noexcept
    {
        return outer.*valuePtr;
    }
    FORCEINLINE const ThisClass& set(Outer& outer, T&& value) const noexcept
    {
        outer.*valuePtr = std::move(value);
        return *this;
    }
    FORCEINLINE const ThisClass& set(Outer& outer, ValueT value) const noexcept
    {
        outer.*valuePtr = value;
        return *this;
    }

    bool equals(const Outer& outer, const Outer& otherOuter) const noexcept override
    {
        if constexpr (isObject)
        {
            return T::staticPropertyMap.equals(get(outer), get(otherOuter));
        }
        else
        {
            return get(outer) == get(otherOuter);
        }
    }

    std::string toString(const Outer& outer) const override
    {
        return toString(BaseClass::getName(), outer, valuePtr);
    }
    StatusCode toJson(const Outer& outer, QJsonObject& parentJsonObject) const override
    {
        return toJson(BaseClass::getName(), outer, valuePtr, parentJsonObject);
    }
    StatusCode fromJson(Outer& outer, const QJsonObject& parentJsonObject) const override
    {
        return fromJson(BaseClass::getName(), outer, valuePtr, parentJsonObject);
    }

    FORCEINLINE static std::string toString(
        std::string_view propertyName,
        const Outer& outer,
        const ValuePtr valuePtr)
    {
#ifdef QT_DEBUG
        CHECK_POINTER_THROW(valuePtr)
#endif // #ifdef QT_DEBUG
        return propertyToString(propertyName, outer.*valuePtr);
    }

    FORCEINLINE static StatusCode toJson(
        std::string_view propertyName,
        const Outer& outer,
        const ValuePtr valuePtr,
        QJsonObject& parentJsonObject)
    {
#ifdef QT_DEBUG
        CHECK_POINTER_THROW(valuePtr)
#endif // #ifdef QT_DEBUG
        return propertyToJson(propertyName, outer.*valuePtr, parentJsonObject);
    }

    FORCEINLINE static StatusCode fromJson(
        std::string_view propertyName,
        Outer& outer,
        ValuePtr valuePtr,
        const QJsonObject& parentJsonObject)
    {
        if constexpr (isConst)
        {
#ifdef QT_DEBUG
            std::remove_cv_t<T> value;
            CHECK_SC_R(propertyFromJson(propertyName, value, parentJsonObject))
            return outer.*valuePtr == value ? StatusCode::Good : StatusCode::Bad;
#endif // #ifdef QT_DEBUG
            return StatusCode::GoodNothingTodo;
        }
        else
        {
#ifdef QT_DEBUG
            CHECK_POINTER_THROW(valuePtr)
#endif // #ifdef QT_DEBUG
            return propertyFromJson(propertyName, outer.*valuePtr, parentJsonObject);
        }
    }

private:
    ValuePtr valuePtr;
};


} // namespace reflection
