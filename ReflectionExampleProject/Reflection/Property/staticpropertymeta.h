#pragma once
#include "Reflection/Utils/canonicalpropertynameparser.h"
#include "Reflection/Utils/canonicalstaticstring.h"
#include "Reflection/Utils/uniquestaticheterogeneousmap.h"
#include "Reflection/Property/staticproperty.h"
#include "Reflection/Property/staticpropertymap.h"
#include "Reflection/Property/staticpropertyproxy.h"

namespace reflection
{

namespace impl
{

template <typename Outer, typename T,
    const char rawName[], T Outer::* valuePtr>
struct StaticPropertyHolder
{
    using StaticPropertyType = reflection::StaticProperty<Outer, T>;
    static constexpr StaticPropertyType staticProperty
    {
        { rawName },
        valuePtr
    };
};

template <typename Outer, const char rawName[]>
struct StaticPropertyMapHolder
{
    using StaticPropertyType = reflection::StaticPropertyMap<Outer>;
    static constexpr StaticPropertyType staticProperty
    {
        { rawName }
    };
};

template <typename Outer, typename StaticPropertyT,
    const char rawName[], const StaticPropertyT& targetStaticProperty>
struct StaticPropertyProxyHolder
{
    using StaticPropertyType = reflection::StaticPropertyProxy<Outer, StaticPropertyT>;
    static constexpr StaticPropertyType staticProperty
    {
        { rawName },
        targetStaticProperty
    };
};

template <const char rawName[], auto valuePtr>
using DefineStaticProperty = StaticPropertyHolder<
    typename MemberPointerTraits<decltype(valuePtr)>::Outer,
    typename MemberPointerTraits<decltype(valuePtr)>::T,
    rawName,
    valuePtr>;

template <typename Outer, const char rawName[]>
using DefineStaticPropertyMap = StaticPropertyMapHolder<
    Outer, rawName>;

template <typename Outer, const char rawName[], const auto& targetStaticProperty>
using DefineStaticPropertyProxy = StaticPropertyProxyHolder<
    Outer,
    std::remove_cv_t<std::remove_reference_t<decltype(targetStaticProperty)>>,
    rawName,
    targetStaticProperty>;


auto getNonintrusiveReflectionDeclarator(...) -> std::true_type;

template <typename T>
using NonintrusiveReflectionDeclarator = decltype(
    getNonintrusiveReflectionDeclarator(
        static_cast<T*>(nullptr)));

auto getNonintrusiveReflectionInheritanceDeclarator(...)
    -> std::true_type;

template <typename T>
using NonintrusiveReflectionInheritanceDeclarator = decltype(
    getNonintrusiveReflectionInheritanceDeclarator(
        static_cast<T**>(nullptr)));

} // namespace impl

template <typename Outer>
struct StaticPropertyMeta
{
    using ThisClass = StaticPropertyMeta<Outer>;
    using OuterClass = Outer;
    using KeyType = const char[];
    using DefaultTypeChecker = impl::UniqueStaticHeterogeneousMapElementTrueChecker;

    static constexpr KeyType staticStorageKey{ "" };

    static constexpr auto size() noexcept
    {
        return uniqueStaticHeterogeneousMapKeysCount<ThisClass, KeyType>([]{});
    }

    static constexpr bool empty() noexcept
    {
        return size() == 0;
    }

    template <KeyType propertyName>
    static constexpr auto contains()
    {
        constexpr auto canonicalPropertyName =
            CanonicalStaticString<propertyName>::value;
        return uniqueStaticHeterogeneousMapExists<
            ThisClass, KeyType, canonicalPropertyName>([]{});
    }

    static constexpr auto getStaticPropertyMap()
    {
        return uniqueStaticHeterogeneousMapGetValue<
            StaticPropertyMap<Outer>, KeyType, staticStorageKey>([]{});
    }

    template <KeyType propertyName>
    static constexpr auto at()
    {
        constexpr auto canonicalPropertyName =
            CanonicalStaticString<propertyName>::value;
        return uniqueStaticHeterogeneousMapGetValue<ThisClass, KeyType, canonicalPropertyName>([]{});
    }

    template <typename TypeChecker = DefaultTypeChecker,
        typename F>
    static void forEach(F&& func)
    {
        uniqueStaticHeterogeneousMapForEach<
            ThisClass, KeyType, TypeChecker>(
                []{}, std::forward<F>(func));
    }

    template <typename TypeChecker = DefaultTypeChecker,
        typename F, typename P>
    static void forEachIf(F&& func, P&& pred)
    {
        uniqueStaticHeterogeneousMapForEachIf<
            ThisClass, KeyType, TypeChecker>(
                []{}, std::forward<F>(func), std::forward<P>(pred));
    }

    template <typename Comparator = std::equal_to<void>,
        typename TypeChecker = DefaultTypeChecker,
        typename F>
    static void doForKey(F&& func, KeyType key, Comparator comparator = Comparator())
    {
        uniqueStaticHeterogeneousMapDoForKey<
            ThisClass, KeyType, Comparator, TypeChecker>(
                []{}, std::forward<F>(func), key, comparator);
    }

    template <KeyType key, typename F>
    static void doForKey(F&& func)
    {
        uniqueStaticHeterogeneousMapDoForKey<
            ThisClass, KeyType, key>(
                []{}, std::forward<F>(func));
    }

