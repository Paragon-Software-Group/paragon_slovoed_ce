#include "SldIntFormatter.h"

static SldU16StringRef to_string(EMetadataUnitType aUnits)
{
	static const SldU16StringRef suffixes[] = {
		SldU16StringRefLiteral("px"),
		SldU16StringRefLiteral("pt"),
		SldU16StringRefLiteral("em"),
		SldU16StringRefLiteral("mm"),
		SldU16StringRefLiteral("%")
	};
	static_assert(sizeof(suffixes) / sizeof(suffixes[0]) == eMetadataUnitType_end,
				  "The unit suffix data table must be updated.");
	return aUnits < eMetadataUnitType_end ? suffixes[aUnits] : SldU16StringRef();
}

UInt16* sld2::fmt::detail::format(UInt16 *ptr, UInt32 aValue, UInt32 aRadix)
{
	static const char nums[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	if (aValue)
	{
		// main reason for doing these "special cases" is avoiding a div of the
		// "generic" case for the 2 most used bases and also doing base10 with a
		// simple addition which generates much better code (without any loads)
		if (aRadix == 10)
			do { *--ptr = '0' + aValue % 10; } while (aValue /= 10);
		else if (aRadix == 16)
			do { *--ptr = nums[aValue % 16]; } while (aValue /= 16);
		else
			do { *--ptr = nums[aValue % aRadix]; } while (aValue /= aRadix);
	}
	else
	{
		*--ptr = '0';
	}
	return ptr;
}

UInt16* sld2::fmt::detail::pappend(UInt16 *ptr, SldU16StringRef aString)
{
	for (UInt32 len = aString.size(); len; len--)
		*--ptr = aString[len - 1];
	return ptr;
}

UInt16* sld2::fmt::detail::format(UInt16 *ptr, TSizeValue aValue)
{
	if (aValue.Units >= eMetadataUnitType_end)
		return nullptr;

	ptr = pappend(ptr, to_string((EMetadataUnitType)aValue.Units));

	const UInt32 fractional = aValue.Fractional();
	if (fractional > 0)
	{
		if (fractional < 10)
		{
			*--ptr = (UInt16)('0' + fractional);
			*--ptr = '0';
		}
		else
		{
			if (fractional % 10 != 0)
				*--ptr = (UInt16)('0' + fractional % 10);
			*--ptr = (UInt16)('0' + fractional / 10);
		}
		*--ptr = '.';
	}

	const bool negative = aValue.Value < 0;
	const Int32 integer = aValue.Integer();
	ptr = format(ptr, negative ? -integer : integer, 10);
	if (negative)
		*--ptr = '-';
	return ptr;
}
