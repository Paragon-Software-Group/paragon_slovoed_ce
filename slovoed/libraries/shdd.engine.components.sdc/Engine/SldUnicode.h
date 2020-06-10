#pragma once
#ifndef _SLD_UNICODE_H_
#define _SLD_UNICODE_H_

#include "SldPlatform.h" // UInt[8|16|32]

/**
 * Templates for various utf conversions
 *
 * These can be used to implement "single pass" string transformations while decoding
 * and(or) writing utf encoded text.
 *
 * As a general rule the *en*coding functions do not check if the code points are inside
 * the current (as of 26.11.2015) range of 0..U+10FFFF
 */
namespace sld2 {

namespace utf {

typedef UInt32 size_type;
typedef UInt32 code_point;

namespace counter {
// counters - return the number of code units required to represent the code point

static inline size_type utf8(code_point cp)
{
	return
		// ASCII range
		cp < 0x80    ? 1 :
		// U+0080..U+07FF
		cp < 0x800   ? 2 :
		// U+0800..U+FFFF
		cp < 0x10000 ? 3 :
		// U+10000..U+10FFFF
		               4;
}

static inline size_type utf16(code_point cp)
{
	return cp < 0x10000 ? 1 : 2;
}

static inline size_type utf32(code_point)
{
	return 1;
}

} // namespace counters

namespace detail {

// base class for utf counters
struct counter_base {
	counter_base() : count(0) {}
	size_type result() const { return count; }
protected:
	size_type count;
};

// base class for utf writers
template <typename Char>
struct writer_base {
	typedef Char char_type;
	char_type *data;
	writer_base(char_type *data) : data(data) {}
};

} // namespace detail

/**
 * UTF counters
 *  functor classes that can be used to count the number of needed code units
 *  to represent a stream of code points
 *
 * the final number of needed code units can be queried through the .result() method
 */
struct utf8_counter : public detail::counter_base {
	void operator()(code_point cp) { count += counter::utf8(cp); }
};

struct utf16_counter : public detail::counter_base {
	void operator()(code_point cp) { count += counter::utf16(cp); }
};

struct utf32_counter : public detail::counter_base {
	void operator()(code_point cp) { count += counter::utf32(cp); }
};

/**
 * UTF writers
 *  functor classes that can be used to write a stream (or a single) code point to
 *  the buffer in the given utf encoding
 *  they have a single field @data - pointer to the next after the last code unit in
 *  the output buffer
 *
 * must be initialized with a buffer with enough space to hold the resulting stream
 * *DO NOT* do any checks regarding the available size on their own
 * as a small help to dealing with this operator() returns the count of code units
 * written to the output buffer for the given code point
 */
struct utf8_writer : public detail::writer_base<UInt8> {
	utf8_writer(char_type *data) : writer_base(data) {}
	size_type operator()(code_point cp)
	{
		// ASCII range
		if (cp < 0x80)
		{
			*data++ = (UInt8)(cp);
			return 1;
		}
		// U+0080..U+07FF
		else if (cp < 0x800)
		{
			*data++ = (UInt8)(0xC0 | (cp >> 6));
			*data++ = (UInt8)(0x80 | (cp & 0x3F));
			return 2;
		}
		// U+0800..U+FFFF
		else if (cp < 0x10000)
		{
			*data++ = (UInt8)(0xE0 | (cp >> 12));
			*data++ = (UInt8)(0x80 | ((cp >> 6) & 0x3F));
			*data++ = (UInt8)(0x80 | (cp & 0x3F));
			return 3;
		}
		// U+10000..U+10FFFF
		else
		{
			*data++ = (UInt8)(0xF0 | (cp >> 18));
			*data++ = (UInt8)(0x80 | ((cp >> 12) & 0x3F));
			*data++ = (UInt8)(0x80 | ((cp >> 6) & 0x3F));
			*data++ = (UInt8)(0x80 | (cp & 0x3F));
			return 4;
		}
	}
};

struct utf16_writer : public detail::writer_base<UInt16> {
	utf16_writer(char_type *data) : writer_base(data) {}
	size_type operator()(code_point cp)
	{
		// 0..U+FFFF
		if (cp < 0x10000)
		{
			*data++ = (UInt16)(cp);
			return 1;
		}
		// U+10000..U+10FFFF
		else
		{
			*data++ = (UInt16)(0xD800 + ((UInt32)(cp - 0x10000) >> 10));
			*data++ = (UInt16)(0xDC00 + ((UInt32)(cp - 0x10000) & 0x3FF));
			return 2;
		}
	}
};

struct utf32_writer : public detail::writer_base<UInt32> {
	utf32_writer(char_type *data) : writer_base(data) {}
	size_type operator()(code_point cp)
	{
		*data++ = cp;
		return 1;
	}
};

/**
 * UTF decoders
 *  functions return the number of code units read from the *source* buffer
 *  that means the return can be used to implement some form of utf validation
 *
 * @write must simply be a callable
 *
 * XXX: Should we instead make the functions skip invalid utf? Maybe replace invalid
 * utf by some "placeholder" symbols? We can do it through a template param I guess.
 */

// utf-8 decoder
template <typename Writer>
inline size_type decode(const UInt8 *data, size_type size, Writer &write)
{
	const size_type size_ = size;
	while (size)
	{
		size_type count = 0;
		const UInt32 chr = *data;
		// 0xxxxxxx -> U+0000..U+007F
		if (chr < 0x80)
		{
			write(chr);
			count = 1;
		}
		// 110xxxxx -> U+0080..U+07FF
		else if (chr - 0xC0 < 0x20 && size >= 2 && (data[1] & 0xC0) == 0x80)
		{
			write(((chr & ~0xC0) << 6) | (data[1] & 0x3F));
			count = 2;
		}
		// 1110xxxx -> U+0800-U+FFFF
		else if (chr - 0xE0 < 0x10 && size >= 3 && (data[1] & 0xC0) == 0x80 && (data[2] & 0xC0) == 0x80)
		{
			write(((chr & ~0xE0) << 12) | ((data[1] & 0x3F) << 6) | (data[2] & 0x3F));
			count = 3;
		}
		// 11110xxx -> U+10000..U+10FFFF
		else if (chr - 0xF0 < 0x08 && size >= 4 && (data[1] & 0xC0) == 0x80 && (data[2] & 0xC0) == 0x80 && (data[3] & 0xC0) == 0x80)
		{
			write(((chr & ~0xF0) << 18) | ((data[1] & 0x3F) << 12) | ((data[2] & 0x3F) << 6) | (data[3] & 0x3F));
			count = 4;
		}
		// 10xxxxxx or 11111xxx -> invalid
		else
			break;
		data += count;
		size -= count;
	}
	return size_ - size;
}

// utf-16 decoder
template <typename Writer>
inline size_type decode(const UInt16 *data, size_type size, Writer &write)
{
	const size_type size_ = size;
	while (size)
	{
		size_type count = 0;
		const UInt32 chr = *data;
		// U+0000..U+D7FF & U+E000..U+FFFF
		if ((chr < 0xD800) || ((UInt32)(chr - 0xE000) < 0x2000))
		{
			write(chr);
			count = 1;
		}
		// surrogate pair lead
		else if ((UInt32)(chr - 0xD800) < 0x400 &&
				 size >= 2 && (UInt32)(data[1] - 0xDC00) < 0x400)
		{
			write(0x10000 + ((chr & 0x3ff) << 10) + (data[1] & 0x3ff));
			count = 2;
		}
		else
			break;
		data += count;
		size -= count;
	}
	return size_ - size;
}

// utf-32 decoder
template <typename Writer>
inline size_type decode(const UInt32* data, size_type size, Writer &write)
{
	for (size_type n = 0; n < size; n++)
		write(data[n]);
	return size;
}

/**
 * UTF traits
 *  traits used to implement generic encoding/decoding algorithms
 *
 *  @char_type - the "internal" type used to represent a code unit in the given encoding,
 *               must be one of UInt8, UInt16, UInt32 for utf8, utf16 and utf32
 *  @counter   - functor type used to count the code units
 *  @writer    - functor type used to write out the code units
 */

// implementation
namespace impl {
template <typename T> struct traits {};
template <> struct traits<UInt8> {
	typedef UInt8 char_type;
	typedef utf8_counter counter;
	typedef utf8_writer writer;
};
template <> struct traits<UInt16> {
	typedef UInt16 char_type;
	typedef utf16_counter counter;
	typedef utf16_writer writer;
};
template <> struct traits<UInt32> {
	typedef UInt32 char_type;
	typedef utf32_counter counter;
	typedef utf32_writer writer;
};
}

// default traits
template <typename T> struct traits : impl::traits<T> {};

// explicit specialization for chars - treated as utf8
template <> struct traits<char> : traits<UInt8> {};

/**
 * As we can't really use wchar_t inside the engine (for now?) we can't supply a
 * wchar_t traits specialization. Fortunately defining it is quite easy:
 *   @code
 *   namespace sld2 { namespace utf {
 *   template <size_type size> struct wchar_selector {};
 *   template <> struct wchar_selector<sizeof(UInt16)> : impl::traits<UInt16> {};
 *   template <> struct wchar_selector<sizeof(UInt32)> : impl::traits<UInt32> {};
 *   template <> struct traits<wchar_t> : wchar_selector<sizeof(wchar_t)> {};
 *   } }
 *   @endcode
 */

/**
 * Convert a string from one utf encoding to the other
 *
 * @param[in]  source - source string to be converted
 * @param[in]  length - the length (in code units) of the source string
 * @param[out] dest   - the pointer to the buffer where to write the converted string
 *                      may be NULL
 *
 * Note: the function does not write nor count the nul terminator
 * Note: if you care about conversion errors you have to handle 0 source length case
 *       on your own as it returns 0 in this case too
 *
 * @return the count of code units in the converted string
 *         on conversion error (malformed input) - 0
 */
template <typename CharTo, typename CharFrom>
inline size_type convert(const CharFrom *source, size_type length, CharTo *dest)
{
	if (!source || length == 0)
		return 0;

	// get "real" character type of the from string
	typedef typename traits<CharFrom>::char_type char_from_type;

	if (dest)
	{
		typedef typename traits<CharTo>::char_type char_to_type;

		typename traits<CharTo>::writer writer((char_to_type*)dest);
		const size_type decode_len = decode((const char_from_type*)source, length, writer);
		if (decode_len != length)
			return 0;

		return (size_type)(writer.data - (char_to_type*)dest);
	}
	else
	{
		typename traits<CharTo>::counter counter;
		const size_type decode_len = decode((const char_from_type*)source, length, counter);
		if (decode_len != length)
			return 0;

		return counter.result();
	}
}

} // namespace utf
} // namespace sld2

#endif // _SLD_UNICODE_H_
