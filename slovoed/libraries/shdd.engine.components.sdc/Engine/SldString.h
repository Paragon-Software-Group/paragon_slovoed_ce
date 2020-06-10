#ifndef _SLD_STRING_H_
#define _SLD_STRING_H_

#include "SldStringReference.h"

/// собственная реализация std::string
/// Максимальный размер символа 4 байта
template <typename Char, typename traits = sld2::char_traits<Char>>
class CSldString
{
public:
	// typical std container boilerplate
	typedef typename traits::value_type value_type;
	typedef typename traits::pointer pointer;
	typedef typename traits::const_pointer const_pointer;
	typedef typename traits::reference reference;
	typedef typename traits::const_reference const_reference;
	typedef typename traits::size_type size_type;
	static SLD_CONSTEXPR_OR_CONST size_type npos = size_type(-1);

	/// стандартный конуструктор
	CSldString() : m_String(nullptr), m_Size(0), m_Capacity(0) {}

	/// конуструктор, принимающий строку
	CSldString(const value_type* aString) : CSldString()
	{
		assign(aString);
	}

	/// конуструктор, принимающий не null-terminated строку
	CSldString(const value_type* aString, size_type aLength) : CSldString()
	{
		assign(aString, aLength);
	}

  CSldString(size_type aCount, const value_type& aChar) : CSldString()
  {
    assign(aCount, aChar);
  }

	/// конструктор копирования
	CSldString(const CSldString& aString) : CSldString()
	{
		assign(aString);
	}

	/// move конструктор
	CSldString(CSldString&& aString)
		: m_String(aString.m_String), m_Size(aString.m_Size), m_Capacity(aString.m_Capacity)
	{
		// clear the moved object
		aString.m_String = NULL;
		aString.m_Size = aString.m_Capacity = 0;
	}

	/// конструктор из string reference
	explicit CSldString(sld2::BasicStringRef<Char, traits> aString) : CSldString()
	{
		assign(aString);
	}

	/// деструктор
	~CSldString()
	{
		if (m_String)
			sldMemFree(m_String);
	}

	/// оператор присваивания
	CSldString& operator=(const value_type* aString)
	{
		return assign(aString);
	}

	/// оператор присваивания
	CSldString& operator=(sld2::BasicStringRef<Char, traits> aString)
	{
		return assign(aString.data(), aString.length());
	}

	/// оператор присваивания
	CSldString& operator=(const CSldString& aString)
	{
		return assign(aString);
	}

	/// move оператор присваивания
	CSldString& operator=(CSldString&& aString)
	{
		return assign(sld2::move(aString));
	}

	/// оператор добавления в конец строки
	CSldString& operator+=(const CSldString& aString)
	{
		return append(aString);
	}

	CSldString& operator+=(const value_type* aString)
	{
		return append(aString);
	}

  CSldString& operator+=(const value_type& aChar)
  {
    return append((size_type)1, aChar);
  }

	CSldString& operator+=(sld2::BasicStringRef<Char, traits> aString)
	{
		return append(aString);
	}

	/// очистка строчки (не очищает выделенную память)
	void clear()
	{
		m_Size = 0;
	}

	/// добавляет новый символ в конец строки
	void push_back(const value_type aNewChar)
	{
		MemGrow(1);
		m_String[m_Size] = aNewChar;
		m_Size++;
		m_String[m_Size] = 0;
	}

	/// удаляет последний символ из строки
	void pop_back()
	{
		m_String[--m_Size] = 0;
	}

	/// добавление в конец строки
	CSldString& append(sld2::BasicStringRef<Char, traits> aString)
	{
		return append(aString.data(), aString.size());
	}
	CSldString& append(const value_type* aString, size_type aCount)
	{
		if (aCount)
		{
			MemGrow(aCount);
			sld2::memmove_n(m_String + m_Size, aString, aCount);
			m_Size += aCount;
			m_String[m_Size] = 0;
		}
		return *this;
	}
  CSldString& append(size_type aCount, const value_type& aChar)
  {
    if (aCount)
    {
      MemGrow(aCount);
      sld2::uninitialized_fill_n(m_String + m_Size, aChar, aCount);
      m_Size += aCount;
      m_String[m_Size] = 0;
    }
    return *this;
  }

