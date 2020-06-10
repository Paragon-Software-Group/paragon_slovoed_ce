#include "MetadataManager.h"

#include <functional>
#include <list>
#include <unordered_set>

#include "Engine/SldMacros.h"
#include "Engine/SldTypeDefs.h"

#include "Log.h"
#include "Tools.h"
#include "util.h"

// local typedefs to save on the typing
namespace { typedef MetadataManager::TaggedMetadata TaggedMetadata; }
namespace sld {
namespace {
typedef TStructuredMetadataHeader MetadataHeader;
typedef TStructuredMetadataResourceInfoStruct ResourceInfo;
typedef TStructuredMetadataMetaEnumInfoStruct MetaEnumInfo;
typedef TStructuredMetadataResourceHeader ResourceHeader;
} // anon namespace
} // namespace sld

// the "maximum" resource size in bytes (not a truly *hard* limit)
enum : size_t { kMaxResourceSize = 1024 * 64 };
enum { kLLCScaleFactor = 4 };

// type assigned to all metadata strings, must match the actual storage used
enum { MetadataStringType = eMetadataStringSource_Embedded };

namespace {

template <typename T> struct struct_size_helper { static constexpr size_t value = sizeof(T); };
template <> struct struct_size_helper<TMetadataEmptyStruct> { static constexpr size_t value = 0; };

template <ESldStyleMetaTypeEnum Type>
struct struct_size : public struct_size_helper<typename SldMetadataTraits<Type>::struct_type> {};

}

// массив размеров структурированных метаданных
// indexed by ESldStyleMetaTypeEnum
static const size_t kMetaDataSize[] = {
	/* [eMetaText] = */      0,
	/* [eMetaPhonetics] = */ 0,
#define SIZE(enum_) struct_size< enum_ >::value,
	SLD_FOREACH_METADATA_TYPE(SIZE)
#undef SIZE
};
static_assert(sld::array_size(kMetaDataSize) == eMeta_Last,
			  "Stale metadata sizes info.");

static inline bool operator==(const TaggedMetadata &lhs, const TaggedMetadata &rhs)
{
	if (lhs.type != rhs.type)
		return false;
	if (lhs.ptr == rhs.ptr)
		return true;
	return lhs.ptr != nullptr && rhs.ptr != nullptr &&
		memcmp(lhs.ptr, rhs.ptr, kMetaDataSize[lhs.type]) == 0;
}

// TaggedMetadata support for unordered maps
namespace std {

template<> struct hash<TaggedMetadata> {
	size_t operator()(const TaggedMetadata &v) const {
		size_t hash = fnv1a::hash(&v.type, sizeof(v.type));
		if (v.ptr != nullptr)
			hash = fnv1a::accumulate(hash, v.ptr, kMetaDataSize[v.type]);
		return hash;
	}
};

} // namespace std

/**
 *  Adds a new string to metadata strings "db".
 *
 *  @param [in] aString - new metadata string
 *
 *  @return the "reference" to the string
 */
TMetadataString MetadataManager::addString(sld::wstring_ref aString)
{
	TMetadataString string;

	if (aString.empty())
		return string;

	string.type = MetadataStringType;
	string.data = m_strings.insert(aString);
	return string;
}

/**
 *  Returns a stored string by reference
 *
 *  @param [in] aString - "reference" to the string
 *
 *  @return string stored for a given reference or empty string if the reference is invalid
 */
sld::wstring_ref MetadataManager::getString(const TMetadataString &aString) const
{
	return aString.type == MetadataStringType ? m_strings.get(aString.data) : nullptr;
}

/**
 *  "Removes" the metadata for the given reference
 *
 *  @param [in] aRef  - metadata reference
 *
 *  @return status if we did find the metadata described
 */
