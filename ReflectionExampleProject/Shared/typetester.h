#pragma once
#include <type_traits>
#include <memory>

template <typename T>
struct IsPointerHelper : std::false_type {};

template <typename T>
struct IsPointerHelper<T*> : std::true_type {};

template <typename T>
struct IsPointer : IsPointerHelper<typename std::decay<T>::type> {};

template <typename T>
struct IsSmartPointerHelper : std::false_type {};

template <typename T>
struct IsSmartPointerHelper<std::shared_ptr<T>> : std::true_type {};

template <typename T>
struct IsSmartPointer : IsSmartPointerHelper<typename std::decay<T>::type> {};

template <typename T>
struct IsAnyPointer : std::false_type {};

template <typename T>
struct IsAnyPointer<T*> : IsPointer<T*> {};

template <typename T>
struct IsAnyPointer<std::shared_ptr<T>> : IsSmartPointer<std::shared_ptr<T>> {};

template <typename T>
struct RemoveAllPointers
{
    using Type = std::remove_cv_t<T>;
};

template <typename T>
struct RemoveAllPointers<T*>
{
    using Type = typename RemoveAllPointers<T>::Type;
};

template <typename T>
using RemoveAllPointersT = typename RemoveAllPointers<T>::Type;

template <typename T>
struct PointersRank : std::integral_constant<std::size_t, 0>
{};

template <typename T>
struct PointersRank<T*> : public std::integral_constant<std::size_t, PointersRank<T>::value + 1>
{};

template <typename T>
inline constexpr std::size_t PointersRankV = PointersRank<T>::value;

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
struct IsPlain
    : std::disjunction<
          std::is_null_pointer<T>,
          std::is_integral<T>,
          std::is_floating_point<T>> {};

template <typename T>
struct IsString
    : std::disjunction<
          std::is_same<T, std::string>,
          std::is_same<T, std::string_view>> {};

template <typename T>
struct IsArray
    : std::disjunction<
          IsSpecialization<T, std::vector>,
          IsSpecialization<T, std::list>,
          IsSpecializationSized<T, std::array>> {};

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
