#ifndef _SLD_UTIL_H_
#define _SLD_UTIL_H_

#include "SldPlatform.h"
#include "SldMacros.h"

namespace sld2 {

template <typename T>
static inline SLD_CONSTEXPR const T& (min)(const T& lhs, const T& rhs)
{
	return lhs < rhs ? lhs : rhs;
}

template <typename T>
static inline SLD_CONSTEXPR const T& (max)(const T& lhs, const T& rhs)
{
	return lhs > rhs ? lhs : rhs;
}

// returns an uint32 bit mask with "first" @x bits set to 1; valid only for @x in [0..32]
static inline SLD_CONSTEXPR UInt32 bit_mask32(unsigned x)
{
	return (x >= sizeof(UInt32) * 8) ? ~0U : (1U << x) - 1;
}

// returns a number @n divided by @d rounded up
static inline SLD_CONSTEXPR UInt32 div_round_up(UInt32 n, UInt32 d)
{
	return (n + d - 1) / d;
}

// returns a number "aligned" to the next multiple of @a (a power of 2!)
template <unsigned a>
inline SLD_CONSTEXPR UInt32 align_up2(UInt32 n)
{
	static_assert(a != 0 && (a & (~a + 1)) == a, "@a must be a power of 2");
	return (n + a - 1) & ~(a - 1);
}

// returns the size of an array
template <typename T, UInt32 Size>
inline SLD_CONSTEXPR UInt32 array_size(const T(&)[Size]) { return Size; }

/**
 * Reads a uint32 from "packed bit array" of uint32s
 *
 * @param [in]  data     - pointer to the bit array
 * @param [in]  bitCount - the count of bits used to represent a single value
 * @param [in]  index    - the index of a value in the array
 *
 * @return an unpacked uint32
 */
static inline UInt32 readBits(const UInt32 *data, unsigned bitCount, UInt32 index)
{
	const UInt32 dataIdx = (bitCount * index) / 32;
	const unsigned bitPos = (bitCount * index) % 32;

	const unsigned remaining = 32 - bitPos;
	// the value is stored completely inside a single integer
	if (remaining >= bitCount)
		return (data[dataIdx] >> bitPos) & bit_mask32(bitCount);

	// the value is split between 2 integers
	return (data[dataIdx] >> bitPos) | ((data[dataIdx + 1] & bit_mask32(bitCount - remaining)) << remaining);
}

namespace murmur3
{

static inline UInt32 getblock32(const UInt8 *p)
{
	UInt32 u32;
	sldMemCopy(&u32, p, sizeof(UInt32));
	return u32;
}

static inline UInt32 rotl(UInt32 x, UInt8 r)
{
	return (x << r) | (x >> (32 - r));
}

static inline UInt32 mix(UInt32 h, UInt32 k)
{
	k *= 0xcc9e2d51u; // c1
	k = rotl(k, 15);
	k *= 0x1b873593u; // c2

	return h ^ k;
}

static inline UInt32 fmix(UInt32 h)
{
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;

	return h;
}

} // namespace murmur

/// MurmurHash3
static inline UInt32 MurmurHash3(const void *key, UInt32 len, UInt32 seed)
{
	const UInt8 *data = (const UInt8*)key;

	// body
	for (const UInt8 *end = data + len; data + 4 <= end; data += 4)
	{
		const UInt32 k = murmur3::getblock32(data);

		seed = murmur3::mix(seed, k);
		seed = murmur3::rotl(seed, 13);
		seed = seed * 5 + 0xe6546b64;
	}

	// tail
	UInt32 k = 0;
	switch (len & 3)
	{
	case 3: k ^= data[2] << 16;
	case 2: k ^= data[1] << 8;
	case 1: k ^= data[0];
		seed = murmur3::mix(seed, k);
	};

	// finalization
	return murmur3::fmix(seed ^ len);
}

/// реализация std::swap
template<typename Type>
static inline void swap(Type & aFirst, Type & aSecond)
{
	Type tmp = move(aFirst);
	aFirst = move(aSecond);
	aSecond = move(tmp);
}

/**
 * Нахождение "нижней границы" в остортированном масиве методом бинарного поиска
 *
 * @param [in]  aData  - указатель на первый элемент массива
 * @param [in]  aCount - количество эллементов в массиве
 * @param [in]  aLess  - предикат который должен вернуть "меньше" ли переданный
 *                       в него элемент искомого; с сигнатурой `bool (const T&)`
 *
 * @return индекс первого элемента который "больше или равен" искомому
 *         если aCount - значит все элементы "меньше" искомого
 *         в противном случае нужно проводить более точное сранение
 */
template <typename T, typename Less>
inline UInt32 lower_bound(const T *aData, UInt32 aCount, Less&& aLess)
{
	UInt32 low = 0;
	UInt32 hi = aCount;
	while (low < hi)
	{
		UInt32 mid = (hi + low) >> 1;
		if (aLess(aData[mid]))
			low = mid + 1;
		else
			hi = mid;
	}
	return low;
}

/**
* Бинарный поиск элемента в отсортированном массиве
*
* @param [in]  aData			- указатель на первый элемент массива
* @param [in]  aElementCount	- количество эллементов в массиве
* @param [in]  aSerchItem		- искомый элемент
* @param [out] aResultIndex		- индекс первого вхождения элемента, если он был найден
*								  индекс, по которому можно вставить элемент, если он не был найден
*
* @return	true - элемент был найден
*			false - элемент не был найден
*/
template<typename Type1, typename Type2>
static inline bool binary_search(const Type1 *aData, const UInt32 aElementCount, const Type2 &aSearchItem, UInt32 *aResultIndex = nullptr)
{
	struct less {
		const Type2 &searchItem;
		bool operator()(const Type1 &aItem) const { return aItem < searchItem; }
	};
	UInt32 index = lower_bound(aData, aElementCount, less{ aSearchItem });
	if (aResultIndex)
		*aResultIndex = index;

	if (index == aElementCount)
		return false;

	return aData[index] == aSearchItem;
}

template<typename Type>
inline bool InRange(const Type aValue, const Type aMin, const Type aMax)
{
	return aValue >= aMin && aValue < aMax;
}

// copies @aCount "objects" from @aToPtr to @aFromPtr
// the sizes of T and U must match; the blocks of memory should *not* overlap
template <typename T, typename U>
inline void memcopy_n(T *aToPtr, const U *const aFromPtr, UInt32 aCount)
{
	static_assert(sizeof(T) == sizeof(U), "");
	sldMemCopy(aToPtr, aFromPtr, aCount * sizeof(T));
}

// copies @aCount "objects" from @aToPtr to @aFromPtr
// the sizes of T and U must match
template <typename T, typename U>
inline void memmove_n(T *aToPtr, const U *const aFromPtr, UInt32 aCount)
{
	static_assert(sizeof(T) == sizeof(U), "");
	sldMemMove(aToPtr, aFromPtr, aCount * sizeof(T));
}

// clears (sets to all 0) @aCount objects at @aPtr
template <typename T>
inline void memzero_n(T* aPtr, UInt32 aCount)
{
	static_assert(is_trivial<T>::value, "Trying to memzero an array of non-trivial values!");
	sldMemSet(aPtr, 0, aCount * sizeof(T));
}

// destroys @aCount objects starting from @aPtr
template <typename T, enable_if<!is_trivially_destructible<T>::value> = 0>
inline void destroy_n(T *aPtr, UInt32 aCount)
{
	for (UInt32 i = 0; i < aCount; i++)
		destroy_at(aPtr + i);
}
template <typename T, enable_if<is_trivially_destructible<T>::value> = 0>
inline void destroy_n(T*, UInt32) { /* noop */ }

// value constructs @aCount objects inside memory pointed to by @aPtr
template <typename T, enable_if<!is_trivial<T>::value> = 0>
inline void value_construct_n(T *aPtr, UInt32 aCount)
{
	sld2_assume(aPtr != nullptr);
	for (UInt32 i = 0; i < aCount; i++)
		::new (static_cast<void*>(aPtr + i)) T();
}
template <typename T, enable_if<is_trivial<T>::value> = 0>
inline void value_construct_n(T *aPtr, UInt32 aCount)
{
	memzero_n(aPtr, aCount);
}

// default constructs @aCount objects inside memory pointed to by @aPtr
template <typename T, enable_if<!is_trivially_default_constructible<T>::value> = 0>
inline void default_construct_n(T *aPtr, UInt32 aCount)
{
	sld2_assume(aPtr != nullptr);
	for (UInt32 i = 0; i < aCount; i++)
		::new (static_cast<void*>(aPtr + i)) T;
}
template <typename T, enable_if<is_trivially_default_constructible<T>::value> = 0>
inline void default_construct_n(T*, UInt32) { /* noop */ }

// copy constructs @aCount objects from @aSrc inside memory pointed to by @aPtr
template <typename T, enable_if<!is_trivially_copyable<T>::value> = 0>
inline void uninitialized_copy_n(T *aPtr, const T *aSrc, UInt32 aCount)
{
	for (UInt32 i = 0; i < aCount; i++)
		construct_at(aPtr + i, aSrc[i]);
}
template <typename T, enable_if<is_trivially_copyable<T>::value> = 0>
inline void uninitialized_copy_n(T *aPtr, const T *aSrc, UInt32 aCount)
{
	memcopy_n(aPtr, aSrc, aCount);
}

// move constructs @aCount objects from @aSrc inside memory pointed to by @aPtr
template <typename T, enable_if<!is_trivially_copyable<T>::value> = 0>
inline void uninitialized_move_n(T *aPtr, T *aSrc, UInt32 aCount)
{
	for (UInt32 i = 0; i < aCount; i++)
		construct_at(aPtr + i, move(aSrc[i]));
}
template <typename T, enable_if<is_trivially_copyable<T>::value> = 0>
inline void uninitialized_move_n(T *aPtr, T *aSrc, UInt32 aCount)
{
	memcopy_n(aPtr, aSrc, aCount);
}

namespace detail {

template <typename T>
inline void uninitialized_fill_n(T *aPtr, const T &aSrc, UInt32 aCount, meta::tag_t<true>)
{
	static_assert(is_trivially_copyable<T>::value && sizeof(T) == 1, "");
	Int8 v = *(const Int8*)(&aSrc);
	sldMemSet(aPtr, v, aCount);
}

template <typename T>
inline void uninitialized_fill_n(T *aPtr, const T &aSrc, UInt32 aCount, meta::tag_t<false>)
{
	for (UInt32 i = 0; i < aCount; i++)
		construct_at(aPtr + i, aSrc);
}

} // namespace detail

// copy constructs @aCount objects inside memory pointed to by @aPtr using @aSrc as a value
template <typename T>
inline void uninitialized_fill_n(T *aPtr, const T &aSrc, UInt32 aCount)
{
	using tag = meta::tag_t<sizeof(T) == 1 && is_trivially_copyable<T>::value>;
	detail::uninitialized_fill_n(aPtr, aSrc, aCount, tag{});
}

// clears (sets to all 0) a single object by a reference to it
template <typename T>
inline void memzero(T &aRef)
{
	// XXX: add static_assert() for non-trivial types?
	sldMemSet(&aRef, 0, sizeof(T));
}

// nullptr_t type for explicit overloading on null pointers
using nullptr_t = decltype(nullptr);

// A pretty full featured std::unique_ptr clone
// 2 major differences are:
//  * no support for a custom deleter - it always uses sldDelete() for destroying the object
//  * no support for arrays (as it's impossible to do with sldDelete())
template <typename T>
class UniquePtr
{
	static_assert(!is_array<T>::value, "UniquePtr does not support arrays!");
public:
	using pointer = T*;
	using element_type = T;

