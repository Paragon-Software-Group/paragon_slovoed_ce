#ifndef _C_SLD_INT_FORMATTER_H_
#define _C_SLD_INT_FORMATTER_H_

#include "SldPlatform.h"
#include "SldStringReference.h"
#include "SldTypes.h"

namespace sld2 {
namespace fmt {
namespace detail {

template <UInt32 len>
inline SLD_CONSTEXPR UInt16* start(UInt16 (&data)[len])
{
	return data + len - 1;
}

template <UInt32 len>
inline SLD_CONSTEXPR UInt32 length(const UInt16(&data)[len], const UInt16 *ptr)
{
	return (UInt32)(data - ptr + len - 1);
}

template <UInt32 len>
inline SLD_CXX14_CONSTEXPR SldU16StringRef finalize(UInt16(&data)[len], const UInt16 *ptr)
{
	// as we are returning a string ref we could drop this terminating null
	// set it in any case just to be safe
	data[len - 1] = '\0';
	return SldU16StringRef(ptr, length(data, ptr));
}

UInt16* format(UInt16 *ptr, UInt32 aValue, UInt32 aRadix);

UInt16* format(UInt16 *ptr, TSizeValue aValue);

UInt16* pappend(UInt16 *ptr, SldU16StringRef aString);

} // namespace detail

template <UInt32 N>
inline SldU16StringRef format(UInt16(&aData)[N], UInt32 aValue, UInt32 aRadix)
{
	static_assert(N > 10, "Not enough space in the passed in buffer.");

	const UInt16 *ptr = detail::format(detail::start(aData), aValue, aRadix);
	return detail::finalize(aData, ptr);
}

template <UInt32 N>
inline SldU16StringRef format(UInt16(&aData)[N], Int32 aValue, UInt32 aRadix)
{
	static_assert(N > 11, "Not enough space in the passed in buffer.");

	const bool negative = aValue < 0;
	UInt16 *ptr = detail::format(detail::start(aData), negative ? -aValue : aValue, aRadix);
	if (negative)
		*--ptr = '-';
	return detail::finalize(aData, ptr);
}

template <UInt32 N>
inline SldU16StringRef format(UInt16(&aData)[N], TSizeValue aValue)
{
	static_assert(N > 14, "Not enough space in the passed in buffer.");

	const UInt16 *ptr = detail::format(detail::start(aData), aValue);
	return ptr ? detail::finalize(aData, ptr) : SldU16StringRef();
}

template <UInt32 BufferSize>
class FormatterBase
{
	static_assert(BufferSize > 15, "Too small embedded buffer.");

public:
	template <typename T, enable_if<is_integral<T>::value> = 0>
	SldU16StringRef format(T aValue, UInt32 aRadix = 10) {
		static_assert(sizeof(T) <= sizeof(UInt32), "Only <= 32 bit types supported.");
		using IntType = conditional<is_signed<T>::value, Int32, UInt32>;
		return fmt::format(m_data, static_cast<IntType>(aValue), aRadix);
	}

	SldU16StringRef format(TSizeValue aValue) { return fmt::format(m_data, aValue); }

protected:
	inline SLD_CXX14_CONSTEXPR UInt16* start() {
		return detail::start(m_data);
	}

	inline SLD_CONSTEXPR UInt32 length(const UInt16 *ptr) const {
		return detail::length(m_data, ptr);
	}

	inline SLD_CXX14_CONSTEXPR SldU16StringRef finalize(UInt16 *ptr) {
		return detail::finalize(m_data, ptr);
	}

	static inline UInt16* format(UInt16 *ptr, UInt32 value, UInt32 radix) {
		return detail::format(ptr, value, radix);
	}

	static inline UInt16* pappend(UInt16 *ptr, SldU16StringRef string) {
		return detail::pappend(ptr, string);
	}

private:
	// embedded buffer used for formatting; always filled in reverse order
	UInt16 m_data[BufferSize];
};

} // namespace fmt

/// формaттер с буфером на 32 символа
class IntFormatter : public fmt::FormatterBase<32> {};

} // namespace sld2

#endif // _C_SLD_INT_FORMATTER_H_
