#include "SldStringStore.h"

#include "SldSDCReadMy.h"
#include "SldTextDecoders.h"
#include "SldTools.h"
#include "SldUtil.h"

namespace {
namespace decoders {

// local bitmask func to skip on the "useless" branching
static inline UInt32 bit_mask(unsigned x) { return (1u << x) - 1u; }

struct BitStream
{
	BitStream(const UInt8 *data_, UInt32 offset)
	{
		// offset is in *bits*
		const UInt32 consumed = offset % 8;

		data = data_ + offset / 8;
		advance();
		remaining -= consumed;
		word >>= consumed;
	}

	// reads some data from the current position
	// the size is in *bits* and the callers must guarantee it's <= 32
	inline UInt32 read(unsigned size)
	{
		if (remaining == 0)
			advance();

		UInt32 num = word;
		if (size < remaining)
		{
			num &= bit_mask(size);
		}
		else if (size > remaining)
		{
			unsigned shift = remaining;
			advance();
			size -= shift;
			num |= (word & bit_mask(size)) << shift;
		}

		remaining -= size;
		word >>= size;
		return num;
	}

	inline void advance()
	{
		sldMemCopy(&word, data, sizeof(word));
		data += sizeof(word);
		remaining = sizeof(word) * 8;
	}

	// the current data pointer
	const UInt8 *data;
	// the current word
	UInt32 word;
	// the count of *bits* remaining in the current word
	unsigned remaining;
};

// local enum for saner switches
enum {
	eCharStore = SLD_COMPRESSION_METHOD_TEXT
};

static inline bool isValidEncoding(UInt32 aMethod)
{
	return aMethod == eCharStore;
}

// import the actual decoders
using namespace sld2::decoders;

static ESldError check(UInt32 method, const void *decoderData)
{
	switch (method)
	{
	case eCharStore: return CharStore::check(decoderData); break;
	default: return eCommonWrongCompressionType;
	}
}

struct decodeResult { ESldError error; UInt32 length; };
static decodeResult decode(UInt32 method, const void *decoderData, BitStream bits, sld2::Span<UInt16> string)
{
	UInt32 length;
	switch (method)
	{
	case eCharStore: length = CharStore::decode(decoderData, bits, string); break;
	default: return{ eCommonWrongCompressionType, 0 }; // realistically this can't really be hit
	}
	// decoders count emitted symbols including the nul-terminator
	return{ length != 0 ? eOK : eCommonWrongCharIndex, length - 1 };
}

} // namespace decoders
} // anon namespace

/**
 * Конструктор
 *
 * @param[in]  aReader - загрузчик ресурсов
 * @param[in]  aType   - тип ресурсов в которых содержатся данные
 * @param[out] aError  - статус инициализации объекта
 *
 * Если aError != eOK объект находится в "неопределенном" состоянии с единственным
 * возможноым (определенным) действием - деструкцией
 */