	UniquePtr() : ptr_(nullptr) {}

	UniquePtr(nullptr_t) : UniquePtr() {}

	explicit UniquePtr(pointer ptr) : ptr_(ptr) {}

	UniquePtr(UniquePtr&& ref) : ptr_(ref.release()) {}

	template <class U, enable_if<is_convertible<typename UniquePtr<U>::pointer, pointer>::value> = 0>
	UniquePtr(UniquePtr<U>&& ref) : ptr_(ref.release()) {}

	~UniquePtr() { reset(); }

	UniquePtr& operator=(UniquePtr&& ref) { reset(ref.release()); return *this; }

	UniquePtr& operator=(nullptr_t) { reset(); return *this; }

	// non-copyable
	UniquePtr(const UniquePtr&) = delete;
	UniquePtr& operator=(const UniquePtr&) = delete;

	// returns a pointer to the managed object and releases the ownership
	pointer release()
	{
		pointer ptr = ptr_;
		ptr_ = nullptr;
		return ptr;
	}

	// replaces the managed object
	void reset(pointer ptr = pointer())
	{
		pointer old = ptr_;
		ptr_ = ptr;
		if (old)
			sldDelete(old);
	}

	// returns a pointer to the managed object
	pointer get() const { return ptr_; }

	// checks if there is associated managed object
	explicit operator bool() const { return get() != nullptr; }

