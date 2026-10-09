#pragma once
#include <QString>
#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QDir>

namespace reflection
{

class FileSystem final
{
    static constexpr auto textFormats = {"obj", "txt", "mtl", "vs", "fs", "css"};
    static constexpr auto binaryFormats = {"png", "jpg", "jpeg", "ft"};

public:
	FileSystem() = delete;
	~FileSystem() = delete;

    static QString removeExtension(const QString& filename)
    {
        const auto dotIndex = filename.lastIndexOf(".");
        return (dotIndex != -1)
                   ? filename.left(dotIndex + 1)
                   : filename;
    }
    static QString getExtension(const QString& filename)
    {
        const auto dotIndex = filename.lastIndexOf(".");
        return (dotIndex != -1)
                   ? filename.right(filename.length() - dotIndex - 1)
                   : "";
    }

    static QString getAbsolutePath(const QString& path)
    {
        if (path.contains(":"))
            return path;
        return QApplication::applicationDirPath() + path;
    }

    static bool createFullPathDirs(const QString& path)
    {
        QString dirPath = getExtension(path) == "" ? path : QFileInfo(path).absolutePath();
        QDir dir(dirPath);
        if (dir.exists())
            return true;
        return dir.mkpath(dirPath);
    }

private:
    static bool isFileTextFormat(const QString& extension)
    {
        for (const auto& txt : textFormats)
            if (extension.toLower() == txt)
                return true;
        return false;
    }
    static bool isFileBinaryFormat(const QString& extension)
    {
        for (const auto& binary : binaryFormats)
            if (extension.toLower() == binary)
                return true;
        return false;
    }
};

} // namespace reflection
