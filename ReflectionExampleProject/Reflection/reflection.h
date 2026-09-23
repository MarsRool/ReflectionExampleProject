#pragma once

#include "Shared/uniqueidcounter.h"
#include "Shared/uniquestaticmap.h"
#include "Shared/uniquestaticheterogeneousmap.h"
#include "Reflection/Property/Static/staticproperty.h"
#include "Reflection/Property/Static/staticpropertymap.h"
#include "Reflection/Property/Static/staticpropertyproxy.h"
#include "Reflection/reflectable.h"

#define DECL_VALUE(Type, Name, InitialValue) \
    Type Name{ InitialValue };

#define DECL_STATIC_PROPERTY(Type, Name) \
    static constexpr char Name ## StaticPropertyName[] = #Name; \
    static constexpr reflection::StaticProperty<ThisClass, Type> Name ## StaticProperty \
    { Name ## StaticPropertyName, &ThisClass::Name };

#define DECL_PROPERTY_INDEX(Name) \
    static constexpr std::size_t Name ## PropertyIndex \
    { \
        []() \
        { \
            using StaticPropertyPtr = typename ThisClass::ThisStaticPropertyMap::StaticPropertyPtr; \
            using StaticPropertyDoublePtr = typename ThisClass::ThisStaticPropertyMap::StaticPropertyDoublePtr; \
            static constexpr StaticPropertyPtr staticPropertyPtr = &ThisClass::Name ## StaticProperty; \
            static constexpr StaticPropertyDoublePtr staticPropertyDoublePtr = &staticPropertyPtr; \
            static constexpr auto value = staticPropertyMap.template add<Name ## StaticPropertyName, staticPropertyDoublePtr>(); \
            Q_UNUSED(value); \
            return uniqueId<ThisClass>([]{}); \
        }() \
    }; \
    using Z_ ## Name ## PropertyForceInitializer = std::array<bool, Name ## PropertyIndex>;

#define DECL_PROPERTIES_COUNT(Name) \
    static constexpr std::size_t propertiesCount{ uniqueId<ThisClass>([]{}) };

#define DECL_PROPERTY_INIT(Type, Name, InitialValue) \
    DECL_VALUE(Type, Name, InitialValue) \
    DECL_STATIC_PROPERTY(Type, Name) \
    DECL_PROPERTY_INDEX(Name)

#define DECL_PROPERTY_DEFAULT(Type, Name) \
    DECL_PROPERTY_INIT(Type, Name, Type{})


#define DECL_BASE_CLASS(BaseClassName) \
    static constexpr bool Z_hasBaseClass_ ## BaseClassName = \
    { \
        [](auto baseClassInst) \
        { \
            using BaseClassType = std::remove_pointer_t<decltype(baseClassInst)>;\
            if constexpr (!std::is_same_v<decltype(baseClassInst), void*>) \
            { \
                using BaseStaticPropertyMap = reflection::StaticPropertyMap<BaseClassType>; \
                using StaticPropertyMapProxy = reflection::StaticPropertyProxy<ThisClass, BaseStaticPropertyMap>; \
                using StaticPropertyPtr = const reflection::BaseStaticProperty<ThisClass>* const; \
                using StaticPropertyDoublePtr = StaticPropertyPtr*; \
                static constexpr StaticPropertyMapProxy basePropertyMapProxy{ reflection::basePropertyName, BaseClassType::staticPropertyMap }; \
                static constexpr StaticPropertyPtr basePropertyMapProxyPtr = &basePropertyMapProxy; \
                static constexpr StaticPropertyDoublePtr basePropertyMapProxyDoublePtr = &basePropertyMapProxyPtr; \
                static constexpr auto value = staticPropertyMap.template add<reflection::basePropertyName, basePropertyMapProxyDoublePtr>(); \
                Q_UNUSED(value); \
                return true; \
            } \
            else \
            { \
                return false; \
            } \
        }(static_cast<BaseClassName*>(nullptr)) \
    }; \
    using Z_BaseClass_ ## BaseClassName ## PropertyForceInitializer = std::integral_constant<bool, Z_hasBaseClass_ ## BaseClassName>; \

#define DECL_REFLECTION_BODY(ClassName) \
    using ThisClass = ClassName; \
    using ThisStaticPropertyMap = reflection::StaticPropertyMap<ThisClass>; \
    struct Meta \
    { \
        static constexpr char rawAlias[] = #ClassName; \
    }; \
    static constexpr ThisStaticPropertyMap staticPropertyMap{ ThisClass::Meta::rawAlias }; \
    DECL_PROPERTY_INIT(const std::string_view, type, ThisClass::Meta::rawAlias)
