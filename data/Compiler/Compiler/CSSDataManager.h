#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "allocators.h"
#include "sld_Types.h"
#include "string_ref.h"
#include "StringStore.h"
#include "Tools.h"

#include "Engine/SldMetadataTypes.h"

class AddedFileResourceMap;
class CImageContainer;
class ThreadPool;

enum : uint32_t { InvalidCSSBlockIndex = ~0U };

static inline size_t struct_size(const TSldCSSProperty *prop) {
	return sizeof(*prop) + sizeof(prop->values[0]) * prop->count;
}

static inline size_t struct_size(const TSldCSSPropertyBlock *block) {
	return sizeof(*block) + sizeof(block->props[0]) * block->count;
}

// HashProxy for hashing css property/block pointers
namespace sld {

template <typename T>
struct HashProxy {
	const T *ptr;

	HashProxy() : ptr(nullptr) {}
	HashProxy(const T *ptr) : ptr(ptr) {}
};

static inline bool equals(const TSldCSSProperty *lhs, const TSldCSSProperty *rhs)
{
	if (lhs->type != rhs->type || lhs->count != rhs->count)
		return false;
	return memcmp(lhs, rhs, struct_size(lhs)) == 0;
}

static inline bool equals(const TSldCSSPropertyBlock *lhs, const TSldCSSPropertyBlock *rhs)
{
	return lhs->count == rhs->count && memcmp(lhs, rhs, struct_size(lhs)) == 0;
}

template <typename T>
inline bool operator==(const HashProxy<T> &lhs, const HashProxy<T> &rhs)
{
	return lhs.ptr == rhs.ptr || (lhs.ptr && rhs.ptr && equals(lhs.ptr, rhs.ptr));
}

} // namespace sld

// HashProxy support for unordered maps
namespace std {
template <typename T>
struct hash<sld::HashProxy<T>> {
	size_t operator()(const sld::HashProxy<T> &v) const {
		return v.ptr ? fnv1a::hash(v.ptr, struct_size(v.ptr)) : 0;
	}
};
} // namespace std

class CSSDataManager
{
	// allocate in 16kb pages
	enum : size_t { AllocPageSize = 1 << 14 };

public:
	CSSDataManager() : allocator(AllocPageSize) {};
	~CSSDataManager();

	// make it noncopyable to simplify reasoning about data lifetimes
	CSSDataManager(const CSSDataManager&) = delete;
	const CSSDataManager& operator=(const CSSDataManager&) = delete;

	// generates a list of all strings
	StringStore generateStringStore(CompressionConfig aCompression, ThreadPool &aThreadPool) const;

	// returns the type of the resource used to store css strings
	uint32_t stringsResourceType() const;

	// parses css style attributes (class & style) returning their css block index
	std::pair<uint32_t, bool> parseCSSAttribs(sld::wstring_ref aClass, sld::wstring_ref aStyle, CImageContainer &aImageContainer);

	// parses a css *file* string
	int parseCSSFile(sld::wstring_ref aContents, CImageContainer &aImageContainer, sld::wstring_ref aFileName = sld::wstring_ref());

	// rewrites all the indices for internal urls containing images urls
	void rewriteImageUrls(const AddedFileResourceMap &aImageMap);

	// returns unused css classes
	std::vector<std::wstring> unusedCSSClasses() const;

	// returns the total number of css properties resources
	uint32_t propsResourceCount() const;
	// returns the total number of css blocks resources
	uint32_t blocksResourceCount() const;

	// generates the 'main' css data header resource binary "blob"
	MemoryBuffer generateHeader() const;
	// generates the binary "blob" for the css properties resource of the given index
	MemoryBuffer generatePropsResource(uint32_t index) const;
	// generates the binary "blob" for the css blocks resource of the given index
	MemoryBuffer generateBlocksResource(uint32_t index) const;

	class ParseContext;
	struct CSSBlock;

	template <typename T>
	using ProxyHashMap = std::unordered_map<sld::HashProxy<T>, uint32_t>;

	typedef sld::BumpPtrAllocator Allocator;

	// stores css props and blocks
	struct Data {
		// properties
		std::vector<TSldCSSProperty*> props;
		ProxyHashMap<TSldCSSProperty> propsMap;
		// blocks
		std::vector<TSldCSSPropertyBlock*> blocks;
		ProxyHashMap<TSldCSSPropertyBlock> blocksMap;
	};

	struct ClassIndex {
		uint32_t index; // InvalidCSSBlockIndex for 'classes' with empty blocks
		uint32_t useCount;
		ClassIndex(uint32_t index) : index(index), useCount(0) {}
	};

private:

	// interns a given string
	sld::wstring_ref strdup(sld::wstring_ref aString) { return m_strdupStrings.strdup(aString); }

	// adds a string returning its index
	uint32_t addString(sld::wstring_ref aString) { return m_strings.insert(aString); }

private:
	// data for managing css strings
	sld::StringRefMap m_strings;
	sld::StringRefStore m_strdupStrings;

	// css props/blocks from css files
	Data m_classData;
	// css class -> index map
	std::unordered_map<sld::wstring_ref, ClassIndex> m_classIndex;

	// added css blocks/properties
	Data m_data;

	// hashing style strings is faster than parsing...
	std::unordered_map<sld::wstring_ref, uint32_t> m_inlineStyles;
	// css class string -> index map
	std::unordered_map<sld::wstring_ref, uint32_t> m_classStyles;

	/// css indexes buffer for sorting
	std::vector<uint32_t> m_cssIndexesBuffer;

	// used to allocate all of the props/blocks
	Allocator allocator;
};
