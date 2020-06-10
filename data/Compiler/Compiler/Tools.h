#pragma once

#include <chrono>
#include <unordered_map>

#include "ICompressBy.h"
#include "format.h"
#include "sld_Types.h"
#include "string_ref.h"
#include "util.h"

class ThreadPool;

/// Выбирает реализацию сжатия данных.
ICompressBy* CreateCompressClass(CompressionConfig aConfig, ThreadPool &aThreadPool);

#if defined(__GNUC__) || defined(__clang__)

/// Возвращает количество бит необходимых для представления числа
static inline UInt32 GetCodeLen(UInt64 aCode)
{
	return aCode ? (64 - __builtin_clzll(aCode)) : 0;
}

#elif defined(_MSC_VER) && _WIN64

#include <intrin.h>
#pragma intrinsic(_BitScanReverse64)

/// Возвращает количество бит необходимых для представления числа
static inline UInt32 GetCodeLen(UInt64 aCode)
{
	unsigned long index = 0;
	return _BitScanReverse64(&index, aCode) ? index + 1 : 0;
}

#else

/// Возвращает количество бит необходимых для представления числа
static inline UInt32 GetCodeLen(UInt64 aCode)
{
	UInt32 count = 0;
	while (aCode)
	{
		aCode >>= 1;
		count++;
	}
	return count;
}

#endif

/// Производит memmove и возвращает указатель на память после вставленной
void* memmoveshift(void * aDst, const void * aSrc, size_t aSize);

/// Кодирует dictid из строчки в соответствующее число
int EncodeDictIdForETB(const std::wstring& str, UIntUnion& un);

/// Конвертирует wstring в TSizeValue
TSizeValue ParseSizeValue(sld::wstring_ref aValue, const EMetadataUnitType aDefaultUnits = eMetadataUnitType_UNKNOWN);

// string builder keeping the string inside itslef on a stack
namespace sld {
namespace detail {

template <typename Char, size_t Size>
class BasicStringBuilder
{
public:
	BasicStringBuilder() : w(data_) { data_[0] = '\0'; }

	BasicStringBuilder(sld::basic_string_ref<Char> str) : BasicStringBuilder() { append(str); }

	template <typename... Args>
	BasicStringBuilder(fmt::BasicCStringRef<Char> aFormat, const Args&... aArgs) : BasicStringBuilder() {
		appendf(aFormat, aArgs...);
	}

	// std string like interface
	bool empty() const { return w.size() == 0; }

	size_t size() const { return w.size(); }
	size_t length() const { return size(); }

	BasicStringBuilder& clear() {
		w.clear();
		return *this;
	}

	BasicStringBuilder& append(sld::basic_string_ref<Char> str) {
		w << fmt::BasicStringRef<Char>(str.data(), str.size());
		return *this;
	}

	BasicStringBuilder& append(Char v) {
		w << v;
		return *this;
	}

	BasicStringBuilder& appendf(fmt::BasicCStringRef<Char> aFormat, fmt::ArgList aArgs) {
		fmt::printf(w, aFormat, aArgs);
		return *this;
	}

	template <typename... Args>
	BasicStringBuilder& appendf(fmt::BasicCStringRef<Char> aFormat, const Args& ...args) {
		typedef fmt::internal::ArgArray<sizeof...(Args)> ArgArray;
		typename ArgArray::Type array{ ArgArray::template make<fmt::BasicFormatter<Char>>(args)... };
		return appendf(aFormat, fmt::ArgList(fmt::internal::make_type(args...), array));
	}

	const Char* c_str() const { return w.c_str(); }
	std::basic_string<Char> str() const { return w.str(); }
	sld::basic_string_ref<Char> ref() const { return sld::basic_string_ref<Char>(c_str(), size()); }

	operator const Char* () const { return c_str(); }
	operator std::basic_string<Char>() const { return str(); }
	operator sld::basic_string_ref<Char>() const { return ref(); }

private:
	Char data_[Size];
	fmt::BasicArrayWriter<Char> w;
};

template <typename Char, size_t Size>
inline void format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const BasicStringBuilder<Char, Size> &string) {
	sld::format_arg(f, format_str, string.ref());
}

} // namespace detail

template <size_t Size> using StringBuilder = detail::BasicStringBuilder<char, Size>;
template <size_t Size> using WStringBuilder = detail::BasicStringBuilder<wchar_t, Size>;

} // namespace sld

template <size_t Size>
using STString = sld::WStringBuilder<Size>;

namespace sld {

// simple timer template that returns time in milliseconds
template<typename Clock>
class Stopwatch
{
	typedef std::chrono::microseconds Time;
	typedef std::chrono::time_point<Clock> TimePoint;

	TimePoint start_time;
	TimePoint end_time;
public:
	Stopwatch() { start(); }

