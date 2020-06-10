#ifndef SLD2_TEXT_DECODERS_H_
#define SLD2_TEXT_DECODERS_H_

#include "SldError.h"
#include "SldTypeDefs.h"
#include "SldUtil.h"

// TODO: add comments regarding overall design and usage

namespace sld2 {
namespace decoders {

struct CharChain
{
	static ESldError check(const void *decoderData)
	{
		auto header = (const TCharChainMethodHeader*)decoderData;
		if (header->structSize != sizeof(*header) || header->CodeSize > 32)
			return eCommonTooHighDictionaryVersion;
		return eOK;
	}

	template <typename BitStream>
	static UInt32 decode(const void *decoderData, BitStream&& bits, sld2::Span<UInt16> string_)
	{
		auto header = (const TCharChainMethodHeader*)decoderData;

		const UInt32 codeSize = header->CodeSize;
		const UInt32 codeCount = header->NumberOfChars;
		auto codeTable = (const TCharChainMethodData*)(header + 1);
		auto charData = (const UInt16*)(codeTable + codeCount);

		sld2_assume(codeSize <= 32);

		UInt16 *string = string_.data();
		UInt32 length = string_.size();
		for (;;)
		{
			UInt32 code = bits.read(codeSize);
			if (sld2_unlikely(code >= codeCount))
				return 0;

			const UInt32 len = codeTable[code].Len;
			const UInt16 *source = charData + codeTable[code].Shift;
			for (UInt32 i = 0; i < len; i++)
			{
				string[i] = source[i];
				length--;
				if (length == 0 || source[i] == '\0')
					return string_.size() - length;
			}
			string += len;
		}
	}
};

struct CharCount
{
	static ESldError check(const void *decoderData)
	{
		auto header = (const TCountCharMethodHeader*)decoderData;
		if (header->structSize != sizeof(*header) || header->CodeSize > 32)
			return eCommonTooHighDictionaryVersion;
		return eOK;
	}

	template <typename BitStream>
	static UInt32 decode(const void *decoderData, BitStream&& bits, sld2::Span<UInt16> string)
	{
		auto header = (const TCountCharMethodHeader*)decoderData;

		const UInt32 codeSize = header->CodeSize;
		const UInt32 charCount = header->NumberOfChars;
		auto charData = (const UInt16*)(header + 1);

		sld2_assume(codeSize <= 32);

		UInt32 len = 0;
		do {
			UInt32 code = bits.read(codeSize);
			if (sld2_unlikely(code >= charCount))
				return 0;

			string[len++] = charData[code];
		} while (len < string.size() && string[len - 1]);
		return len;
	}
};

struct CharStore
{
	static ESldError check(const void*) { return eOK; }

	template <typename BitStream>
	static UInt32 decode(const void*, BitStream&& bits, sld2::Span<UInt16> string)
	{
		UInt32 len = 0;
		do {
			UInt32 code = bits.read(16);
			if (sld2_unlikely(code >= 0xffff))
				return 0;

			string[len++] = static_cast<UInt16>(code);
		} while (len < string.size() && string[len - 1]);
		return len;
	}
};

} // namespace decoders
} // namespace sld2

#endif // SLD2_TEXT_DECODERS_H_
