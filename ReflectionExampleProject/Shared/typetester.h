#pragma once
#include <type_traits>

template <typename T>
using ArrayReturnTypeT = std::conditional_t<
    std::is_array_v<T>,
    std::add_pointer_t<std::remove_extent_t<T>>,
    T>;

template <typename Test, template<typename...> typename Ref>
struct IsSpecialization : std::false_type {};

template <template<typename...> typename Ref, typename... Args>
struct IsSpecialization<Ref<Args...>, Ref> : std::true_type {};

template <typename Test, template<typename, std::size_t> typename Ref>
struct IsSpecializationSized : std::false_type {};

template <template<typename, std::size_t> typename Ref, typename Arg, std::size_t Num>
struct IsSpecializationSized<Ref<Arg, Num>, Ref> : std::true_type {};

template <typename T>
struct IsString
    : std::disjunction<
          std::is_same<T, std::string>,
          std::is_same<T, std::string_view>> {};

// TODO: rewrite checker and use cases like a concept having begin(), end() and returning object has operator++
template <typename T>
struct IsArray
    : std::disjunction<
          IsSpecialization<T, std::vector>,
          IsSpecialization<T, std::list>,
          IsSpecializationSized<T, std::array>> {};

// TODO: recheck all type traits
template <typename T>
struct ToStringDetect
{
    template <typename SomeTs>
    using DummyTmpl = void;

    template <typename U, typename = void>
    struct X : std::false_type {};

    template <typename U>
    struct X
        <U, DummyTmpl<decltype(
                std::to_string (
                    static_cast<U>(std::declval<U>())
                    )
                )>
         >
        : std::true_type {};

    static constexpr bool value = X<T>::value;
};
