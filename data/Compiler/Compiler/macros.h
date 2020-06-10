#pragma once

// non clang compatibility
#ifndef __has_feature
#  define __has_feature(x) 0
#endif
#ifndef __has_cpp_attribute
#  define __has_cpp_attribute(x) 0
#endif

// gcc version check macro
#define GCC_VERSION(major,minor) (defined(__GNUC__) && ((100 * __GNUC__ + __GNUC_MINOR__) >= 100 * (major) + (minor)))

#if __cpp_constexpr >= 201304
#  define CXX14_CONSTEXPR constexpr
#else
#  define CXX14_CONSTEXPR
#endif

// noinline
#if defined(__GNUC__) || defined(__clang__)
#  define NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#  define NOINLINE __declspec(noinline)
#else
#  define NOINLINE
#endif

// noreturn
#if __cpp_attributes >= 200809 || __has_cpp_attribute(noreturn)
#  define NORETURN [[noreturn]]
#elif defined(__GNUC__) || defined(__clang__)
#  define NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#  define NORETURN __declspec(noreturn)
#else
#  define NORETURN
#endif

// printflike
#if defined(__GNUC__) || defined(__clang__)
#  define PRINTFLIKE(f, a) __attribute__((format(__printf__, f, a)))
#else
#  define PRINTFLIKE(f, a)
#endif

// forced always inline
#if defined(__GNUC__) || defined(__clang__)
#  define ALWAYS_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
#  define ALWAYS_INLINE __forceinline
#else
#  define ALWAYS_INLINE inline
#endif