bool MetadataManager::removeMetadata(MetadataRef &aRef)
{
	if (aRef.index() >= m_metaData.size())
		return false;
	TaggedMetadata &data = m_metaData[aRef.index()];
	if (!aRef.checkType(data.type))
		return false;
	// simplify the logic by simply clearing the metadata instead of really deleting it
	data = TaggedMetadata();
	aRef = MetadataRef();
	return true;
}

// adds the metadata (internal method)
// returns the index of the newly added metadata
uint32_t MetadataManager::addMetadata(ESldStyleMetaTypeEnum aType, const void *aData, const std::wstring &aArticleId)
{
	assert(aType < eMeta_Last);
	const size_t size = kMetaDataSize[aType];
	assert(size != 0);

	void *data = allocator.allocate(size);
	if (!data)
		return InvalidMetadataIndex;
	memcpy(data, aData, size);

	const uint32_t index = static_cast<uint32_t>(m_metaData.size());
	m_metaData.emplace_back(aType, data);
	m_metaDataArticleId[aArticleId].push_back(index);
	return index;
}

// tries to find metadata for the given index + type pair
// if it can't returns a dummy empty metadata
const TaggedMetadata& MetadataManager::findMetadata(uint32_t aIndex, ESldStyleMetaTypeEnum aType) const
{
	static const TaggedMetadata dummy;
	if (aIndex >= m_metaData.size())
		return dummy;
	const TaggedMetadata &data = m_metaData[aIndex];
	return data.type == aType ? data : dummy;
}

// string "serialization" routines
namespace {
namespace strings {

using EmbeddedString = MetadataManager::OptimizedData::EmbeddedString;

namespace impl {

template <typename T> void serialize(const MetadataManager&, EmbeddedString&, T&) {}

void serialize(const MetadataManager &mgr, EmbeddedString &strings, TMetadataString &string);

#include "MetadataManagerStrings.incl"

}

static_assert(std::tuple_size<decltype(EmbeddedString::data)>::value == impl::MaxFieldCount,
			  "Not enough room for a maximum number of metadata strings possible.");

void impl::serialize(const MetadataManager &mgr, EmbeddedString &strings, TMetadataString &string)
{
	sld::wstring_ref str = mgr.getString(string);
	if (str.empty())
	{
		string = TMetadataString();
		return;
	}

	// calculate the true length of the string in utf-16
	const uint32_t length = sld::as_utf16_len(str);

	assert(length < UINT16_MAX && "Can't store embedded strings longer than 64k!");
	assert(strings.length + length < UINT16_MAX && "Can't store embedded strings longer than 64k!");
	assert(strings.count < strings.data.size() && "Not enough room in a strings array!");

	strings.data[strings.count++] = string.data;

	string.type = eMetadataStringSource_Embedded;
	string.data = strings.length;

	strings.length += length + 2;
}

template <typename T>
void serialize(const MetadataManager &mgr, EmbeddedString &strings, T *metaData)
{
	if (metaData)
		impl::serialize(mgr, strings, *metaData);
}

std::vector<EmbeddedString> serialize(const MetadataManager &mgr, const std::vector<TaggedMetadata> &metaData)
{
	std::vector<EmbeddedString> strings;
	strings.resize(metaData.size());
	for (auto&& data : enumerate(metaData))
	{
		EmbeddedString &stringData = strings[data.index];
		switch (data->type)
		{
#define SERIALIZE(enum_) case enum_: serialize(mgr, stringData, data->get<enum_>()); break;
			SLD_FOREACH_METADATA_TYPE(SERIALIZE)
#undef SERIALIZE
		default: break;
		}
	}
	return strings;
}

}
}

// Packing routines
namespace {

// a couple of naming conventions:
//  `origIndex` always denotes the "original" metadata index set when adding metaData
//  `oldIndex` always denotes the "old" index of the metadata from the pov of the current "pass"
//  `newIndex` or simply `index` denotes the new index of metadata (potentially after the current pass)

// fwd decl
struct PackCtx;

struct Resource
{
	// index in the resource -> index in ctx.metaData
	std::vector<uint32_t> index;
	// index in the resource -> an array of "original" indexes
	std::vector<std::vector<uint32_t>> newIndexToOrig;
	// 'old' index -> index in the resource
	std::unordered_map<uint32_t, uint32_t> oldIndexToNew;

