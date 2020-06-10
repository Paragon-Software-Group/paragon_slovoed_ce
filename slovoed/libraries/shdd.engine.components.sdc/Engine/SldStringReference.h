#ifndef _SLD_STRING_REFERENCE_H_
#define _SLD_STRING_REFERENCE_H_

#include "SldPlatform.h"
#include "SldMacros.h"
#include "SldUtil.h"

// character traits
namespace sld2 {

template <typename CharT>
struct char_traits_base
{
	using value_type      = CharT;
	using pointer         = CharT*;
	using const_pointer   = const CharT*;
	using reference       = CharT&;
	using const_reference = const CharT&;
	using size_type       = UInt32;

	static SLD_CXX14_CONSTEXPR size_type length(const CharT *aStr)
	{
		size_type size = 0;
		for (; *aStr; aStr++)
			size++;
		return size;
	}

	static SLD_CONSTEXPR inline Int32 cmp(CharT lhs, CharT rhs)
	{
		return sizeof(CharT) < sizeof(Int32) ?
				((Int32)lhs - (Int32)rhs) :    // "narrow" strings can use the sub trick
				(lhs < rhs ? -1 : lhs != rhs); // "wide" strings should use the explicit compare
	}

	static SLD_CXX14_CONSTEXPR Int32 compare(const CharT *aStr1, const CharT *aStr2, size_type aCount)
	{
		for (; aCount; aStr1++, aStr2++, --aCount)
		{
			const Int32 c = cmp(*aStr1, *aStr2);
			if (c != 0)
				return c;
		}
		return 0;
	}

  static const value_type* find(const value_type* aStr, size_type aCount, value_type aChar)
  {
    for (size_type i = 0; i < aCount; ++i)
    {
      if (aStr[i] == aChar)
        return aStr + i;
    }

    return nullptr;
  }
};

template <typename CharT>
struct char_traits : public char_traits_base<CharT> {};

} // namespace sld2

namespace sld2 {

// A pretty bare bones string_view implementation
template <typename CharT, typename traits = sld2::char_traits<CharT>>
class BasicStringRef
{
public:
	// typical std container boilerplate
	using value_type = typename traits::value_type;
	using pointer    = typename traits::const_pointer;
	using reference  = typename traits::const_reference;
	using iterator   = pointer;
	using size_type  = typename traits::size_type;
	static SLD_CONSTEXPR_OR_CONST size_type npos = size_type(-1);

	// constructors
	SLD_CONSTEXPR BasicStringRef() : ptr_(nullptr), len_(0) {}

	SLD_CONSTEXPR BasicStringRef(nullptr_t) : ptr_(nullptr), len_(0) {}

	// NOTE: This *ideally* should be constexpr in c++14 but sadly doing so would basically
	// preclude us using SldU16StringLiteral() to initialize string references as it contains
	// a reinterpret_cast<> (old C-style cast which is treated as such to be more precise)
	// and they are explicitly forbidden in constexpr context. So choosing between the 2 it's
	// the obviously better choice to drop constexpr here (for now? theoretically we could
	// fix it all up by transitioning the Engine to char16_t as the chosen char type).
	BasicStringRef(pointer str)
		: ptr_(str), len_(str ? traits::length(str) : 0) {}

	SLD_CONSTEXPR BasicStringRef(pointer str, size_type len)
		: ptr_(str), len_(len) {}

	// capacity
	SLD_CONSTEXPR size_type size()   const { return len_; }
	SLD_CONSTEXPR size_type length() const { return size(); }
	SLD_CONSTEXPR bool empty()       const { return size() == 0; }

	// element access
	SLD_CONSTEXPR reference operator[](size_type pos) const { return ptr_[pos]; }

	SLD_CONSTEXPR reference front() const { return ptr_[0]; }
	SLD_CONSTEXPR reference back()  const { return ptr_[size() - 1]; }

	// NOTE: The return is *not* guaranteed to be a null-terminated string!
	SLD_CONSTEXPR pointer data() const { return ptr_; }

	// substring
	SLD_CONSTEXPR BasicStringRef substr(size_type pos, size_type n = npos) const
	{
		return ((pos == 0 || pos < size()) && (pos == n || pos + n <= size()))
			? BasicStringRef(data() + pos, n == npos ? size() - pos : n) : BasicStringRef();
	}

	SLD_CXX14_CONSTEXPR Int32 compare(BasicStringRef other) const
	{
		const Int32 cmp = traits::compare(data(), other.data(), (sld2::min)(size(), other.size()));
		return cmp != 0 ? cmp : (size() == other.size() ? 0 : size() < other.size() ? -1 : 1);
	}

	// basic iterators
	iterator begin() const { return iterator(data()); }
	iterator   end() const { return iterator(data() + size()); }

private:
	pointer ptr_;
	size_type len_;
};

// macro used to implement string ref <-> c string comparison operators
// expects the "main" string reference based one to be already defined
#define DEFINE_OP_CMP(_op) \
template<typename CharT, typename Traits> \
inline bool operator _op (BasicStringRef<CharT, Traits> lhs, const CharT *const rhs) { \
	return lhs _op BasicStringRef<CharT, Traits>(rhs); \
} \
template<typename CharT, typename Traits> \
inline bool operator _op (const CharT *const lhs, BasicStringRef<CharT, Traits> rhs) { \
	return BasicStringRef<CharT, Traits>(lhs) _op rhs; \
}

// Equality (can be implemented faster than comparison)
template<typename CharT, typename Traits>
inline bool operator==(BasicStringRef<CharT, Traits> lhs, BasicStringRef<CharT, Traits> rhs)
{
	return lhs.size() == rhs.size() && (lhs.data() == rhs.data() || lhs.compare(rhs) == 0);
}
DEFINE_OP_CMP(==)

// Inequality
template<typename CharT, typename Traits>
inline bool operator!=(BasicStringRef<CharT, Traits> lhs, BasicStringRef<CharT, Traits> rhs)
{
	return !(lhs == rhs);
}
DEFINE_OP_CMP(!=)

// macro used to implement all of the "ordering" comparison operators
#define DEFINE_OP_CMP2(_op) \
template<typename CharT, typename Traits> \
inline bool operator _op (BasicStringRef<CharT, Traits> lhs, BasicStringRef<CharT, Traits> rhs) { \
	return lhs.compare(rhs) _op 0; \
} \
DEFINE_OP_CMP(_op)

DEFINE_OP_CMP2(<)
DEFINE_OP_CMP2(>)
DEFINE_OP_CMP2(<=)
DEFINE_OP_CMP2(>=)

#undef DEFINE_OP_CMP
#undef DEFINE_OP_CMP2

} // namespace sld2

/// двухбайтовая ссылка на строчку
using SldU16StringRef = sld2::BasicStringRef<UInt16>;

// UTF-16 string reference literal macro
// Creates a statically sized string reference. Must be used with string literals.
// *DOES NOT* support embedded zeroes (as it's sizeof based)
#ifdef SLD_HAS_CONSTEXPR
#  define SldU16StringRefLiteral(_X) SldU16StringRef(SldU16StringLiteral(_X), sld2::array_size(_X) - 1)
#else
#  define SldU16StringRefLiteral(_X) SldU16StringRef(SldU16StringLiteral(_X), sizeof(_X) / sizeof((_X)[0]) - 1)
#endif

#endif
