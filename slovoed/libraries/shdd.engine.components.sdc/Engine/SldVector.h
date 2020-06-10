#ifndef _SLD_VECTOR_H_
#define _SLD_VECTOR_H_

#include "SldPlatform.h"
#include "SldUtil.h"

/// собственная реализация std::vector
template <typename VectorElement>
class CSldVector
{
	// machinery for tag dispatching
	enum { Generic = 0, TriviallyCopyable, Pointer };
	using copyable_tag =
		sld2::meta::tag_t<sld2::is_trivially_copyable<VectorElement>::value ? TriviallyCopyable : Generic>;

public:
	typedef VectorElement			value_type;
	typedef UInt32					size_type;
	typedef VectorElement*			pointer;
	typedef const VectorElement*	const_pointer;
	typedef VectorElement&			reference;
	typedef const VectorElement&	const_reference;
	typedef pointer             	iterator;
	typedef const_pointer       	const_iterator;
	static SLD_CONSTEXPR_OR_CONST size_type	npos = size_type(-1);

	CSldVector() : ptr_(nullptr), size_(0), capacity_(0) {}

	~CSldVector() { _assign(nullptr, 0); }

	explicit CSldVector(size_type count)
		: ptr_(nullptr), size_(0), capacity_(0)
	{
		if (allocate(count))
			sld2::value_construct_n(ptr_, count);
	}

	CSldVector(size_type count, const_reference val)
		: ptr_(nullptr), size_(0), capacity_(0)
	{
		if (allocate(count))
			sld2::uninitialized_fill_n(ptr_, val, count);
	}

	CSldVector(sld2::default_init_t, size_type count)
		: ptr_(nullptr), size_(0), capacity_(0)
	{
		if (allocate(count))
			sld2::default_construct_n(ptr_, count);
	}

	CSldVector(const CSldVector &other)
		: ptr_(nullptr), size_(0), capacity_(0)
	{
		if (allocate(other.size()))
			sld2::uninitialized_copy_n(ptr_, other.data(), other.size());
	}

	CSldVector& operator=(const CSldVector &other)
	{
		clear();
		if (other.size() > capacity_)
			reallocate(other.size());
		sld2::uninitialized_copy_n(ptr_, other.data(), other.size());
		size_ = other.size();
		return *this;
	}

	CSldVector(CSldVector&& other)
		: ptr_(other.ptr_), size_(other.size_), capacity_(other.capacity_)
	{
		// clear the moved object
		other.ptr_ = nullptr;
		other.size_ = other.capacity_ = 0;
	}

	CSldVector & operator=(CSldVector&& other)
	{
		_assign(other.ptr_, other.capacity_);
		size_ = other.size_;
		// clear the moved object
		other.ptr_ = nullptr;
		other.size_ = other.capacity_ = 0;
		return *this;
	}

	template <size_type Size>
	CSldVector(const value_type(&data)[Size])
		: ptr_(nullptr), size_(0), capacity_(0)
	{
		if (allocate(Size))
			sld2::uninitialized_copy_n(ptr_, data, Size);
	}

	explicit CSldVector(sld2::Span<const value_type> span)
		: ptr_(nullptr), size_(0), capacity_(0)
	{
		if (allocate(span.size()))
			sld2::uninitialized_copy_n(ptr_, span.data(), span.size());
	}

	bool operator==(const CSldVector &other)
	{
		if (size() != other.size())
			return false;

		for (size_type i = 0; i < size(); i++)
		{
			if (ptr_[i] != other[i])
				return false;
		}
		return true;
	}

	// data access
	reference operator[](size_type pos)             { return ptr_[pos]; }
	const_reference operator[](size_type pos) const { return ptr_[pos]; }
	reference front()             { return ptr_[0]; }
	const_reference front() const { return ptr_[0]; }
	reference back()              { return ptr_[size_ - 1]; }
	const_reference back() const  { return ptr_[size_ - 1]; }
	pointer data()                { return ptr_; }
	const_pointer data() const    { return ptr_; }

	// capacity
	bool empty() const         { return size_ == 0; }
	size_type size() const     { return size_; }
	size_type capacity() const { return capacity_; }

	void reserve(size_type count)
	{
		if (count > capacity_)
			reallocate(count);
	}

	void shrink_to_fit()
	{
		if (size_ == capacity_)
			return;
		if (size_ == 0)
			_assign(nullptr, 0);
		else
			reallocate(size_);
	}

	// modifiers
	reference push_back(const_reference item)
	{
		grow(1);
		sld2::construct_at(&ptr_[size_++], item);
		return back();
	}

	reference push_back(value_type&& item)
	{
		grow(1);
		sld2::construct_at(&ptr_[size_++], sld2::move(item));
		return back();
	}

	template <typename... Args>
	reference emplace_back(Args&&... args)
	{
		grow(1);
		sld2::construct_at(&ptr_[size_++], sld2::forward<Args>(args)...);
		return back();
	}

