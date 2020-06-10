#pragma once
#ifndef _C_SLD_METADATA_MANAGER_
#define _C_SLD_METADATA_MANAGER_

#include "SldPlatform.h"
#include "SldTypeDefs.h"
#include "SldSDCReadMy.h"
#include "SldStringReference.h"

class CSldStringStore;

class CSldMetadataManager
{
	typedef TStructuredMetadataHeader MetadataHeader;
	typedef TStructuredMetadataResourceHeader ResourceHeader;
	typedef TStructuredMetadataResourceInfoStruct ResourceInfo;
	typedef TStructuredMetadataMetaEnumInfoStruct MetaEnumInfo;

public:
	CSldMetadataManager(CSDCReadMy &aReader, const CSDCReadMy::Resource &aHeaderResource, ESldError &aError);

	/**
	 *  Загружает метаданные по индексу и указателю на структуру
	 *
	 *  @param [in] aIndex - индекс метаданных
	 *  @param [in] aData  - указатель на структуру метаданных куда копировать
	 *
	 *  @return код ошибки
	 */
	template<typename T>
	ESldError LoadMetadata(UInt32 aIndex, T *aData) {
		static_assert(T::metaType < eMeta_Last, "Invalid/unknown metadata type.");
		return LoadMetadata(aIndex, T::metaType, (UInt32)sizeof(T), aData);
	}

	/// Возвращает строку метаданных живущую в string store
	ESldError GetString(UInt32 aIndex, SldU16StringRef &aString);

private:

	// loads metadata using the global index, type and size
	ESldError LoadMetadata(UInt32 aIndex, ESldStyleMetaTypeEnum aType, UInt32 aSize, void *aData);

	// looks up the resource and local indexes by the metadata global index
	ESldError GetResourceIndex(UInt32 aGlobalIndex, UInt32 *aResourceIndex, UInt32 *aResourceSlot, UInt32 *aLocalIndex) const;

	// 
	CSDCReadMy &m_reader;

	// cached data from the header
	UInt32 m_resourceHeaderSize;

	// resources
	CSDCReadMy::Resource m_resource[TStructuredMetadataHeader::MaxSlotCount];

	// resource info
	sld2::DynArray<ResourceInfo> m_resourceInfo;

	// metadata info
	sld2::DynArray<MetaEnumInfo> m_metadataInfo;

	// the string store
	sld2::UniquePtr<CSldStringStore> m_strings;
};

#endif // _C_SLD_METADATA_MANAGER_
