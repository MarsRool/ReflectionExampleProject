#pragma once

#include <QString>
#include <string_view>

namespace extensions::impl
{

inline QString toQString(std::string_view str)
{
    return QString::fromUtf8(
        str.data(), static_cast<qsizetype>(str.size()));
}

} // namespace extensions::impl
