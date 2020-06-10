#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <type_traits>

#include "macros.h"

// a simple range template for range based for loops
// the only requirement is the the iterator must be copyable
template <typename It>
class RangeImpl
{
public:
	typedef It iterator;

	RangeImpl(It first, It last) : begin_(first), end_(last) {}
	iterator begin() const { return begin_; };
	iterator end() const { return end_; };
private:
	It begin_, end_;
};

namespace sld {
namespace detail {

template <typename T>
struct IntegerIterator
{
	IntegerIterator(T value) : val_(value) {}
	bool operator!=(const IntegerIterator& other) const { return val_ != other.val_; }
	IntegerIterator& operator++() { val_++; return *this; }
	T operator*() const { return val_; }
private:
	T val_;
};

} // namespace detail
} // namespace sld

// python like xrange() - [start, stop)
template <typename T, typename It = typename sld::detail::IntegerIterator<T>>
static inline RangeImpl<It> xrange(T start, T stop)
{
	static_assert(std::is_integral<T>::value, "xrange() works only for integral ranges");
	return{ It(start), It(stop) };
}

// python like xrange() - [0, stop)
template <typename T, typename It = typename sld::detail::IntegerIterator<T>>
static inline RangeImpl<It> xrange(T stop)
{
	static_assert(std::is_integral<T>::value, "xrange() works only for integral ranges");
	return{ It(0), It(stop) };
}

namespace sld {
namespace traits {

template<typename T>
struct has_size
{
	template<typename U, size_t(U::*)() const> struct SFINAE {};
	template<typename U> static char Test(SFINAE<U, &U::size>*);
	template<typename U> static int Test(...);
	static constexpr bool value = sizeof(Test<T>(0)) == sizeof(char);
};

} // namespace traits
} // namespace sld

// iterate over the container indices (container must have a ::size() method)
template <typename C, typename = typename std::enable_if<sld::traits::has_size<C>::value>::type>
static inline RangeImpl<sld::detail::IntegerIterator<size_t>> indices(const C &cont)
{
	return{ 0, cont.size() };
}

// iterate over the arrays indices
template <typename T, size_t N, typename It = typename sld::detail::IntegerIterator<size_t>>
static inline RangeImpl<It> indices(T(&)[N])
{
	return{ 0, N };
}

namespace sld {
namespace detail {

template <typename It, template <class inner> class WrapIt>
class WrapperIteratorRange
{
public:
	typedef WrapIt<It> iterator;

	WrapperIteratorRange(It first, It last) : begin_(first), end_(last) {}
	iterator begin() const { return begin_; };
	iterator end() const { return end_; };
private:
	It begin_, end_;
};

template <typename It>
struct EnumerateValue
{
	using pointer = typename std::iterator_traits<It>::pointer;
	using reference = typename std::iterator_traits<It>::reference;

	pointer operator->() { return it_.operator->(); }
	const pointer operator->() const { return it_.operator->(); }

	reference operator*() { return *it_; }
	const reference operator*() const { return *it_; }

	EnumerateValue(size_t index, It it) : index(index), it_(it) {}

	const size_t index;
private:
	It it_;
};

template <typename It>
class EnumerateIterator
{
public:
	EnumerateIterator(It it) : it_(it), i_(0) {}
	EnumerateValue<It> operator*() const { return{ i_, it_ }; }
	bool operator!=(const EnumerateIterator& other) const { return it_ != other.it_; }
	EnumerateIterator& operator++() { ++i_; ++it_; return *this; }
private:
	It it_;
	size_t i_;
};

template<typename It>
struct is_const_iterator
{
	using pointer = typename std::iterator_traits<It>::pointer;
	static constexpr bool value = std::is_const<typename std::remove_pointer<pointer>::type>::value;
};

template <typename It>
class DereferencingIterator
{ // holy molly templates!
	static_assert(std::is_pointer<typename std::iterator_traits<It>::value_type>::value,
				  "The iterator must be over a collection of pointers!");
	static constexpr bool is_const = is_const_iterator<It>::value;
public:
	// iterator_traits boilerplate
	using iterator_category = typename std::iterator_traits<It>::iterator_category;
	using value_type = typename std::remove_pointer<typename std::iterator_traits<It>::value_type>::type;
	using difference_type = typename std::iterator_traits<It>::difference_type;
	using pointer = typename std::conditional<is_const, const value_type*, value_type*>::type;
	using reference = typename std::conditional<is_const, const value_type&, value_type&>::type;

	DereferencingIterator(It it) : it_(it) {}
	bool operator!=(const DereferencingIterator& other) const { return it_ != other.it_; }
	DereferencingIterator& operator++() { ++it_; return *this; }
	pointer operator->() const { return *it_; }
	reference operator*() const { auto val = *it_; assert(val); return *val; }
private:
	It it_;
};

template <typename It>
class MapDerefIterator
{
	static constexpr bool is_const = is_const_iterator<It>::value;
public:
	// iterator_traits boilerplate
	using iterator_category = typename std::iterator_traits<It>::iterator_category;
	using value_type = typename std::iterator_traits<It>::value_type::second_type;
	using difference_type = typename std::iterator_traits<It>::difference_type;
	using pointer = typename std::conditional<is_const, const value_type*, value_type*>::type;
	using reference = typename std::conditional<is_const, const value_type&, value_type&>::type;

	MapDerefIterator(It it) : it_(it) {}
	bool operator!=(const MapDerefIterator& other) const { return it_ != other.it_; }
	MapDerefIterator& operator++() { ++it_; return *this; }
	pointer operator->() const { return &it_->second; }
	reference operator*() const { return it_->second; }
private:
	It it_;
};

} // namespace detail

template <typename It>
using DereferencingRange = detail::WrapperIteratorRange<It, detail::DereferencingIterator>;

template <typename It>
using EnumeratingRange = detail::WrapperIteratorRange<It, detail::EnumerateIterator>;

template <typename It>
using MapDerefRange = detail::WrapperIteratorRange<It, detail::MapDerefIterator>;

} // namespace sld

