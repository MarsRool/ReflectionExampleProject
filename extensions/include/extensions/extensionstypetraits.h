#pragma once

#include <cstddef>
#include <iterator>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace extensions::impl
{

using std::begin;
using std::end;

template <typename T>
struct AlwaysFalse : std::false_type {};

template <typename T>
using CleanType = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename T>
using IsString = std::disjunction<
    std::is_same<CleanType<T>, std::string>,
    std::is_same<CleanType<T>, std::string_view>>;

template <typename T, typename = void>
struct IsIterable : std::false_type {};

template <typename T>
struct IsIterable<T, std::void_t<
    decltype(begin(std::declval<const T&>())),
    decltype(end(std::declval<const T&>()))>> : std::true_type {};

template <typename T, typename = void>
struct IsInsertable : std::false_type {};

template <typename T>
struct IsInsertable<T, std::void_t<
    typename T::value_type,
    decltype(std::declval<T&>().clear()),
    decltype(std::declval<T&>().insert(
        std::declval<decltype(std::declval<T&>().end())>(),
        std::declval<typename T::value_type>()))>> : std::true_type {};

template <typename T, typename = void>
struct HasReserve : std::false_type {};

template <typename T>
struct HasReserve<T, std::void_t<
    decltype(std::declval<T&>().reserve(std::declval<std::size_t>()))>>
    : std::true_type {};

template <typename Reference>
using IsWritableElement = std::conjunction<
    std::is_lvalue_reference<Reference>,
    std::negation<std::is_const<std::remove_reference_t<Reference>>>>;

template <typename T, typename = void>
struct IsWritableSizedRange : std::false_type {};

template <typename T>
struct IsWritableSizedRange<T, std::void_t<
    decltype(std::size(std::declval<T&>())),
    decltype(*begin(std::declval<T&>())),
    decltype(end(std::declval<T&>()))
>> : IsWritableElement<decltype(*begin(std::declval<T&>()))> {};

template <typename T, typename = void>
struct IsToStringAvailable : std::false_type {};

template <typename T>
struct IsToStringAvailable<T, std::void_t<
    decltype(std::to_string(std::declval<T>()))>> : std::true_type {};


template <typename Iterator>
constexpr decltype(auto) dereferenceValue(Iterator& it)
{
    using ValueType = typename std::iterator_traits<Iterator>::value_type;
    using DereferencedType = CleanType<decltype(*it)>;

    if constexpr (std::is_same_v<DereferencedType, ValueType>)
        return *it;
    else
        return static_cast<ValueType>(*it);
}

} // namespace extensions::impl