CSldStringStore::CSldStringStore(CSDCReadMy &aReader, UInt32 aType, ESldError &aError)
	: m_indexable(false), m_encodingMethod(~0u), m_resourceCount(0), m_reader(aReader)
{
#define error_out(_err) do { aError = _err; return; } while(0)

	sld2::memzero(m_currentResource);

	ESldError error;
	auto headerResource = m_reader.GetResource(aType, 0);
	if (headerResource != eOK)
		error_out(headerResource.error());

	const UInt8 *ptr = headerResource.ptr();

	// forward compat
	auto storedHeader = (const TSldStringStoreHeader*)ptr;
	if (storedHeader->_size > sizeof(*storedHeader) ||
		storedHeader->resourceHeaderSize > sizeof(TSldStringStoreResourceHeader) ||
		!decoders::isValidEncoding(storedHeader->compressionMethod) ||
		storedHeader->_flags0 != 0)
	{
		error_out(eCommonTooHighDictionaryVersion);
	}

	// create a local zero initialized header and copy the stored contents in it
	TSldStringStoreHeader _header = {};
	sldMemCopy(&_header, storedHeader, storedHeader->_size);
	const TSldStringStoreHeader *header = &_header;

	// skip the header
	ptr += storedHeader->_size;

	// copy the descriptor table if there is one
	if (header->indexable && header->resourceDescriptorSize)
	{
		error = sld2::arrayCopy(ptr, header->resourceCount, header->resourceDescriptorSize, &m_descriptorTable);
		if (error != eOK)
			error_out(error == eCommonWrongSizeOfData ? eCommonTooHighDictionaryVersion : error);

		ptr += header->resourceDescriptorSize * header->resourceCount;
	}

	// check the decoder header
	error = decoders::check(header->compressionMethod, ptr);
	if (error != eOK)
		error_out(error);

	// decoder data + staging buffer mem alloc
	{
		// decoder data and the encoded strings resource data are all placed
		// inside a single contiguous buffer one after the other with some alignement
		// sprinkled here and there

		// everything at the end of the header resource is the decoder data
		const UInt32 size = headerResource.size() - (UInt32)(ptr - headerResource.ptr());
		// align the start of the encoded string buffer to avoid unaligned accesses
		const UInt32 alignedSize = sld2::align_up2<sizeof(UInt32)>(size);

		// as bit stream is greedy (as in it reads in UInt32 chunks and can read past the "end")
		// overallocate a bit so that we don't read into uninitialized memory
		if (!m_decoderData.resize(alignedSize + header->maxResourceSize + sizeof(UInt32) * 2))
			error_out(eMemoryNotEnoughMemory);

		sldMemCopy(m_decoderData.data(), ptr, size);
		m_currentResource.data = m_decoderData.data() + alignedSize;
	}

	// allocate the string staging buffer storage
	if (!m_stringBuf.resize(header->maxStringLength))
		error_out(eMemoryNotEnoughMemory);

	m_indexable = header->indexable;
	m_encodingMethod = header->compressionMethod;
	m_resourceCount = header->resourceCount;

	m_currentResource.type = aType;
	m_currentResource.maxSize = header->maxResourceSize;

	aError = eOK;
#undef error_out
}

// loads the resource of the given index
static ESldError load(CSDCReadMy &aReader, CSldStringStore::CurrentResource &aResource, UInt32 aIndex)
{
	if (aIndex == aResource.index && aResource.size != 0)
		return eOK;

	UInt32 dataSize = aResource.maxSize;
	ESldError error = aReader.GetResourceData(aResource.data, aResource.type, aIndex + 1, &dataSize);
	if (error != eOK)
		return error;

	aResource.index = aIndex;
	aResource.size = dataSize;
	return eOK;
}

// looks up the resource and local indexes by the struct global index
static bool getStringIndex(sld2::Span<const TSldStringStoreResourceDescriptor> aDescriptors,
						   UInt32 aGlobalIndex, UInt32 *aResourceIndex, UInt32 *aLocalIndex, UInt32 *aIndexCount)
{
	UInt32 startIndex = 0;
	for (UInt32 i = 0; i < aDescriptors.size(); i++)
	{
		const TSldStringStoreResourceDescriptor &descriptor = aDescriptors[i];
		if (aGlobalIndex < descriptor.endIndex)
		{
			*aResourceIndex = i;
			*aLocalIndex = aGlobalIndex - startIndex;
			*aIndexCount = descriptor.endIndex - startIndex;
			return true;
		}
		startIndex = descriptor.endIndex;
	}
	return false;
}

/**
 * Возвращает строку по глобальному индексу
 *
 * @param[in]  aIndex     - глобальный индекс строки
 * @param[out] aString    - ссылка куда будет записана декодированя строка
 *
 * Время жизни строки привязано к данному объекту; *контент* строки актуален только до следующего
 * вызова любого неконстантного метода данного объекта.
 *
 * @return код ошибки
 */
ESldError CSldStringStore::GetString(UInt32 aIndex, SldU16StringRef &aString)
{
	if (!m_indexable || m_resourceCount == 0)
		return eCommonWrongIndex;

	UInt32 resourceIndex, localIndex, stringCount;
	if (!getStringIndex(m_descriptorTable, aIndex, &resourceIndex, &localIndex, &stringCount))
		return eCommonWrongIndex;

	ESldError error = load(m_reader, m_currentResource, resourceIndex);
	if (error != eOK)
		return error;

	// get the offset of the string, the offsets table is placed at the end of the resource so we have to first
	// calculate its size to be able to get a pointer to it's start address
	auto header = (const TSldStringStoreResourceHeader*)m_currentResource.data;
	const UInt32 offsetsTableSize = sld2::div_round_up(header->offsetBits * stringCount, sizeof(UInt32) * 8) * sizeof(UInt32);
	const UInt32 *offsets = (const UInt32*)(m_currentResource.data + m_currentResource.size - offsetsTableSize);
	const UInt32 offset = sld2::readBits(offsets, header->offsetBits, localIndex);

	return DecodeString(offset, aString);
}

