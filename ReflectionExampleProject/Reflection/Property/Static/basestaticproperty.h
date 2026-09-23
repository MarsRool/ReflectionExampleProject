#pragma once
#include <string>
#include <string_view>
#include "Shared/macroes.h"
#include "Shared/checkmacroes.h"
#include "Shared/customexception.h"

namespace reflection
{

template <typename Outer>
class BaseStaticProperty
{
public:
    using ThisClass = BaseStaticProperty<Outer>;

    constexpr BaseStaticProperty(std::string_view name) noexcept
        : name(name)
    {}
    BaseStaticProperty(const ThisClass&) = delete;
    BaseStaticProperty(ThisClass&&) noexcept = delete;

    constexpr FORCEINLINE std::string_view getName() const noexcept { return name; }
    FORCEINLINE void setName(std::string_view name)
    {
        if (this->name != name)
            this->name = name;
    }

    bool operator==(const ThisClass&) const noexcept = delete;
    ThisClass& operator=(const ThisClass&) = delete;
    ThisClass& operator=(ThisClass&&) noexcept = delete;

private:
    std::string_view name;
};

} // namespace reflection
