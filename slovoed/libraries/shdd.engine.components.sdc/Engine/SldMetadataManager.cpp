#include "SldMetadataManager.h"

#include "SldMacros.h"
#include "SldMetadataTypes.h"
#include "SldStringStore.h"
#include "SldTools.h"
#include "SldUtil.h"

namespace {

template <typename T> struct struct_size_helper { static SLD_CONSTEXPR_OR_CONST size_t value = sizeof(T); };
template <> struct struct_size_helper<TMetadataEmptyStruct> { static SLD_CONSTEXPR_OR_CONST size_t value = 0; };

template <ESldStyleMetaTypeEnum Type>
struct struct_size : public struct_size_helper<typename SldMetadataTraits<Type>::struct_type> {};

// массив размеров структурированных метаданных, indexed by ESldStyleMetaTypeEnum
static const size_t kMetaDataSize[] = {
	/* [eMetaText] = */      0,
	/* [eMetaPhonetics] = */ 0,
#define SIZE(enum_) struct_size< enum_ >::value,
	SLD_FOREACH_METADATA_TYPE(SIZE)
#undef SIZE
};
static_assert(sizeof(kMetaDataSize) / sizeof(kMetaDataSize[0]) == eMeta_Last, "Stale metadata sizes info.");

}

CSldMetadataManager::CSldMetadataManager(CSDCReadMy &aReader, const CSDCReadMy::Resource &aHeaderResource, ESldError &aError)
	: m_reader(aReader), m_resourceHeaderSize(0)
{
#define error_out(_err) do { aError = _err; return; } while(0)

	const UInt8 *data = aHeaderResource.ptr();

	// get info from the main header
	auto storedHeader = (const MetadataHeader*)data;

	// check for incompatibilities
	if (storedHeader->_size > sizeof(*storedHeader) ||
	    storedHeader->resourceHeaderSize > sizeof(ResourceHeader) ||
	    storedHeader->metadataInfoCount > eMeta_Last)
	{
		error_out(eCommonTooHighDictionaryVersion);
	}

	MetadataHeader header = {};
	sldMemCopy(&header, storedHeader, storedHeader->_size);

	data += header._size;

	// copy metadata info (follows the main header)
	ESldError error = sld2::arrayCopy(data, header.metadataInfoCount, header.metadataInfoSize, &m_metadataInfo);
	if (error != eOK)
		error_out(error == eCommonWrongSizeOfData ? eCommonTooHighDictionaryVersion : error);
	data += header.metadataInfoCount * header.metadataInfoSize;

	for (UInt32 i = 0; i < m_metadataInfo.size(); i++)
	{
		// this one is broken and can't ever be used
		if (i == eMeta_UnusedBroken)
			continue;

		const auto& info = m_metadataInfo[i];
		if (info._pad0 != 0 || info.metadataStructSize > kMetaDataSize[i])
			error_out(eCommonTooHighDictionaryVersion);
	}

	// copy resource headers (follows metadata info)
	error = sld2::arrayCopy(data, header.resourceCount, header.resourceInfoSize, &m_resourceInfo);
	if (error != eOK)
		error_out(error == eCommonWrongSizeOfData ? eCommonTooHighDictionaryVersion : error);
	data += header.resourceCount * header.resourceInfoSize;

	if (header.stringsResourceType)
	{
		auto strings = sld2::make_unique<CSldStringStore>(m_reader, header.stringsResourceType, error);
		if (!strings || error != eOK)
			error_out(strings ? error : eMemoryNotEnoughMemory);

		m_strings = sld2::move(strings);
	}

	m_resourceHeaderSize = header.resourceHeaderSize;

	aError = eOK;
#undef error_out
}

/**
 * Возвращает строку метаданных живущую в string store
 *
 * @param[in]  aIndex     - глобальный индекс строки
 * @param[out] aString    - ссылка куда будет записана строка
 *
 * Время жизни строки привязано к данному объекту; *контент* строки актуален только до следующего
 * вызова данного метода.
 *
 * @return код ошибки
 */
ESldError CSldMetadataManager::GetString(UInt32 aIndex, SldU16StringRef &aString)
{
	return m_strings ? m_strings->GetString(aIndex, aString) : eCommonWrongIndex;
}

// loads metadata using the global index, type and size
ESldError CSldMetadataManager::LoadMetadata(UInt32 aIndex, ESldStyleMetaTypeEnum aType, UInt32 aSize, void *aData)
{
	if (!aData)
		return eMemoryNullPointer;

	// we don't have any metadata info for this type
	if (aType >= (ESldStyleMetaTypeEnum)m_metadataInfo.size() || aType >= eMeta_Last)
		return eMetadataErrorInvalidStructureRef;

	// look up the resource and local indexes by the metadata global index
	UInt32 resourceIndex, localIndex, resourceSlot;
	ESldError error = GetResourceIndex(aIndex, &resourceIndex, &resourceSlot, &localIndex);
	if (error != eOK)
		return error;

	// load resource for the given index
	auto &resource = m_resource[resourceSlot];
	if (resource.type() != SLD_RESOURCE_STRUCTURED_METADATA_DATA || resource.index() != resourceIndex)
	{
		auto res = m_reader.GetResource(SLD_RESOURCE_STRUCTURED_METADATA_DATA, resourceIndex);
		if (res != eOK)
			return res.error();
		resource = res.resource();
	}

	const UInt8 *ptr = resource.ptr();
	const ResourceHeader* header = (const ResourceHeader*)ptr;
	if (header->_pad0 != 0)
		return eCommonTooHighDictionaryVersion;

	// check type
	if (sld2::readBits((const UInt32*)(ptr + m_resourceHeaderSize), header->typeBits, localIndex) != aType)
		return eMetadataErrorInvalidStructureRef;

	// get offset
	const UInt32 *offsets = (const UInt32*)(ptr + header->offsetsTableOffset);
	const UInt32 offset = sld2::readBits(offsets, header->offsetBits, localIndex);

	sldMemMove(aData, ptr + offset, m_metadataInfo[aType].metadataStructSize);
	return eOK;
}

// looks up the resource and local indexes by the metadata global index
ESldError CSldMetadataManager::GetResourceIndex(UInt32 aGlobalIndex, UInt32 *aResourceIndex,
                                                UInt32 *aResourceSlot, UInt32 *aLocalIndex) const
{
	// the descriptors (and their respective resources) are guaranteed to be stored with a
	// monotonically increasing `endIndex` where the previous resource's `startIndex` is the last
	// ones `endIndex` (with a 0 implicit `startIndex` for the first resource).
	// so we can simply iterate over the descriptors returning the first one where the global
	// index is less than `endIndex`
	// XXX: we can actually even binary search over this due to the sorted nature of it
	UInt32 startIndex = 0;
	for (UInt32 i = 0; i < m_resourceInfo.size(); i++)
	{
		const ResourceInfo *info = &m_resourceInfo[i];
		if (aGlobalIndex < info->endIndex)
		{
			*aResourceIndex = i;
			*aResourceSlot = info->slot;
			*aLocalIndex = aGlobalIndex - startIndex;
			return eOK;
		}
		startIndex = info->endIndex;
	}
	return eMetadataErrorInvalidStructureRef;
}
