#pragma once
#include <string_view>

namespace reflection
{

template <typename Outer>
struct BaseStaticProperty
{
    using ThisClass = BaseStaticProperty<Outer>;

    const std::string_view name;
};

} // namespace reflection
