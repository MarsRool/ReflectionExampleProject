#pragma once

#include "reflection/property/nonintrusiveregistrationparser.h"
#include "reflection/property/staticpropertytraversal.h"

#define REFLECTION_UNPAREN_(...) __VA_ARGS__
#define REFLECTION_INVOKE_(Macro, Args) Macro Args

#define REFLECTION_PROPERTY_NAME_WRAPPER_MULTI_(StructPrefix, RawName) \
    struct ReflectionMetaInfo ## StructPrefix \
    { \
        static constexpr char value[] = RawName; \
    };

#define REFLECTION_PROPERTY_NAME_WRAPPER_MULTI_2_(StructPrefix, RawName1, RawName2) \
    struct ReflectionMetaInfo ## StructPrefix \
    { \
        static constexpr char value1[] = RawName1; \
        static constexpr char value2[] = RawName2; \
    };


#define REFLECTION_BEGIN_CUSTOM(Type, TypeRawName) \
    friend struct ::reflection::IntrusiveLineScanner<Type>; \
    template <std::size_t line = 0, typename Outer = Type> \
    struct z_reflection_slot \
    { \
        using OuterType = Outer; \
    }; \
    friend constexpr auto z_reflection_type_info(::reflection::TypeTag<Type>) \
    { \
        return ::reflection::StaticTypeInfo<Type>{ TypeRawName }; \
    } \
    friend constexpr auto z_reflection_begin( \
        ::reflection::TypeTag<Type>) \
    { \
        return std::integral_constant<std::size_t, __LINE__>{}; \
    }

