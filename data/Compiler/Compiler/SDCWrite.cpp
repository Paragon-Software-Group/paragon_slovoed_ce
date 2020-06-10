#include "SDCWrite.h"

#include "sld_Types.h"
#include "sld_Platform.h"
#include "SDC_CRC.h"
#include "StringFunctions.h"
#include "Log.h"
#include "Tools.h"

const CSDCWrite::CompressionConfig CSDCWrite::DefaultCompression = { eSDCResourceCompression_None, 0, 0 };
const CSDCWrite::CompressionConfig CSDCWrite::NoCompression = { eSDCResourceCompression_None, 0, 0 };

CSDCWrite::CSDCWrite(void)
{
	m_DatabaseType = 1;
	m_DictID = 0;
	m_IsInApp = 0;
	useOverrideCompressionCfg = false;
	overrideCompressionCfg = DefaultCompression;
}

CSDCWrite::~CSDCWrite(void) {}

void CSDCWrite::OverrideCompressionConfig(const CompressionConfig &aConfig)
{
	overrideCompressionCfg = aConfig;
	useOverrideCompressionCfg = true;
}

/**
 * ��������� ������
 *
 * @param[in] aData           - ������ �� ����� � ������� �������
 * @param[in] aType           - ��� �������
 * @param[in] aIndex          - ����� �������
 * @param[in] maxResourceSize - ������������ ������ ������� ��� ����������
 * @param[in] aCompression    - ��� ���������� ������� ������������ ����� ���� ������
 *
 * @return ��� ������
 *
 * ���� maxResourceSize=0, �� ������ ����������� ��� ����, �� �������� �� �������,
 * ���� maxResourceSize!=0 � ������ ������ ������ maxResourceSize, �� ������ ����������� �� �����
 * � ����������� � ���� ���������� ��������, � ����������� ����������� ������.
 */
SDCError CSDCWrite::AddResource(MemoryRef aData, UInt32 aType, UInt32 aIndex, UInt32 maxResourceSize, const CompressionConfig &aCompression)
{
	if (aData.empty())
		return SDC_WRITE_EMPTY_RESOURCE;

	if (maxResourceSize == 0)
		return AddResource(aData, aType, aIndex, aCompression);

	UInt32 size = static_cast<UInt32>(aData.size());
	UInt32 wrote_size = 0;
	while (size)
	{
		const UInt32 resource_size = size < maxResourceSize ? size : maxResourceSize;

		SDCError error = AddResource(MemoryRef(aData.data() + wrote_size, resource_size), aType, aIndex++, aCompression);
		if (error != SDC_OK)
			return error;

		size -= resource_size;
		wrote_size += resource_size;
	}

	return SDC_OK;
}

/**
 * ��������� ������
 *
 * @param[in] aPtr         - ��������� �� ������
 * @param[in] aSize        - ������ ������ ��� ��������
 * @param[in] aType        - ��� �������
 * @param[in] aIndex       - ����� �������
 * @param[in] aCompression - ��� ���������� ������� ������������ ����� ���� ������
 *
 * @return ��� ������
 */
SDCError CSDCWrite::AddResource(const void *aPtr, UInt32 aSize, UInt32 aType, UInt32 aIndex, const CompressionConfig &aCompression)
{
	if (!aPtr)
	{
		sld::printf("\nError! NULL aPtr: Size = 0x%Xu, aType = 0x%Xu, aIndex = 0x%Xu", aSize, aType, aIndex);
		return SDC_MEM_NULL_POINTER;
	}
	if (!aSize)
		return SDC_WRITE_EMPTY_RESOURCE;

	return AddResource(MemoryBuffer(aPtr, aSize), aType, aIndex, aCompression);
}

/**
 * ��������� ������
 *
 * @param[in] aData        - ����� � ������� �������
 * @param[in] aType        - ��� �������
 * @param[in] aIndex       - ����� �������
 * @param[in] aCompression - ��� ���������� ������� ������������ ����� ���� ������
 *
 * @return ��� ������
 */
