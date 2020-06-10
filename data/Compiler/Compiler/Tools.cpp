#include "Tools.h"

#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>

#include "sld_Platform.h"
#include "CompressByStore.h"
#include "StringFunctions.h"

#include "PugiXML/pugixml.hpp"

#include "Engine/SldUnicode.h"

/// Выбирает реализацию сжатия данных.
ICompressBy* CreateCompressClass(CompressionConfig aConfig, ThreadPool &aThreadPool)
{
	switch (aConfig.type)
	{
		/// Без сжатия, данные просто сохраняются как есть.
		case eCompressionTypeNoCompression:
			return new CCompressByStore;
		default:
			return NULL;
	}
	return NULL;
}

/**********************************************************************
* Обычный memmove, но возращаем aDst + aSize
***********************************************************************/
void* memmoveshift(void* aDst, const void * aSrc, size_t aSize)
{
	return static_cast<char*>(memmove(aDst, aSrc, aSize)) + aSize;
}


/**********************************************************************
* Кодирует dictid из строчки в соответствующее число
*
* @param[in] str - строковый id словаря
* @param[out] un - union, в который записывается получившиеся число
***********************************************************************/
int EncodeDictIdForETB(const std::wstring& aDictId, UIntUnion& id)
{
	if (aDictId.length() != 8)
		return ERROR_WRONG_DICT_ID;

	id.ui_32 = wcstoul(aDictId.c_str(), NULL, 16);
	return ERROR_NO;
}

/** ********************************************************************
 * Конвертирует wstring в TSizeValue
 *
 * @param[in] aValue - конвертируемая переменная
 *
 * @return TSizeValue представление переданного размера
 ************************************************************************/
TSizeValue ParseSizeValue(sld::wstring_ref aValue, const EMetadataUnitType aDefaultUnits)
{
	static const sld::wstring_ref suffixes[] = { L"px", L"pt", L"em", L"mm", L"%" };
	static_assert(sld::array_size(suffixes) == eMetadataUnitType_end,
				  "ParseSizeValue(): the unit data table must be updated.");

	if (aValue.empty())
		return TSizeValue();

	const STString<128> string(aValue);
	const wchar_t *str = string.c_str();

	int sign = 1;
	if (*str == '-')
	{
		sign = -1;
		str++;
	}

	wchar_t *end = nullptr;
	long integer = wcstol(str, &end, 10);
	if (integer == LONG_MAX || (str == end && *end != L'.'))
		return TSizeValue();

	str = end;
	// parse fractional part
	unsigned long fractional = 0;
	if (*str == L'.')
	{
		str++;
		if (!*str)
			return TSizeValue(); // treat "n." (number followed by dot) as malformed

		if (*str == L'-')
			return TSizeValue(); // "n.-"... yeah, well... have to check it as wcstoul "eats" '-'

		fractional = wcstoul(str, &end, 10);
		if (fractional == ULONG_MAX)
			return TSizeValue();

		const size_t length = end - str;
		if (length == 0)
			return TSizeValue(); // treat "n." (number followed by dot) as malformed

		if (length == 1)
			fractional *= 10;

		if (length > 2)
		{
			unsigned long div = 1;
			for (size_t i = 2; i < length; i++)
				div *= 10;
			fractional /= div;
		}

		str = end;
	}

	long value = sign * (integer * 100 + fractional);
	for (int index = 0; index < eMetadataUnitType_end; index++)
	{
		if (str == suffixes[index])
			return TSizeValue(value, (EMetadataUnitType)index);
	}
	return TSizeValue(value, aDefaultUnits);
}

void sld::fmt::format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const MemSize &sz)
{
	size_t size = sz.size;
	auto&& w = f.writer();
	if (size < 1000)
	{
		w.write("{}b", size);
		return;
	}

	const char *suffix;
	if (size < 1000 * 1000)
	{
		suffix = "kb";
	}
	else if (size < 1000 * 1000 * 1000)
	{
		size /= 1000;
		suffix = "mb";
	}
	else
	{
		size /= 1000 * 1000;
		suffix = "gb";
	}
	w.write("{:.03f}{}", size / 1000.0, suffix);
}

void sld::format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const wstring_ref &string)
{
	thread_local std::string str;
	as_utf8(string, str);
	f.format(format_str, fmt::internal::MakeArg<std::decay<decltype(f)>::type>(str));
}

