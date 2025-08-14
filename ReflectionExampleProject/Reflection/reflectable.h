#pragma once

#include <mutex>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCborValue>
#include <QCborMap>
#include <QFile>

#include "Shared/filesystem.h"
#include "Reflection/serializationformat.h"
#include "Reflection/Property/Instance/property.h"
#include "Reflection/Property/Instance/propertymap.h"

namespace reflection
{

template <class Outer>
class Reflectable
{
public:
    using ThisClass = Reflectable<Outer>;
    using ThisPropertyMap = PropertyMap<Outer>;
    using StaticPropertyPtr = const BaseStaticProperty<Outer>*;
    using ThisStaticPropertyMap = StaticPropertyMap<Outer>;
    using PropertyMapGetter = std::function<ThisPropertyMap&()>;

    static constexpr char basePropertyName[] = "_base";
    static constexpr char typePropertyName[] = "type";

    static constexpr bool outerHasBaseClass = !std::is_void_v<typename Outer::BaseClass>;

    Reflectable() = default;

    ThisPropertyMap getPropertyMap() const
    {
        return ThisPropertyMap(
            const_cast<Outer&>(static_cast<const Outer&>(*this)),
            Outer::staticPropertyMap);
    }

    StaticPropertyPtr findParentByType(std::string_view type) const
    {
        return findParentByType(getPropertyMap(), type);
    }
    template <class OtherOuter>
    StaticPropertyMap<OtherOuter>* findParentByType(std::string_view type) const
    {
        return findParentByType<OtherOuter>(getPropertyMap(), type);
    }

	StatusCode save(SerializationFormat serializationFormat, const QString& filenameWithoutExt) const noexcept;
	StatusCode load(SerializationFormat serializationFormat, const QString& filenameWithoutExt) noexcept;

    bool operator==(const ThisClass&) const noexcept = default;

    static std::string_view getType() { return Outer::typeValue; }
    static StaticPropertyPtr findParentByType(const ThisPropertyMap& propertyMap, std::string_view type);
    template <class OtherOuter>
    static StaticPropertyMap<OtherOuter>* findParentByType(const ThisPropertyMap& propertyMap, std::string_view type);
};

template <class Outer>
StatusCode Reflectable<Outer>::save(SerializationFormat serializationFormat, const QString& filenameWithoutExt) const noexcept
{
	TRY
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
        CHECK_SC_R(getPropertyMap().toJson(jsonObject))
		saveFile.write(serializationFormat == SerializationFormat::Json
			? QJsonDocument(jsonObject).toJson()
			: QCborValue::fromJsonValue(jsonObject).toCbor());

        qInfo() << "Reflectable save complete: " << saveFile.fileName();
		return StatusCode::Good;
    CATCH_R2("Reflectable::save ex: ", StatusCode::Bad)
}

template <class Outer>
StatusCode Reflectable<Outer>::load(SerializationFormat serializationFormat, const QString& filenameWithoutExt) noexcept
{
	TRY
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

        CHECK_SC_R(getPropertyMap().fromJson(loadDoc.object()))

        qInfo() << "Reflectable load complete:" << loadFile.fileName();
		return StatusCode::Good;
    CATCH_R2("Reflectable::load ex: ", StatusCode::Bad)
}

template <class Outer>
Reflectable<Outer>::StaticPropertyPtr Reflectable<Outer>::findParentByType(const ThisPropertyMap& propertyMap, std::string_view type)
{
    if (type.empty() || !propertyMap.contains(basePropertyName))
        return nullptr;
    const auto* parent = propertyMap.at(basePropertyName);
    CHECK_POINTER_R2(parent, nullptr)
    if (parent->getType() == type)
    {
        return parent;
    }
    return findParentByType(*parent, type);
}

template <class Outer>
template <class OtherOuter>
StaticPropertyMap<OtherOuter>* Reflectable<Outer>::findParentByType(const ThisPropertyMap& propertyMap, std::string_view type)
{
    auto* parent = findParentByType(propertyMap, type);
    return parent ? dynamic_cast<StaticPropertyMap<OtherOuter>*>(parent) : nullptr;
}

} // namespace reflection
