#pragma once

#include <type_traits>
#include <utility>

#include "reflection/property/staticproperty.h"
#include "reflection/property/statictypeinfo.h"

namespace reflection
{

template <typename T>
struct TypeTag {};


namespace impl
{

template <typename PointerToMember>
struct MemberPointerTraits;

template <typename ClassType, typename DataType>
struct MemberPointerTraits<DataType ClassType::*>
{
    using Outer = ClassType;
    using T = DataType;
};

template <typename F, typename = void>
struct IsNullableCallable : std::false_type {};

template <typename F>
struct IsNullableCallable<F, std::void_t<
    decltype(static_cast<bool>(std::declval<F&>() == nullptr))>> : std::true_type {};


template<typename T, typename = void>
struct HasIntrusive : std::false_type {};

template<typename T>
struct HasIntrusive<T, std::void_t<
    decltype(z_reflection_type_info(TypeTag<T>{})),
    decltype(z_reflection_begin(TypeTag<T>{})),
    decltype(z_reflection_end(TypeTag<T>{}))>> : std::true_type {};

template<typename T, typename = void>
struct HasNonIntrusive : std::false_type {};

template<typename T>
struct HasNonIntrusive<T, std::void_t<
    decltype(z_reflection_properties_nonintrusive(TypeTag<T>{}))>> : std::true_type {};

template <typename T, typename = void>
struct HasNonIntrusiveBases : std::false_type {};

template <typename T>
struct HasNonIntrusiveBases<T, std::void_t<decltype(
    z_reflection_base_class_properties_nonintrusive(TypeTag<T>{}))>>
    : std::true_type {};

template <typename T>
using IsReflected = std::disjunction<
    HasIntrusive<std::remove_cv_t<std::remove_reference_t<T>>>,
    HasNonIntrusive<std::remove_cv_t<std::remove_reference_t<T>>>>;


template <typename T>
struct StaticPropertyTraits : std::false_type {};

template <typename Outer, typename T>
struct StaticPropertyTraits<StaticProperty<Outer, T>> : std::true_type
{
    using ValueType = T;
};

template <typename T>
struct BaseClassTraits : std::false_type {};

template <typename Outer>
struct BaseClassTraits<StaticTypeInfo<Outer>> : std::true_type
{
    using OuterType = Outer;
};

template <typename Entry>
using EntryValueType = std::remove_cv_t<
    std::remove_reference_t<decltype(Entry::value)>>;

template <typename Entry>
using IsBaseClassEntry = BaseClassTraits<EntryValueType<Entry>>;

template <typename Entry>
using IsStaticPropertyEntry = StaticPropertyTraits<EntryValueType<Entry>>;

} // namespace impl

} // namespace reflection