	/// присваивание значения
	CSldString& assign(const CSldString& aString)
	{
		return assign(aString.c_str(), aString.size());
	}
	CSldString& assign(sld2::BasicStringRef<Char, traits> aString)
	{
		return assign(aString.data(), aString.size());
	}
	CSldString& assign(const value_type* aString)
	{
		return assign(aString, traits::length(aString));
	}
	CSldString& assign(const value_type* aString, size_type aCount)
	{
		if (aCount)
		{
			reserve(aCount);
			sld2::memmove_n(m_String, aString, aCount);
		}
		m_Size = aCount;
		if (m_String)
			m_String[m_Size] = 0;
		return *this;
	}
  CSldString& assign(size_type aCount, const value_type& aChar)
  {
    if (aCount)
    {
      reserve(aCount);
      sld2::uninitialized_fill_n(m_String, aChar, aCount);
    }
    m_Size = aCount;
    if (m_String)
      m_String[m_Size] = 0;
    return *this;
  }
	CSldString& assign(CSldString&& aString)
	{
		if (m_String)
			sldMemFree(m_String);

		m_String = aString.m_String;
		m_Size = aString.m_Size;
		m_Capacity = aString.m_Capacity;
		// clear the moved object
		aString.m_String = NULL;
		aString.m_Size = aString.m_Capacity = 0;

		return *this;
	}

	/// удостоверивается что памяти выделено достаточно для aSize символов
	void reserve(size_type aSize)
	{
		if (aSize >= m_Capacity)
			reallocate(aSize);
	}

	/// изменяет размер строки, новые символы (если размер больше текущего) инициализируюся в '\0'
	void resize(size_type aSize)
	{
		if (aSize < m_Size)
		{
			m_String[aSize] = 0;
		}
		else if (aSize > m_Size)
		{
			reserve(aSize);
			sldMemZero(m_String + m_Size, (m_Capacity - m_Size) * sizeof(value_type));
		}
		m_Size = aSize;
	}

	/// возвращает подстроку
	CSldString substr(size_type aPos, size_type aCount = npos) const
	{
		if (aPos > size())
			return CSldString();
		if (aCount == npos || aPos + aCount > size())
			aCount = size() - aPos;
		return CSldString(c_str() + aPos, aCount);
	}

	/// возвращает константный указатель на строчку
	const value_type* c_str() const
	{
		return m_Size ? m_String : (value_type*)&m_Size;
	}

	/// возвращает указатель на строчку
	value_type* data()
	{
		return m_String;
	}

  const value_type* data() const
  {
    return m_String;
  }

	/// доступ к элементам строки
	value_type& operator[](size_type aPos)             { return m_String[aPos]; }
	const value_type& operator[](size_type aPos) const { return m_String[aPos]; }
	value_type& front()                                { return m_String[0]; }
	const value_type& front() const                    { return m_String[0]; }
	value_type& back()                                 { return m_String[m_Size - 1]; }
	const value_type& back()  const                    { return m_String[m_Size - 1]; }

	/// сравнение строк
	Int32 compare(sld2::BasicStringRef<Char, traits> aStr) const
	{
		return compare(0, size(), aStr.data(), aStr.size());
	}
	Int32 compare(size_type aPos, size_type aLen, const value_type *aStr, size_type aCount) const
	{
		return compare(aPos, aLen, sld2::BasicStringRef<Char, traits>(aStr, aCount));
	}
	Int32 compare(size_type aPos, size_type aLen, sld2::BasicStringRef<Char, traits> aStr) const
	{
		if (size() < aPos)
			return (Int32)npos; // the original throws here... we can't really
		return sld2::BasicStringRef<Char, traits>(*this).substr(aPos, aLen).compare(aStr);
	}

