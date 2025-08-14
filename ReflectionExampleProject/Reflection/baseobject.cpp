#include "baseobject.h"

namespace reflection
{

BaseObject::BaseObject(const BaseObject& other)
    : BaseObject()
{
    name = other.name;
}

BaseObject::BaseObject(BaseObject&& other)
    : BaseObject()
{
    name = std::move(other.name);
}

} // namespace reflection
