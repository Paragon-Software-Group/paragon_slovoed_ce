#ifndef _SDC_WRITE_H_
#define _SDC_WRITE_H_

#include "Engine/SDC.h"

#include <cstdio>
#include <string>
#include <vector>

#include "util.h"

/// ����� �������� � ������ ����� � ������� SDC.
class CSDCWrite
{
public:

	struct CompressionConfig
	{
		// compression type to be used
		ESDCResourceCompressionType type;
		// minimal resource size to be compressed
		uint32_t minSize;
		// compression threshold - [0.0 .. 1.0]
		double threshold;
	};

	static const CompressionConfig DefaultCompression;
	static const CompressionConfig NoCompression;

	/// �����������
	CSDCWrite(void);
	/// ����������
	~CSDCWrite(void);

	/// ��������� ������.
	SDCError AddResource(const void *aPtr, UInt32 aSize, UInt32 aType, UInt32 aIndex, const CompressionConfig &aCompression = DefaultCompression);
	/// ��������� ������
	SDCError AddResource(MemoryRef aData, UInt32 aType, UInt32 aIndex, const CompressionConfig &aCompression = DefaultCompression);
	/// ��������� ������.
	SDCError AddResource(MemoryRef aData, UInt32 aType, UInt32 aIndex, UInt32 maxResourceSize, const CompressionConfig &aCompression = DefaultCompression);
	/// ��������� ������
	SDCError AddResource(MemoryBuffer&& aData, UInt32 aType, UInt32 aIndex, const CompressionConfig &aCompression = DefaultCompression);

	/// ��������� ������.
	SDCError AddResource(const std::wstring &fileName, UInt32 aType, UInt32 aIndex);

	/// ��������� ������ � ���������� SDC
	SDCError Write(const wchar_t *fileName);
	
	/// ������������� ������������� ����������.
	void SetDictID(UInt32 DictID);
	/// ������������� ��� ����������� ����������.
	void SetDatabaseType(UInt32 aDatabaseType);
	/// ������������� ���� ����, ��� ��� ����-����
	void SetInApp(UInt32 aIsInApp);

	/// ��������� ������������ ��� �������� ������ � ��������
	struct Resource
	{
		/// ����� � ������� �������
		MemoryBuffer buf;
		/// ��� �������
		uint32_t Type;
		/// ����� �������
		uint32_t Index;
		/// ������ ������� � ������������ ����
		uint32_t UncompressedSize;
		/// ��� ������������ ����������
		ESDCResourceCompressionType CompressionType;

		// ��������� ����������
		CompressionConfig compressionConfig;

		Resource(uint32_t aType, uint32_t aIndex, MemoryBuffer&& aData)
			: buf(std::move(aData)), Type(aType), Index(aIndex),
			  UncompressedSize(static_cast<uint32_t>(buf.size())),
			  CompressionType(eSDCResourceCompression_None)
		{}
	};

	/// ��������� ��������� �������� ��������� ������ ��������
	void OverrideCompressionConfig(const CompressionConfig &aConfig);

private:

	/// ��������� ������ � �������� ����.
	SDCError Write(FILE *out);
private:

	/// ������� � ��� ������������ ���������.
	std::vector<Resource> m_Data;

	/// ������������� ����������.
	UInt32 m_DictID;

	/// ��� ����������� ����������.
	UInt32 m_DatabaseType;
	/// ���� ����, ��� ��� ����-����
	UInt32 m_IsInApp;

	/// ���������������� �������� ������
	bool useOverrideCompressionCfg;
	CompressionConfig overrideCompressionCfg;
};

#endif // _SDC_WRITE_H_
