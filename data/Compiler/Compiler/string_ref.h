/*
  An almost fully featured string_view implemetation.

  Based on:
   * StringRef implementation in LLVM (http://llvm.org)
   * Marshall Clow's implementation,
   * N3422 by Jeffrey Yasskin [http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3442.html]
   * N4480 [http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4480.html#string.view]
*/

#ifndef _STRING_REF_HPP
#define _STRING_REF_HPP

#include <cstddef>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <string>

#include "util.h" // for fnv1a
#include "macros.h"

// forward string_ref declaration
namespace sld {

template<typename charT, typename traits = std::char_traits<charT> > class basic_string_ref;

typedef basic_string_ref<char, std::char_traits<char> >         string_ref;
typedef basic_string_ref<wchar_t, std::char_traits<wchar_t> >   wstring_ref;
typedef basic_string_ref<char16_t, std::char_traits<char16_t> > u16string_ref;

} // namespace sld

namespace sld {

namespace detail
{
    //  A helper functor because sometimes we don't have lambdas
    template <typename charT, typename traits>
    class string_ref_traits_eq {
    public:
        string_ref_traits_eq(charT ch) : ch_(ch) {}
        bool operator () (charT val) const { return traits::eq(ch_, val); }
        charT ch_;
    };
}

template<typename charT, typename traits>
class basic_string_ref
{
public:
    // types
    typedef traits traits_type;
    typedef charT value_type;
    typedef const charT* pointer;
    typedef const charT& reference;
    typedef const charT& const_reference;
    typedef pointer const_iterator;
    typedef const_iterator iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef const_reverse_iterator reverse_iterator;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    static constexpr size_type npos = size_type(-1);

    // basic_string_ref constructors and assignment operators
    constexpr basic_string_ref() noexcept : ptr_(nullptr), len_(0) {}
    constexpr basic_string_ref(const basic_string_ref &rhs) noexcept = default;
    basic_string_ref& operator=(const basic_string_ref &rhs) noexcept = default;
    template<typename Allocator>
    basic_string_ref(const std::basic_string<charT, traits, Allocator>& str) noexcept
        : ptr_(str.data()), len_(str.length()) {}
    constexpr basic_string_ref(const charT *const str)
        : ptr_(str), len_(str ? traits::length(str) : 0) {}
    constexpr basic_string_ref(const charT *const str, size_type len)
        : ptr_(str), len_(len) {}

    // basic_string_ref iterator support
    constexpr const_iterator   begin() const noexcept { return ptr_; }
    constexpr const_iterator  cbegin() const noexcept { return ptr_; }
    constexpr const_iterator     end() const noexcept { return ptr_ + len_; }
    constexpr const_iterator    cend() const noexcept { return ptr_ + len_; }
    const_reverse_iterator  rbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator    rend() const noexcept { return const_reverse_iterator(begin()); }
    const_reverse_iterator   crend() const noexcept { return const_reverse_iterator(begin()); }

    // basic_string_ref capacity
    constexpr size_type size()     const noexcept { return len_; }
    constexpr size_type length()   const noexcept { return len_; }
    constexpr size_type max_size() const noexcept { return npos - 1; } // ??
    constexpr bool empty()         const noexcept { return len_ == 0; }

    // basic_string_ref element access
    constexpr const charT& operator[](size_type pos) const { return ptr_[pos]; }
    CXX14_CONSTEXPR const charT& at(size_type pos) const
    {
        if (pos >= len_)
            _Xoor("basic_string_ref::at");
        return ptr_[pos];
    }
    constexpr const charT& front() const { return ptr_[0]; }
    constexpr const charT& back()  const { return ptr_[len_ - 1]; }
    // NOTE: The return is not guaranteed to be a null-terminated string.
    constexpr const charT* data()  const noexcept { return ptr_; }

    // basic_string_ref modifiers
    CXX14_CONSTEXPR void remove_prefix(size_type n)
    {
        if (n > len_)
            n = len_;
        ptr_ += n;
        len_ -= n;
    }
    CXX14_CONSTEXPR void remove_suffix(size_type n)
    {
        if (n > len_)
            n = len_;
        len_ -= n;
    }
    CXX14_CONSTEXPR void swap(basic_string_ref& s) noexcept { std::swap(*this, s); };

    // basic_string_ref string operations
    template<typename Allocator>
    explicit operator std::basic_string<charT, traits, Allocator>() const
    {
        return std::basic_string<charT, traits, Allocator>(ptr_, len_);
    }
    template <typename Allocator = std::allocator<charT>>
    std::basic_string<charT, traits, Allocator> to_string() const
    {
        return std::basic_string<charT, traits, Allocator>(ptr_, len_);
    }

    size_type copy(charT* s, size_type n, size_type pos = 0) const
    {
        if (pos > size())
            _Xoor("basic_string_ref::copy");
        if (n == npos || pos + n > size())
            n = size() - pos;
        std::copy_n(data() + pos, n, s);
        return n;
    }

