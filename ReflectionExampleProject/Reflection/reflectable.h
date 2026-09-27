#pragma once

#include <mutex>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCborValue>
#include <QCborMap>
#include <QFile>

#include "Shared/filesystem.h"
#include "Reflection/serializationformat.h"
#include "Reflection/Property/Static/staticproperty.h"
#include "Reflection/Property/Static/staticpropertymap.h"
#include "Reflection/Utils/valueutils.h"

namespace reflection
{

template <typename Outer, typename = std::enable_if_t<IsObject<Outer>::value, void>>
StatusCode load(Outer& value, SerializationFormat serializationFormat, const QString& filenameWithoutExt) noexcept
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

        CHECK_SC_R(Outer::staticPropertyMap.fromJson(value, loadDoc.object()))

        qInfo() << "load complete:" << loadFile.fileName();
        return StatusCode::Good;
    CATCH_R2("load ex: ", StatusCode::Bad)
}

} // namespace reflection
