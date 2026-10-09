#pragma once
#include <string_view>
#include <QString>

namespace extensions
{

enum class SerializationFormat
{
	Json,
	Binary
};

namespace impl
{

inline QString toQString(std::string_view str)
{
    return QString::fromUtf8(
        str.data(), static_cast<qsizetype>(str.size()));
}

} // namespace impl

} // namespace extensions
