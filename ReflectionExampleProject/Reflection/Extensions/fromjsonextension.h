#pragma once
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QCborMap>

#include "Reflection/Utils/macroes.h"
#include "Reflection/Utils/filesystem.h"
#include "Reflection/Extensions/extensionstypetraits.h"
#include "Reflection/Extensions/serializationformat.h"
#include "Reflection/reflection.h"

namespace extensions
{

using reflection::StatusCode;

template <typename Outer, typename = std::enable_if_t<reflection::IsObject<Outer>::value>>
StatusCode load(Outer& value,
    SerializationFormat serializationFormat,
    const QString& filenameWithoutExt) noexcept
{
    try
    {
        const auto filepath = reflection::FileSystem::getAbsolutePath(filenameWithoutExt
            + (serializationFormat == SerializationFormat::Json ? + ".json" : ".dat"));
        QFile loadFile(filepath);

        if (!loadFile.open(QIODevice::ReadOnly))
        {
            qWarning() << "Couldn't open file while loading " << filepath;
            return StatusCode::AccessDenied;
        }

        QByteArray loadData = loadFile.readAll();

        QJsonParseError err;
        QJsonDocument loadDoc(serializationFormat == SerializationFormat::Json
            ? QJsonDocument::fromJson(loadData, &err)
            : QJsonDocument(QCborValue::fromCbor(loadData).toMap().toJsonObject()));

        if (err.error != QJsonParseError::ParseError::NoError)
            qCritical() << "Error parsing json: " << err.errorString();

        CHECK_SC_R(convertFromJson(value, loadDoc.object(), TypeTag<Outer>{}))

        qInfo() << "load complete:" << loadFile.fileName();
        return StatusCode::Good;
    }
    catch (const std::exception& ex)
    {
        qCritical() << "load ex: " << ex.what();
        return StatusCode::Bad;
    }
    catch (...)
    {
        qCritical() << "load ex: ...";
        return StatusCode::Bad;
    }
}

template <typename Outer, typename = std::enable_if_t<reflection::IsObject<Outer>::value>>
StatusCode convertFromJson(Outer& value, const QJsonObject& parentJsonObject, TypeTag<Outer> = {})
{
    using Meta = reflection::StaticPropertyMeta<Outer>;
    constexpr auto staticPropertyMapPtr = Meta::getStaticPropertyMap();

    return propertyMapFromJson(value, parentJsonObject,
        PointerTag<staticPropertyMapPtr>{});
}

template <typename Outer, typename StaticPropertyT, const reflection::StaticPropertyProxy<Outer, StaticPropertyT>* staticPropertyProxyPtr>
StatusCode propertyProxyFromJson(typename reflection::StaticPropertyProxy<Outer, StaticPropertyT>::TargetOuterClass& outer,
    const QJsonObject& parentJsonObject,
    PointerTag<staticPropertyProxyPtr>)
{
    static_assert(staticPropertyProxyPtr != nullptr);

    using ProxyClass = reflection::StaticPropertyProxy<Outer, StaticPropertyT>;
    using TargetStaticPropertyClass = typename ProxyClass::TargetStaticPropertyClass;
    using TargetOuterClass = typename reflection::StaticPropertyProxy<Outer, StaticPropertyT>::TargetOuterClass;

    constexpr const auto& targetStaticProperty = staticPropertyProxyPtr->staticProperty;

    if constexpr (reflection::IsSpecialization<TargetStaticPropertyClass, reflection::StaticProperty>::value)
    {
        return propertyFromJson(staticPropertyProxyPtr->name, outer, parentJsonObject,
            PointerToMemberTag<targetStaticProperty->valuePtr>{});
    }
    else if constexpr (reflection::IsSpecialization<TargetStaticPropertyClass, reflection::StaticPropertyMap>::value)
    {
        return propertyMapFromJson(staticPropertyProxyPtr->name, outer, parentJsonObject,
            TypeTag<TargetOuterClass>{});
    }
    else
    {
        static_assert(false, "propertyProxyFromJson: unexpected static property type");
        return StatusCode::Unexpected;
    }
}

template <typename Outer, const reflection::StaticPropertyMap<Outer>* staticPropertyMapPtr>
StatusCode propertyMapFromJson(Outer& outer,
    const QJsonObject& parentJsonObject,
    PointerTag<staticPropertyMapPtr>)
{
    static_assert(staticPropertyMapPtr != nullptr);

    return propertyMapFromJson(staticPropertyMapPtr->name, outer, parentJsonObject,
        TypeTag<Outer>{});
}

template <typename Outer>
StatusCode propertyMapFromJson(std::string_view propertyName,
    Outer& outer,
    const QJsonObject& parentJsonObject,
    TypeTag<Outer>)
{
    using Meta = reflection::StaticPropertyMeta<Outer>;

    StatusCode statusCode = StatusCode::Good;
    const auto jsonValue = parentJsonObject[impl::toQString(propertyName)];
    CHECK_R2(jsonValue.isObject(), StatusCode::Bad)
    const auto jsonObject = jsonValue.toObject();

    const auto stringComparator = [](std::string_view s1, std::string_view s2)
    {
        return s1.compare(s2) == 0;
    };

    for (auto iter = jsonObject.constBegin(); iter != jsonObject.constEnd(); ++iter)
    {
        const auto name = iter.key().toStdString();

        Meta::doForKey(
            [&outer, &statusCode, &jsonObject](auto, auto constValue)
        {
            constexpr auto staticPropertyPtr = decltype(constValue)::value;
            if constexpr (staticPropertyPtr == nullptr)
            {
                statusCode = StatusCode::NotFound;
                return;
            }

            using StaticPropertyClass = std::remove_cv_t<std::remove_pointer_t<decltype(staticPropertyPtr)>>;
            using PointerTagClass = PointerTag<staticPropertyPtr>;

            if constexpr (reflection::IsSpecialization<StaticPropertyClass, reflection::StaticProperty>::value)
            {
                CHECK_SC_D(propertyFromJson(outer, jsonObject, PointerTagClass{}),
                           statusCode = sc;)
            }
            else if constexpr (reflection::IsSpecialization<StaticPropertyClass, reflection::StaticPropertyMap>::value)
            {
                CHECK_SC_D(propertyMapFromJson(outer, jsonObject, PointerTagClass{}),
                           statusCode = sc;)
            }
            else if constexpr (reflection::IsSpecialization<StaticPropertyClass, reflection::StaticPropertyProxy>::value)
            {
                CHECK_SC_D(propertyProxyFromJson(outer, jsonObject, PointerTagClass{}),
                           statusCode = sc;)
            }
            else
            {
                static_assert(false, "propertyMapFromJson: unexpected static property type");
            }
        }, name.c_str(), stringComparator);
    }

    return statusCode;
}

template <typename Outer, typename T, const reflection::StaticProperty<Outer, T>* staticPropertyPtr>
StatusCode propertyFromJson(Outer& outer,
    const QJsonObject& parentJsonObject,
    PointerTag<staticPropertyPtr>)
{
    static_assert(staticPropertyPtr != nullptr);

    return propertyFromJson(staticPropertyPtr->name, outer, parentJsonObject,
        PointerToMemberTag<staticPropertyPtr->valuePtr>{});
}

template <typename Outer, typename T, T Outer::* valuePtr>
StatusCode propertyFromJson(std::string_view propertyName,
    Outer& outer,
    const QJsonObject& parentJsonObject,
    PointerToMemberTag<valuePtr>)
{
    static_assert(valuePtr != nullptr);

    if constexpr (std::is_const_v<T>)
    {
#ifdef QT_DEBUG
        using NonConstValueType = std::remove_cv_t<T>;

        NonConstValueType value;
        CHECK_SC_R(namedValueFromJson(propertyName, value, parentJsonObject,
            TypeTag<NonConstValueType>{}))
        return outer.*valuePtr == value ? StatusCode::Good : StatusCode::Bad;
#else
        return StatusCode::GoodNothingTodo;
#endif
    }
    else
    {
        return namedValueFromJson(propertyName, outer.*valuePtr, parentJsonObject, TypeTag<T>{});
    }
}

template <typename T>
StatusCode namedValueFromJson(std::string_view propertyName,
    T& value,
    const QJsonObject& parentJsonObject,
    TypeTag<T>)
{
    const auto jsonValue = parentJsonObject[impl::toQString(propertyName)];
    CHECK_SC_R(valueFromJson(value, jsonValue, TypeTag<T>{}))
    return StatusCode::Good;
}

template <typename T>
StatusCode valueFromJson(T& value, const QJsonValue& jsonValue, TypeTag<T>)
{
    using Type = std::remove_reference_t<T>;

    if constexpr (reflection::IsObject<Type>::value)
    {
        using Meta = reflection::StaticPropertyMeta<Type>;
        constexpr auto staticPropertyMapPtr = Meta::getStaticPropertyMap();
        static_assert(staticPropertyMapPtr != nullptr);
        constexpr auto propertyName = staticPropertyMapPtr->name;

        CHECK_R2(jsonValue.isObject(), StatusCode::Bad)
        const auto jsonObject = jsonValue.toObject();
        QJsonObject parentJsonObject;
        parentJsonObject[impl::toQString(propertyName)] = jsonObject;

        return convertFromJson(value, parentJsonObject, TypeTag<Type>{});
    }
    else if constexpr (std::is_pointer_v<Type>
                       && reflection::IsObject<std::remove_pointer_t<Type>>::value)
    {
        using PlainType = std::remove_pointer_t<Type>;

        using Meta = reflection::StaticPropertyMeta<PlainType>;
        constexpr auto staticPropertyMapPtr = Meta::getStaticPropertyMap();
        static_assert(staticPropertyMapPtr != nullptr);
        constexpr auto propertyName = staticPropertyMapPtr->name;

        CHECK_POINTER_R(value)
        CHECK_R2(jsonValue.isObject(), StatusCode::Bad)
        const auto jsonObject = jsonValue.toObject();
        QJsonObject parentJsonObject;
        parentJsonObject[impl::toQString(propertyName)] = jsonObject;

        return convertFromJson(*value, parentJsonObject, TypeTag<PlainType>{});
    }
    else if constexpr (reflection::IsString<Type>::value)
    {
        CHECK_R2(jsonValue.isString(), StatusCode::Bad)
        value = jsonValue.toString().toStdString();
    }
    else if constexpr (std::is_same_v<Type, bool>)
    {
        CHECK_R2(jsonValue.isBool(), StatusCode::Bad)
        value = jsonValue.toBool();
    }
    else if constexpr (std::is_integral_v<Type>)
    {
        CHECK_R2(jsonValue.isDouble(), StatusCode::Bad)
        value = static_cast<Type>(jsonValue.toInteger());
    }
    else if constexpr (std::is_floating_point_v<Type>)
    {
        CHECK_R2(jsonValue.isDouble(), StatusCode::Bad)
        value = static_cast<Type>(jsonValue.toDouble());
    }
    else if constexpr (reflection::IsIterable<Type>::value)
    {
        CHECK_R2(jsonValue.isArray(), StatusCode::Bad)
        const auto jsonArray = jsonValue.toArray();

        return valueFromJsonArray(value, jsonArray, TypeTag<Type>{});
    }
    else
    {
        static_assert(false, "valueFromJson: unexpected type");
        return StatusCode::Unexpected;
    }
    return StatusCode::Good;
}

template <typename T, typename = std::enable_if_t<reflection::IsIterable<T>::value>>
StatusCode valueFromJsonArray(T& value, const QJsonArray& jsonArray, TypeTag<T>)
{
    if constexpr (reflection::IsInsertable<T>::value)
    {
        using ValueType = typename T::value_type;

        StatusCode statusCode = StatusCode::Good;

        value.clear();

        if constexpr (reflection::HasReserve<T>::value)
        {
            value.reserve(jsonArray.size());
        }

        auto inserter = std::inserter(value, value.end());

        for (auto iter = jsonArray.constBegin(); iter != jsonArray.constEnd(); ++iter)
        {
            ValueType arrayElement;
            CHECK_SC_D(valueFromJson(arrayElement, *iter, TypeTag<ValueType>{}), statusCode = sc; continue;)
            *inserter = std::move(arrayElement);
        }

        return statusCode;
    }
    else
    {
        StatusCode statusCode = StatusCode::Good;

        auto iter = jsonArray.constBegin();

        auto valueIter = std::begin(value);
        auto valueEndIter = std::end(value);

        using ValueType = std::remove_reference_t<decltype(*valueIter)>;

        for (; iter != jsonArray.constEnd() && valueIter != valueEndIter; ++iter, ++valueIter)
        {
            ValueType arrayElement;
            CHECK_SC_D(valueFromJson(arrayElement, *iter, TypeTag<ValueType>{}), statusCode = sc; continue;)
            *valueIter = std::move(arrayElement);
        }

        for (; valueIter != valueEndIter; ++valueIter)
        {
            *valueIter = ValueType{};
        }

        return statusCode;
    }
}

} // namespace extensions