/**
 * Возвращает строку по индексу ресурса и смещению
 *
 * @param[in]  aResourceIndex - индекс ресурса в котором искать строку
 * @param[in]  aOffset        - смещение относительно начала ресурса (в *битах*)
 * @param[out] aString        - ссылка куда будет записана декодированя строка
 *
 * Время жизни строки привязано к данному объекту; *контент* строки актуален только до следующего
 * вызова любого неконстантного метода данного объекта.
 *
 * @return код ошибки
 */
ESldError CSldStringStore::GetStringDirect(UInt32 aResourceIndex, UInt32 aOffset, SldU16StringRef &aString)
{
	if (aResourceIndex >= m_resourceCount)
		return eCommonWrongIndex;

	ESldError error = load(m_reader, m_currentResource, aResourceIndex);
	if (error != eOK)
		return error;

	return DecodeString(aOffset, aString);
}

// decodes a string at the given offset from the current resource
ESldError CSldStringStore::DecodeString(UInt32 aOffset, SldU16StringRef &aString)
{
	using namespace decoders;

	auto header = (const TSldStringStoreResourceHeader*)m_currentResource.data;
	if (aOffset > header->maxOffset)
		return eCommonWrongIndex;

	BitStream bits(m_currentResource.data, aOffset);
	auto result = decoders::decode(m_encodingMethod, m_decoderData.data(), bits, m_stringBuf);
	if (result.error == eOK)
		aString = SldU16StringRef(m_stringBuf.data(), result.length);
	return result.error;
}

CSldSingleStringStore::CSldSingleStringStore(CSDCReadMy &aReader) : m_reader(aReader) {}

/**
 * Загружает ресурс в котором содержатся данные
 *
 * @param[in] aType  - тип ресурса
 * @param[in] aIndex - индекс ресурса (по умолчанию 0)
 *
 * @return код ошибки
 */
ESldError CSldSingleStringStore::Load(UInt32 aType, UInt32 aIndex)
{
	auto res = m_reader.GetResource(aType, aIndex);
	if (res != eOK)
		return res.error();
	m_resource = res.resource();

	auto header = (const TSldSingleStringStoreHeader*)m_resource.ptr();
	if (header->_size > sizeof(*header) ||
		!decoders::isValidEncoding(header->compressionMethod))
	{
		return eCommonTooHighDictionaryVersion;
	}

	// check if we'll be able to safely read the encoded data
	if ((m_resource.size() - header->stringOffset) % sizeof(UInt32) != 0)
		return eCommonWrongResourceSize;

	return decoders::check(header->compressionMethod, m_resource.ptr() + header->_size);
}

/// Возвращает размер хранимой строки *включая nul-терминатор*, 0 при ошибке
UInt32 CSldSingleStringStore::Size() const
{
	if (m_resource.empty())
		return 0;

	return ((const TSldSingleStringStoreHeader*)m_resource.ptr())->stringLength;
}


/**
 * Декодирует строку в предоставленый буфер
 *
 * @param[in]    aString - буфер куда будет произведено декодирование строки
 * @param[inout] aLength - указатель на переменную где хранится размер переданного буфера,
 *                         сюда же будет записан размер декодированной строки
 *
 * @return код ошибки
 */
ESldError CSldSingleStringStore::Decode(UInt16 *aString, UInt32 *aLength) const
{
	using namespace decoders;

	if (!aString || !aLength)
		return eMemoryNullPointer;
	if (*aLength < Size())
		return eMemoryNotEnoughMemory;
	if (m_resource.empty())
		return eResourceCantGetResource;

	auto header = (const TSldSingleStringStoreHeader*)m_resource.ptr();

	const void *decoderData = m_resource.ptr() + header->_size;
	BitStream bits(m_resource.ptr() + header->stringOffset, 0);
	auto string = sld2::make_span(aString, *aLength);

	auto result = decoders::decode(header->compressionMethod, decoderData, bits, string);
	if (result.error == eOK)
		*aLength = result.length;
	return result.error;
}
