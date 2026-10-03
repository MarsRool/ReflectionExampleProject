#pragma once

#include "Reflection/Property/staticproperty.h"
#include "Reflection/Property/staticpropertymap.h"
#include "Reflection/Property/staticpropertyproxy.h"
#include "Reflection/Property/staticpropertymeta.h"

#define DECL_VALUE(Type, Name, InitialValue) \
    Type Name{ InitialValue };

#define DECL_STATIC_PROPERTY(Type, Name) \
    static_assert([]() \
    { \
        static constexpr char rawName[] = #Name; \
        using Meta = reflection::StaticPropertyMeta<ThisClass>; \
        constexpr auto staticPropertyPtr = Meta::template define<Type, &ThisClass::Name, rawName>(); \
        staticPropertyMap.template add< \
            decltype(staticPropertyPtr), rawName, staticPropertyPtr>(); \
        return true; \
    }());

#define DECL_PROPERTY_INIT(Type, Name, InitialValue) \
    DECL_VALUE(Type, Name, InitialValue) \
    DECL_STATIC_PROPERTY(Type, Name) \

#define DECL_PROPERTY_DEFAULT(Type, Name) \
    DECL_PROPERTY_INIT(Type, Name, Type{})

//TODO: make smth similar to StaticPropertyMeta for proxy and remove the definition of static constexpr StaticPropertyMapProxy

#define DECL_BASE_CLASS_INIT(BaseClassType, BaseClassRawName) \
    static_assert([]() \
    { \
        using BaseStaticPropertyMap = reflection::StaticPropertyMap<BaseClassType>; \
        using StaticPropertyMapProxy = reflection::StaticPropertyProxy<ThisClass, BaseStaticPropertyMap>; \
        static constexpr char baseClassAlias[] = BaseClassRawName; \
        static constexpr StaticPropertyMapProxy basePropertyMapProxy{ baseClassAlias, BaseClassType::staticPropertyMap }; \
        constexpr auto basePropertyMapProxyPtr = &basePropertyMapProxy; \
        staticPropertyMap.template add< \
            decltype(basePropertyMapProxyPtr), baseClassAlias, basePropertyMapProxyPtr>(); \
        return true; \
    }());

#define DECL_BASE_CLASS(BaseClassType) \
    DECL_BASE_CLASS_INIT(BaseClassType, "_base_" #BaseClassType)


// TODO: remove staticPropertyMap definition

#define DECL_REFLECTION_BODY(ClassName) \
    using ThisClass = ClassName; \
    using ThisStaticPropertyMap = reflection::StaticPropertyMap<ThisClass>; \
    struct Meta \
    { \
        static constexpr char rawAlias[] = #ClassName; \
    }; \
    static constexpr ThisStaticPropertyMap staticPropertyMap{ ThisClass::Meta::rawAlias }; \
    DECL_PROPERTY_INIT(const std::string_view, type, ThisClass::Meta::rawAlias)
