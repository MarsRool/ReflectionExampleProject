#pragma once
#include "Reflection/Property/staticpropertymeta.h"

#define DECL_VALUE(Type, FieldName, InitialValue) \
Type FieldName{ InitialValue };

#define DECL_STATIC_PROPERTY(Type, FieldName) \
static_assert([]() \
    { \
        using Meta = reflection::StaticPropertyMeta<ThisClass>; \
        static constexpr char propertyName[]{ #FieldName }; \
        constexpr auto staticPropertyPtr = Meta::template defineStaticProperty< \
            propertyName, &ThisClass::FieldName>(); \
        return staticPropertyPtr != nullptr; \
    }());

#define DECL_PROPERTY_INIT(Type, FieldName, InitialValue) \
    DECL_VALUE(Type, FieldName, InitialValue) \
    DECL_STATIC_PROPERTY(Type, FieldName) \

#define DECL_PROPERTY_DEFAULT(Type, FieldName) \
    DECL_PROPERTY_INIT(Type, FieldName, Type{})

#define DECL_BASE_CLASS_INIT(BaseClassType, BaseClassRawName) \
    static_assert([]() \
    { \
        using Meta = reflection::StaticPropertyMeta<ThisClass>; \
        using BaseMeta = reflection::StaticPropertyMeta<BaseClassType>; \
        static constexpr char propertyName[]{ BaseClassRawName }; \
        constexpr auto baseStaticPropertyMap = BaseMeta::getStaticPropertyMap(); \
        if constexpr (baseStaticPropertyMap == nullptr) { \
            return false; \
        } else { \
            constexpr auto staticPropertyPtr = Meta::template defineStaticPropertyProxy< \
                propertyName, *baseStaticPropertyMap>(); \
            return staticPropertyPtr != nullptr; \
        } \
    }());

#define DECL_BASE_CLASS(BaseClassType) \
    DECL_BASE_CLASS_INIT(BaseClassType, "_base_" #BaseClassType)

#define DECL_STATIC_PROPERTY_MAP(ClassType, ClassRawName) \
    static_assert([]() \
    { \
        using Meta = reflection::StaticPropertyMeta<ClassType>; \
        static constexpr char propertyName[]{ ClassRawName }; \
        constexpr auto staticPropertyPtr = Meta::template defineStaticPropertyMap< \
            propertyName>(); \
        return staticPropertyPtr != nullptr; \
    }());

#define DECL_REFLECTION_BODY(ClassType) \
    using ThisClass = ClassType; \
    DECL_STATIC_PROPERTY_MAP(ClassType, #ClassType)
