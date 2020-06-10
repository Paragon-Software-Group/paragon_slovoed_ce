#ifndef _SLD_PLATFORM_H_
#define _SLD_PLATFORM_H_

#include <new> // required for the placement new()

#include "SldMacros.h"
#include "SldTypeTraits.h"

/**
 * SLD__HAVE_SDC_BASE_TYPES_H
 *
 * If defined instructs the Engine to include `SDCBaseTypes.h` instead of
 * defining fundamental types itself. The header *must* define all of the
 * required fundamental types:
 *   Int[8|16|32|64]
 *   UInt[8|16|32|64]
 *   UInt4Ptr
 *   Float32
 *
 * The file should be somewhere on the include path
 *
 * NOTE: There is not much reason to use this unless the toolchain/platform
 *       does not provide <stdint.h>
 */
#ifdef SLD__HAVE_SDC_BASE_TYPES_H

#include "SDCBaseType.h"

#else

#include <stdint.h>

// Базовые типы данных
typedef uint64_t  UInt64;
typedef int64_t   Int64;
typedef uint32_t  UInt32;
typedef int32_t   Int32;
typedef uint16_t  UInt16;
typedef int16_t   Int16;
typedef uint8_t   UInt8;
typedef int8_t    Int8;
typedef uintptr_t UInt4Ptr;

typedef float     Float32;

#endif // SLD__HAVE_SDC_BASE_TYPES_H

/**
 * SLD__HAVE_MEM_MGR
 *
 * If defined instructs the Engine to define all of the memory management
 * functions as exported (instead of static inlines) expecting them to be
 * present during linking.
 *
 * NOTE: Unless you need to use a custom allocator there is no point using
 *       this
 *
 * There are also 2 defines for more fine-grained control of this:
 *  SLD__HAVE_MEM_MGR_ALLOCATOR
 *    controls memory allocation/deallocation functions (malloc()/free() & friends)
 *  Sld__HAVE_MEM_MGR_MISC
 *    controls miscellaneous memory functions (memset()/memcpy() & friends)
 * So it's eg. possible to use a custom memory allocator and still use library
 * memcpy()/memset().
 */
#ifdef SLD__HAVE_MEM_MGR
#  define SLD__HAVE_MEM_MGR_ALLOCATOR
#  define SLD__HAVE_MEM_MGR_MISC
#endif

#ifdef SLD__HAVE_MEM_MGR_ALLOCATOR

 /// Выделяет память в количестве aSize байт
void *sldMemNew(UInt32 aSize);

/// Освобождает память
void sldMemFree(void *aPointer);

/// Перевыделяет память в количестве aSize байт
void *sldMemRealloc(void *aPointer, UInt32 aSize);

/// Выделяет память в количестве aSize байт заполненную нулями
void *sldMemNewZero(UInt32 aSize);

#else

#include <stdlib.h> /* malloc, calloc, free */

/**
 * Выделяет блок памяти
 *
 * @param[in] aSize - размер блока памяти в байтах
 *
 * @return указатель на блок памяти или NULL в случае ошибки
 */
static inline void *sldMemNew(UInt32 aSize)
{
	return malloc(aSize);
}

/**
 * Выделяет память заполненную 0
 *
 * @param[in] aSize - размер блока памяти в байтах
 *
 * @return указатель на блок памяти или NULL в случае ошибки
 ************************************************************************/
static inline void *sldMemNewZero(UInt32 aSize)
{
	return calloc(1, aSize);
}

/**
 * Изменяет размер выделенного блока памяти
 *
 * @param aPtr  - указатель на блок
 * @param aSize - размер блока памяти в байтах
 *
 * @return указатель на блок памяти или NULL в случае ошибки
 */
static inline void *sldMemRealloc(void *aPtr, UInt32 aSize)
{
	return realloc(aPtr, aSize);
}

/**
 * Освобождает память выделенную с помощью функций #sldMemNew, #sldMemNewZero, #sldMemRealloc
 *
 * @param[in] aPtr - указатель на память которую необходимо освободить
 */
static inline void sldMemFree(void *aPtr)
{
	free(aPtr);
}

#endif // SLD__HAVE_MEM_MGR_ALLOCATOR

#ifdef SLD__HAVE_MEM_MGR_MISC

/// Копирует память
void sldMemMove(void *aToPtr, const void *aFromPtr, UInt32 aSize);

/// Копирует память
void sldMemCopy(void *aToPtr, const void *aFromPtr, UInt32 aSize);

/// Заполняет память значениями aValue
void sldMemSet(void *aPtr, Int32 aValue, UInt32 aSize);

#else

#include <string.h> /* memset, memmove, memcpy */

/**
 * Копирует память из одного места в другое
 *
 * @param[out] aToPtr   - указатель на блок памяти куда следует поместить данные из aFromPtr
 * @param[in]  aFromPtr - указатель на блок памяти откуда нужно брать данные для копирования в aToPtr
 * @param[in]  aSize    - количество данных для копирования
 */
