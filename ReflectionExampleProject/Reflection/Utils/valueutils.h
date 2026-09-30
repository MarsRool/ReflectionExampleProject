#pragma once

#include <type_traits>
#include <vector>
#include <array>
#include <list>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "Shared/macroes.h"
#include "Shared/typetester.h"

namespace reflection
{

namespace extensions
{

template <typename T>
struct TypeTag {};

template <typename T, T* ptr>
struct PointerHolderTag
{
    static constexpr T* value = ptr;
};

template <typename Outer, typename T, T Outer::* ptr>
struct PointerToMemberHolderTag
{
    static constexpr T Outer::* value = ptr;
};

namespace impl
{

template <typename PointerToMember>
struct MemberPointerTraits;

template <typename ClassType, typename DataType>
struct MemberPointerTraits<DataType ClassType::*> {
    using Outer = ClassType;
    using T = DataType;
};

} // namespace impl

template <auto* ptr>
using PointerTag = PointerHolderTag<std::remove_pointer_t<decltype(ptr)>, ptr>;

template <auto ptr>
using PointerToMemberTag = PointerToMemberHolderTag<
    typename impl::MemberPointerTraits<decltype(ptr)>::Outer,
    typename impl::MemberPointerTraits<decltype(ptr)>::T,
    ptr>;

} // namespace extensions

template <typename Outer>
class BaseProperty;

template <typename Outer>
class PropertyMap;

template <typename T, typename = std::void_t<>>
struct IsObject : std::false_type {};

template <typename T>
struct IsObject<T, std::void_t<decltype(T::staticPropertyMap)>> : std::true_type {};

template <typename T>
struct IsProperty : IsSpecialization<T, BaseProperty> {};

template <typename T>
struct ValueTransfer
    : std::conditional<std::disjunction_v<std::is_arithmetic<T>, std::is_enum<T>>,
                       T,
                       std::conditional_t<IsString<T>::value,
                                          std::string_view,
                                          const T&>> {};

} // namespace reflection