	// access to the managed object
	add_lvalue_reference<T> operator*() const { return *get(); }
	pointer operator->() const { return get(); }

private:
	pointer ptr_;
};

template <typename T, typename... Args>
inline UniquePtr<T> make_unique(Args&&... aArgs) {
	return UniquePtr<T>(sldNew<T>(sld2::forward<Args>(aArgs)...));
}

template <class T> inline bool operator==(const UniquePtr<T> &x, nullptr_t) { return !x; }
template <class T> inline bool operator==(nullptr_t, const UniquePtr<T> &x) { return !x; }

// tag type used to construct default initialized elements in containers
struct default_init_t {};
static SLD_CONSTEXPR_OR_CONST default_init_t default_init = default_init_t();

} // namespace sld2


// Span
namespace sld2 {

// a "view" into a bunch of elements
template <typename ElementType>
class Span
{
public:
	using element_type = ElementType;
	using index_type   = UInt32;
	using pointer      = ElementType*;
	using reference    = ElementType&;
	using iterator     = pointer;

	SLD_CONSTEXPR Span()
		: data_(nullptr), size_(0) {}

	SLD_CONSTEXPR Span(nullptr_t) : Span() {}

	SLD_CONSTEXPR Span(pointer ptr, index_type count)
		: data_(ptr), size_(ptr ? count : 0) {}

