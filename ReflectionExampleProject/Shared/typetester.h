#pragma once
#include <type_traits>
#include <memory>

template <class T>
struct IsPointerHelper : std::false_type {};

template <class T>
struct IsPointerHelper<T*> : std::true_type {};

template <class T>
struct IsPointer : IsPointerHelper<typename std::decay<T>::type> {};

template <class T>
struct IsSmartPointerHelper : std::false_type {};

template <class T>
struct IsSmartPointerHelper<std::shared_ptr<T>> : std::true_type {};

template <class T>
struct IsSmartPointer : IsSmartPointerHelper<typename std::decay<T>::type> {};

template <class T>
struct IsAnyPointer : std::false_type {};

template <class T>
struct IsAnyPointer<T*> : IsPointer<T*> {};

template <class T>
struct IsAnyPointer<std::shared_ptr<T>> : IsSmartPointer<std::shared_ptr<T>> {};

template <class T>
struct RemoveAllPointers
{
    using Type = std::remove_cv_t<T>;
};

template <class T>
struct RemoveAllPointers<T*>
{
    using Type = typename RemoveAllPointers<T>::Type;
};

template <class T>
using RemoveAllPointersT = typename RemoveAllPointers<T>::Type;

template <class T>
struct PointersRank : std::integral_constant<std::size_t, 0>
{};

template <class T>
struct PointersRank<T*> : public std::integral_constant<std::size_t, PointersRank<T>::value + 1>
{};

template <class T>
inline constexpr std::size_t PointersRankV = PointersRank<T>::value;

template <class T>
using ArrayReturnTypeT = std::conditional_t<
    std::is_array_v<T>,
    std::add_pointer_t<std::remove_extent_t<T>>,
    T>;

template <class Test, template<class...> class Ref>
struct IsSpecialization : std::false_type {};

template <template<class...> class Ref, class... Args>
struct IsSpecialization<Ref<Args...>, Ref> : std::true_type {};

template <class Test, template<class, std::size_t> class Ref>
struct IsSpecializationSized : std::false_type {};

template <template<class, std::size_t> class Ref, class Arg, std::size_t Num>
struct IsSpecializationSized<Ref<Arg, Num>, Ref> : std::true_type {};

template <class T>
struct IsPlain
    : std::disjunction<
          std::is_null_pointer<T>,
          std::is_integral<T>,
          std::is_floating_point<T>> {};

template <class T>
struct IsString
    : std::disjunction<
          std::is_same<T, std::string>,
          std::is_same<T, std::string_view>> {};

template <class T>
struct IsArray
    : std::disjunction<
          IsSpecialization<T, std::vector>,
          IsSpecialization<T, std::list>,
          IsSpecializationSized<T, std::array>> {};

template <class T>
struct ToStringDetect
{
    template <class SomeTs>
    using DummyTmpl = void;

    template <class U, class = void>
    struct X : std::false_type {};

    template <class U>
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
