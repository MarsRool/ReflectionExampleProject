#pragma once
#include <type_traits>

#include "Shared/macroes.h"

namespace reflection
{

template <typename T>
struct IsString
    : std::disjunction<
        std::is_same<T, std::string>,
        std::is_same<T, std::string_view>> {};

template <typename Test, template<typename...> typename Ref>
struct IsSpecialization : std::false_type {};

template <template<typename...> typename Ref, typename... Args>
struct IsSpecialization<Ref<Args...>, Ref> : std::true_type {};

template <typename Test, template<typename, std::size_t> typename Ref>
struct IsSpecializationSized : std::false_type {};

template <template<typename, std::size_t> typename Ref, typename Arg, std::size_t Num>
struct IsSpecializationSized<Ref<Arg, Num>, Ref> : std::true_type {};

template <typename T, typename = std::void_t<>>
struct IsIterable : std::false_type {};

template <typename T>
struct IsIterable<T, std::void_t<
    decltype(std::begin(std::declval<T&>())),
    decltype(std::end(std::declval<T&>())),
    decltype(++std::declval<decltype(std::begin(std::declval<T&>()))&>())>> : std::true_type {};

template <typename T, typename = std::void_t<>>
struct IsInsertable : std::false_type {};

template <typename T>
struct IsInsertable<T, std::void_t<
    typename T::value_type,
    typename T::iterator,
    decltype(std::declval<T&>().insert(
        std::declval<typename T::iterator>(),
        std::declval<typename T::value_type>()))>> : std::true_type {};

template <typename T, typename = std::void_t<>>
struct HasReserve : std::false_type {};

template <typename T>
struct HasReserve<T, std::void_t<
    decltype(std::declval<T&>().reserve(std::declval<std::size_t>()))>> : std::true_type {};

template <typename T, typename = std::void_t<>>
struct IsToStringAvailable : std::false_type {};

template <typename T>
struct IsToStringAvailable<T, std::void_t<
    decltype(std::to_string(std::declval<T>()))>> : std::true_type {};

template <typename T>
struct ValueTransfer
    : std::conditional<std::disjunction_v<std::is_arithmetic<T>, std::is_enum<T>>,
        T,
        std::conditional_t<IsString<T>::value,
            std::string_view,
            const T&>> {};

template <typename T>
using ArrayReturnTypeT = std::conditional_t<
    std::is_array_v<T>,
    std::add_pointer_t<std::remove_extent_t<T>>,
    T>;


template <typename Outer>
class PropertyMap;

template <typename T, typename = std::void_t<>>
struct IsObject : std::false_type {};

template <typename T>
struct IsObject<T, std::void_t<decltype(T::staticPropertyMap)>> : std::true_type {};


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

} // namespace reflection