SDCError CSDCWrite::AddResource(MemoryBuffer&& aData, UInt32 aType, UInt32 aIndex, const CompressionConfig &aCompression)
{
	if (aData.empty())
		return SDC_WRITE_EMPTY_RESOURCE;

	// �������� �������� �� ��, ������� �� ��� ������, ������� �� �������� ��������
	for (const Resource &resource : m_Data)
	{
		if (resource.Type == aType && resource.Index == aIndex)
			return SDC_WRITE_ALREADY_EXIST;
	}

	Resource resource(aType, aIndex, std::move(aData));
	// don't override compression config for resources with explictily disabled compression
	if (useOverrideCompressionCfg && aCompression.type != eSDCResourceCompression_None)
		resource.compressionConfig = overrideCompressionCfg;
	else
		resource.compressionConfig = aCompression;

	m_Data.push_back(std::move(resource));

	return SDC_OK;
}

/**
 * ��������� ������
 *
 * @param[in] aData        - ������ �� ����� � ������� �������
 * @param[in] aType        - ��� �������
 * @param[in] aIndex       - ����� �������
 * @param[in] aCompression - ��� ���������� ������� ������������ ����� ���� ������
 *
 * @return ��� ������
 */
SDCError CSDCWrite::AddResource(MemoryRef aData, UInt32 aType, UInt32 aIndex, const CompressionConfig &aCompression)
{
	return AddResource(MemoryBuffer(aData.data(), aData.size()), aType, aIndex, aCompression);
}

/**
 * ��������� ������, �������� ��� �� �����
 *
 * @param[in] fileName - ��� �����
 * @param[in] aType    - ��� �������
 * @param[in] aIndex   - ����� �������
 *
 * @return ��� ������
 */
SDCError CSDCWrite::AddResource(const std::wstring &fileName, UInt32 aType, UInt32 aIndex)
{
	auto fileData = sld::read_file(fileName);
	if (fileData.empty())
	{
		sld::printf("\nError! Can't read file %s", sld::as_ref(fileName));
		return SDC_READ_CANT_READ;
	}

	return AddResource(std::move(fileData), aType, aIndex);
}


/**
	������������� ������������� ����������.

	@param DictID - ������������� ����������.
*/
void CSDCWrite::SetDictID(UInt32 DictID)
{
	m_DictID = DictID;
}

/***********************************************************************
* ������������� ��� ����������� ����������.
*
* @param aDatabaseType	- ��� ����������� ����������
************************************************************************/
void CSDCWrite::SetDatabaseType(UInt32 aDatabaseType)
{
	m_DatabaseType = aDatabaseType;
}


/***********************************************************************
* ������������� ���� ����, ��� ��� ����-����
*
* @param aDatabaseType	- ���� ����-����
*						  0 - ��� ������ ������ ����
*						  1 - ��� ����-����
*
************************************************************************/
void CSDCWrite::SetInApp(UInt32 aIsInApp)
{
	m_IsInApp = aIsInApp;
}

/**
	���������� ������ � ���� ���� �������� ������� ���� ����� ��������� � ��������� ������.

	@param fileName - ��� ����� � ������� ����� ��������� ������.

	@return ��� ������.
*/
SDCError CSDCWrite::Write(const wchar_t *fileName)
{
	if (!fileName)
		return SDC_MEM_NULL_POINTER;

	FILE *out = sld::fopen(fileName, L"wb");
	if (!out)
	{
		sld::printf("\nError! Can't open file %s", sld::as_ref(fileName));
		return SDC_READ_CANT_OPEN_FILE;
	}

	SDCError error = Write(out);
	fclose(out);
	return error;
}

