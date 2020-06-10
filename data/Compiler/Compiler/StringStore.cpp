#include "StringStore.h"

#include <cassert>
#include <memory>

#include "Engine/SldTypeDefs.h"

#include "ICompressBy.h"
#include "Tools.h"

namespace {

// struct containing useful info for encoding
struct Context
{
	std::vector<StringStore::Resource> resources;

	// user settings
	const size_t maxResourceSize;
	const bool indexable;

	// calculated
	size_t maxStringLength;

	Context(size_t maxResourceSize, unsigned flags)
		: maxResourceSize(maxResourceSize), indexable((flags & StringStore::Indexable) != 0),
		  maxStringLength(0)
	{}
};

}

// allocates and prefills a memory buffer
static MemoryBuffer createResourceBlob(MemoryRef data, bool indexable, const std::vector<uint32_t> &offsets)
{
	TSldStringStoreResourceHeader header = {};

	header.maxOffset = offsets.back();

	const size_t dataSize = sizeof(header) + align_up(data.size(), sizeof(uint32_t));
	size_t totalSize = dataSize;
	if (indexable)
	{
		header.offsetBits = GetCodeLen(header.maxOffset);
		const size_t offsetsSize = BitStore::calcDataSize(header.offsetBits, offsets.size());
		totalSize += offsetsSize;
	}

	MemoryBuffer buf(totalSize);
	memcpy(buf.data(), &header, sizeof(header));
	memcpy(buf.data() + sizeof(header), data.data(), data.size());

	if (indexable)
	{
		BitStore offsetsData((uint32_t*)(buf.data() + dataSize), header.offsetBits);
		for (size_t i : indices(offsets))
			offsetsData.store(i, offsets[i]);
	}

	return buf;
}

// main encoding function
static void encode(Context &ctx, ICompressBy *compressor, const std::vector<std::u16string> &strings)
{
	const size_t resourceHeaderSize = sizeof(TSldStringStoreResourceHeader);

	COut cout; // this kinda violates the api, but it's crap anyway... oh well...
	compressor->Init(cout);

	for (uint32_t i = 0; i < strings.size();)
	{
		cout.ClearContent();

		StringStore::Resource resource;

		for (; resourceHeaderSize + cout.GetCurrentPos() / 8 < ctx.maxResourceSize && i < strings.size(); i++)
		{
			resource.offsets.push_back(resourceHeaderSize * 8 + cout.GetCurrentPos());

			const std::u16string &s = strings[i];
			int error = compressor->AddText((const UInt16*)s.c_str(), static_cast<UInt32>(s.size()) + 1, 0);
			if (error != ERROR_NO)
				throw sld::exception(error);

			if (s.size() > ctx.maxStringLength)
				ctx.maxStringLength = static_cast<uint32_t>(s.size());
		}

		const MemoryRef data = cout.GetData();
		if (data.empty())
			continue;

		resource.data = createResourceBlob(data, ctx.indexable, resource.offsets);
		ctx.resources.push_back(std::move(resource));
	}
}

// generates a blob of data containing string store header
static MemoryBuffer createHeaderBlob(const Context &ctx, ICompressBy *compressor, TSldStringStoreHeader header)
{
	const UInt8 *treeData;
	UInt32 treeSize;
	int error = compressor->GetCompressedTree(0, &treeData, &treeSize);
	if (error != ERROR_NO)
		throw sld::exception(error);

	MemoryBuffer blob(header._size + treeSize + header.resourceDescriptorSize * header.resourceCount);
	uint8_t *ptr = blob.data();

	memcpy(ptr, &header, header._size);
	ptr += header._size;

	if (ctx.indexable)
	{
		uint32_t globalIndex = 0;
		for (const StringStore::Resource &resource : ctx.resources)
		{
			TSldStringStoreResourceDescriptor descriptor = {};

			globalIndex += static_cast<uint32_t>(resource.offsets.size());
			descriptor.endIndex = globalIndex;

			memcpy(ptr, &descriptor, sizeof(descriptor));
			ptr += sizeof(descriptor);
		}
	}

	if (treeSize)
	{
		memcpy(ptr, treeData, treeSize);
		ptr += treeSize;
	}

	return blob;
}

/**
 * Creates a new string store from the source string array
 *
 * @param[in] aStrings         - the source strings array to compress
 * @param[in] aCompression     - compression config
 * @param[in] aThreadPool      - thread pool used by the compressor
 * @param[in] aMaxResourceSize - the maximum created resource size to use
 * @param[in] aFlags           - compression flags (see Flags enum on the class)
 *
 * Outside of flags the main knob here is the maximum resource size as the resulting
 * string offsets directly depend on it. So the smaller the size - the less bits we need
 * to encode the offset.
 *
 * On any error throws sld::exception.
 *
 * @return the compressed string store
 */
