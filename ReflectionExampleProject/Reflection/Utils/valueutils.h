#pragma once

#include <type_traits>
#include <vector>
#include <array>
#include <list>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "Shared/macroes.h"
#include "Shared/checkmacroes.h"
#include "Shared/typetester.h"

namespace reflection
{

namespace extensions
{

template <typename T>
struct TypeTag {};

template <typename T, const T* ptr>
struct PointerHolderTag
{
    static constexpr const T* value = ptr;
};

template <typename Outer, typename T, const T Outer::* ptr>
struct PointerToMemberHolderTag
{
    static constexpr const T Outer::* value = ptr;
};

} // namespace extensions

template <typename Outer>
class BaseProperty;

template <typename Outer>
class PropertyMap;

template <typename T, typename = std::void_t<>>
struct IsObject : std::false_type {};

template <typename T>
struct IsObject<T, std::void_t<decltype(T::staticPropertyMap)>> : std::true_type {};

template <typename T>
struct IsProperty : IsSpecialization<T, BaseProperty> {};

template <typename T>
struct ValueTransfer
    : std::conditional<IsPlain<T>::value,
                       T,
                       std::conditional_t<IsString<T>::value,
                                          std::string_view,
                                          const T&>> {};

template <typename T>
StatusCode valueFromJson(T& value, const QJsonValue& jsonValue);

template <typename T>
StatusCode valueFromJsonArray(std::vector<T>& value, const QJsonArray& jsonArray)
{
    StatusCode statusCode = StatusCode::Good;
    value.clear();
    value.reserve(jsonArray.size());

    for (auto iter = jsonArray.constBegin(); iter != jsonArray.constEnd(); ++iter)
    {
        T arrayElement;
        CHECK_SC_D(valueFromJson(arrayElement, *iter), statusCode = sc; continue;)
        value.emplace_back(std::move(arrayElement));
    }

    return statusCode;
}

template <typename T>
StatusCode valueFromJsonArray(std::list<T>& value, const QJsonArray& jsonArray)
{
    StatusCode statusCode = StatusCode::Good;
    value.clear();

    for (auto iter = jsonArray.constBegin(); iter != jsonArray.constEnd(); ++iter)
    {
        T arrayElement;
        CHECK_SC_D(valueFromJson(arrayElement, *iter), statusCode = sc; continue;)
        value.emplace_back(std::move(arrayElement));
    }

    return statusCode;
}

template <typename T, std::size_t Num>
StatusCode valueFromJsonArray(std::array<T, Num>& value, const QJsonArray& jsonArray)
{
    StatusCode statusCode = StatusCode::Good;

    std::size_t index = 0;
    for (auto iter = jsonArray.constBegin(); iter != jsonArray.constEnd(); ++iter, ++index)
    {
        CHECK_D(index < value.size(), statusCode = StatusCode::Bad; break;)
        T arrayElement;
        CHECK_SC_D(valueFromJson(arrayElement, *iter), statusCode = sc; continue;)
        value[index] = std::move(arrayElement);
    }

    for (; index < value.size(); ++index)
    {
        value[index] = T{};
    }

    return statusCode;
}

template <typename T>
StatusCode valueFromJson(T& value, const QJsonValue& jsonValue)
{
    using Type = std::remove_reference_t<T>;

    if constexpr (IsObject<Type>::value)
    {
        CHECK_R2(jsonValue.isObject(), StatusCode::Bad)
        const auto jsonObject = jsonValue.toObject();
        QJsonObject parentJsonObject;
        const std::string_view propertyName = Type::staticPropertyMap.getName();
        parentJsonObject[QString::fromStdString(std::string(propertyName))] = jsonObject;
        return Type::staticPropertyMap.fromJson(value, parentJsonObject);
    }
    else if constexpr (IsArray<Type>::value)
    {
        CHECK_R2(jsonValue.isArray(), StatusCode::Bad)
        const auto jsonArray = jsonValue.toArray();

        return valueFromJsonArray(value, jsonArray);
    }
    else if constexpr (IsString<Type>::value)
    {
        CHECK_R2(jsonValue.isString(), StatusCode::Bad)
        value = jsonValue.toString().toStdString();
    }
    else if constexpr (std::is_null_pointer_v<Type>)
    {
        CHECK_R2(jsonValue.isNull(), StatusCode::Bad)
    }
    else if constexpr (std::is_same_v<Type, bool>)
    {
        CHECK_R2(jsonValue.isBool(), StatusCode::Bad)
        value = jsonValue.toBool();
    }
    else if constexpr (IsPlain<Type>::value)
    {
        CHECK_R2(jsonValue.isDouble(), StatusCode::Bad)
        value = static_cast<Type>(jsonValue.toDouble());
    }
    else
    {
        return StatusCode::Unexpected;
    }
    return StatusCode::Good;
}

template <typename T>
StatusCode propertyFromJson(std::string_view propertyName, T& value, const QJsonObject& parentJsonObject)
{
    const auto jsonValue = parentJsonObject[propertyName.data()];
    CHECK_SC_R(valueFromJson(value, jsonValue))
    return StatusCode::Good;
}

} // namespace reflection
