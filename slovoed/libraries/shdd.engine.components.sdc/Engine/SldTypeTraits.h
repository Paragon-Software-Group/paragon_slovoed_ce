#ifndef _SLD_TYPE_TRAITS_H_
#define _SLD_TYPE_TRAITS_H_

#include <type_traits>

namespace sld2 {

// '_t' helper aliases, should be added without the actual _t

template<typename T>
using remove_reference = typename std::remove_reference<T>::type;

template<bool B, class T, class F>
using conditional = typename std::conditional<B, T, F>::type;

template <class T>
using add_lvalue_reference = typename std::add_lvalue_reference<T>::type;

// as the non-template parameter usages of enable_if are atrocious introduce
// a non 1-to-1 mapping to it which always has int as type to be used like
//   template <..., sld2::enable_if<Condition> = 0>
template <bool B>
using enable_if = typename std::enable_if<B, int>::type;

// aliases for std is_XXX check metafunctions

template <typename T> using is_lvalue_reference = std::is_lvalue_reference<T>;
template <typename T, typename U> using is_same = std::is_same<T, U>;
template <typename T> using is_array = std::is_array<T>;
template <typename T> using is_pointer = std::is_pointer<T>;
template <typename T, typename U> using is_convertible = std::is_convertible<T, U>;
template <typename T> using is_trivial = std::is_trivial<T>;
template <typename T> using is_trivially_destructible = std::is_trivially_destructible<T>;
template <typename T> using is_integral = std::is_integral<T>;
template <typename T> using is_signed = std::is_signed<T>;

// workaround for missing "is_trivially_XXX" in libstdc++ < 5.0
// use gcc version as a proxy as they are kinda linked
#if defined(__GLIBCXX__) && (__GNUC__ < 5)
template <typename T> using is_trivially_copyable = is_trivial<T>;
template <typename T> using is_trivially_default_constructible = is_trivial<T>;
#else
template <typename T> using is_trivially_copyable = std::is_trivially_copyable<T>;
template <typename T> using is_trivially_default_constructible = std::is_trivially_default_constructible<T>;
#endif

// random metaprogramming helpers
namespace meta {

template <int V> struct tag_t {};

} // namespace meta

} // namespace sld2

#endif // _SLD_TYPE_TRAITS_H_

