#pragma once
#include <string>
#include <string_view>
#include "Shared/macroes.h"
#include "Shared/checkmacroes.h"
#include "Shared/customexception.h"

namespace reflection
{

template <class Outer>
class BaseStaticProperty
{
public:
    using ThisClass = BaseStaticProperty<Outer>;

    constexpr BaseStaticProperty(std::string_view name) noexcept
        : name(name)
    {}
    BaseStaticProperty(const ThisClass&) = delete;
    BaseStaticProperty(ThisClass&&) noexcept = delete;
    virtual ~BaseStaticProperty() = default;

    constexpr FORCEINLINE std::string_view getName() const noexcept { return name; }
    FORCEINLINE void setName(std::string_view name)
    {
        if (this->name != name)
            this->name = name;
    }

    FORCEINLINE bool equalsName(const ThisClass& other) const noexcept
    {
        return name == other.name;
    }
    virtual bool equals(const Outer& outer, const ThisClass& other, const Outer& otherOuter) const noexcept
    {
        Q_UNUSED(outer)
        Q_UNUSED(otherOuter)
        return equalsName(other);
    }

    virtual std::string toString(const Outer& outer) const
    {
        Q_UNUSED(outer)
        return {};
    }
    virtual StatusCode toJson(const Outer& outer, QJsonObject& parentJsonObject) const
    {
        Q_UNUSED(outer)
        Q_UNUSED(parentJsonObject)
        return StatusCode::GoodNothingTodo;
    }
    virtual StatusCode fromJson(Outer& outer, const QJsonObject& parentJsonObject) const
    {
        Q_UNUSED(outer)
        Q_UNUSED(parentJsonObject)
        return StatusCode::GoodNothingTodo;
    }

    bool operator==(const ThisClass&) const noexcept = delete;
    auto operator<=>(const ThisClass&) const noexcept = delete;
    ThisClass& operator=(const ThisClass&) = delete;
    ThisClass& operator=(ThisClass&&) noexcept = delete;

private:
    std::string_view name;
};

} // namespace reflection