	// resource size in bytes
	size_t memSize = 0;

	using IndexRange = RangeImpl<const uint32_t*>;

	void addMetadata(const PackCtx &ctx, uint32_t oldIndex, IndexRange toOrig);

	void addMetadata(const PackCtx &ctx, uint32_t oldIndex, const std::vector<uint32_t> &toOrig) {
		addMetadata(ctx, oldIndex, IndexRange(toOrig.data(), toOrig.data() + toOrig.size()));
	}

	void addMetadata(const PackCtx &ctx, uint32_t oldIndex, uint32_t origIndex) {
		addMetadata(ctx, oldIndex, IndexRange(&origIndex, &origIndex + 1));
	}
};

struct ResourceBuilder
{
	// the list of all the resources
	std::list<Resource> resources;
	const char *const label;

	ResourceBuilder(const char *const label) : label(label) { resources.emplace_back(); }

	Resource& current() { return resources.back(); }
	const Resource& current() const { return resources.back(); }

	bool check(size_t maxResourceSize) const
	{
		return resources.empty() ? false : resources.back().memSize > maxResourceSize;
	}

	void closeCurrent()
	{
		if (!current().index.empty())
			resources.emplace_back();
	}

	void finish()
	{
		// go through the resources cleanig up completely empty ones
		for (auto it = resources.begin(); it != resources.end();)
		{
			if (it->index.empty())
				it = resources.erase(it);
			else
				++it;
		}
	}

	size_t metadataCount() const
	{
		size_t c = 0;
		for (const Resource &res : resources) c += res.index.size();
		return c;
	}

	size_t memSize() const
	{
		size_t size = 0;
		for (const Resource &res : resources) size += res.memSize;
		return size;
	}
};

struct PackCtx
{
	// the "new" metadata vector
	std::vector<TaggedMetadata> metaData;
	// mapping from the original index to the index inside metaData
	std::vector<uint32_t> origIndexToNew;
	// mapping from the "current" index to the original ones
	std::vector<std::vector<uint32_t>> indexToOrig;
	// mapping from the original index to article "id"
	std::vector<uint32_t> origIndexToArticleId;
	// mapping from the article "id" to original indices
	std::vector<std::vector<uint32_t>> articleIdToOrigIndex;
	// mapping from the current index to the set of article "ids"
	std::vector<std::vector<uint32_t>> indexToArticleId;
	// the "index" of *truly* shared metadata (indexed by the "new" index)
	std::vector<bool> sharedMetadata;

	// The "options" part
	// the maximum size (in bytes) of a single resource (not a *hard* limit!)
	size_t maxResourceSize;