template <typename T>
auto enumerate(T&& container) -> sld::EnumeratingRange<decltype(container.begin())>
{
	return sld::EnumeratingRange<decltype(container.begin())>(container.begin(), container.end());
}

template <typename T, size_t N>
auto enumerate(T(&data)[N]) -> sld::EnumeratingRange<decltype(std::begin(data))>
{
	return sld::EnumeratingRange<decltype(std::begin(data))>(std::begin(data), std::end(data));
}

template <typename T>
auto iterate(T *data, size_t count) -> RangeImpl<T*>
{
	return{ data, data + count };
}

namespace sld {
template <size_t> struct fnv1a_selector {};
template <> struct fnv1a_selector<4> {
	static constexpr size_t prime = size_t(16777619UL);
	static constexpr size_t offset_basis = size_t(2166136261UL);
};
template <> struct fnv1a_selector<8> {
	static constexpr size_t prime = size_t(1099511628211ULL);
	static constexpr size_t offset_basis = size_t(14695981039346656037ULL);
};
} // namespace sld

// simple fnv-1a hash implementation
struct fnv1a
{
	static constexpr size_t prime = sld::fnv1a_selector<sizeof(size_t)>::prime;
	static constexpr size_t offset_basis = sld::fnv1a_selector<sizeof(size_t)>::offset_basis;

	static size_t accumulate(size_t hash, const void *data, size_t size)
	{
		for (size_t i : xrange(size))
		{
			hash ^= ((const uint8_t*)data)[i];
			hash *= prime;
		}
		return hash;
	}

	static size_t hash(const void *data, size_t size)
	{
		return accumulate(offset_basis, data, size);
	}
};

template <typename T, typename U>
constexpr T div_round_up(T n, U d) { return static_cast<T>((n + d - 1) / d); }

template <typename T, typename U>
constexpr T align_up(T v, U n) { return static_cast<T>(div_round_up(v, n) * n); }

static inline constexpr uint32_t bit_mask(unsigned x)
{
	return (x >= sizeof(uint32_t) * 8) ? (uint32_t)-1 : (1U << x) - 1;
}

// A pretty simple memory buffer class that can release it's storage with some caveats:
//  * the buffer size is immutable, once constructed you can't resize the buffer
//  * the memory allocated is always zero initialized
class MemoryBuffer
{
public:
	MemoryBuffer() : data_(nullptr), size_(0) {}

	MemoryBuffer(std::nullptr_t) : data_(nullptr), size_(0) {}

	explicit MemoryBuffer(size_t size) : size_(size)
	{
		data_ = size ? new uint8_t[size]() : nullptr;
	}

	MemoryBuffer(const void *data, size_t size) : size_(size)
	{
		data_ = size_ ? memdup(data, size_) : nullptr;
	}

	MemoryBuffer(const MemoryBuffer &other) : size_(other.size())
	{
		data_ = size_ ? memdup(other.data(), size_) : nullptr;
	}

	MemoryBuffer(MemoryBuffer&& other)
	{
		size_ = other.size_;
		data_ = other.release();
	}

	MemoryBuffer& operator=(const MemoryBuffer &other)
	{
		clear();
		size_ = other.size();
		data_ = size_ ? memdup(other.data(), size_) : nullptr;
		return *this;
	}

	MemoryBuffer& operator=(MemoryBuffer&& other)
	{
		clear();
		size_ = other.size_;
		data_ = other.release();
		return *this;
	}

	~MemoryBuffer() { clear(); }

	// releases the underlying storage buffer clearing the object in the process
	// as the memory is allocated through c++ new[] it must be freed with delete[]
	uint8_t* release()
	{
		uint8_t *ptr = data_;
		data_ = nullptr;
		size_ = 0;
		return ptr;
	}

	// "shrinks" the size of the stored buffer
	void shrink(size_t size)
	{
		assert(size <= size_);
		if (size < size_)
			size_ = size;
	}

	// std::vector like accessors
	size_t size() const { return size_; }
	bool empty() const { return size_ == 0; }

	uint8_t* data() { return data_; }
	const uint8_t* data() const { return data_; }

	uint8_t& operator[](size_t n) { return data_[n]; }
	uint8_t operator[](size_t n) const { return data_[n]; }

private:
	static uint8_t* memdup(const void *const data, size_t size)
	{
		assert(data && size);
		uint8_t *ptr = new uint8_t[size];
		memcpy(ptr, data, size);
		return ptr;
	}

	void clear()
	{
		if (data_)
			delete[] data_;
		data_ = nullptr;
		size_ = 0;
	}

	uint8_t *data_;
	size_t size_;
};

// A pretty simple non-owning reference to a bunch of data
class MemoryRef
{
public:
	typedef uint8_t byte;

	MemoryRef() : data_(nullptr), size_(0) {}
	MemoryRef(std::nullptr_t) : data_(nullptr), size_(0) {}
	MemoryRef(const void *data, size_t size) : data_(data), size_(size) {}
	MemoryRef(const MemoryBuffer &buf) : data_(buf.data()), size_(buf.size()) {}

	const byte* data() const { return (const byte*)data_; }
	const size_t size() const { return size_; }
	bool empty() const { return size_ == 0; }

private:
	const void* data_;
	size_t size_;
};

namespace sld {

template <typename T>
inline MemoryRef make_memref(const T *data, size_t count) {
	return MemoryRef(data, count * sizeof(T));
}

} // namespace sld