	/// возвращает длинну строки
	inline size_type size() const
	{
		return m_Size;
	}

	/// возвращает длинну строки
	inline size_type length() const
	{
		return m_Size;
	}

	/// проверяет, пустая ли строка
	bool empty() const
	{
		return m_Size == 0;
	}
	/// возвращает 4-х байтовый хэш строчки
	UInt32 hash() const
	{
		return sld2::MurmurHash3(m_String, m_Size * sizeof(value_type), 0x736c6432);
	}

	CSldString & insert(const size_type aPos, const value_type aChar)
	{
		MemGrow(1);
		const size_type realPos = aPos > size() ? size() : aPos;
		sld2::memmove_n(m_String + realPos + 1, m_String + realPos, size() - realPos + 1);
		m_String[realPos] = aChar;
		m_Size++;
		return *this;
	}

	CSldString & insert(const size_type aPos, sld2::BasicStringRef<Char, traits> aStr)
	{
		if (aStr.size())
		{
			MemGrow(aStr.size());
			const size_type realPos = aPos > size() ? size() : aPos;
			sld2::memmove_n(m_String + realPos + aStr.size(), m_String + realPos, size() - realPos + 1);
			sld2::memmove_n(m_String + realPos, aStr.data(), aStr.size());
			m_Size += aStr.size();
		}
		return *this;
	}

	CSldString & erase(size_type pos = 0, size_type len = npos)
	{
		if (pos > m_Size)
			return *this;

		if (len == npos || pos + len > m_Size)
			len = m_Size - pos;

		sld2::memmove_n(m_String + pos, m_String + pos + len, m_Size - (pos + len) + 1);
		m_Size -= len;

		return *this;
	}

  size_type find(const CSldString& aStr, size_type aPos = 0) const
  {
    return find(aStr.m_String, aPos, aStr.size());
  }

  size_type find(sld2::BasicStringRef<Char, traits> aStr, size_type aPos = 0) const
  {
    return find(aStr.data(), aPos, aStr.size());
  }

  size_type find(const value_type* aStr, size_type aPos = 0) const
  {
    return find(aStr, aPos, traits::length(aStr));
  }

  size_type find(const value_type* aStr, size_type aPos, size_type aCount) const
  {
    if (aCount == 0 && aPos <= m_Size)
      return aPos;

    size_type remaining = m_Size - aPos;
    if (aPos < m_Size && aCount <= remaining)
    {
      const value_type* match = nullptr;
      const value_type* suffix = nullptr;
      for (remaining -= aCount - 1, suffix = m_String + aPos; 
           (match = traits::find(suffix, remaining, *aStr)) != 0;
           remaining -= static_cast<size_type>(match - suffix + 1), suffix = match + 1)
      {
        if (traits::compare(match, aStr, aCount) == 0)
          return static_cast<size_type>(match - m_String);
      }
    }

    return npos;
  }

  size_type find(value_type aChar, size_type aPos = 0) const
  {
    return find((const value_type*)&aChar, aPos, 1);
  }

	operator sld2::BasicStringRef<Char, traits>() const
	{
		return sld2::BasicStringRef<Char, traits>(m_String, m_Size);
	}

private:

	/// расширяет текущую память, если это необходимо
	void MemGrow(size_type aAmount)
	{
		const size_type size = m_Size + aAmount;
		if (size < m_Capacity)
			return;
		reallocate((size >> 3) + (size < 9 ? 3 : 6) + size);
	}

	// перевыделяет память под строку с нужным размером
	void reallocate(size_type aSize)
	{
		m_Capacity = aSize + 1;
		m_String = sldMemReallocT(m_String, m_Capacity);
	}

	/// строка
	value_type *		m_String;

	/// длинна строки (исключая nul-терминатор)
	size_type			m_Size;

	/// размер доступной памяти (включая nul-терминатор)
	size_type			m_Capacity;
};