static inline void sldMemMove(void *aToPtr, const void *const aFromPtr, UInt32 aSize)
{
	memmove(aToPtr, aFromPtr, aSize);
}

/**
 * Копирует память из одного места в другое
 *
 * @param[out] aToPtr   - указатель на блок памяти куда следует поместить данные из aFromPtr
 * @param[in]  aFromPtr - указатель на блок памяти откуда нужно брать данные для копирования в aToPtr
 * @param[in]  aSize    - количество данных для копирования
 *
 * ВАЖНО: блоки памяти не должны пересекаться
 */
static inline void sldMemCopy(void *aToPtr, const void *const aFromPtr, UInt32 aSize)
{
	memcpy(aToPtr, aFromPtr, aSize);
}

/**
 * Заполняет память значениями
 *
 * @param[in] aPtr   - указатель на блок памяти который необходимо заполнить
 * @param[in] aValue - значение которым необходимо заполнить блок памяти
 * @param[in] aSize  - размер блока подлежащего заполнению
 */
static inline void sldMemSet(void *aPtr, Int32 aValue, UInt32 aSize)
{
	memset(aPtr, aValue, aSize);
}

#endif // SLD__HAVE_MEM_MGR_MISC

/**
 * Очищает память (заполняет 0)
 *
 * @param[in] aPtr  - указатель на блок памяти который необходимо очистить
 * @param[in] aSize - размер блока подлежащего очистке
 */
static inline void sldMemZero(void *aPtr, UInt32 aSize)
{
	sldMemSet(aPtr, 0, aSize);
}

/**
 * Выделяет память под объект(ы) типа @T
 *
 * @param[in] aCount - количество объектов (>0, по дефолту - 1)
 *
 * @return указатель на объект(ы) или NULL в случае ошибки
 */
template <typename T>
static inline T* sldMemNew(UInt32 aCount = 1)
{
	return aCount > 0 ? (T*)sldMemNew(sizeof(T) * aCount) : NULL;
}

/**
 * Выделяет память под объект(ы) типа @T заполненную 0
 *
 * @param[in] aCount - количество объектов (>0, по дефолту - 1)
 *
 * @return указатель на объект(ы) или NULL в случае ошибки
 */
template <typename T>
static inline T* sldMemNewZero(UInt32 aCount = 1)
{
	return aCount > 0 ? (T*)sldMemNewZero(sizeof(T) * aCount) : NULL;
}

/**
 * Изменяет размер выделенного блока памяти под массив объектов типа @T
 *
 * @param[in] aPtr   - указатель на массив объектов
 * @param[in] aCount - количество объектов
 *
 * @return указатель на массив объектов или NULL в случае ошибки
 */
template <typename T>
static inline T* sldMemReallocT(T *aPtr, UInt32 aCount)
{
	return (T*)sldMemRealloc(aPtr, sizeof(T) * aCount);
}

// custom implementation of std::move, std::forward and the supporting meta machinery
namespace sld2 {

/// реализация std::move
template<class T>
inline remove_reference<T>&& move(T&& aArg)
{
	return static_cast<remove_reference<T>&&>(aArg);
}

/// реализация std::forward
template<class T>
inline T&& forward(remove_reference<T>& aArg)
{
	return static_cast<T&&>(aArg);
}

template<class T>
inline T&& forward(remove_reference<T>&& aArg)
{
	static_assert(!is_lvalue_reference<T>::value, "Can not forward an rvalue as an lvalue.");
	return static_cast<T&&>(aArg);
}

template <typename T, typename... Args>
inline void construct_at(T* aPtr, Args&&... aArgs)
{
	sld2_assume(aPtr != nullptr);
	::new (static_cast<void*>(aPtr)) T(forward<Args>(aArgs)...);
}

template <typename T, enable_if<!is_trivially_destructible<T>::value> = 0>
inline void destroy_at(T *aPtr) { aPtr->~T(); }
template <typename T, enable_if<is_trivially_destructible<T>::value> = 0>
inline void destroy_at(T*) { /* noop */ }

} // namespace sld2

/**
 * Создает объект типа @T передавая аргументы @aArgs в конструктор
 *
 * Использовать вместо чистого new(), удалять через sldDelete()
 *
 * @return указатель на вновь созданный объект или NULL при
 *         невозможности выделить память
 */
template<typename T, typename... Args>
inline T *sldNew(Args&&... aArgs)
{
	T *p = sldMemNew<T>();
	if (p)
		sld2::construct_at(p, sld2::forward<Args>(aArgs)...);
	return p;
}

/**
 * Уничтожает объект созданный через sldNew()
 *
 * @param[in] aPtr - указатель на объект
 */
template <typename T>
inline void sldDelete(T *aPtr)
{
	sld2::destroy_at(aPtr);
	sldMemFree(aPtr);
}

#endif //_SLD_PLATFORM_H_
