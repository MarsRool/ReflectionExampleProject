#pragma once

#include "Shared/uniqueidcounter.h"
#include "Shared/uniquestaticmap.h"
#include "Reflection/Property/Static/staticproperty.h"
#include "Reflection/Property/Static/staticpropertymap.h"
#include "Reflection/Property/Static/staticpropertyproxy.h"
#include "Reflection/Property/Instance/customproperty.h"
#include "Reflection/Property/Instance/property.h"
#include "Reflection/reflectable.h"

#define DECL_VALUE(Type, Name, InitialValue) \
    Type Name{ InitialValue };\

#define DECL_PROPERTY_GETTER(Type, Name) \
    reflection::Property<ThisClass, Type> get ## Name ## Property() const \
    { \
        return reflection::Property<ThisClass, Type>( \
            const_cast<ThisClass&>(*this), \
            ThisClass::Name ## StaticProperty); \
    }

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
    DECL_PROPERTY_GETTER(Type, Name) \
    DECL_STATIC_PROPERTY(Type, Name) \
    DECL_PROPERTY_INDEX(Name)

#define DECL_PROPERTY_DEFAULT(Type, Name) \
    DECL_PROPERTY_INIT(Type, Name, Type{})


#define DECL_PROPERTY_CUSTOM_INIT(Type, Name, Getter, Setter) \
    reflection::BaseStaticProperty Name ## StaticProperty{ #Name }; \
    reflection::CustomProperty<Type> Name ## CustomProperty \
    { \
        Getter, \
        Setter, \
        testStaticProperty \
    };


#define DECL_REFLECTABLE_BASE(SharedExport, ClassName) \
    class SharedExport ClassName : public reflection::Reflectable<ClassName>

#define DECL_REFLECTABLE(SharedExport, ClassName, BaseClassName) \
    class SharedExport ClassName : public BaseClassName, public reflection::Reflectable<ClassName>

#define DECL_REFLECTION_BODY(ClassName, BaseClassName) \
    public: \
        using BaseClass = BaseClassName; \
        using ThisClass = ClassName; \
        using ThisPropertyMap = reflection::PropertyMap<ThisClass>; \
        using ThisStaticPropertyMap = reflection::StaticPropertyMap<ThisClass>; \
        using ReflectionClass = reflection::Reflectable<ClassName>; \
        struct Meta \
        { \
            static constexpr char rawAlias[] = #ClassName; \
        }; \
        static constexpr ThisStaticPropertyMap staticPropertyMap{ ThisClass::Meta::rawAlias }; \
        static constexpr bool hasBaseClass = reflection::ReflectionBaseClassHandler< \
            ThisClass, BaseClass, reflection::Reflectable<ThisClass>::basePropertyName>::initializeReflectionInheritance(); \
        using Z_ ## BaseClass ## PropertyForceInitializer = std::integral_constant<bool, hasBaseClass>; \
        DECL_PROPERTY_INIT(const std::string_view, type, ThisClass::Meta::rawAlias)
