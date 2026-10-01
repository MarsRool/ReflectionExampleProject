#pragma once

#include "Reflection/Property/staticproperty.h"
#include "Reflection/Property/staticpropertymap.h"
#include "Reflection/Property/staticpropertyproxy.h"

#define DECL_VALUE(Type, Name, InitialValue) \
    Type Name{ InitialValue };

#define DECL_STATIC_PROPERTY(Type, Name) \
    static_assert([]() \
    { \
        static constexpr char rawName[] = #Name; \
        using Meta = reflection::StaticPropertyClassMeta<ThisClass>; \
        Meta::template define<Type, &ThisClass::Name, rawName>(); \
        constexpr auto staticPropertyPtr = Meta::template get<&ThisClass::Name>(); \
        staticPropertyMap.template add< \
            decltype(staticPropertyPtr), rawName, staticPropertyPtr>(); \
        return true; \
    }());

#define DECL_PROPERTY_INIT(Type, Name, InitialValue) \
    DECL_VALUE(Type, Name, InitialValue) \
    DECL_STATIC_PROPERTY(Type, Name) \

#define DECL_PROPERTY_DEFAULT(Type, Name) \
    DECL_PROPERTY_INIT(Type, Name, Type{})

//TODO: make smth similar to StaticPropertyClassMeta for proxy and remove the definition of static constexpr StaticPropertyMapProxy

#define DECL_BASE_CLASS(BaseClassName) \
    static_assert([]() \
    { \
        using BaseStaticPropertyMap = reflection::StaticPropertyMap<BaseClassName>; \
        using StaticPropertyMapProxy = reflection::StaticPropertyProxy<ThisClass, BaseStaticPropertyMap>; \
        static constexpr char baseClassAlias[] = "_base_" #BaseClassName; \
        static constexpr StaticPropertyMapProxy basePropertyMapProxy{ baseClassAlias, BaseClassName::staticPropertyMap }; \
        constexpr auto basePropertyMapProxyPtr = &basePropertyMapProxy; \
        staticPropertyMap.template add< \
            decltype(basePropertyMapProxyPtr), baseClassAlias, basePropertyMapProxyPtr>(); \
        return true; \
    }());

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