template <typename Char, typename traits>
inline CSldString<Char, traits> operator+(const CSldString<Char, traits>& lhs, const CSldString<Char, traits>& rhs)
{
  CSldString<Char, traits> sum;
  sum.reserve(lhs.size() + rhs.size());
  sum += lhs;
  sum += rhs;
  return sum;
}

template <typename Char, typename traits>
inline CSldString<Char, traits> operator+(const Char* lhs, const CSldString<Char, traits>& rhs)
{
  CSldString<Char, traits> sum;
  sum.reserve(traits::length(lhs) + rhs.size());
  sum += lhs;
  sum += rhs;
  return sum;
}

template <typename Char, typename traits>
inline CSldString<Char, traits> operator+(const Char lhs, const CSldString<Char, traits>& rhs)
{
  CSldString<Char, traits> sum;
  sum.reserve(1 + rhs.size());
  sum += lhs;
  sum += rhs;
  return sum;
}

template <typename Char, typename traits>
inline CSldString<Char, traits> operator+(const CSldString<Char, traits>& lhs, const Char* rhs)
{
  CSldString<Char, traits> sum;
  sum.reserve(lhs.size() + traits::length(rhs));
  sum += lhs;
  sum += rhs;
  return sum;
}

template <typename Char, typename traits>
inline CSldString<Char, traits> operator+(const CSldString<Char, traits>& lhs, const Char rhs)
{
  CSldString<Char, traits> sum;
  sum.reserve(lhs.size() + 1);
  sum += lhs;
  sum += rhs;
  return sum;
}

namespace sld2 {

template <typename Char>
typename CSldString<Char>::size_type sorted_insert(CSldString<Char> &aString, const Char aChar)
{
	typename CSldString<Char>::size_type resultPos;
	if (!sld2::binary_search(aString.data(), aString.size(), aChar, resultPos))
		aString.insert(resultPos, aChar);
	return resultPos;
}

template<typename Char, typename traits = char_traits<Char>>
inline CSldString<Char, traits> to_string(sld2::BasicStringRef<Char, traits> str)
{
	return CSldString<Char, traits>(str.data(), str.size());
}

} // namespace sld2

#define DEFINE_OP_CMP(_op) \
template <typename Char, typename traits> \
inline bool operator _op (const CSldString<Char, traits> &lhs, const CSldString<Char, traits> &rhs) { \
	return sld2::BasicStringRef<Char, traits>(lhs) _op sld2::BasicStringRef<Char, traits>(rhs); \
} \
template <typename Char, typename traits> \
inline bool operator _op (const CSldString<Char, traits> &lhs, const Char *rhs) { \
	return sld2::BasicStringRef<Char, traits>(lhs) _op sld2::BasicStringRef<Char, traits>(rhs); \
} \
template <typename Char, typename traits> \
inline bool operator _op (const Char *rhs, const CSldString<Char, traits> &lhs) { \
	return sld2::BasicStringRef<Char, traits>(lhs) _op sld2::BasicStringRef<Char, traits>(rhs); \
} \
template<typename Char, typename traits> \
inline bool operator _op (sld2::BasicStringRef<Char, traits> lhs, const CSldString<Char, traits> &rhs) { \
	return lhs _op sld2::BasicStringRef<Char, traits>(rhs); \
} \
template<typename Char, typename traits> \
inline bool operator _op (const CSldString<Char, traits> &lhs, sld2::BasicStringRef<Char, traits> rhs) { \
	return sld2::BasicStringRef<Char, traits>(lhs) _op rhs; \
}

DEFINE_OP_CMP(==)
DEFINE_OP_CMP(!=)
DEFINE_OP_CMP(<)
DEFINE_OP_CMP(>)
DEFINE_OP_CMP(<=)
DEFINE_OP_CMP(>=)

#undef DEFINE_OP_CMP

/// двухбайтовая строчка
typedef CSldString<UInt16> SldU16String;

#endif