	PackCtx(size_t maxResourceSize = kMaxResourceSize) :
		maxResourceSize(maxResourceSize)
	{}
};

void Resource::addMetadata(const PackCtx &ctx, uint32_t oldIndex, IndexRange toOrig)
{
	uint32_t newIndex = InvalidMetadataIndex;
	const auto it = oldIndexToNew.find(oldIndex);
	if (it != oldIndexToNew.end())
	{
		newIndex = it->second;

		auto& index = newIndexToOrig[newIndex];
		index.insert(index.end(), toOrig.begin(), toOrig.end());
	}
	else
	{
		newIndex = static_cast<uint32_t>(index.size());

		index.push_back(oldIndex);
		newIndexToOrig.emplace_back(toOrig.begin(), toOrig.end());
		oldIndexToNew[oldIndex] = newIndex;

		memSize += kMetaDataSize[ctx.metaData[oldIndex].type] + sizeof(uint32_t);
	}
}

static void prepare(PackCtx &ctx, const std::vector<TaggedMetadata> &metaData,
                    const std::unordered_map<std::wstring, std::vector<uint32_t>> &articleIdToIndex)
{
	ctx.origIndexToNew.resize(metaData.size(), InvalidMetadataIndex);

	// generate new data by removing dupes
	std::unordered_map<TaggedMetadata, uint32_t> dupes;
	for (size_t index : indices(metaData))
	{
		const TaggedMetadata &data = metaData[index];
		if (data.type == eMetaUnknown || data.ptr == nullptr)
			continue;

		const uint32_t origIndex = static_cast<uint32_t>(index);
		auto it = dupes.find(data);
		if (it == dupes.end())
		{
			const uint32_t newIndex = static_cast<uint32_t>(ctx.metaData.size());
			dupes.emplace(data, newIndex);
			ctx.metaData.push_back(data);
			ctx.indexToOrig.push_back({ origIndex });
			ctx.origIndexToNew[origIndex] = newIndex;
		}
		else
		{
			const uint32_t newIndex = it->second;
			ctx.indexToOrig[newIndex].push_back(origIndex);
			ctx.origIndexToNew[origIndex] = newIndex;
		}
	}

	// to guarantee binary compat between different runs/platform we have to sort
	// articleIdToIndex map by article id
	using inner = decltype(articleIdToIndex.begin())::value_type;
	std::vector<std::reference_wrapper<const inner>> articleIdToIndexSorted(articleIdToIndex.begin(), articleIdToIndex.end());
	std::sort(articleIdToIndexSorted.begin(), articleIdToIndexSorted.end(),
			  [](const inner &lhs, const inner &rhs) { return lhs.first < rhs.first; });

	// build a bunch of indexes re: article ids
	ctx.origIndexToArticleId.resize(ctx.origIndexToNew.size());
	ctx.articleIdToOrigIndex.resize(articleIdToIndexSorted.size());
	for (const auto article : enumerate(articleIdToIndexSorted))
	{
		const uint32_t articleId = static_cast<uint32_t>(article.index);
		ctx.articleIdToOrigIndex[articleId] = article->get().second;
		for (const uint32_t origIndex : ctx.articleIdToOrigIndex[articleId])
		{
			if (ctx.origIndexToNew[origIndex] != InvalidMetadataIndex)
				ctx.origIndexToArticleId[origIndex] = articleId;
		}
	}

	ctx.indexToArticleId.resize(ctx.metaData.size());
	{
		std::unordered_set<uint32_t> articleIds;
		for (auto&& v : enumerate(ctx.indexToOrig))
		{
			articleIds.clear();

			for (uint32_t origIndex : *v)
				articleIds.insert(ctx.origIndexToArticleId[origIndex]);

			ctx.indexToArticleId[v.index].assign(articleIds.cbegin(), articleIds.cend());
		}
	}

	// build the index of truly shared metadata
	ctx.sharedMetadata.resize(ctx.metaData.size(), false);
	for (size_t index : indices(ctx.metaData))
		ctx.sharedMetadata[index] = ctx.indexToArticleId[index].size() > 1;
}

// the main packing routine
static void pack(const PackCtx &ctx, Resource &llc, ResourceBuilder &unique)
{
	// create a list of most used metadata and add them into their own resource
	struct UsageInfo {
		uint32_t index;
		unsigned count;
		UsageInfo(size_t index, size_t count) : index(uint32_t(index)), count(unsigned(count)) {}
	};
	std::vector<UsageInfo> mostUsed;
	for (size_t index : indices(ctx.metaData))
	{
		if (ctx.sharedMetadata[index])
			mostUsed.emplace_back(index, ctx.indexToArticleId[index].size());
	}
	std::sort(mostUsed.begin(), mostUsed.end(),
			  [](UsageInfo lhs, UsageInfo rhs) { return rhs.count < lhs.count; });

	for (const UsageInfo &info : mostUsed)
	{
		llc.addMetadata(ctx, info.index, ctx.indexToOrig[info.index]);
		if (llc.memSize > ctx.maxResourceSize * kLLCScaleFactor)
			break;
	}

	RateLimit rlimit;
	size_t addedArticlesCount = 0;
	std::vector<bool> addedArticles(ctx.articleIdToOrigIndex.size(), false);

	auto addArticle = [&](size_t articleId) {
		if (addedArticles[articleId])
			return;

		for (const uint32_t origIndex : ctx.articleIdToOrigIndex[articleId])
		{
			const uint32_t index = ctx.origIndexToNew[origIndex];
			if (index == InvalidMetadataIndex || llc.oldIndexToNew.find(index) != llc.oldIndexToNew.end())
				continue;

			unique.current().addMetadata(ctx, index, origIndex);
		}

		addedArticlesCount++;
		addedArticles[articleId] = true;
		if (rlimit.check())
			sld::printf("\r  added %lu articles", addedArticlesCount);
	};

	std::unordered_set<uint32_t> articles;
	std::vector<uint32_t> sortedArticles;

	sld::printf("\n");

	for (size_t articleId : indices(ctx.articleIdToOrigIndex))
	{
		addArticle(articleId);

		// the main idea here is to pull in articles which actually share metadata
		// with the articles already inside the current resource
		do {
			mostUsed.clear();
			const Resource &current = unique.current();
			for (auto&& idx : enumerate(current.index))
			{
				const uint32_t index = *idx;
				if (ctx.sharedMetadata[index] &&
					// not fully in the current resource
					current.newIndexToOrig[idx.index].size() != ctx.indexToOrig[index].size() &&
					// not in llc
					llc.oldIndexToNew.find(index) == llc.oldIndexToNew.end())
				{
					mostUsed.emplace_back(index, ctx.indexToArticleId[index].size());
				}
			}

			if (mostUsed.empty())
				break;

			// sort in ascending order so that we pack least shared metadata first
			// this is actually a win most of the time as we increase our chances of
			// having all of the articles wich share some metadata struct inside a single resource
			std::sort(mostUsed.begin(), mostUsed.end(),
					  [](UsageInfo lhs, UsageInfo rhs) { return rhs.count > lhs.count; });

			// this basically follows the previous comment by always choosing least shared metadata
			articles.clear();
			for (size_t i = 0, count = mostUsed.front().count;
			     i < mostUsed.size() && mostUsed[i].count == count;
			     i++)
			{
				for (uint32_t id : ctx.indexToArticleId[mostUsed[i].index])
				{
					if (!addedArticles[id])
						articles.insert(id);
				}
			}

			if (articles.empty())
				break;

			// sorting is required for binary compat between platforms/stdlibs/hash impls
			sortedArticles.assign(articles.begin(), articles.end());
			std::sort(sortedArticles.begin(), sortedArticles.end());
			for (uint32_t id : sortedArticles)
			{
				addArticle(id);
				if (unique.check(ctx.maxResourceSize))
					break;
			}
		} while (!unique.check(ctx.maxResourceSize));

		if (unique.check(ctx.maxResourceSize))
			unique.closeCurrent();
	}

	sld::printf("\r  added %lu articles", addedArticlesCount);
}

static size_t metadataCount(const std::list<ResourceBuilder> &builders)
{
	size_t count = 0;
	for (const ResourceBuilder &builder : builders) count += builder.metadataCount();
	return count;
}

// build resource list, index map and metadata array
static void finish(const PackCtx &ctx, const std::list<ResourceBuilder> &builders,
                   MetadataManager::OptimizedData &data)
{
	// build the new metadata index and resources
	std::vector<uint32_t> indexMap;
	indexMap.reserve(metadataCount(builders));

	data.m_indexMap.resize(ctx.origIndexToNew.size(), InvalidMetadataIndex);

	uint8_t slot = 0;
	for (const ResourceBuilder &builder : builders)
	{
		if (builder.metadataCount() == 0)
			continue;

		for (const Resource &res : builder.resources)
		{
			data.m_resources.emplace_back(indexMap.size(), res.index.size(), slot);
			indexMap.insert(indexMap.end(), res.index.begin(), res.index.end());
			const uint32_t offset = static_cast<uint32_t>(data.m_resources.back().start);
			for (auto&& mapping : enumerate(res.newIndexToOrig))
			{
				for (uint32_t origIndex : *mapping)
					data.m_indexMap[origIndex] = offset + static_cast<uint32_t>(mapping.index);
			}
		}
		slot++;
	}

	data.m_metaData.reserve(indexMap.size());
	for (size_t index : indices(indexMap))
		data.m_metaData.push_back(ctx.metaData[indexMap[index]]);
}

} // anon namespace