namespace sorted {
static inline bool byType(const SlovoEdContainerResourcePosition &lhs, const SlovoEdContainerResourcePosition &rhs) {
	return lhs.Type < rhs.Type;
}
static inline bool byIndex(const SlovoEdContainerResourcePosition &lhs, const SlovoEdContainerResourcePosition &rhs) {
	return lhs.Index < rhs.Index;
}
} // namespace sorted

static bool sort(std::vector<SlovoEdContainerResourcePosition> &resources)
{
	if (resources.size() < 2)
		return false;

	// ��������� �� ���� �������
	std::sort(resources.begin(), resources.end(), sorted::byType);

	// ��������� �� ������� ������� ������ ���������� �������� � ����� �����
	auto start = resources.begin();
	for (auto end = start; end != resources.end(); start = end)
	{
		while (end != resources.end() && end->Type == start->Type)
			++end;

		if (std::distance(start, end) > 1)
			std::sort(start, end, sorted::byIndex);
	}

	return true;
}


// tests decompression
static bool testDecomp(ESDCResourceCompressionType compressionType,
					   const MemoryBuffer &resource, const MemoryBuffer &compressed,
					   std::vector<char> &decompressed)
{
	decompressed.resize(resource.size());

	switch (compressionType)
	{
	default:
		assert(!"Should not be here!"); break;
	}
	return false;
}

// tries to compresses all the resources, returns the number of compressed resources
static uint32_t compress(std::vector<CSDCWrite::Resource> &resources)
{
	// cached "resource" used for decompression testing
	std::vector<char> decompressed;

	for (CSDCWrite::Resource &resource : resources)
	{
		ESDCResourceCompressionType compressionType = resource.compressionConfig.type;
		
		// skip resources marked as uncompressible
		if (compressionType == eSDCResourceCompression_None)
			continue;

		// skip too small resources
		if (resource.buf.size() < resource.compressionConfig.minSize)
			continue;

		MemoryBuffer compressed;
		switch (compressionType)
		{

		case eSDCResourceCompression_None:
		default:
			assert(!"Unhandled compression type!"); break;
		}

		if (compressed.empty())
			continue;

		if (!testDecomp(compressionType, resource.buf, compressed, decompressed))
		{
			assert(!"Decompression test failed!");
			continue;
		}

		// skip resources with bad compression ratio
		const double ratio = 1.0 - (double(compressed.size()) / double(resource.buf.size()));
		if (ratio < resource.compressionConfig.threshold)
			continue;

		// update the resource
		resource.buf = std::move(compressed);
		resource.CompressionType = compressionType;
	}

	size_t totalUncompressedSize = 0;
	size_t totalCompressedSize = 0;
	size_t compressedResourceCount = 0;
	for (const CSDCWrite::Resource &resource : resources)
	{
		totalUncompressedSize += resource.UncompressedSize;
		totalCompressedSize += resource.buf.size();
		if (resource.CompressionType != eSDCResourceCompression_None)
			compressedResourceCount++;
	}

	sldILog("Resource compression stats:\n");
	sldILog("  resource count: %lu, compressed: %lu\n", resources.size(), compressedResourceCount);
	if (compressedResourceCount)
	{
		sldILog("  uncompressed size: %s, compressed size: %s\n",
				sld::fmt::MemSize{ totalUncompressedSize }, sld::fmt::MemSize{ totalCompressedSize });
	}

	return static_cast<uint32_t>(compressedResourceCount);
}

// initializes a compressed resource header from a resource record
static SlovoEdContainerCompressedResourceHeader initCompressedHeader(const CSDCWrite::Resource &resource)
{
	SlovoEdContainerCompressedResourceHeader header;
	memset(&header, 0, sizeof(header));
	header.CompressionType = resource.CompressionType;
	header.UncompressedSize = resource.UncompressedSize;
	return header;
}