	void pop_back()
	{
		sld2::destroy_at(&ptr_[--size_]);
	}

	void clear()
	{
		sld2::destroy_n(ptr_, size_);
		size_ = 0;
	}

	void resize(size_type count)
	{
		struct init {
			void operator()(pointer ptr, size_type n) { sld2::value_construct_n(ptr, n); }
		};
		_resize(count, init{});
	}

	void resize(size_type count, const_reference val)
	{
		struct init {
			const_reference v;
			void operator()(pointer ptr, size_type n) { sld2::uninitialized_fill_n(ptr, v, n); }
		};
		_resize(count, init{ val });
	}

	void resize(sld2::default_init_t, size_type count)
	{
		struct init {
			void operator()(pointer ptr, size_type n) { sld2::default_construct_n(ptr, n); }
		};
		_resize(count, init{});
	}

	template <class InputIterator>
	void assign(InputIterator first, InputIterator last)
	{
		clear();
		assign(first, last, sld2::meta::tag_t<sld2::is_pointer<InputIterator>::value ? Pointer : Generic>{});
	}

	void assign(size_type count, const_reference val)
	{
		clear();
		resize(count, val);
	}

	iterator begin() { return iterator(ptr_); }
	iterator end() { return iterator(ptr_ + size_); }
	const_iterator begin() const { return const_iterator(ptr_); }
	const_iterator end() const { return const_iterator(ptr_ + size_); }
	const_iterator cbegin() const { return const_iterator(ptr_); }
	const_iterator cend() const { return const_iterator(ptr_ + size_); }

	iterator insert(const size_type aPos, const_reference aNewElement)
	{
		if (aPos >= size())
			return iterator(&push_back(aNewElement));

		make_hole(aPos);
		sld2::construct_at(ptr_ + aPos, aNewElement);
		return iterator(ptr_ + aPos);
	}

	iterator insert(const size_type aPos, value_type && aNewElement)
	{
		if (aPos >= size())
			return iterator(&push_back(sld2::move(aNewElement)));

		make_hole(aPos);
		sld2::construct_at(ptr_ + aPos, sld2::move(aNewElement));
		return iterator(ptr_ + aPos);
	}

  iterator insert(const size_type aPos, const size_type aCount, const_reference aNewElement)
  {
    if (aPos >= size())
    {
      const size_type originalSize = size();
      grow(aCount);
      sld2::uninitialized_fill_n(ptr_ + originalSize, aNewElement, aCount);
      size_ += aCount;
      return iterator(ptr_ + originalSize);
    }

    make_hole(aPos, aCount);
    sld2::uninitialized_fill_n(ptr_ + aPos, aNewElement, aCount);
    return iterator(ptr_ + aPos);
  }

	void erase(size_type pos, size_type count = 1)
	{
		if (pos > size_)
			return;
		if (pos + count > size_)
			count = size_ - pos;

		erase(pos, count, copyable_tag{});
		size_ -= count;
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		const size_type pos = static_cast<size_type>(first - begin());
		erase(pos, static_cast<size_type>(last - first));
		return pos < size() ? begin() + pos : end();
	}

	iterator erase(const_iterator pos)
	{
		return erase(pos, pos + 1);
	}

	// conversion to sld2::Span
	operator sld2::Span<value_type>()             { return{ data(), size() }; }
	operator sld2::Span<const value_type>() const { return{ data(), size() }; }

private:
	size_type get_capacity(size_type count) const
	{
		return (count * 10 / 9) + (count < 9 ? 3 : 6);
	}

	void grow(size_type count)
	{
		const size_type newSize = size_ + count;
		if (newSize > capacity_)
			reallocate(get_capacity(newSize));
	}

	void _assign(pointer data, size_type capacity)
	{
		sld2::destroy_n(ptr_, size_);
		if (ptr_)
			sldMemFree(ptr_);
		ptr_ = data;
		capacity_ = capacity;
	}

	// makes a hole at position @pos
	// pos *must* be inside the vector
	void make_hole(size_type pos, size_type count = 1)
	{
		// assert(pos < size_);
		make_hole(pos, count, copyable_tag{});
		size_ += count;
	}

	bool allocate(size_type count)
	{
		if (count == 0) return false;
		ptr_ = sldMemNew<value_type>(count);
		size_ = capacity_ = count;
		return true;
	}

	// reallocates the storage for a given capacity
	// @count *must be* >= size_
	void reallocate(size_type count)
	{
		// assert(count >= size_);
		reallocate(count, copyable_tag{});
	}

	template <typename Init>
	void _resize(size_type count, Init&& init)
	{
		if (count == size_)
			return;

		if (count < size_)
		{
			sld2::destroy_n(ptr_ + count, size_ - count);
		}
		else // count > size_
		{
			if (count > capacity_)
				reallocate(count);
			init(ptr_ + size_, count - size_);
		}
		size_ = count;
	}

