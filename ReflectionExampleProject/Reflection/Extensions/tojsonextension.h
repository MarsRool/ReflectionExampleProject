#pragma once
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QCborMap>

#include "Reflection/Utils/macroes.h"
#include "Reflection/Utils/filesystem.h"
#include "Reflection/Utils/typetraits.h"
#include "Reflection/Extensions/serializationformat.h"
#include "Reflection/reflection.h"

namespace reflection::extensions
{

template <typename Outer, typename = std::enable_if_t<IsObject<Outer>::value>>
StatusCode save(const Outer& value,
    SerializationFormat serializationFormat,
    const QString& filenameWithoutExt) noexcept
{
    try
    {
        const auto filepath = FileSystem::getAbsolutePath(filenameWithoutExt
            + (serializationFormat == SerializationFormat::Json ? + ".json" : ".dat"));
        FileSystem::createFullPathDirs(filepath);
        QFile saveFile(filepath);

        if (!saveFile.open(QIODevice::WriteOnly))
        {
            qWarning() << "Couldn't open file while saving " << filepath;
            return StatusCode::AccessDenied;
        }

        QJsonObject jsonObject;
        CHECK_SC_R(convertToJson(value, jsonObject, TypeTag<Outer>{}))
        saveFile.write(serializationFormat == SerializationFormat::Json
                           ? QJsonDocument(jsonObject).toJson()
                           : QCborValue::fromJsonValue(jsonObject).toCbor());

        qInfo() << "save complete: " << saveFile.fileName();
        return StatusCode::Good;
    }
    catch (const std::exception& ex)
    {
        qCritical() << "save ex: " << ex.what();
        return StatusCode::Bad;
    }
    catch (...)
    {
        qCritical() << "save ex: ...";
        return StatusCode::Bad;
    }
}

template <typename Outer, typename = std::enable_if_t<IsObject<Outer>::value>>
StatusCode convertToJson(const Outer& value, QJsonObject& parentJsonObject, TypeTag<Outer> = {})
{
    using Meta = StaticPropertyMeta<Outer>;
    constexpr const auto staticPropertyMapPtr = Meta::getStaticPropertyMap();

    return propertyMapToJson(value, parentJsonObject,
        PointerTag<staticPropertyMapPtr>{});
}

template <typename Outer, typename StaticPropertyT, const StaticPropertyProxy<Outer, StaticPropertyT>* staticPropertyProxyPtr>
StatusCode propertyProxyToJson(const typename StaticPropertyProxy<Outer, StaticPropertyT>::TargetOuterClass& outer,
    QJsonObject& parentJsonObject,
    PointerTag<staticPropertyProxyPtr>)
{
    static_assert(staticPropertyProxyPtr != nullptr);

    using ProxyClass = StaticPropertyProxy<Outer, StaticPropertyT>;
    using TargetStaticPropertyClass = typename ProxyClass::TargetStaticPropertyClass;
    using TargetOuterClass = typename StaticPropertyProxy<Outer, StaticPropertyT>::TargetOuterClass;

    constexpr const auto& targetStaticProperty = staticPropertyProxyPtr->staticProperty;

    if constexpr (IsSpecialization<TargetStaticPropertyClass, StaticProperty>::value)
    {
        return propertyToJson(staticPropertyProxyPtr->name, outer, parentJsonObject,
            PointerToMemberTag<targetStaticProperty->valuePtr>{});
    }
    else if constexpr (IsSpecialization<TargetStaticPropertyClass, StaticPropertyMap>::value)
    {
        return propertyMapToJson(staticPropertyProxyPtr->name, outer, parentJsonObject,
            TypeTag<TargetOuterClass>{});
    }
    else
    {
        static_assert(false, "propertyProxyToJson: unexpected static property type");
        return StatusCode::Unexpected;
    }
}

template <typename Outer, const StaticPropertyMap<Outer>* staticPropertyMapPtr>
StatusCode propertyMapToJson(const Outer& outer,
    QJsonObject& parentJsonObject,
    PointerTag<staticPropertyMapPtr>)
{
    static_assert(staticPropertyMapPtr != nullptr);

    return propertyMapToJson(staticPropertyMapPtr->name, outer, parentJsonObject, TypeTag<Outer>{});
}

template <typename Outer>
StatusCode propertyMapToJson(std::string_view propertyName,
    const Outer& outer,
    QJsonObject& parentJsonObject,
    TypeTag<Outer>)
{
    using Meta = StaticPropertyMeta<Outer>;

    StatusCode statusCode = StatusCode::Good;
    QJsonObject jsonObject;

    Meta::forEach(
        [&outer, &statusCode, &jsonObject](auto, auto constValue)
    {
        constexpr const auto staticPropertyPtr = decltype(constValue)::value;
        if constexpr (staticPropertyPtr == nullptr)
        {
            return;
        }

        using StaticPropertyClass = std::remove_cv_t<std::remove_pointer_t<decltype(staticPropertyPtr)>>;
        using PointerTagClass = PointerTag<staticPropertyPtr>;

        if constexpr (IsSpecialization<StaticPropertyClass, StaticProperty>::value)
        {
            CHECK_SC_D(propertyToJson(outer, jsonObject, PointerTagClass{}),
                       statusCode = sc;)
        }
        else if constexpr (IsSpecialization<StaticPropertyClass, StaticPropertyMap>::value)
        {
            CHECK_SC_D(propertyMapToJson(outer, jsonObject, PointerTagClass{}),
                       statusCode = sc;)
        }
        else if constexpr (IsSpecialization<StaticPropertyClass, StaticPropertyProxy>::value)
        {
            CHECK_SC_D(propertyProxyToJson(outer, jsonObject, PointerTagClass{}),
                       statusCode = sc;)
        }
        else
        {
            static_assert(false, "propertyMapToJson: unexpected static property type");
        }
    });

    parentJsonObject[propertyName.data()] = jsonObject;
    return statusCode;
}

template <typename Outer, typename T, const StaticProperty<Outer, T>* staticPropertyPtr>
StatusCode propertyToJson(const Outer& outer,
    QJsonObject& parentJsonObject,
    PointerTag<staticPropertyPtr>)
{
    static_assert(staticPropertyPtr != nullptr);

    return propertyToJson(staticPropertyPtr->name, outer, parentJsonObject,
        PointerToMemberTag<staticPropertyPtr->valuePtr>{});
}

template <typename Outer, typename T, T Outer::* valuePtr>
StatusCode propertyToJson(std::string_view propertyName,
    const Outer& outer,
    QJsonObject& parentJsonObject,
    PointerToMemberTag<valuePtr>)
{
    static_assert(valuePtr != nullptr);

    return namedValueToJson(propertyName, outer.*valuePtr, parentJsonObject, TypeTag<const T>{});
}

template <typename T>
StatusCode namedValueToJson(std::string_view propertyName,
    const T& value,
    QJsonObject& parentJsonObject,
    TypeTag<const T>)
{
    QJsonValue jsonValue;
    CHECK_SC_R(valueToJson(value, jsonValue, TypeTag<const T>{}))
    parentJsonObject[propertyName.data()] = std::move(jsonValue);
    return StatusCode::Good;
}

// TODO: change shape of json conversions: we shouldn't create extra layer of json object

template <typename T>
StatusCode valueToJson(const T& value, QJsonValue& jsonValue, TypeTag<const T>)
{
    using Type = std::remove_reference_t<T>;

    if constexpr (IsObject<Type>::value)
    {
        using Meta = StaticPropertyMeta<Type>;
        constexpr const auto staticPropertyMapPtr = Meta::getStaticPropertyMap();
        static_assert(staticPropertyMapPtr != nullptr);

        QJsonObject parentJsonObject;
        CHECK_SC_R(convertToJson(value, parentJsonObject, TypeTag<Type>{}))
        jsonValue = parentJsonObject[staticPropertyMapPtr->name.data()];
    }
    else if constexpr (std::is_pointer_v<Type>
                       && IsObject<std::remove_pointer_t<Type>>::value)
    {
        using PlainType = std::remove_pointer_t<Type>;

        using Meta = StaticPropertyMeta<PlainType>;
        constexpr const auto staticPropertyMapPtr = Meta::getStaticPropertyMap();
        static_assert(staticPropertyMapPtr != nullptr);

        if (value != nullptr)
        {
            QJsonObject parentJsonObject;
            CHECK_SC_R(convertToJson(*value, parentJsonObject, TypeTag<PlainType>{}))
            jsonValue = parentJsonObject[staticPropertyMapPtr->name.data()];
        }
        else
        {
            jsonValue = QJsonValue::Null;
        }
    }
    else if constexpr (IsString<Type>::value)
        jsonValue = QString::fromStdString(std::string(value));
    else if constexpr (std::is_same_v<Type, bool>)
        jsonValue = value;
    else if constexpr (std::is_integral_v<Type>)
        jsonValue = static_cast<qint64>(value);
    else if constexpr (std::is_floating_point_v<Type>)
        jsonValue = static_cast<double>(value);
    else if constexpr (IsIterable<Type>::value)
    {
        StatusCode statusCode = StatusCode::Good;
        QJsonArray jsonArray;

        for (const auto& item : value)
        {
            using ItemType = std::remove_reference_t<decltype(item)>;

            QJsonValue iterJsonValue;
            CHECK_SC_D(valueToJson(item, iterJsonValue, TypeTag<ItemType>{}), statusCode = sc; continue;)
            jsonArray.append(iterJsonValue);
        }

        jsonValue = std::move(jsonArray);
        return statusCode;
    }
    else
    {
        static_assert(false, "valueToJson: unexpected type");
        return StatusCode::Unexpected;
    }

    return StatusCode::Good;
}

} // namespace reflection::extensions