/**
	���������� ������ � �������� ���� ���� �������� ������� ���� 
	����� ��������� � ��������� ������.

	@param out - �������� ����. ���� ������ ������ �� �����������.

	@return ��� ������.
*/
SDCError CSDCWrite::Write(FILE *out)
{
	assert(out);

	// ��������� ���������
	SlovoEdContainerHeader header = {};

	header.Signature = SDC_SIGNATURE;
	header.HeaderSize = sizeof(header);
	header.Version = SDC_CURRENT_VERSION;
	header.DictID = m_DictID;
	header.NumberOfResources = static_cast<uint32_t>(m_Data.size());
	header.ResourceRecordSize = sizeof(SlovoEdContainerResourcePosition);
	header.DatabaseType = m_DatabaseType;
	header.BaseAddPropertyCount = 0;
	header.IsInApp = m_IsInApp;
	header.IsResourcesHaveNames = 1;

	// ������� �������� ��� ������ � ����
	std::vector<SlovoEdContainerResourcePosition> resTable(header.NumberOfResources);
	const UInt32 resTableSize = static_cast<UInt32>(resTable.size() * sizeof(resTable[0]));

	// �������� ����� �������
	header.HasCompressedResources = compress(m_Data) == 0 ? 0 : 1;

	uint32_t fileSize = header.HeaderSize + resTableSize;
	for (const auto resource : enumerate(m_Data))
	{
		SlovoEdContainerResourcePosition &record = resTable[resource.index];
		memset(&record, 0, sizeof(record));

		record.Type = resource->Type;
		record.Index = resource->Index;
		record.Size = static_cast<UInt32>(resource->buf.size());
		record.Shift = fileSize;

		if (resource->CompressionType != eSDCResourceCompression_None)
			record.Size += sizeof(SlovoEdContainerCompressedResourceHeader);

		// as we steal the bit we have to check if it's not already set
		assert(record.Size < (1u << 31));

		fileSize += record.Size;

		if (resource->CompressionType != eSDCResourceCompression_None)
			record.Size |= 1u << 31;
	}

	// ��������� ������� ��������
	header.IsResourceTableSorted = sort(resTable) ? 1 : 0;

	// ��������� ������ ������ ������.
	header.FileSize = fileSize;

	// ��������� CRC32, ������ �������� ����������� ��� ��
	header.CRC = CRC32((const UInt8*)&header, header.HeaderSize, SDC_CRC32_START_VALUE);
	header.CRC = CRC32((const UInt8*)resTable.data(), resTableSize, header.CRC);

	// ������������ �� ������ ������ ��������
	if (fseek(out, header.HeaderSize + resTableSize, SEEK_SET) != 0)
		return SDC_WRITE_CANT_WRITE;

	for (const Resource &resource : m_Data)
	{
		if (resource.CompressionType != eSDCResourceCompression_None)
		{
			const auto hdr = initCompressedHeader(resource);
			header.CRC = CRC32((const UInt8*)&hdr, sizeof(hdr), header.CRC, false);
			if (fwrite(&hdr, sizeof(hdr), 1, out) != 1)
				return SDC_WRITE_CANT_WRITE;
		}

		const UInt8 *data = (const UInt8*)resource.buf.data();
		const size_t size = resource.buf.size();
		header.CRC = CRC32(data, static_cast<UInt32>(size), header.CRC, false);
		if (fwrite(data, 1, size, out) != size)
			return SDC_WRITE_CANT_WRITE;
	}
	header.CRC = CRC32(nullptr, 0, header.CRC, true); // finalize

	if (fseek(out, 0, SEEK_SET) != 0)
		return SDC_WRITE_CANT_WRITE;

	// ��������� ��������� SDC
	if (fwrite(&header, header.HeaderSize, 1, out) != 1)
		return SDC_WRITE_CANT_WRITE;

	// ��������� ������� �������� ��������
	if (fwrite(resTable.data(), 1, resTableSize, out) != resTableSize)
		return SDC_WRITE_CANT_WRITE;

	return SDC_OK;
}
