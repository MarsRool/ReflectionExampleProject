#pragma once

class FileSystem final
{
public:
	FileSystem() = delete;
	~FileSystem() = delete;

	static QString removeExtension(const QString& filename);
	static QString getExtension(const QString& filename);

	static QString getAbsolutePath(const QString& path);

	static bool createFullPathDirs(const QString& path);

private:
	static bool isFileTextFormat(const QString& extension);
	static bool isFileBinaryFormat(const QString& extension);
};