    template <KeyType propertyName>
    static constexpr auto defineStaticPropertyMap()
    {
        using Holder = impl::DefineStaticPropertyMap<Outer, propertyName>;
        using StaticPropertyType = typename Holder::StaticPropertyType;
        constexpr auto staticPropertyPtr = &Holder::staticProperty;
        return uniqueStaticHeterogeneousMapAdd<
            StaticPropertyMap<Outer>, KeyType, const StaticPropertyType*, staticStorageKey, staticPropertyPtr>([]{});
    }

    template <KeyType propertyName, auto valuePtr>
    static constexpr auto defineStaticProperty()
    {
        using Holder = impl::DefineStaticProperty<propertyName, valuePtr>;
        using StaticPropertyType = typename Holder::StaticPropertyType;
        constexpr auto canonicalPropertyName = CanonicalStaticString<propertyName>::value;
        constexpr auto staticPropertyPtr = &Holder::staticProperty;
        return uniqueStaticHeterogeneousMapAdd<
            ThisClass, KeyType, const StaticPropertyType*, canonicalPropertyName, staticPropertyPtr>([]{});
    }

    template <KeyType propertyName, const auto& targetStaticProperty>
    static constexpr auto defineStaticPropertyProxy()
    {
        using Holder = impl::DefineStaticPropertyProxy<Outer, propertyName, targetStaticProperty>;
        using StaticPropertyType = typename Holder::StaticPropertyType;
        constexpr auto canonicalPropertyName = CanonicalStaticString<propertyName>::value;
        constexpr auto staticPropertyPtr = &Holder::staticProperty;
        return uniqueStaticHeterogeneousMapAdd<
            ThisClass, KeyType, const StaticPropertyType*, canonicalPropertyName, staticPropertyPtr>([]{});
    }

    template <KeyType propertyNames, auto... valuePtrs>
    static constexpr bool defineStaticPropertyMulti()
    {
        using Parser = CanonicalPropertyNameParser<propertyNames>;

        static_assert(
            Parser::count == sizeof...(valuePtrs),
            "defineStaticPropertyMulti: "
            "property names count doesn't match value pointers count");

        return defineStaticPropertyMultiImpl<
            propertyNames,
            0,
            valuePtrs...>();
    }

    template <KeyType namePrefix, KeyType baseClassNames, typename... BaseClasses>
    static constexpr bool defineBaseClassMulti()
    {
        using Parser = CanonicalPropertyNameParser<baseClassNames>;

        static_assert(
            Parser::count == sizeof...(BaseClasses),
            "defineBaseClassMulti: "
            "base class names count doesn't match base class types count");

        return defineBaseClassMultiImpl<
            namePrefix, baseClassNames, 0, BaseClasses...>();
    }

private:
    template <
        KeyType propertyNames,
        std::size_t propertyIndex,
        auto valuePtr,
        auto... valuePtrs>
    static constexpr auto defineStaticPropertyMultiImpl()
    {
        using Parser = CanonicalPropertyNameParser<propertyNames>;

        static_assert(
            propertyIndex < Parser::count,
            "defineStaticPropertyMultiImpl: propertyIndex is out of range");

        using PropertyName =
            typename Parser::template CanonicalName<propertyIndex>;

        constexpr auto staticPropertyPtr =
            defineStaticProperty<PropertyName::value, valuePtr>();

        static_assert(
            staticPropertyPtr != nullptr,
            "defineStaticPropertyMultiImpl: failed to define static property");

        if constexpr (sizeof...(valuePtrs) == 0)
        {
            return true;
        }
        else
        {
            return defineStaticPropertyMultiImpl<
                propertyNames,
                propertyIndex + 1,
                valuePtrs...>();
        }
    }

    template <
        KeyType namePrefix,
        KeyType baseClassNames,
        std::size_t baseClassIndex,
        typename BaseClass,
        typename... BaseClasses>
    static constexpr bool defineBaseClassMultiImpl()
    {
        using Parser = CanonicalPropertyNameParser<baseClassNames>;

        static_assert(
            baseClassIndex < Parser::count,
            "defineBaseClassMultiImpl: baseClassIndex is out of range");

        using BaseClassName =
            CanonicalStaticStringConcat<
                namePrefix,
                Parser::template CanonicalName<baseClassIndex>::value>;

        constexpr auto baseStaticPropertyMapPtr =
            StaticPropertyMeta<BaseClass>::getStaticPropertyMap();

        static_assert(
            baseStaticPropertyMapPtr != nullptr,
            "defineBaseClassMultiImpl: base class does not have static property map");

        constexpr auto staticPropertyPtr =
            defineStaticPropertyProxy<BaseClassName::value, *baseStaticPropertyMapPtr>();

        static_assert(
            staticPropertyPtr != nullptr,
            "defineBaseClassMultiImpl: failed to define base class proxy");

        if constexpr (sizeof...(BaseClasses) == 0)
        {
            return true;
        }
        else
        {
            return defineBaseClassMultiImpl<
                namePrefix,
                baseClassNames,
                baseClassIndex + 1,
                BaseClasses...>();
        }
    }

    static_assert(
        sizeof(impl::NonintrusiveReflectionDeclarator<Outer>) > 0); // NOLINT(bugprone-sizeof-expression)

    static_assert(
        sizeof(impl::NonintrusiveReflectionInheritanceDeclarator<Outer>) > 0); // NOLINT(bugprone-sizeof-expression)
};

} // namespace reflection