	// starts the clock
	void start()
	{
		start_time = end_time = Clock::now();
	}

	// stops the clock (returns the current elapsed time in ms)
	double stop()
	{
		end_time = Clock::now();
		return elapsed();
	}

	// get elapsed time in ms
	double elapsed() const
	{
		const auto delta = end_time < start_time ?
			std::chrono::duration_cast<Time>(Clock::now() - start_time) :
			std::chrono::duration_cast<Time>(end_time - start_time);
		return static_cast<double>(delta.count()) / 1000.0;
	}
};

} // namespace sld

class Stopwatch : public sld::Stopwatch<std::chrono::high_resolution_clock> {};

class RateLimit
{
	using Clock = std::chrono::steady_clock;
	using Ms = std::chrono::milliseconds;
public:
	RateLimit(Ms delay = Ms(200)) : delay(delay) {}
	bool check()
	{
		const auto cur_time = Clock::now();
		const auto diff = std::chrono::duration_cast<Ms>(cur_time - last_time);
		if (diff < delay)
			return false;

		last_time = cur_time;
		return true;
	}
private:
	Clock::time_point last_time;
	const Ms delay;
};

struct BitStore
{
	BitStore(uint32_t *data, unsigned bitCount) : data(data), bitCount(bitCount) {}

	// stores @value at the given @index
	void store(size_t index, uint32_t value)
	{
		const size_t dataIdx = (bitCount * index) / 32;
		const unsigned bitPos = (bitCount * index) % 32;

		const unsigned remaining = 32 - bitPos;
		if (remaining >= bitCount)
		{
			// the value is stored completely inside a single integer
			// clear the bits first and set the value after that
			data[dataIdx] &= ~(bit_mask(bitCount) << bitPos);
			data[dataIdx] |= (value & bit_mask(bitCount)) << bitPos;
		}
		else
		{
			// the value is split between 2 integers
			// clear the bits first and set the value after that
			data[dataIdx] &= ~(bit_mask(remaining) << bitPos);
			data[dataIdx + 1] &= bit_mask(bitCount - remaining);

			data[dataIdx] |= (value & bit_mask(remaining)) << bitPos;
			data[dataIdx + 1] |= (value >> remaining) & bit_mask(bitCount - remaining);
		}
	}

	// calculates the total size (in bytes) to store @count values of @bitCount size
	static size_t calcDataSize(unsigned bitCount, size_t count)
	{
		return div_round_up(bitCount * count, sizeof(uint32_t) * 8) * sizeof(uint32_t);
	}

	uint32_t *data;
	unsigned bitCount;
};

namespace sld {

void as_utf8(const wstring_ref &wstr, std::string &str);
std::string as_utf8(const wstring_ref &str);

void as_wide(const string_ref &str, std::wstring &wstr);
std::wstring as_wide(const string_ref &str);
std::wstring as_wide(const u16string_ref &str);

void as_utf16(const wstring_ref &str, std::u16string &u16str);
std::u16string as_utf16(const wstring_ref &str);
uint32_t as_utf16_len(const wstring_ref &str);

// reads file contents
MemoryBuffer read_file(const std::wstring &path);

// reads text file contents
std::wstring read_text_file(const std::wstring &path);

template <size_t Length>
static void copy(UInt16(&aDest)[Length], wstring_ref aSrc)
{
	const std::u16string str = as_utf16(aSrc);
	assert(Length > str.length());
	memcpy(aDest, str.c_str(), sizeof(str[0]) * (str.length() + 1));
}

template <typename T, size_t N>
constexpr size_t array_size(T(&)[N]) { return N; }

} // namespace sld

// classes for managing a collection of string refs
namespace sld {

class StringRefStore
{
public:
	StringRefStore() = default;

	StringRefStore(const StringRefStore&) = delete;
	StringRefStore& operator=(const StringRefStore&) = delete;

	StringRefStore(StringRefStore&&) = default;
	StringRefStore& operator=(StringRefStore&&) = default;

	~StringRefStore();

	wstring_ref strdup(wstring_ref str);

protected:
	std::vector<wstring_ref> strings_;
};

class StringRefMap : protected StringRefStore
{
public:
	// adds a string returning an index to it
	uint32_t insert(wstring_ref str);

	// return an index of the given string (~0u if not found)
	uint32_t get(wstring_ref index) const;

	// returns a string at the given index
	wstring_ref get(uint32_t index) const {
		return index < size() ? strings_[index] : nullptr;
	}

	size_t size() const { return strings_.size(); }
	bool empty() const { return size() == 0; }

	// returns a collection of all strings
	const std::vector<wstring_ref>& strings() const { return strings_; }

private:
	std::unordered_map<wstring_ref, uint32_t> mapIndex;
};

} // namespace sld