	template <typename OtherElementType,
		enable_if<is_convertible<typename Span<OtherElementType>::pointer, pointer>::value> = 0>
	SLD_CONSTEXPR Span(const Span<OtherElementType> &other)
		: data_(other.data()), size_(other.size()) {}

	template <typename OtherElementType,
		enable_if<is_convertible<typename Span<OtherElementType>::pointer, pointer>::value> = 0>
	SLD_CONSTEXPR Span(Span<OtherElementType>&& other)
		: data_(other.data()), size_(other.size()) {}

	// subspans
	SLD_CONSTEXPR Span subspan(index_type pos, index_type count = index_type(-1)) const {
		return ((pos == 0 || pos < size()) && (pos == index_type(-1) || pos + count <= size()))
			? Span(data() + pos, count == index_type(-1) ? size() - pos : count) : Span();
	}

	SLD_CONSTEXPR Span first(index_type count) const { return subspan(0, count); }

	SLD_CONSTEXPR Span last(index_type count) const {
		return count <= size() ? subspan(size() - count) : Span();
	}

	// observers
	SLD_CONSTEXPR index_type size() const { return size_; }
	SLD_CONSTEXPR bool empty() const { return size() == 0; }

	// element access
	SLD_CONSTEXPR reference operator[](index_type idx) const { return data_[idx]; }
	SLD_CONSTEXPR pointer data() const { return data_; }
	SLD_CONSTEXPR reference front() const { return data()[0]; }
	SLD_CONSTEXPR reference back() const { return data()[size() - 1]; }

