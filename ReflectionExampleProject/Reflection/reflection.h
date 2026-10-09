#pragma once
#include "Reflection/Property/staticpropertymeta.h"

#define REFLECTION_UNPAREN(...) __VA_ARGS__

#define DECL_PROPERTY_NAME_WRAPPER(FieldName, FieldRawName) \
    struct Reflection_MetaInfo_ ## FieldName \
    { \
        static constexpr char propertyName[]{ FieldRawName }; \
    };

#define DECL_VALUE(Type, FieldName, InitialValue) \
    Type FieldName{ InitialValue };

#define DECL_VALUE_DEFAULT(Type, FieldName) \
    Type FieldName{};

#define DECL_STATIC_PROPERTY(FieldName) \
    DECL_PROPERTY_NAME_WRAPPER(FieldName, #FieldName) \
    static_assert([]() constexpr \
    { \
        using Meta = reflection::StaticPropertyMeta<ThisClass>; \
        using MetaInfo = Reflection_MetaInfo_ ## FieldName; \
        constexpr auto staticPropertyPtr = Meta::template defineStaticProperty< \
            MetaInfo::propertyName, &ThisClass::FieldName>(); \
        return staticPropertyPtr != nullptr; \
    }());

#define DECL_PROPERTY_INIT(Type, FieldName, InitialValue) \
    DECL_VALUE(Type, FieldName, InitialValue) \
    DECL_STATIC_PROPERTY(FieldName) \

#define DECL_PROPERTY_DEFAULT(Type, FieldName) \
    DECL_VALUE_DEFAULT(Type, FieldName) \
    DECL_STATIC_PROPERTY(FieldName) \


#define DECL_BASE_CLASS_INIT(BaseClassType, BaseClassRawName) \
    DECL_PROPERTY_NAME_WRAPPER(Base_ ## BaseClassType, BaseClassRawName) \
    static_assert([]() constexpr \
    { \
        using Meta = reflection::StaticPropertyMeta<ThisClass>; \
        using BaseMeta = reflection::StaticPropertyMeta<BaseClassType>; \
        using MetaInfo = Reflection_MetaInfo_Base_ ## BaseClassType; \
        constexpr auto baseStaticPropertyMapPtr = BaseMeta::getStaticPropertyMap(); \
        if constexpr (baseStaticPropertyMapPtr == nullptr) { \
            return false; \
        } else { \
            constexpr auto staticPropertyPtr = Meta::template defineStaticPropertyProxy< \
                MetaInfo::propertyName, *baseStaticPropertyMapPtr>(); \
            return staticPropertyPtr != nullptr; \
        } \
    }());

#define DECL_BASE_CLASS(BaseClassType) \
    DECL_BASE_CLASS_INIT(BaseClassType, "_base_" #BaseClassType)


#define DECL_STATIC_PROPERTY_MAP(ClassType, ClassRawName) \
    DECL_PROPERTY_NAME_WRAPPER(ClassType, ClassRawName) \
    static_assert([]() constexpr \
    { \
        using Meta = reflection::StaticPropertyMeta<ThisClass>; \
        using MetaInfo = Reflection_MetaInfo_ ## ClassType; \
        constexpr auto staticPropertyPtr = Meta::template defineStaticPropertyMap< \
            MetaInfo::propertyName>(); \
        return staticPropertyPtr != nullptr; \
    }());

#define DECL_REFLECTION_BODY(ClassType) \
    using ThisClass = ClassType; \
    DECL_STATIC_PROPERTY_MAP(ClassType, #ClassType)


#define DECL_PROPERTY_NAME_WRAPPER_MULTI(...) \
    struct Reflection_MetaInfo \
    { \
        static constexpr char propertyNames[]{ #__VA_ARGS__ }; \
    };

#define DECL_STATIC_PROPERTY_MULTI(...) \
    DECL_PROPERTY_NAME_WRAPPER_MULTI(__VA_ARGS__) \
    static_assert([]() constexpr \
    { \
        using Meta = reflection::StaticPropertyMeta<ThisClass>; \
        return Meta::template defineStaticPropertyMulti< \
            Reflection_MetaInfo::propertyNames, __VA_ARGS__>(); \
    }());

#define DECL_REFLECTION_NONINTRUSIVE(ClassType, ...) \
    namespace reflection_nonintrusive_ ## ClassType \
    { \
        using ThisClass = ClassType; \
        DECL_STATIC_PROPERTY_MAP(ClassType, #ClassType) \
        DECL_STATIC_PROPERTY_MULTI(__VA_ARGS__) \
    }

#define DECL_REFLECTION_TEMPLATE_NONINTRUSIVE(TemplateDeclaration, TemplateArguments, ClassType, ...) \
    namespace reflection_nonintrusive_ ## ClassType \
    { \
        template <REFLECTION_UNPAREN TemplateDeclaration> \
        struct ReflectionDeclarator \
        { \
            using ThisClass = ClassType<REFLECTION_UNPAREN TemplateArguments>; \
            DECL_STATIC_PROPERTY_MAP(ClassType, #ClassType) \
            DECL_STATIC_PROPERTY_MULTI(__VA_ARGS__) \
        }; \
    } \
    template <REFLECTION_UNPAREN TemplateDeclaration> \
        auto getNonintrusiveReflectionDeclarator( \
            ClassType<REFLECTION_UNPAREN TemplateArguments>*) \
        -> reflection_nonintrusive_ ## ClassType::ReflectionDeclarator< \
            REFLECTION_UNPAREN TemplateArguments>;


#define DECL_BASE_CLASS_MULTI(NamePrefix, ...) \
    struct Reflection_BaseMetaInfo \
    { \
        static constexpr char namePrefix[]{ NamePrefix }; \
        static constexpr char baseClassNames[]{ #__VA_ARGS__ }; \
    }; \
    static_assert([]() constexpr \
    { \
        using Meta = reflection::StaticPropertyMeta<ThisClass>; \
        return Meta::template defineBaseClassMulti< \
            Reflection_BaseMetaInfo::namePrefix, \
            Reflection_BaseMetaInfo::baseClassNames, \
            __VA_ARGS__>(); \
    }());

#define DECL_REFLECTION_INHERITANCE_INIT_NONINTRUSIVE(ClassType, NamePrefix, ...) \
    namespace reflection_nonintrusive_ ## ClassType \
    { \
        using ThisClass = ClassType; \
        DECL_BASE_CLASS_MULTI(NamePrefix, __VA_ARGS__) \
    }

#define DECL_REFLECTION_INHERITANCE_NONINTRUSIVE(ClassType, ...) \
    DECL_REFLECTION_INHERITANCE_INIT_NONINTRUSIVE(ClassType, "_base_", __VA_ARGS__)

#define DECL_REFLECTION_INHERITANCE_INIT_TEMPLATE_NONINTRUSIVE( \
    TemplateDeclaration, TemplateArguments, ClassType, NamePrefix, ...) \
        namespace reflection_nonintrusive_ ## ClassType \
        { \
            template <REFLECTION_UNPAREN TemplateDeclaration> \
            struct ReflectionInheritanceDeclarator \
            { \
                using ThisClass = ClassType<REFLECTION_UNPAREN TemplateArguments>; \
                DECL_BASE_CLASS_MULTI(NamePrefix, __VA_ARGS__) \
            }; \
        } \
        template <REFLECTION_UNPAREN TemplateDeclaration> \
        auto getNonintrusiveReflectionInheritanceDeclarator( \
            ClassType<REFLECTION_UNPAREN TemplateArguments>**) \
        -> reflection_nonintrusive_ ## ClassType::ReflectionInheritanceDeclarator< \
            REFLECTION_UNPAREN TemplateArguments>;

#define DECL_REFLECTION_INHERITANCE_TEMPLATE_NONINTRUSIVE( \
    TemplateDeclaration, TemplateArguments, ClassType, ...) \
        DECL_REFLECTION_INHERITANCE_INIT_TEMPLATE_NONINTRUSIVE( \
            TemplateDeclaration, TemplateArguments, ClassType, "_base_", __VA_ARGS__)