    CXX14_CONSTEXPR basic_string_ref substr(size_type pos, size_type n = npos) const
    {
        if (pos > size())
            _Xoor("basic_string_ref::substr");
        if (n == npos || pos + n > size())
            n = size() - pos;
        return basic_string_ref(data() + pos, n);
    }

    // the compare/find functions are not complete mostly because the amount of them is kinda excessive
    CXX14_CONSTEXPR int compare(basic_string_ref x) const noexcept
    {
        const int cmp = traits::compare(ptr_, x.ptr_, (std::min)(len_, x.len_));
        return cmp != 0 ? cmp : (len_ == x.len_ ? 0 : len_ < x.len_ ? -1 : 1);
    }

    CXX14_CONSTEXPR size_type find(basic_string_ref s) const noexcept
    {
        const_iterator iter = std::search(this->cbegin(), this->cend(),
                                          s.cbegin(), s.cend(), traits::eq);
        return iter == this->cend() ? npos : std::distance(this->cbegin(), iter);
    }

    CXX14_CONSTEXPR size_type find(charT c) const noexcept
    {
        const_iterator iter = std::find_if(this->cbegin(), this->cend(),
                                           detail::string_ref_traits_eq<charT, traits>(c));
        return iter == this->cend() ? npos : std::distance(this->cbegin(), iter);
    }

    CXX14_CONSTEXPR size_type rfind(basic_string_ref s) const noexcept
    {
        const_reverse_iterator iter = std::search(this->crbegin(), this->crend(),
                                                  s.crbegin(), s.crend(), traits::eq);
        return iter == this->crend() ? npos : reverse_distance(this->crbegin(), iter);
    }

    CXX14_CONSTEXPR size_type rfind(charT c) const noexcept
    {
        const_reverse_iterator iter = std::find_if(this->crbegin(), this->crend(),
                                                   detail::string_ref_traits_eq<charT, traits>(c));
        return iter == this->crend() ? npos : reverse_distance(this->crbegin(), iter);
    }

    CXX14_CONSTEXPR size_type find_first_of(charT c) const noexcept { return  find(c); }
    CXX14_CONSTEXPR size_type find_last_of(charT c) const noexcept { return rfind(c); }

    CXX14_CONSTEXPR size_type find_first_of(basic_string_ref s) const noexcept
    {
        const_iterator iter = std::find_first_of
            (this->cbegin(), this->cend(), s.cbegin(), s.cend(), traits::eq);
        return iter == this->cend() ? npos : distance(this->cbegin(), iter);
    }

    CXX14_CONSTEXPR size_type find_last_of(basic_string_ref s) const noexcept
    {
        const_reverse_iterator iter = std::find_first_of
            (this->crbegin(), this->crend(), s.cbegin(), s.cend(), traits::eq);
        return iter == this->crend() ? npos : reverse_distance(this->crbegin(), iter);
    }

    CXX14_CONSTEXPR size_type find_first_not_of(basic_string_ref s) const noexcept
    {
        const_iterator iter = find_not_of(this->cbegin(), this->cend(), s);
        return iter == this->cend() ? npos : distance(this->cbegin(), iter);
    }

    CXX14_CONSTEXPR size_type find_first_not_of(charT c) const noexcept
    {
        for (const_iterator iter = this->cbegin(); iter != this->cend(); ++iter)
            if (!traits::eq(c, *iter))
                return distance(this->cbegin(), iter);
        return npos;
    }

    CXX14_CONSTEXPR size_type find_last_not_of(basic_string_ref s) const noexcept
    {
        const_reverse_iterator iter = find_not_of(this->crbegin(), this->crend(), s);
        return iter == this->crend() ? npos : reverse_distance(this->crbegin(), iter);
    }

    CXX14_CONSTEXPR size_type find_last_not_of(charT c) const noexcept
    {
        for (const_reverse_iterator iter = this->crbegin(); iter != this->crend(); ++iter)
            if (!traits::eq(c, *iter))
                return reverse_distance(this->crbegin(), iter);
        return npos;
    }

    // these are not in D4529 but they are very convinient, so...
    constexpr bool starts_with(charT c) const { return !empty() && traits::eq(c, front()); }
    constexpr bool starts_with(basic_string_ref x) const
    {
        return len_ >= x.len_ && traits::compare(ptr_, x.ptr_, x.len_) == 0;
    }

    constexpr bool ends_with(charT c) const { return !empty() && traits::eq(c, back()); }
    constexpr bool ends_with(basic_string_ref x) const
    {
        return len_ >= x.len_ && traits::compare(ptr_ + len_ - x.len_, x.ptr_, x.len_) == 0;
    }

private:
    template <typename r_iter>
    size_type reverse_distance(r_iter first, r_iter last) const
    {
        return len_ - 1 - std::distance(first, last);
    }

