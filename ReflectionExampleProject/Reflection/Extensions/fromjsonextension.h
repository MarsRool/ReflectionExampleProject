#pragma once
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QCborMap>

#include "Reflection/Utils/uniquestaticheterogeneousmap.h"
#include "Reflection/Utils/filesystem.h"
#include "Reflection/Utils/typetraits.h"
#include "Reflection/Extensions/serializationformat.h"

namespace reflection
{

template <typename Outer, typename T>
class StaticProperty;

template <typename Outer>
class StaticPropertyMap;

template <typename Outer, typename StaticPropertyT>
class StaticPropertyProxy;


namespace extensions
{

template <typename Outer, typename = std::enable_if_t<IsObject<Outer>::value>>
StatusCode load(Outer& value,
    SerializationFormat serializationFormat,
    const QString& filenameWithoutExt) noexcept
{
    try
    {
        const auto filepath = FileSystem::getAbsolutePath(filenameWithoutExt
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

template <typename Outer, typename = std::enable_if_t<IsObject<Outer>::value>>
StatusCode convertFromJson(Outer& value, const QJsonObject& parentJsonObject, TypeTag<Outer> = {})
{
    constexpr const auto staticPropertyMapPtr = &Outer::staticPropertyMap;

    return propertyMapFromJson(value, parentJsonObject,
        PointerTag<staticPropertyMapPtr>{});
}

template <typename Outer, typename StaticPropertyT, const StaticPropertyProxy<Outer, StaticPropertyT>* staticPropertyProxyPtr>
StatusCode propertyProxyFromJson(typename StaticPropertyProxy<Outer, StaticPropertyT>::TargetOuterClass& outer,
    const QJsonObject& parentJsonObject,
    PointerTag<staticPropertyProxyPtr>)
{
    static_assert(staticPropertyProxyPtr != nullptr);

    using ProxyClass = StaticPropertyProxy<Outer, StaticPropertyT>;
    using TargetStaticPropertyClass = typename ProxyClass::TargetStaticPropertyClass;

    constexpr const auto& targetStaticProperty = staticPropertyProxyPtr->get();

    if constexpr (IsSpecialization<TargetStaticPropertyClass, StaticProperty>::value)
    {
        constexpr const auto propertyName = staticPropertyProxyPtr->getName();
        constexpr const auto valuePtr = targetStaticProperty.getRaw();

        return propertyFromJson(propertyName, outer, parentJsonObject,
            PointerToMemberTag<valuePtr>{});
    }
    else if constexpr (IsSpecialization<TargetStaticPropertyClass, StaticPropertyMap>::value)
    {
        constexpr const auto propertyName = staticPropertyProxyPtr->getName();

        return propertyMapFromJson(propertyName, outer, parentJsonObject,
            PointerTag<&targetStaticProperty>{});
    }
    else
    {
        static_assert(false, "propertyProxyFromJson: unexpected static property type");
        Q_UNUSED(outer)
        return StatusCode::Unexpected;
    }
}

template <typename Outer, const StaticPropertyMap<Outer>* staticPropertyMapPtr>
StatusCode propertyMapFromJson(Outer& outer,
    const QJsonObject& parentJsonObject,
    PointerTag<staticPropertyMapPtr>)
{
    static_assert(staticPropertyMapPtr != nullptr);

    constexpr const auto propertyName = staticPropertyMapPtr->getName();

    return propertyMapFromJson(propertyName, outer, parentJsonObject,
        PointerTag<staticPropertyMapPtr>{});
}

template <typename Outer, const StaticPropertyMap<Outer>* staticPropertyMapPtr>
StatusCode propertyMapFromJson(std::string_view propertyName,
    Outer& outer,
    const QJsonObject& parentJsonObject,
    PointerTag<staticPropertyMapPtr>)
{
    // Note, staticPropertyMapPtr is not used directly here
    // it's necessary to avoid usage of this overload by mistake
    // static_assert(staticPropertyMapPtr != nullptr);

    using StaticPropertyMapClass = StaticPropertyMap<Outer>;
    using KeyType = typename StaticPropertyMapClass::KeyType;

    StatusCode statusCode = StatusCode::Good;
    const auto jsonValue = parentJsonObject[propertyName.data()];
    CHECK_R2(jsonValue.isObject(), StatusCode::Bad)
    const auto jsonObject = jsonValue.toObject();

    const auto stringComparator = [](std::string_view s1, std::string_view s2)
    {
        return s1.compare(s2) == 0;
    };

    for (auto iter = jsonObject.constBegin(); iter != jsonObject.constEnd(); ++iter)
    {
        const auto name = iter.key().toStdString();

        uniqueStaticHeterogeneousMapDoForKey<Outer, KeyType>([]{},
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

            if constexpr (IsSpecialization<StaticPropertyClass, StaticProperty>::value)
            {
                CHECK_SC_D(propertyFromJson(outer, jsonObject, PointerTagClass{}),
                           statusCode = sc;)
            }
            else if constexpr (IsSpecialization<StaticPropertyClass, StaticPropertyMap>::value)
            {
                CHECK_SC_D(propertyMapFromJson(outer, jsonObject, PointerTagClass{}),
                           statusCode = sc;)
            }
            else if constexpr (IsSpecialization<StaticPropertyClass, StaticPropertyProxy>::value)
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

template <typename Outer, typename T, const StaticProperty<Outer, T>* staticPropertyPtr>
StatusCode propertyFromJson(Outer& outer,
    const QJsonObject& parentJsonObject,
    PointerTag<staticPropertyPtr>)
{
    static_assert(staticPropertyPtr != nullptr);

    constexpr const auto propertyName = staticPropertyPtr->getName();
    constexpr const auto valuePtr = staticPropertyPtr->getRaw();

    return propertyFromJson(propertyName, outer, parentJsonObject,
        PointerToMemberTag<valuePtr>{});
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
        using NonConstValueT = std::remove_cv_t<T>;

        NonConstValueT value;
        CHECK_SC_R(namedValueFromJson(propertyName, value, parentJsonObject,
            TypeTag<NonConstValueT>{}))
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
    const auto jsonValue = parentJsonObject[propertyName.data()];
    CHECK_SC_R(valueFromJson(value, jsonValue, TypeTag<T>{}))
    return StatusCode::Good;
}

template <typename T>
StatusCode valueFromJson(T& value, const QJsonValue& jsonValue, TypeTag<T>)
{
    using Type = std::remove_reference_t<T>;

    if constexpr (IsObject<Type>::value)
    {
        CHECK_R2(jsonValue.isObject(), StatusCode::Bad)
        const auto jsonObject = jsonValue.toObject();
        QJsonObject parentJsonObject;
        parentJsonObject[Type::staticPropertyMap.getName().data()] = jsonObject;

        return convertFromJson(value, parentJsonObject, TypeTag<Type>{});
    }
    else if constexpr (std::is_pointer_v<Type>
                       && IsObject<std::remove_pointer_t<Type>>::value)
    {
        using PlainType = std::remove_pointer_t<Type>;

        CHECK_POINTER_R(value)
        CHECK_R2(jsonValue.isObject(), StatusCode::Bad)
        const auto jsonObject = jsonValue.toObject();
        QJsonObject parentJsonObject;
        parentJsonObject[PlainType::staticPropertyMap.getName().data()] = jsonObject;

        return convertFromJson(*value, parentJsonObject, TypeTag<PlainType>{});
    }
    else if constexpr (IsString<Type>::value)
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
    else if constexpr (IsIterable<Type>::value)
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

template <typename T, typename = std::enable_if_t<IsIterable<T>::value>>
StatusCode valueFromJsonArray(T& value, const QJsonArray& jsonArray, TypeTag<T>)
{
    if constexpr (IsInsertable<T>::value)
    {
        using ValueT = typename T::value_type;

        StatusCode statusCode = StatusCode::Good;

        value.clear();

        if constexpr (HasReserve<T>::value)
        {
            value.reserve(jsonArray.size());
        }

        auto inserter = std::inserter(value, value.end());

        for (auto iter = jsonArray.constBegin(); iter != jsonArray.constEnd(); ++iter)
        {
            ValueT arrayElement;
            CHECK_SC_D(valueFromJson(arrayElement, *iter, TypeTag<ValueT>{}), statusCode = sc; continue;)
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

        using ValueT = std::remove_reference_t<decltype(*valueIter)>;

        for (; iter != jsonArray.constEnd() && valueIter != valueEndIter; ++iter, ++valueIter)
        {
            ValueT arrayElement;
            CHECK_SC_D(valueFromJson(arrayElement, *iter, TypeTag<ValueT>{}), statusCode = sc; continue;)
            *valueIter = std::move(arrayElement);
        }

        for (; valueIter != valueEndIter; ++valueIter)
        {
            *valueIter = ValueT{};
        }

        return statusCode;
    }
}

} // namespace extensions

} // namespace reflection