	// tag dispatching implementations

	void reallocate(size_type count, sld2::meta::tag_t<Generic>)
	{
		pointer ptr = sldMemNew<value_type>(count);
		sld2::uninitialized_move_n(ptr, ptr_, size_);
		_assign(ptr, count);
	}
	void reallocate(size_type count, sld2::meta::tag_t<TriviallyCopyable>)
	{
		ptr_ = sldMemReallocT(ptr_, count);
		capacity_ = count;
	}

	void erase(size_type pos, size_type count, sld2::meta::tag_t<Generic>)
	{
		sld2::destroy_n(ptr_ + pos, count);
		for (size_type i = pos; i < size_ - count; i++)
		{
			sld2::construct_at(ptr_ + i, sld2::move(ptr_[i + count]));
			sld2::destroy_at(ptr_ + i + count);
		}
	}
	void erase(size_type pos, size_type count, sld2::meta::tag_t<TriviallyCopyable>)
	{
		sld2::memmove_n(ptr_ + pos, ptr_ + pos + count, size_ - pos - count);
	}

	void make_hole(size_type pos, size_type count, sld2::meta::tag_t<Generic>)
	{
		const size_type newSize = size_ + count;
		if (newSize <= capacity_)
		{
			// shift the elements starting from pos count places "up"
			for (size_type i = newSize - 1; i > pos + count - 1; i--)
			{
				sld2::construct_at(ptr_ + i, sld2::move(ptr_[i - count]));
				sld2::destroy_at(ptr_ + i - count);
			}
		}
		else
		{
			const size_type newCapacity = get_capacity(newSize);
			// basically open coded reallocate that leaves a hole in the proper place
			pointer ptr = sldMemNew<value_type>(newCapacity);
			// move the elements before the hole
			sld2::uninitialized_move_n(ptr, ptr_, pos);
			// move the elements after the hole
			sld2::uninitialized_move_n(ptr + pos + count, ptr_ + pos, size_ - pos);
			// update the storage
			_assign(ptr, newCapacity);
		}
	}
	void make_hole(size_type pos, size_type count, sld2::meta::tag_t<TriviallyCopyable>)
	{
		const size_type newSize = size_ + count;
		if (newSize > capacity_)
			reallocate(get_capacity(newSize), sld2::meta::tag_t<TriviallyCopyable>{});
		// shift the elements starting from pos count places "up"
		sld2::memmove_n(ptr_ + pos + count, ptr_ + pos, size_ - pos);
	}

	template <class InputIterator>
	void assign(InputIterator first, InputIterator last, sld2::meta::tag_t<Generic>)
	{
		for (; first != last; ++first)
			push_back(*first);
	}

	template <class InputIterator>
	void assign(InputIterator first, InputIterator last, sld2::meta::tag_t<Pointer>)
	{
		size_type count = static_cast<size_type>(last - first);
		if (count > capacity_)
			reallocate(count);
		sld2::uninitialized_copy_n(ptr_, first, count);
		size_ = count;
	}

	// actual data
	value_type *ptr_;
	// current vector size
	size_type size_;
	// current capacity
	size_type capacity_;
};

namespace sld2 {

template <typename T, typename U>
typename CSldVector<T>::size_type find(const CSldVector<T> &aVec, const U &aElement)
{
	for (typename CSldVector<T>::size_type i = 0; i < aVec.size(); i++)
	{
		if (aVec[i] == aElement)
			return i;
	}
	return CSldVector<T>::npos;
}

template <typename T, typename U>
typename CSldVector<T>::size_type sorted_find(const CSldVector<T> &aVec, const U &aElement)
{
	typename CSldVector<T>::size_type resultPos;
	if (sld2::binary_search(aVec.data(), aVec.size(), aElement, &resultPos))
		return resultPos;
	return CSldVector<T>::npos;
}

template <typename T, typename U>
typename CSldVector<T>::iterator sorted_insert(CSldVector<T> &aVec, const U &aNewElement)
{
	typename CSldVector<T>::size_type resultPos;
	if (!sld2::binary_search(aVec.data(), aVec.size(), aNewElement, &resultPos))
		aVec.insert(resultPos, aNewElement);
	return{ aVec.data() + resultPos };
}

template <typename T, typename U>
typename CSldVector<T>::iterator sorted_insert(CSldVector<T> &aVec, U&& aNewElement)
{
	typename CSldVector<T>::size_type resultPos;
	if (!sld2::binary_search(aVec.data(), aVec.size(), aNewElement, &resultPos))
		aVec.insert(resultPos, sld2::forward<U>(aNewElement));
	return{ aVec.data() + resultPos };
}

} // namespace sld2

namespace sld2 {

template <typename T>
CSldVector<T> to_vector(Span<const T> aSpan) { return CSldVector<T>(aSpan); }

} // namespace sld2

#endif