// statistics print out helpers
namespace {

static std::pair<size_t, size_t> totalResourcesSize(const std::list<ResourceBuilder> &builders)
{
	size_t metadataSize = 0;
	size_t indexSize = 0;
	for (const ResourceBuilder &builder : builders)
	{
		metadataSize += builder.memSize();
		indexSize += builder.metadataCount() * sizeof(uint32_t);
	}
	return{ metadataSize, metadataSize - indexSize };
}

static void printBuildersStats(const std::list<ResourceBuilder> &builders,
							   size_t origMetadataCount, size_t prevMetadataCount,
							   const char *const label)
{
	size_t currentMetadataCount = 0;
	for (const ResourceBuilder &builder : builders)
	{
		const size_t count = builder.metadataCount();
		if (count == 0)
			continue;

		currentMetadataCount += count;
		sldILog("   '%s' metadata count: %lu\n", builder.label, builder.metadataCount());
		sldILog("   '%s' resource count: %lu, size: ~%s\n",
				builder.label, builder.resources.size(), sld::fmt::MemSize{ builder.memSize() });
	}
	sldILog("   metadata count after '%s': %lu (%.02f%%", label,
			currentMetadataCount, float(currentMetadataCount) / float(origMetadataCount) * 100.f);
	if (currentMetadataCount > prevMetadataCount)
		sldILog(" +%lu [+%.02f%%]", currentMetadataCount - prevMetadataCount,
				(float(currentMetadataCount) / float(prevMetadataCount) - 1.0f) * 100.f);
	sldILog(")\n");
}

} // anon namespace

