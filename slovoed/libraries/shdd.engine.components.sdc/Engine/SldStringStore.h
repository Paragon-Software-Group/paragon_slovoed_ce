#pragma once
#ifndef _C_SLD_STRING_STORE_H_
#define _C_SLD_STRING_STORE_H_

#include "SldError.h"
#include "SldPlatform.h"
#include "SldStringReference.h"
#include "SldTypeDefs.h"
#include "SldSDCReadMy.h"

/// Класс отвечает за работу с упаковаными строками, по большому счету - "легкими" листами
class CSldStringStore
{
public:
	CSldStringStore(CSDCReadMy &aReader, UInt32 aType, ESldError &aError);

	CSldStringStore(const CSldStringStore&) = delete;
	CSldStringStore& operator=(const CSldStringStore&) = delete;

	/// Возвращает строку по глобальному индексу
	ESldError GetString(UInt32 aIndex, SldU16StringRef &aString);

	/// Возвращает строку по индексу ресурса и смещению
	ESldError GetStringDirect(UInt32 aResourceIndex, UInt32 aOffset, SldU16StringRef &aString);

	// *private*
	struct CurrentResource
	{
		UInt8 *data;
		UInt32 size;
		UInt32 index;
		UInt32 type;
		UInt32 maxSize;
	};

private:

	// decodes a string at the given offset from the current resource
	ESldError DecodeString(UInt32 aOffset, SldU16StringRef &aString);

	// if the string store is is globally indexable
	bool m_indexable;

	// encoding method used for data inside the store
	UInt32 m_encodingMethod;
	// the total count of resources with encoded data
	UInt32 m_resourceCount;

	// descriptor table, only for indexable string stores
	sld2::DynArray<TSldStringStoreResourceDescriptor> m_descriptorTable;

	// internal string buffer
	sld2::DynArray<UInt16> m_stringBuf;

	// heap allocated decoder data (also holds current resource data)
	sld2::DynArray<UInt8> m_decoderData;

	// resource reader
	CSDCReadMy &m_reader;

	// currently loaded resource with compressed data
	CurrentResource m_currentResource;
};

/// Класс отвечает за работу с ресурсами содержащими 1 упакованную строчку
class CSldSingleStringStore
{
public:
	CSldSingleStringStore(CSDCReadMy &aReader);

	CSldSingleStringStore(const CSldSingleStringStore&) = delete;
	CSldSingleStringStore& operator=(const CSldSingleStringStore&) = delete;

	/// Загружает ресурс с по заданному типу и индексу
	ESldError Load(UInt32 aType, UInt32 aIndex = 0);

	/// Возвращает размер хранимой строки включая nul-терминатор
	UInt32 Size() const;

	/// Декодирует строку в предоставленый буфер
	ESldError Decode(UInt16 *aString, UInt32 *aLength) const;

private:

	// loaded resource
	CSDCReadMy::Resource m_resource;

	// resource reader
	CSDCReadMy &m_reader;
};

#endif // _C_SLD_STRING_STORE_H_