    template <typename Iterator>
    Iterator find_not_of(Iterator first, Iterator last, basic_string_ref s) const
    {
        for (; first != last; ++first)
            if (0 == traits::find(s.ptr_, s.len_, *first))
                return first;
        return last;
    }

    NORETURN NOINLINE void _Xoor(const char *const msg) const
    {
        throw std::out_of_range(msg);
    }

    const charT *ptr_;
    size_type len_;
};

template<typename charT, typename traits, typename Allocator = std::allocator<charT> >
inline std::basic_string<charT, traits, Allocator> to_string(basic_string_ref<charT, traits> str)
{
    return std::basic_string<charT, traits, Allocator>(str.data(), str.size());
}

//  Comparison operators

// helper to easily declare comparison operators, expects the main string_ref template to be already declared
// @_op should be a relational operator (!=, <, etc.)
#define DEFINE_OP_CMP(_op) \
template<typename charT, typename traits, typename Allocator> \
inline bool operator _op (basic_string_ref<charT, traits> x, const std::basic_string<charT, traits, Allocator> & y) { \
    return x _op basic_string_ref<charT, traits>(y); \
} \
template<typename charT, typename traits, typename Allocator> \
inline bool operator _op (const std::basic_string<charT, traits, Allocator> & x, basic_string_ref<charT, traits> y) { \
    return basic_string_ref<charT, traits>(x) _op y; \
} \
template<typename charT, typename traits> \
inline bool operator _op (basic_string_ref<charT, traits> x, const charT *const y) { \
    return x _op basic_string_ref<charT, traits>(y); \
} \
template<typename charT, typename traits> \
inline bool operator _op (const charT *const x, basic_string_ref<charT, traits> y) { \
    return basic_string_ref<charT, traits>(x) _op y; \
}

/// Note that equality can be implemented faster than comparison.
template<typename charT, typename traits>
inline bool operator==(basic_string_ref<charT, traits> x, basic_string_ref<charT, traits> y)
{
    typedef typename basic_string_ref<charT, traits>::size_type size_type;
    const size_type len = x.size();
    if (len != y.size())
        return false;
    return x.data() == y.data() || len == 0 || x.compare(y) == 0;
}
DEFINE_OP_CMP(==)

//  Inequality
template<typename charT, typename traits>
inline bool operator!=(basic_string_ref<charT, traits> x, basic_string_ref<charT, traits> y)
{
    return !(x == y);
}
DEFINE_OP_CMP(!=)

//  Less than
template<typename charT, typename traits>
inline bool operator<(basic_string_ref<charT, traits> x, basic_string_ref<charT, traits> y)
{
    return x.compare(y) < 0;
}
DEFINE_OP_CMP(<)

//  Greater than
template<typename charT, typename traits>
inline bool operator>(basic_string_ref<charT, traits> x, basic_string_ref<charT, traits> y)
{
    return x.compare(y) > 0;
}
DEFINE_OP_CMP(>)

//  Less than or equal to
template<typename charT, typename traits>
inline bool operator<=(basic_string_ref<charT, traits> x, basic_string_ref<charT, traits> y)
{
    return x.compare(y) <= 0;
}
DEFINE_OP_CMP(<=)

//  Greater than or equal to
template<typename charT, typename traits>
inline bool operator>=(basic_string_ref<charT, traits> x, basic_string_ref<charT, traits> y)
{
    return x.compare(y) >= 0;
}
DEFINE_OP_CMP(>=)

#undef DEFINE_OP_CMP

// numeric conversions
//
// These are short-term implementataions. But there is not library support and writing full-blown
// standards compliant variants is out of scope.
inline int stoi(wstring_ref str, size_t* idx = 0, int base = 10) {
    return std::stoi(std::wstring(str), idx, base);
}

inline long stol(wstring_ref str, size_t* idx = 0, int base = 10) {
    return std::stol(std::wstring(str), idx, base);
}

inline unsigned long stoul(wstring_ref str, size_t* idx = 0, int base = 10) {
    return std::stoul(std::wstring(str), idx, base);
}

inline long long stoll(wstring_ref str, size_t* idx = 0, int base = 10) {
    return std::stoll(std::wstring(str), idx, base);
}

inline unsigned long long stoull(wstring_ref str, size_t* idx = 0, int base = 10) {
    return std::stoull(std::wstring(str), idx, base);
}

inline float stof(wstring_ref str, size_t* idx = 0) {
    return std::stof(std::wstring(str), idx);
}

inline double stod(wstring_ref str, size_t* idx = 0) {
    return std::stod(std::wstring(str), idx);
}

inline long double stold(wstring_ref str, size_t* idx = 0) {
    return std::stold(std::wstring(str), idx);
}

}

// Hash support for sld::string_ref
namespace std
{

template<typename charT, typename traits>
struct hash<sld::basic_string_ref<charT, traits>> {
    size_t operator()(sld::basic_string_ref<charT, traits> const& str) const {
        return fnv1a::hash(str.data(), str.size() * sizeof(charT));
    }
};

} // namespace std

#endif