// returns "optimized" (packed) metadata
MetadataManager::Optimized MetadataManager::optimized() const
{
	// the returned structure
	Optimized ret;

	// serialize strings
	ret.m_embeddedStrings = strings::serialize(*this, m_metaData);
	// XXX: move it into the optimized data? effectively making optimized() destructive
	ret.m_strings = &m_strings;

	PackCtx ctx;

	sldILog("Packing structured metadata (max resource size: %s):\n",
			sld::fmt::MemSize{ ctx.maxResourceSize });
	sldILog("   initial metadata count: %lu\n", m_metaData.size());

	Stopwatch timer;

	// prepare the data for optimization
	prepare(ctx, m_metaData, m_metaDataArticleId);

	sldILog(" preparation (filtering out dupes) took %.02fms\n", timer.stop());
	sldILog("   unique metadata count before 'packing': %lu (%.02f%%)\n",
			ctx.metaData.size(), float(ctx.metaData.size()) / float(m_metaData.size()) * 100.f);
	size_t potentialMetaDataSize = 0;
	for (const TaggedMetadata &metaData : ctx.metaData)
		potentialMetaDataSize += kMetaDataSize[metaData.type];
	sldILog("   potential metadata total resource size: %s (without the index)\n",
			sld::fmt::MemSize{ potentialMetaDataSize });

	if (potentialMetaDataSize < ctx.maxResourceSize)
	{
		ret.m_metaData = std::move(ctx.metaData);
		ret.m_indexMap = std::move(ctx.origIndexToNew);
		ret.m_resources.emplace_back(0, ret.m_metaData.size(), 0);

		const size_t finalSize = potentialMetaDataSize + ret.m_metaData.size() * sizeof(uint32_t);
		sldILog(" metadata fits inside a single resource, final size with index: ~%s\n",
				sld::fmt::MemSize{ finalSize });
	}
	else
	{
		const size_t origMetaDataCount = ctx.metaData.size();
		timer.start();

		// the list of resource "builders"
		std::list<ResourceBuilder> builders;
		builders.emplace_back("llc");
		Resource& llc = builders.back().current();
		builders.emplace_back("unique");
		ResourceBuilder &unique = builders.back();

		// "pack" all the things!
		pack(ctx, llc, unique);

		for (ResourceBuilder &builder : builders)
			builder.finish();

		// print out some stats
		sldILog(" packing took: %.02fms\n", timer.stop());
		printBuildersStats(builders, m_metaData.size(), origMetaDataCount, "packing");

		// build resource list, index map and metadata array
		finish(ctx, builders, ret);

		// print out final metadata resource size
		const auto finalSize = totalResourcesSize(builders);
		sldILog(" total resource count after 'packing': %lu, size: %s (%s ",
				ret.m_resources.size(),
				sld::fmt::MemSize{ finalSize.first },
				sld::fmt::MemSize{ finalSize.second });
		if (finalSize.second > potentialMetaDataSize)
			sldILog("[+%s] ", sld::fmt::MemSize{ finalSize.second - potentialMetaDataSize });
		sldILog("without the index)\n");
	}

	// check integrity
	for (const auto data : enumerate(m_metaData))
	{
		// skip removed metadata
		if (data->type == eMetaUnknown || data->ptr == nullptr)
			continue;

		const uint32_t index = ret.m_indexMap[data.index];
		const TaggedMetadata &newData = ret.m_metaData[index];
		(void)newData; // silence unused variable warning in "release" builds
		assert(*data == newData && "Mismatched metadata after packing.");
	}

	return ret;
}

