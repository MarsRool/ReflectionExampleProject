#pragma once
#include "Reflection/Utils/typetraits.h"

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

template <auto* ptr>
using PointerTag = PointerHolderTag<std::remove_pointer_t<decltype(ptr)>, ptr>;

template <auto ptr>
using PointerToMemberTag = PointerToMemberHolderTag<
    typename reflection::MemberPointerTraits<decltype(ptr)>::Outer,
    typename reflection::MemberPointerTraits<decltype(ptr)>::T,
    ptr>;

} // namespace extensions