StringStore StringStore::compress(const std::vector<sld::wstring_ref> &aStrings,
								  CompressionConfig aCompression, ThreadPool &aThreadPool,
								  size_t aMaxResourceSize, unsigned aFlags)
{
	assert(!(aFlags & ~(Indexable)));

	if (aStrings.empty())
		return StringStore();

	std::unique_ptr<ICompressBy> compressor(CreateCompressClass(aCompression, aThreadPool));
	if (compressor == nullptr)
		throw sld::exception(ERROR_WRONG_COMPRESSION_METHOD);

	std::vector<std::u16string> strings(aStrings.size());
	for (size_t i : indices(aStrings))
		strings[i] = sld::as_utf16(aStrings[i]);

	int error = compressor->SetMode(ICompressBy::eInitialization);
	if (error != ERROR_NO)
		throw sld::exception(error);

	// build the dictionary
	for (auto&& string : strings)
	{
		error = compressor->AddText((const UInt16*)string.c_str(),
									static_cast<UInt32>(string.size()) + 1, 0);
		if (error != ERROR_NO)
			throw sld::exception(error);
	}

	// compress
	error = compressor->SetMode(ICompressBy::eCompress);
	if (error != ERROR_NO)
		throw sld::exception(error);

	Context context(aMaxResourceSize, aFlags);

	// encode
	encode(context, compressor.get(), strings);

	// generating the header
	TSldStringStoreHeader header = {};
	header._size = sizeof(header);

	header.indexable = context.indexable;
	header.resourceCount = static_cast<UInt32>(context.resources.size());
	header.resourceHeaderSize = sizeof(TSldStringStoreResourceHeader);
	if (context.indexable)
		header.resourceDescriptorSize = sizeof(TSldStringStoreResourceDescriptor);

	header.compressionMethod = aCompression.type;
	header.maxStringLength = static_cast<UInt16>(context.maxStringLength) + 1;

	for (const StringStore::Resource &resource : context.resources)
	{
		if (resource.data.size() > header.maxResourceSize)
			header.maxResourceSize = static_cast<UInt32>(resource.data.size());
	}

	StringStore data;
	data.m_header = createHeaderBlob(context, compressor.get(), header);
	data.m_data = std::move(context.resources);

	return data;
}

// returns the total generated resource count
uint32_t StringStore::resourceCount() const
{
	if (m_data.empty())
		return 0;
	return static_cast<uint32_t>(m_data.size()) + 1;
}

// returns a memory blob for the given resource index
MemoryBuffer StringStore::getResourceBlob(uint32_t aIndex) const
{
	// resource index 0 is the header, everything else is compressed data
	if (aIndex == 0)
		return m_header;

	aIndex--;
	return aIndex < m_data.size() ? m_data[aIndex].data : MemoryBuffer();
}

// returns the bit count needed to encode a resource index for any string
unsigned StringStore::resourceIndexBits() const
{
	return GetCodeLen(m_data.size());
}

// returns the bit count needed to encode an offset for any string
unsigned StringStore::maxOffsetIndexBits() const
{
	uint32_t maxOffset = 0;
	for (const Resource &resource : m_data)
	{
		uint32_t offset = resource.offsets.back();
		if (offset > maxOffset)
			maxOffset = offset;
	}
	return GetCodeLen(maxOffset);
}

// returns a table containg <resource index, offset> for all encoded string
std::vector<std::pair<uint32_t, uint32_t>> StringStore::buildDirectOffsetsTable() const
{
	std::vector<std::pair<uint32_t, uint32_t>> index;
	for (auto&& resource : enumerate(m_data))
	{
		for (uint32_t offset : resource->offsets)
			index.emplace_back(static_cast<uint32_t>(resource.index), offset);
	}
	return index;
}

/**
 * Creates a new simple string store resource from a source string
 *
 * @param[in] aString          - the source string to compress
 * @param[in] aCompression     - compression config
 * @param[in] aThreadPool      - thread pool used by the compressor
 *
 * On any error throws sld::exception.
 *
 * @return the compressed string resource
 */
MemoryBuffer StringStore::compress(sld::wstring_ref aString, CompressionConfig aCompression, ThreadPool &aThreadPool)
{
	std::unique_ptr<ICompressBy> compressor(CreateCompressClass(aCompression, aThreadPool));
	if (compressor == nullptr)
		throw sld::exception(ERROR_WRONG_COMPRESSION_METHOD);

	COut cout;
	compressor->Init(cout);

	const std::u16string string = sld::as_utf16(aString);
	const UInt32 stringLength = static_cast<UInt32>(string.size()) + 1;

	for (auto mode : { ICompressBy::eInitialization, ICompressBy::eCompress })
	{
		int error = compressor->SetMode(mode);
		if (error != ERROR_NO)
			throw sld::exception(error);

		error = compressor->AddText((const UInt16*)string.c_str(), stringLength, 0);
		if (error != ERROR_NO)
			throw sld::exception(error);
	}

	// generate the resource blob
	const UInt8 *treeData;
	UInt32 treeSize;
	int error = compressor->GetCompressedTree(0, &treeData, &treeSize);
	if (error != ERROR_NO)
		throw sld::exception(error);

	const auto encodedData = cout.GetData();

	// generate the resource blob
	TSldSingleStringStoreHeader header = {};
	header._size = sizeof(header);

	header.compressionMethod = aCompression.type;
	header.stringLength = stringLength;

	// align both the start and the size of the encoded string data to uint32
	// the latter is *required* by the Engine (due to the "greedy" nature of the bitreader)
	// the former will let us avoid unaligned accesses when decoding
	header.stringOffset = align_up(header._size + treeSize, sizeof(UInt32));
	const size_t blobSize = header.stringOffset + align_up(encodedData.size(), sizeof(UInt32));

	// allocate & copy the stuff in
	MemoryBuffer blob(blobSize);
	memcpy(blob.data(), &header, header._size);
	if (treeData)
		memcpy(blob.data() + header._size, treeData, treeSize);
	if (encodedData.size())
		memcpy(blob.data() + header.stringOffset, encodedData.data(), encodedData.size());

	return blob;
}