// returns the "new" metadata index for the given metadata reference
uint32_t MetadataManager::Optimized::getIndex(const MetadataRef &aRef) const
{
	const uint32_t oldIndex = aRef.index();
	if (oldIndex >= static_cast<uint32_t>(m_indexMap.size()))
		return InvalidMetadataIndex;

	const uint32_t newIndex = m_indexMap[oldIndex];
	if (newIndex == InvalidMetadataIndex)
		return RemovedMetadataIndex;

	const TaggedMetadata &data = m_metaData[newIndex];
	return aRef.checkType(data.type) ? newIndex : InvalidMetadataIndex;
}

void MetadataManager::Optimized::appendBlockString(const MetadataRef &aRef, std::wstring &aString) const
{
	const uint32_t oldIndex = aRef.index();
	if (oldIndex >= static_cast<uint32_t>(m_embeddedStrings.size()))
		return;

	const auto &embeddedString = m_embeddedStrings[aRef.index()];
	for (uint32_t idx : iterate(embeddedString.data.data(), embeddedString.count))
	{
		const sld::wstring_ref str = m_strings->get(idx);
		assert(!str.empty() && "Trying to add empty embedded metadata string!");

		aString.push_back(0x2); // STX
		aString.append(str.data(), str.size());
		aString.push_back(0x3); // ETX
	}
}

