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

template <class Outer>
class BaseProperty;

template <class Outer>
class PropertyMap;

template <class Outer>
class Reflectable;

template <class T>
struct IsObject : std::is_base_of<Reflectable<T>, T> {};

template <class T>
struct IsProperty : IsSpecialization<T, BaseProperty> {};

template <class T>
struct ValueTransfer
    : std::conditional<IsPlain<T>::value,
                       T,
                       std::conditional_t<IsString<T>::value,
                                          std::string_view,
                                          const T&>> {};

template <class T>
inline std::string valueToString(const T& value)
{
    using Type = std::remove_reference_t<T>;

    if constexpr (IsObject<Type>::value)
        return value.Reflectable<Type>::getPropertyMap().toString();
    if constexpr (IsArray<Type>::value)
    {
        std::string result{ "[ " };
        std::size_t index = 0;
        const std::size_t size = std::size(value);

        for (const auto& item : value)
        {
            result += valueToString(value);
            if (index != size - 1)
                result += ", ";
            ++index;
        }

        result += " ]";
        return result;
    }
    if constexpr (IsString<Type>::value)
        return '\"' + std::string(value) + '\"';
    if constexpr (std::is_same_v<Type, bool>)
        return value ? "true" : "false";
    if constexpr (std::is_null_pointer_v<Type>)
        return "null";
    if constexpr (ToStringDetect<Type>::value)
        return std::to_string(value);
    Q_UNUSED(value)
    return "unknown toString";
}

template <class T>
inline StatusCode valueToJson(const T& value, QJsonValue& jsonValue)
{
    using Type = std::remove_reference_t<T>;

    if constexpr (IsObject<Type>::value)
    {
        auto propertyMap = value.Reflectable<Type>::getPropertyMap();
        QJsonObject parentJsonObject;
        CHECK_SC_R(propertyMap.toJson(parentJsonObject))
        jsonValue = parentJsonObject[propertyMap.getName().data()];
    }
    else if constexpr (IsArray<Type>::value)
    {
        StatusCode statusCode = StatusCode::Good;
        QJsonArray jsonArray;

        for (const auto& item : value)
        {
            QJsonValue iterJsonValue;
            CHECK_SC_D(valueToJson(item, iterJsonValue), statusCode = sc; continue;)
            jsonArray.append(iterJsonValue);
        }

        jsonValue = std::move(jsonArray);
        return statusCode;
    }
    else if constexpr (IsString<Type>::value)
        jsonValue = QString::fromStdString(std::string(value));
    else if constexpr (std::is_null_pointer_v<Type>)
        jsonValue = QJsonValue::Null;
    else if constexpr (std::is_same_v<Type, bool>)
        jsonValue = value;
    else if constexpr (std::is_integral_v<Type>)
        jsonValue = static_cast<qint64>(value);
    else if constexpr (std::is_floating_point_v<Type>)
        jsonValue = static_cast<double>(value);
    else
    {
        return StatusCode::Unexpected;
    }
    return StatusCode::Good;
}

template <class T>
StatusCode valueFromJson(T& value, const QJsonValue& jsonValue);

template <class T>
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

template <class T>
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

template <class T, std::size_t Num>
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

template <class T>
StatusCode valueFromJson(T& value, const QJsonValue& jsonValue)
{
    using Type = std::remove_reference_t<T>;

    if constexpr (IsObject<Type>::value)
    {
        CHECK_R2(jsonValue.isObject(), StatusCode::Bad)
        const auto jsonObject = jsonValue.toObject();
        QJsonObject parentJsonObject;
        const std::string_view propertyName = value.Reflectable<Type>::getPropertyMap().getStaticPropertyMap().getName();
        parentJsonObject[QString::fromStdString(std::string(propertyName))] = jsonObject;
        return value.Reflectable<Type>::getPropertyMap().fromJson(parentJsonObject);
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

template <class T>
inline std::string propertyToString(std::string_view propertyName, const T& value)
{
    return '\"' + std::string(propertyName) + "\": " + valueToString(value);
}

template <class T>
inline StatusCode propertyToJson(std::string_view propertyName, const T& value, QJsonObject& parentJsonObject)
{
    QJsonValue jsonValue;
    CHECK_SC_R(valueToJson(value, jsonValue))
    parentJsonObject[propertyName.data()] = std::move(jsonValue);
    return StatusCode::Good;
}

template <class T>
inline StatusCode propertyFromJson(std::string_view propertyName, T& value, const QJsonObject& parentJsonObject)
{
    const auto jsonValue = parentJsonObject[propertyName.data()];
    CHECK_SC_R(valueFromJson(value, jsonValue))
    return StatusCode::Good;
}

} // namespace reflection
