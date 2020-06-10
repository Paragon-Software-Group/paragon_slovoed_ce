#ifndef _SLD_DYN_ARRAY_H_
#define _SLD_DYN_ARRAY_H_

#include "SldPlatform.h"
#include "SldUtil.h"

namespace sld2 {

/**
 * DynArray - "урезаный" аналог CSldVector<>
 *
 * Основных отличий 2:
 *  - полное отсутствие возможности "динамического" изменения размера, т.е. количество объектов
 *    всегда равно количеству выделеной памяти и класс "жестко" отрабатывает ::resize() всегда
 *    перевыделяя память
 *  - полный "трэкинг" ошибок выделения памяти в конструкторах и ::resize(), т.е. класс можно
 *    использовать в ситуациях когда необходимо возвращать эти ошибки выше, при этом сам класс
 *    никогда не "отсегфолтит"
 *
 * Так же класс позволяет "отдавать" выделенный блок памяти (только для тривиально destructible
 * объектов).
 *
 * В общем, в сочетании с sld2::default_init класс идеально подходит для замены временных буферов
 * с относительно неизменным размером.
 */

template <typename T>
class DynArray
{
	static_assert(is_array<T>::value ? is_trivial<T>::value : true,
				  "arrays of only trivial types supported");

public:
	using value_type      = T;
	using size_type       = UInt32;
	using pointer         = value_type*;
	using const_pointer   = const value_type*;
	using reference       = value_type&;
	using const_reference = const value_type&;
	using iterator        = pointer;
	using const_iterator  = const_pointer;

	DynArray() : size_(0), ptr_(nullptr) {}

	~DynArray() { _replace(nullptr, 0); }

	explicit DynArray(size_type size) : DynArray()
	{
		if (allocate(size))
			value_construct_n(data(), size);
	}

	DynArray(size_type size, const_reference val) : DynArray()
	{
		if (allocate(size))
			uninitialized_fill_n(data(), val, size);
	}

	DynArray(default_init_t, size_type size) : DynArray()
	{
		if (allocate(size))
			default_construct_n(data(), size);
	}

	DynArray(DynArray&& other)
		: size_(other.size()), ptr_(other.data())
	{
		// clear the moved object
		other.ptr_ = nullptr;
		other.size_ = 0;
	}

	DynArray(const DynArray &other) : DynArray()
	{
		if (allocate(other.size()))
			uninitialized_copy_n(data(), other.data(), other.size());
	}

	template <size_type Size>
	explicit DynArray(const value_type(&data_)[Size]) : DynArray()
	{
		if (allocate(Size))
			uninitialized_copy_n(data(), data_, Size);
	}

	explicit DynArray(Span<const value_type> span) : DynArray()
	{
		if (allocate(span.size()))
			uninitialized_copy_n(data(), span.data(), span.size());
	}

	DynArray& operator=(DynArray&& other)
	{
		_replace(other.data(), other.size());
		// clear the moved object
		other.ptr_ = nullptr;
		other.size_ = 0;
		return *this;
	}

	DynArray& operator=(const DynArray &other)
	{
		assign(other.data(), other.size());
		return *this;
	}

	// data access
	reference operator[](size_type pos)             { return data()[pos]; }
	const_reference operator[](size_type pos) const { return data()[pos]; }
	reference front()             { return data()[0]; }
	const_reference front() const { return data()[0]; }
	reference back()              { return data()[size() - 1]; }
	const_reference back() const  { return data()[size() - 1]; }
	pointer data()                { return ptr_; }
	const_pointer data() const    { return ptr_; }

	// capacity
	bool empty() const         { return size() == 0; }
	size_type size() const     { return size_; }

	bool resize(size_type count)
	{
		struct init {
			void operator()(pointer ptr, size_type n) { value_construct_n(ptr, n); }
		};
		return reallocate(count, init{});
	}

	bool resize(size_type count, const_reference val)
	{
		struct init {
			const_reference v;
			void operator()(pointer ptr, size_type n) { uninitialized_fill_n(ptr, v, n); }
		};
		return reallocate(count, init{ val });
	}

