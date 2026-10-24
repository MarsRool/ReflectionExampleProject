#pragma once

#include <string_view>

namespace reflection
{

template <typename Outer>
struct StaticTypeInfo
{
    using OuterType = Outer;

    const std::string_view name;
};

} // namespace reflection
