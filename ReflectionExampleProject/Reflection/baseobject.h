#pragma once
#include "Reflection/reflection.h"

namespace reflection
{

template <class ClassType, class BaseClassType, const char baseName[]>
class ReflectionBaseClassHandler
{
public:
    using ThisClass = ClassType;
    using BaseClass = BaseClassType;
    using BaseStaticPropertyMap = StaticPropertyMap<BaseClass>;
    using StaticPropertyMapProxy = StaticPropertyProxy<ThisClass, BaseStaticPropertyMap>;
    static_assert(!std::is_void_v<BaseClass>);

    static constexpr StaticPropertyMapProxy basePropertyMapProxy{ baseName, BaseClassType::staticPropertyMap };
    static consteval bool initializeReflectionInheritance()
    {
        using StaticPropertyPtr = const BaseStaticProperty<ThisClass>*;
        constexpr StaticPropertyPtr basePropertyMapProxyPtr = &basePropertyMapProxy;
        constexpr auto value = ThisClass::staticPropertyMap.
            template add<baseName, basePropertyMapProxyPtr>();
        Q_UNUSED(value);
        return true;
    }
};

template <class ClassType, const char baseName[]>
class ReflectionBaseClassHandler<ClassType, void, baseName>
{
public:
    using ThisClass = ClassType;
    using BaseClass = void;

    static consteval bool initializeReflectionInheritance()
    {
        return false;
    }
};

class BaseObject : public reflection::Reflectable<BaseObject>
{
    DECL_REFLECTION_BODY(BaseObject, void)
    BaseObject() : reflection::Reflectable<BaseObject>() {}
    BaseObject(const BaseObject& other);
    BaseObject(BaseObject&& other);

    DECL_PROPERTY_DEFAULT(std::string, name)
    DECL_PROPERTIES_COUNT()

    bool operator==(const ThisClass&) const noexcept = default;
};

} // namespace reflection