	bool resize(default_init_t, size_type count)
	{
		struct init {
			void operator()(pointer ptr, size_type n) { default_construct_n(ptr, n); }
		};
		return reallocate(count, init{});
	}

	bool assign(size_type count, const_reference val)
	{
		struct init {
			const_reference v;
			void operator()(pointer ptr, size_type n) const { uninitialized_fill_n(ptr, v, n); }
		};
		return _assign(count, init{ val });
	}

	bool assign(const_pointer ptr, size_type count)
	{
		struct init {
			const_pointer p;
			void operator()(pointer ptr, size_type n) const { uninitialized_copy_n(ptr, p, n); }
		};
		return _assign(count, init{ ptr });
	}

	bool assign(Span<const value_type> span) { return assign(span.data(), span.size()); }

	template <size_type N>
	bool assign(const value_type(&data_)[N]) { return assign(data_, N); }

	iterator begin() { return iterator(data()); }
	iterator end() { return iterator(data() + size()); }
	const_iterator begin() const { return const_iterator(data()); }
	const_iterator end() const { return const_iterator(data() + size()); }
	const_iterator cbegin() const { return const_iterator(data()); }
	const_iterator cend() const { return const_iterator(data() + size()); }

	// releases the underlying memory to the caller
	// memory *MUST* be later freed with sldMemFree()
	pointer release()
	{
		static_assert(is_trivially_destructible<value_type>::value,
					  "::release() on non-trivially destructible types is unsupported as unsafe.");
		pointer ptr = data();
		ptr_ = nullptr;
		size_ = 0;
		return ptr;
	}

	// conversion to sld2::Span
	operator Span<value_type>()             { return{ data(), size() }; }
	operator Span<const value_type>() const { return{ data(), size() }; }

private:
	void _replace(pointer ptr, size_type count)
	{
		destroy_n(data(), size());
		if (ptr_)
			sldMemFree(ptr_);
		ptr_ = ptr;
		size_ = count;
	}

	bool allocate(size_type count)
	{
		if (count == 0)
			return false;

		pointer ptr = sldMemNew<value_type>(count);
		if (ptr)
		{
			ptr_ = ptr;
			size_ = count;
		}
		return ptr != nullptr;
	}

	template <typename Init>
	bool reallocate(size_type count, Init&& init)
	{
		if (count == size())
			return true;

		if (count == 0)
		{
			_replace(nullptr, 0);
			return true;
		}

		const size_type oldSize = size();
		if (reallocate(count, meta::tag_t<is_trivially_copyable<value_type>::value>{}))
		{
			if (count > oldSize)
				init(data() + oldSize, count - oldSize);
			return true;
		}
		return false;
	}

	bool reallocate(size_type count, meta::tag_t<false>)
	{
		pointer ptr = sldMemNew<value_type>(count);
		if (ptr)
		{
			uninitialized_move_n(ptr, data(), (min)(size(), count));
			_replace(ptr, count);
		}
		return ptr != nullptr;
	}

	bool reallocate(size_type count, meta::tag_t<true>)
	{
		pointer ptr = sldMemReallocT(ptr_, count);
		if (ptr)
		{
			ptr_ = ptr;
			size_ = count;
		}
		return ptr != nullptr;
	}

	template <typename Init>
	bool _assign(size_type count, Init&& init)
	{
		if (count == 0)
		{
			_replace(nullptr, 0);
			return true;
		}

		destroy_n(data(), size());
		if (count != size())
		{
			// this tricks reallocate into not moving any elements (as we've already destroyed them)
			size_ = 0;
			if (!reallocate(count, meta::tag_t<is_trivially_copyable<value_type>::value>{}))
				return false;
		}
		init(data(), count);
		return true;
	}

	// current size
	size_type size_;
	// actual data
	pointer ptr_;
};

template <typename T>
DynArray<T> to_dynarray(Span<const T> aSpan) { return DynArray<T>(aSpan); }

} // namespace sld2

#endif // _SLD_DYN_ARRAY_H_