namespace sld2 {
namespace utf {

template <size_type size> struct wchar_selector {};
template <> struct wchar_selector<sizeof(UInt16)> : impl::traits<UInt16> {};
template <> struct wchar_selector<sizeof(UInt32)> : impl::traits<UInt32> {};
template <> struct traits<wchar_t> : wchar_selector<sizeof(wchar_t)> {};
template <> struct traits<char16_t> : impl::traits<UInt16> {};

template <typename CharTo, typename CharFrom>
void convert(const sld::basic_string_ref<CharFrom> &source, std::basic_string<CharTo> &dest)
{
	dest.clear();
	if (source.empty())
		return;

	if (sizeof(CharTo) == sizeof(CharFrom))
	{
		dest.assign(source.begin(), source.end());
		return;
	}

	const size_type len = convert<CharTo>(source.data(), (size_type)source.size(), nullptr);
	if (len == 0)
		return;

	dest.resize(len);
	convert(source.data(), (size_type)source.size(), &dest[0]);
}

template <typename CharTo, typename CharFrom>
std::basic_string<CharTo> convert(const sld::basic_string_ref<CharFrom> &source)
{
	std::basic_string<CharTo> string;
	convert(source, string);
	return string;
}

} // namespace utf
} // namespace sld2

void sld::as_utf8(const wstring_ref &wstr, std::string &str)
{
	sld2::utf::convert(wstr, str);
}

std::string sld::as_utf8(const sld::wstring_ref &str)
{
	return sld2::utf::convert<char>(str);
}

void sld::as_wide(const sld::string_ref &str, std::wstring &wstr)
{
	sld2::utf::convert(str, wstr);
}

std::wstring sld::as_wide(const sld::string_ref &str)
{
	return sld2::utf::convert<wchar_t>(str);
}

std::wstring sld::as_wide(const sld::u16string_ref &str)
{
	return sld2::utf::convert<wchar_t>(str);
}

void sld::as_utf16(const sld::wstring_ref &str, std::u16string &u16str)
{
	sld2::utf::convert(str, u16str);
}

std::u16string sld::as_utf16(const sld::wstring_ref &str)
{
	return sld2::utf::convert<char16_t>(str);
}

uint32_t sld::as_utf16_len(const sld::wstring_ref &str)
{
	uint32_t length = static_cast<uint32_t>(str.size());
	if (sizeof(wchar_t) == sizeof(char16_t))
		return length;
	return sld2::utf::convert<char16_t>(str.data(), length, nullptr);
}

/**
 * Reads a file into a vector
 * On any error returns an empty vector
 */
MemoryBuffer sld::read_file(const std::wstring &path)
{
	FILE *fp = fopen(path, L"rb");
	if (!fp)
		return MemoryBuffer();

	fseek(fp, 0, SEEK_END);
	const auto length = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	MemoryBuffer buf(length);
	const size_t count = fread(buf.data(), sizeof(uint8_t), length, fp);

	fclose(fp);

	if (count != static_cast<size_t>(length))
		return MemoryBuffer();

	return buf;
}

