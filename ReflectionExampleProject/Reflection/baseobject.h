#pragma once
#include "Reflection/reflection.h"

namespace reflection
{

class BaseObject : public reflection::Reflectable<BaseObject>
{
    DECL_REFLECTION_BODY(BaseObject, void)
    BaseObject() : reflection::Reflectable<BaseObject>() {}
    BaseObject(const BaseObject& other);
    BaseObject(BaseObject&& other);

    DECL_PROPERTY_DEFAULT(std::string, name)
    DECL_PROPERTIES_COUNT()

    bool operator==(const ThisClass& other) const noexcept
    {
        return reflection::Reflectable<ThisClass>::operator==(other);
    }
};

} // namespace reflection
