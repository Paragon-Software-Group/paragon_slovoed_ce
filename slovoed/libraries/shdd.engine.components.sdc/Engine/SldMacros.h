#pragma once
#ifndef _SLD_MACROS_H_
#define _SLD_MACROS_H_

// non clang compatibility
#ifndef __has_feature
#  define __has_feature(x) 0
#endif

#ifndef __has_builtin
#  define __has_builtin(x) 0
#endif

// gcc version check macro
#define SLD_GCC_VERSION(major,minor) (defined(__GNUC__) && ((100 * __GNUC__ + __GNUC_MINOR__) >= 100 * (major) + (minor)))

// unicode literals availability check
#ifndef SLD_HAS_UNICODE_LITERALS
#  if __cpp_unicode_literals >= 200710 || __has_feature(cxx_unicode_literals) || SLD_GCC_VERSION(4,5) || _MSC_VER >= 1900
#    define SLD_HAS_UNICODE_LITERALS
#  endif
#endif

// constexpr support availability check
#ifndef SLD_HAS_CONSTEXPR
#  if __cpp_constexpr >= 200704 || __has_feature(cxx_constexpr)
#    define SLD_HAS_CONSTEXPR
#  endif
#endif

// extended constexpr support availability check
#ifndef SLD_HAS_CXX14_CONSTEXPR
#  if __cpp_constexpr >= 201304
#    define SLD_HAS_CXX14_CONSTEXPR
#  endif
#endif

// UTF-16 string literal macro. To be used like `SldU16StringLiteral("text")`
#ifdef SLD_HAS_UNICODE_LITERALS
// Use "true" utf16 literals on platforms/compilers supporting unicode string literals
#  define SldU16StringLiteral(_TEXT_) (const UInt16*)(u ## _TEXT_)
#elif defined(_MSC_VER)
// As wchar_t on windows is 2 bytes we can "safely" use wide char string literals there
#  define SldU16StringLiteral(_TEXT_) (const UInt16*)(L ## _TEXT_)
#else
#  error The platform/compiler does not support utf16 string literals.
#endif

// UTF-16 string literal macro. To be used like `SldU16StringLiteral("text")`
#ifdef SLD_HAS_UNICODE_LITERALS
// Use "true" utf16 literals on platforms/compilers supporting unicode string literals
#  define SldU16Char(_CHAR_) (UInt16)(u ## _CHAR_)
#elif defined(_MSC_VER)
// As wchar_t on windows is 2 bytes we can "safely" use wide char string literals there
#  define SldU16Char(_CHAR_) (UInt16)(L ## _CHAR_)
#else
#  error The platform/compiler does not support utf16 string literals.
#endif

// constexpr && constexpr_or_const macros
#ifdef SLD_HAS_CONSTEXPR
#  define SLD_CONSTEXPR constexpr
#  define SLD_CONSTEXPR_OR_CONST constexpr
#else
#  define SLD_CONSTEXPR
#  define SLD_CONSTEXPR_OR_CONST const
#endif

// cxx14 extended constexpr macro
#ifdef SLD_HAS_CXX14_CONSTEXPR
#  define SLD_CXX14_CONSTEXPR constexpr
#else
#  define SLD_CXX14_CONSTEXPR
#endif

// assume()
#if __has_builtin(__builtin_assume)
#  define sld2_assume(cond_) __builtin_assume(cond_)
#elif defined(__GNUC__)
#  define sld2_assume(cond_) do { if (!(cond_)) __builtin_unreachable(); } while (0)
#elif defined(_MSC_VER)
#  define sld2_assume(cond_) __assume(cond_)
#else
#  define sld2_assume(cond_)
#endif

// likely() / unlikely()
#if defined(__GNUC__) || defined(__clang__)
#  define sld2_likely(cond_)   __builtin_expect((cond_),1)
#  define sld2_unlikely(cond_) __builtin_expect((cond_),0)
#else
#  define sld2_likely(cond_)   (cond_)
#  define sld2_unlikely(cond_) (cond_)
#endif

/**
 * Returns a pointer to the "parent" object from a pointer to one of its members
 *
 * @param[in] ptr_    - a pointer to one of the members
 * @param[in] ptype_  - parent object *type*
 * @param[in] member_ - a pointer to member object (basically a pointer to @ptr_)
 *
 * e.g. given a type like
 *   struct A {
 *     B b;
 *   };
 * and a pointer to b
 *   B *bptr;
 * one can get a pointer to the containing object A with
 *   A* aptr = sld2_container_of(bptr, A, &A::b);
 *
 * DOES NOT WORK for classes (parent ones) with virtual bases
 *
 * @return a pointer to the "parent" object
 */
#define sld2_container_of(ptr_, ptype_, member_) \
	((ptype_*)((char const volatile*)(ptr_) - ((char const volatile*)(&(((ptype_*)nullptr)->*member_)))))

#endif // _SLD_MACROS_H_
