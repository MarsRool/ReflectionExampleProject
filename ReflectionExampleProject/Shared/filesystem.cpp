#include "Shared/filesystem.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QApplication>

const std::vector<QString> textFormats = {"obj", "txt", "mtl", "vs", "fs", "css"};
const std::vector<QString> binaryFormats = {"png", "jpg", "jpeg", "ft"};

QString FileSystem::removeExtension(const QString& filename)
{
	int dotIndex = filename.lastIndexOf(".");
	return (dotIndex != -1)
		? filename.left(dotIndex + 1)
		: filename;
}

QString FileSystem::getExtension(const QString& filename)
{
	int dotIndex = filename.lastIndexOf(".");
	return (dotIndex != -1)
		? filename.right(filename.length() - dotIndex - 1)
		: "";
}

QString FileSystem::getAbsolutePath(const QString& path)
{
	if (path.contains(":"))
		return path;
	return QApplication::applicationDirPath() + path;
}

bool FileSystem::createFullPathDirs(const QString& path)
{
	QString dirPath = getExtension(path) == "" ? path : QFileInfo(path).absolutePath();
	QDir dir(dirPath);
	if (dir.exists())
		return true;
	return dir.mkpath(dirPath);
}

bool FileSystem::isFileTextFormat(const QString& extension)
{
	for (const auto& txt: textFormats)
		if (extension.toLower() == txt)
			return true;
	return false;
}

bool FileSystem::isFileBinaryFormat(const QString& extension)
{
	for (const auto& binary: binaryFormats)
		if (extension.toLower() == binary)
			return true;
	return false;
}