// text file reading helpers
namespace {

// file encoding guesser; based off pugixml
enum file_encoding {
	encoding_utf32_be,
	encoding_utf32_le,
	encoding_utf16_be,
	encoding_utf16_le,
	encoding_utf8
};
static file_encoding guess_file_encoding(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
	// look for BOM in first few bytes
	if (d0 == 0 && d1 == 0 && d2 == 0xfe && d3 == 0xff) return encoding_utf32_be;
	if (d0 == 0xff && d1 == 0xfe && d2 == 0 && d3 == 0) return encoding_utf32_le;
	if (d0 == 0xfe && d1 == 0xff) return encoding_utf16_be;
	if (d0 == 0xff && d1 == 0xfe) return encoding_utf16_le;
	if (d0 == 0xef && d1 == 0xbb && d2 == 0xbf) return encoding_utf8;

	// no known BOM detected, assume utf8
	return encoding_utf8;
}

// creates a source string in the required code unit "representation" without
// the bom if present
// on error returns an empty string reference
template <typename CharT>
static sld::basic_string_ref<CharT> make_source_string(const MemoryBuffer &data)
{
	if (data.empty())
		return sld::basic_string_ref<CharT>();

	if (data.size() % sizeof(CharT) != 0)
		return sld::basic_string_ref<CharT>();

	sld::basic_string_ref<CharT> string((const CharT*)data.data(), data.size() / sizeof(CharT));

	/* XXX: this is rather awkward... but we have to remove the bom somehow...
	 *
	 * For now rely on the fact that only utf8 can be without bom so we have to check
	 * it for it's presence; utf16 and utf32 always have a bom (at least by our logic)
	 * so we can simply skip it's [bom] size in code units
	 *
	 * Ideally we'd have a function inside sld2::utf that returns a single code_point
	 * and it's length in code units, so we could simply do
	 *   size_type bom_len;
	 *   if (decode(string.data(), string.size(), &bom_len) == 0xfeff)
	 *       string.remove_prefix(bom_len);
	 */
	if (sizeof(CharT) == 1 && string.size() >= 3 && string[0] == 0xef && string[1] == 0xbb && string[2] == 0xbf)
		string.remove_prefix(3);
	else if (sizeof(CharT) == 2)
		string.remove_prefix(2);
	else if (sizeof(CharT) == 4)
		string.remove_prefix(1);

	return string;
}

// converts text file data from the source encoding @CharT to wchar_t
template <typename CharT>
static std::wstring convert_text(const MemoryBuffer &data)
{
	using namespace sld2::utf;

	const sld::basic_string_ref<CharT> source = make_source_string<CharT>(data);
	if (source.empty())
		return std::wstring();

	// get the total length of the converted string
	const size_type len = convert<wchar_t>(source.data(), (size_type)source.size(), nullptr);
	if (len == 0)
		return std::wstring();

	// preallocate the string
	std::wstring string;
	string.resize(len);

	// custom writer that converts '\r\n' into single '\n' during the conversion
	struct Writer
	{
		traits<wchar_t>::writer writer;
		code_point prev;

		Writer(wchar_t *data) : writer((traits<wchar_t>::char_type*)data), prev('\0') {}

		size_type operator()(code_point cp)
		{
			if (prev == '\r' && cp == '\n')
				writer.data--;
			prev = cp;
			return writer(cp);
		}
	} writer((wchar_t*)string.data());

	decode(source.data(), (size_type)source.size(), writer);

	// as we can get a shorter string (because of the replaces) we have to update it's size
	// after the actual recoding
	string.resize((const wchar_t*)writer.writer.data - string.data());

	return string;
}

} // anon namespace

/**
 * Reads a *text* file into a wide string
 * On any error returns an empty string
 */
std::wstring sld::read_text_file(const std::wstring &path)
{
	MemoryBuffer data = read_file(path);
	if (data.empty())
		return std::wstring();

	file_encoding encoding = data.size() < 4 ? encoding_utf8 :
		guess_file_encoding(data[0], data[1], data[2], data[3]);

	// XXX: this currently works only on le *architectures* as we lack byteswap support
	switch (encoding)
	{
	case encoding_utf32_be:
	case encoding_utf16_be:
		return std::wstring();

	case encoding_utf32_le: return convert_text<UInt32>(data);
	case encoding_utf16_le: return convert_text<UInt16>(data);
	case encoding_utf8:     return convert_text<UInt8>(data);

	default:
		break;
	}

	return std::wstring();
}

sld::StringRefStore::~StringRefStore()
{
	for (sld::wstring_ref str : strings_)
	{
		// we can safely do it here as all of the refs were allocated by ourselves
		if (str.data())
			free(const_cast<wchar_t*>(str.data()));
	}
}

sld::wstring_ref sld::StringRefStore::strdup(wstring_ref str)
{
	const size_t len = str.size();
	if (len == 0)
	{
		str = wstring_ref();
	}
	else
	{
		wchar_t *string = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
		wcsncpy(string, str.data(), len);
		string[len] = 0;
		str = wstring_ref(string, len);
	}
	strings_.emplace_back(str);
	return str;
}

// adds a string returning an index to it
uint32_t sld::StringRefMap::insert(wstring_ref str)
{
	const auto it = mapIndex.find(str);
	if (it != mapIndex.end())
		return it->second;

	const uint32_t idx = static_cast<uint32_t>(size());
	str = strdup(str);
	mapIndex.emplace(str, idx);

	return idx;
}

// return an index of the given string (~0u if not found)
uint32_t sld::StringRefMap::get(wstring_ref str) const
{
	const auto it = mapIndex.find(str);
	return it != mapIndex.end() ? it->second : ~0u;
}