// generates the 'main' metadata header resource binary "blob"
MemoryBuffer MetadataManager::Optimized::generateHeader() const
{
	const size_t size = sizeof(sld::MetadataHeader) +   // main resource header
		sizeof(sld::MetaEnumInfo) * eMeta_Last +        // metadata info array
		sizeof(sld::ResourceInfo) * m_resources.size(); // resource info array
	
	MemoryBuffer blob(size);
	uint8_t *mem = (uint8_t*)blob.data();

	sld::MetadataHeader *header = (sld::MetadataHeader*)mem;
	header->_size = sizeof(*header);

	header->metadataInfoCount = eMeta_Last;
	header->metadataInfoSize = sizeof(sld::MetaEnumInfo);

	// find maximum resource slot
	uint8_t maxSlot = 0;
	for (const Resource &res : m_resources)
		maxSlot = std::max(maxSlot, res.slot);

	// check some limits
	assert(maxSlot < sld::MetadataHeader::MaxSlotCount);
	assert(m_metaData.size() < sld::MetadataHeader::MaxMetadataCount);

	header->resourceCount = static_cast<uint32_t>(m_resources.size());
	header->resourceInfoSize = sizeof(sld::ResourceInfo);
	header->resourceHeaderSize = sizeof(sld::ResourceHeader);
	header->maxSlot = maxSlot;
	header->stringsResourceType = 0;

	mem += header->_size;

	// setup metadata info array
	sld::MetaEnumInfo *metaInfo = (sld::MetaEnumInfo*)mem;
	for (int i = 0; i < eMeta_Last; i++)
		metaInfo[i].metadataStructSize = static_cast<uint32_t>(kMetaDataSize[i]);

	mem += header->metadataInfoCount * header->metadataInfoSize;

	// setup resource info array
	sld::ResourceInfo *resourceInfo = (sld::ResourceInfo*)mem;
	for (const auto resource : enumerate(m_resources))
	{
		resourceInfo[resource.index].endIndex = static_cast<uint32_t>(resource->start + resource->count);
		resourceInfo[resource.index].slot = resource->slot;
	}

	sldILog("'Primary' metadata header size: %s\n", sld::fmt::MemSize{ size });

	return blob;
}

// generates the binary "blob" for the metadata resource of the given index
MemoryBuffer MetadataManager::Optimized::generateResource(uint32_t index) const
{
	assert(index < m_resources.size());
	if (index >= m_resources.size())
		return MemoryBuffer();

	const size_t startIndex = m_resources[index].start;
	const size_t count = m_resources[index].count;

	// total metadata structs size
	size_t structsSize = 0;
	for (size_t i : xrange(count))
		structsSize += kMetaDataSize[m_metaData[startIndex + i].type];

	// the size of the "header" and the index at the start of a resource
	size_t headerIndexSize = sizeof(sld::ResourceHeader);

	// number of bits required to represent a metadata type
	const unsigned typeBits = GetCodeLen(eMeta_Last);
	headerIndexSize += BitStore::calcDataSize(typeBits, count);

	// number of bits required to represent metadata struct offset
	// may have to iterate a bit as offset size depends on the size of the offset table itself
	unsigned offsetBits = 0;
	size_t offsetsSize = 0;
	do
	{
		offsetBits = GetCodeLen(structsSize + headerIndexSize + offsetsSize);
		offsetsSize = BitStore::calcDataSize(offsetBits, count);
	} while (offsetBits != GetCodeLen(structsSize + headerIndexSize + offsetsSize));

	headerIndexSize += offsetsSize;

	MemoryBuffer blob(structsSize + headerIndexSize);
	uint8_t* data = blob.data();

	sld::ResourceHeader *header = (sld::ResourceHeader*)data;
	header->typeBits = typeBits;
	header->offsetBits = offsetBits;
	header->offsetsTableOffset = static_cast<uint32_t>(headerIndexSize - offsetsSize);

	BitStore types((uint32_t*)(data + sizeof(*header)), typeBits);
	BitStore offsets((uint32_t*)(data + header->offsetsTableOffset), offsetBits);
	uint8_t *binaryData = data + headerIndexSize;
	for (size_t i : xrange(count))
	{
		const TaggedMetadata& metaData = m_metaData[startIndex + i];

		types.store(i, metaData.type);
		offsets.store(i, static_cast<uint32_t>(binaryData - data));

		const size_t metaSize = kMetaDataSize[metaData.type];
		memcpy(binaryData, metaData.ptr, metaSize);
		binaryData += metaSize;
	}

	return blob;
}