	// iterators
	iterator begin() const { return iterator(data()); }
	iterator end() const   { return iterator(data() + size()); }

private:
	pointer data_;
	index_type size_;
};

template <typename T>
Span<T> make_span(T *aData, typename Span<T>::index_type aCount) { return{ aData, aCount }; }

template <typename T, typename Span<T>::index_type N>
Span<T> make_span(T(&aData)[N]) { return{ aData, N }; }

} // namespace sld2

namespace sld2 {

// bitset support code
namespace bitset {

using Word = UInt32;

static inline SLD_CONSTEXPR Word bitMask(UInt32 aPos) { return 1u << (aPos % (sizeof(Word) * 8)); }
static inline SLD_CONSTEXPR Word bitWord(UInt32 aPos) { return aPos / (sizeof(Word) * 8); }

static inline SLD_CONSTEXPR bool test(const Word *aSet, UInt32 aPos) {
	return (aSet[bitWord(aPos)] & bitMask(aPos)) != 0;
}

static inline SLD_CXX14_CONSTEXPR void set(Word *aSet, UInt32 aPos) {
	aSet[bitWord(aPos)] |= bitMask(aPos);
}

static inline SLD_CXX14_CONSTEXPR void clear(Word *aSet, UInt32 aPos) {
	aSet[bitWord(aPos)] &= ~bitMask(aPos);
}

// returns the size of the table (in words) to represent @count bits
static inline SLD_CONSTEXPR UInt32 size(UInt32 count) {
	return div_round_up(count, sizeof(Word) * 8);
}

} // namespace bitset
} // namespace sld2

// Array
namespace sld2 {

// an std::array clone
template <typename T, UInt32 N>
struct Array
{
	static_assert(N != 0, "Empty arrays are unsupported.");

	using value_type      = T;
	using size_type       = UInt32;
	using pointer         = value_type*;
	using const_pointer   = const value_type*;
	using reference       = value_type&;
	using const_reference = const value_type&;
	using iterator        = pointer;
	using const_iterator  = const_pointer;

	// data access
	SLD_CXX14_CONSTEXPR reference operator[](size_type pos)       { return data()[pos]; }
	SLD_CONSTEXPR const_reference operator[](size_type pos) const { return data()[pos]; }
	SLD_CXX14_CONSTEXPR reference front()       { return data()[0]; }
	SLD_CONSTEXPR const_reference front() const { return data()[0]; }
	SLD_CXX14_CONSTEXPR reference back()        { return data()[size() - 1]; }
	SLD_CONSTEXPR const_reference back() const  { return data()[size() - 1]; }
	SLD_CXX14_CONSTEXPR pointer data()          { return __array_impl; }
	SLD_CONSTEXPR const_pointer data() const    { return __array_impl; }

	// capacity
	SLD_CONSTEXPR bool empty() const     { return size() == 0; }
	SLD_CONSTEXPR size_type size() const { return N; }

	// iterators
	SLD_CXX14_CONSTEXPR iterator begin() { return iterator(data()); }
	SLD_CXX14_CONSTEXPR iterator end()   { return iterator(data() + size()); }
	SLD_CONSTEXPR const_iterator cbegin() const { return const_iterator(data()); }
	SLD_CONSTEXPR const_iterator cend() const   { return const_iterator(data() + size()); }
	SLD_CONSTEXPR const_iterator begin() const  { return cbegin(); }
	SLD_CONSTEXPR const_iterator end() const    { return cend(); }

	// conversion to sld2::Span
	SLD_CXX14_CONSTEXPR operator Span<value_type>()       { return{ data(), size() }; }
	SLD_CONSTEXPR operator Span<const value_type>() const { return{ data(), size() }; }

	T __array_impl[N];
};

}

#endif // _SLD_UTIL_H_