#define REFLECTION_BEGIN(Type) \
    REFLECTION_BEGIN_CUSTOM(Type, #Type)

#define REFLECTION_END() \
    friend constexpr auto z_reflection_end( \
        ::reflection::TypeTag<typename z_reflection_slot<>::OuterType>) \
    { \
        return std::integral_constant<std::size_t, __LINE__>{}; \
    }


#define REFLECTION_PROPERTY_CUSTOM(Type, Name, RawName, ...) \
    Type Name{ __VA_ARGS__ }; \
    friend constexpr auto z_reflection_entry( \
        z_reflection_slot<__LINE__> slot) \
    { \
        using Outer = typename decltype(slot)::OuterType; \
        (void)slot; \
        return ::reflection::StaticProperty<Outer, Type>{ RawName, &Outer::Name }; \
    }

#define REFLECTION_PROPERTY_IMPL_(Type, Name, ...) \
    REFLECTION_PROPERTY_CUSTOM(Type, Name, #Name, __VA_ARGS__)

#define REFLECTION_PROPERTY(...) \
    REFLECTION_INVOKE_(REFLECTION_PROPERTY_IMPL_, (__VA_ARGS__, ))


#define REFLECTION_BASE_CLASS_CUSTOM(Type, TypeRawName) \
    friend constexpr auto z_reflection_entry( \
        z_reflection_slot<__LINE__>) \
    { \
        return ::reflection::StaticTypeInfo<Type>{ TypeRawName }; \
    }

#define REFLECTION_BASE_CLASS(Type) \
    REFLECTION_BASE_CLASS_CUSTOM(Type, "_base_" #Type)


// The final nullptr/void is an internal sentinel, allowing omitted variadic
// arguments in strict C++17. The parsers exclude it from the resulting tuple.
#define REFLECTION_CUSTOM_NONINTRUSIVE_IMPL_(Type, TypeRawName, MembersRawNames, IncludesSentinelName, ...) \
    REFLECTION_PROPERTY_NAME_WRAPPER_MULTI_(Type, MembersRawNames) \
    constexpr auto z_reflection_type_info_nonintrusive( \
        ::reflection::TypeTag<Type>) \
    { \
        return ::reflection::StaticTypeInfo<Type>{ TypeRawName }; \
    } \
    constexpr auto z_reflection_properties_nonintrusive( \
        ::reflection::TypeTag<Type>) \
    { \
        return ::reflection::NonIntrusivePropertyParser< \
            Type, \
            ReflectionMetaInfo ## Type::value, \
            IncludesSentinelName, __VA_ARGS__>::staticProperties; \
    }

#define REFLECTION_CUSTOM_NONINTRUSIVE_WITH_SENTINEL_(Type, TypeRawName, MembersRawNames, ...) \
    REFLECTION_CUSTOM_NONINTRUSIVE_IMPL_(Type, TypeRawName, MembersRawNames, false, __VA_ARGS__)

#define REFLECTION_CUSTOM_NONINTRUSIVE(...) \
    REFLECTION_INVOKE_(REFLECTION_CUSTOM_NONINTRUSIVE_WITH_SENTINEL_, \
        (__VA_ARGS__, nullptr))

#define REFLECTION_NONINTRUSIVE_IMPL_(Type, ...) \
    REFLECTION_CUSTOM_NONINTRUSIVE_IMPL_(Type, #Type, #__VA_ARGS__, true, __VA_ARGS__)

#define REFLECTION_NONINTRUSIVE(...) \
    REFLECTION_INVOKE_(REFLECTION_NONINTRUSIVE_IMPL_, (__VA_ARGS__, nullptr))


#define REFLECTION_TEMPLATE_CUSTOM_NONINTRUSIVE_IMPL_( \
    TemplateDeclaration, TemplateArguments, Type, TypeRawName, MembersRawNames, IncludesSentinelName, ...) \
    REFLECTION_PROPERTY_NAME_WRAPPER_MULTI_(Template ## Type, MembersRawNames) \
    template <REFLECTION_UNPAREN_ TemplateDeclaration> \
    constexpr auto z_reflection_type_info_nonintrusive( \
        ::reflection::TypeTag<Type<REFLECTION_UNPAREN_ TemplateArguments>>) \
    { \
        return ::reflection::StaticTypeInfo<Type<REFLECTION_UNPAREN_ TemplateArguments>>{ TypeRawName }; \
    } \
    template <REFLECTION_UNPAREN_ TemplateDeclaration> \
    constexpr auto z_reflection_properties_nonintrusive( \
        ::reflection::TypeTag<Type<REFLECTION_UNPAREN_ TemplateArguments>>) \
    { \
        return ::reflection::NonIntrusivePropertyParser< \
            Type<REFLECTION_UNPAREN_ TemplateArguments>, \
            ReflectionMetaInfo ## Template ## Type::value, \
            IncludesSentinelName, __VA_ARGS__>::staticProperties; \
    }

#define REFLECTION_TEMPLATE_CUSTOM_NONINTRUSIVE_WITH_SENTINEL_( \
    TemplateDeclaration, TemplateArguments, Type, TypeRawName, MembersRawNames, ...) \
    REFLECTION_TEMPLATE_CUSTOM_NONINTRUSIVE_IMPL_( \
        TemplateDeclaration, TemplateArguments, Type, TypeRawName, MembersRawNames, false, __VA_ARGS__)

#define REFLECTION_TEMPLATE_CUSTOM_NONINTRUSIVE(...) \
    REFLECTION_INVOKE_(REFLECTION_TEMPLATE_CUSTOM_NONINTRUSIVE_WITH_SENTINEL_, \
        (__VA_ARGS__, nullptr))

#define REFLECTION_TEMPLATE_NONINTRUSIVE_IMPL_( \
    TemplateDeclaration, TemplateArguments, Type, ...) \
    REFLECTION_TEMPLATE_CUSTOM_NONINTRUSIVE_IMPL_( \
        TemplateDeclaration, TemplateArguments, Type, #Type, #__VA_ARGS__, true, __VA_ARGS__)

#define REFLECTION_TEMPLATE_NONINTRUSIVE(...) \
    REFLECTION_INVOKE_(REFLECTION_TEMPLATE_NONINTRUSIVE_IMPL_, \
        (__VA_ARGS__, nullptr))


#define REFLECTION_BASE_CLASSES_CUSTOM_NONINTRUSIVE_IMPL_( \
    Type, NamePrefix, BaseClassesRawNames, IncludesSentinelName, ...) \
    REFLECTION_PROPERTY_NAME_WRAPPER_MULTI_2_(Base ## Type, NamePrefix, BaseClassesRawNames) \
    constexpr auto z_reflection_base_class_properties_nonintrusive( \
        ::reflection::TypeTag<Type>) \
    { \
        return ::reflection::NonIntrusiveBaseClassParser< \
            Type, \
            ReflectionMetaInfo ## Base ## Type::value1, \
            ReflectionMetaInfo ## Base ## Type::value2, \
            IncludesSentinelName, __VA_ARGS__>::baseClassTypeInfos; \
    }

#define REFLECTION_BASE_CLASSES_CUSTOM_NONINTRUSIVE_WITH_SENTINEL_( \
    Type, NamePrefix, BaseClassesRawNames, ...) \
    REFLECTION_BASE_CLASSES_CUSTOM_NONINTRUSIVE_IMPL_( \
        Type, NamePrefix, BaseClassesRawNames, false, __VA_ARGS__)

#define REFLECTION_BASE_CLASSES_CUSTOM_NONINTRUSIVE(...) \
    REFLECTION_INVOKE_(REFLECTION_BASE_CLASSES_CUSTOM_NONINTRUSIVE_WITH_SENTINEL_, \
        (__VA_ARGS__, void))

#define REFLECTION_BASE_CLASSES_NONINTRUSIVE_IMPL_(Type, ...) \
    REFLECTION_BASE_CLASSES_CUSTOM_NONINTRUSIVE_IMPL_(Type, "_base_", #__VA_ARGS__, true, __VA_ARGS__)

#define REFLECTION_BASE_CLASSES_NONINTRUSIVE(...) \
    REFLECTION_INVOKE_(REFLECTION_BASE_CLASSES_NONINTRUSIVE_IMPL_, \
        (__VA_ARGS__, void))


#define REFLECTION_BASE_CLASSES_TEMPLATE_CUSTOM_NONINTRUSIVE_IMPL_( \
    TemplateDeclaration, TemplateArguments, Type, NamePrefix, BaseClassesRawNames, IncludesSentinelName, ...) \
    REFLECTION_PROPERTY_NAME_WRAPPER_MULTI_2_(BaseTemplate ## Type, NamePrefix, BaseClassesRawNames) \
    template <REFLECTION_UNPAREN_ TemplateDeclaration> \
    constexpr auto z_reflection_base_class_properties_nonintrusive( \
        ::reflection::TypeTag<Type<REFLECTION_UNPAREN_ TemplateArguments>>) \
    { \
        return ::reflection::NonIntrusiveBaseClassParser< \
            Type<REFLECTION_UNPAREN_ TemplateArguments>, \
            ReflectionMetaInfo ## BaseTemplate ## Type::value1, \
            ReflectionMetaInfo ## BaseTemplate ## Type::value2, \
            IncludesSentinelName, __VA_ARGS__>::baseClassTypeInfos; \
    }

#define REFLECTION_BASE_CLASSES_TEMPLATE_CUSTOM_NONINTRUSIVE_WITH_SENTINEL_( \
    TemplateDeclaration, TemplateArguments, Type, NamePrefix, BaseClassesRawNames, ...) \
    REFLECTION_BASE_CLASSES_TEMPLATE_CUSTOM_NONINTRUSIVE_IMPL_( \
        TemplateDeclaration, TemplateArguments, Type, NamePrefix, BaseClassesRawNames, false, __VA_ARGS__)

#define REFLECTION_BASE_CLASSES_TEMPLATE_CUSTOM_NONINTRUSIVE(...) \
    REFLECTION_INVOKE_(REFLECTION_BASE_CLASSES_TEMPLATE_CUSTOM_NONINTRUSIVE_WITH_SENTINEL_, \
        (__VA_ARGS__, void))

#define REFLECTION_BASE_CLASSES_TEMPLATE_NONINTRUSIVE_IMPL_( \
    TemplateDeclaration, TemplateArguments, Type, ...) \
    REFLECTION_BASE_CLASSES_TEMPLATE_CUSTOM_NONINTRUSIVE_IMPL_( \
        TemplateDeclaration, TemplateArguments, Type, "_base_", #__VA_ARGS__, true, __VA_ARGS__)

#define REFLECTION_BASE_CLASSES_TEMPLATE_NONINTRUSIVE(...) \
    REFLECTION_INVOKE_(REFLECTION_BASE_CLASSES_TEMPLATE_NONINTRUSIVE_IMPL_, \
        (__VA_ARGS__, void))
