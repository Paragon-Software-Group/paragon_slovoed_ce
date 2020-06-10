#pragma once

#include <array>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "allocators.h"
#include "sld_Types.h"
#include "Tools.h"
#include "util.h"

#include "Engine/SldMetadataTypes.h"

class ThreadPool;

enum : uint32_t {
	InvalidMetadataIndex = ~0U,
	// can be returned *only* from the Optimized::getIndex()
	RemovedMetadataIndex = InvalidMetadataIndex - 1
};

namespace sld {
namespace detail {

// base metadata reference class that supports type consistency checking in debug builds
class MetadataRefBase
{
public:
#ifdef NDEBUG
	MetadataRefBase(ESldStyleMetaTypeEnum) {}

	bool checkType(ESldStyleMetaTypeEnum aType) const { return true; }
#else
	MetadataRefBase(ESldStyleMetaTypeEnum aType) : _type(aType) {}

	bool checkType(ESldStyleMetaTypeEnum aType) const { return _type == aType; }
private:
	ESldStyleMetaTypeEnum _type;
#endif
};

} // namespace sld
} // namespace detail

// the metadata "reference"
class MetadataRef final : protected sld::detail::MetadataRefBase
{
public:
	MetadataRef() : MetadataRefBase(eMetaUnknown), _index(InvalidMetadataIndex) {}
	MetadataRef(ESldStyleMetaTypeEnum aType, uint32_t aIndex) : MetadataRefBase(aType), _index(aIndex) {}

	uint32_t index() const {
		return _index;
	}

	explicit operator bool() const {
		return _index != InvalidMetadataIndex;
	}

	using MetadataRefBase::checkType;
private:
	uint32_t _index;
};

class MetadataManager
{
	// allocate in 2mb chunks
	enum : size_t { AllocPageSize = 1 << 21 };

public:
	MetadataManager() : allocator(AllocPageSize) {}

	// make it noncopyable to simplify reasoning about metadata lifetimes
	MetadataManager(const MetadataManager&) = delete;
	const MetadataManager& operator=(const MetadataManager&) = delete;

	// adds a string returning a reference to it
	TMetadataString addString(sld::wstring_ref aString);

	// returns a string by reference
	sld::wstring_ref getString(const TMetadataString &aString) const;

	// adds a metadata struct returning a reference to it
	template <typename T, ESldStyleMetaTypeEnum dataType = T::metaType>
	MetadataRef addMetadata(const T &aData, const std::wstring &aArticleId) {
		static_assert(dataType < eMeta_Last, "Invalid metadata type.");
		uint32_t index = addMetadata(dataType, &aData, aArticleId);
		return MetadataRef(dataType, index);
	}

	// simple wrapper around metadata pointers returned from findMetadata()
	// to shield us from possible bugs in case someone decides to delete (or free()) the ptr
	template <ESldStyleMetaTypeEnum Enum, bool IsConst>
	class meta_ref {
		static_assert(Enum < eMeta_Last, "Invalid metadata type.");
	public:
		using value_type = typename SldMetadataTraits<Enum>::struct_type;
		using reference = typename std::conditional<IsConst, const value_type&, value_type&>::type;
		using pointer = typename std::conditional<IsConst, const value_type*, value_type*>::type;

		explicit meta_ref() : _ptr(nullptr) {}
		explicit meta_ref(pointer p) : _ptr(p) {}

		meta_ref& operator=(std::nullptr_t) {
			_ptr = nullptr;
			return *this;
		}

		explicit operator bool() const { return _ptr != nullptr; }

		reference operator*() const { return *_ptr; }
		pointer operator->() const { return _ptr; }

	private:
		pointer _ptr;
	};

	// finds metadata by the given reference
	template <ESldStyleMetaTypeEnum Enum, typename Ref = meta_ref<Enum, true>>
	Ref findMetadata(const MetadataRef &aRef) const {
		return aRef.checkType(Enum) ? Ref(findMetadata(aRef.index(), Enum).get<Enum>()) : Ref();
	}
	template <ESldStyleMetaTypeEnum Enum, typename Ref = meta_ref<Enum, false>>
	Ref findMetadata(const MetadataRef &aRef) {
		return aRef.checkType(Enum) ? Ref(findMetadata(aRef.index(), Enum).get<Enum>()) : Ref();
	}

	// "removes" the metadata by the given reference
	bool removeMetadata(MetadataRef &aRef);

	size_t metadataCount() const { return m_metaData.size(); }

	// basically a tagged pointer for metadata structs
	struct TaggedMetadata
	{
		TaggedMetadata() : type(eMetaUnknown), ptr(nullptr) {}
		TaggedMetadata(ESldStyleMetaTypeEnum type, void *data) : type(type), ptr(data) {}

		template <ESldStyleMetaTypeEnum Enum, typename T = typename SldMetadataTraits<Enum>::struct_type>
		T* get() const {
			return type == Enum ? static_cast<T*>(ptr) : nullptr;
		}

		ESldStyleMetaTypeEnum type;
		void *ptr;
	};

	struct OptimizedData
	{
		struct Resource {
			size_t start;
			size_t count;
			uint8_t slot;
			Resource(size_t start, size_t count, uint8_t slot)
				: start(start), count(count), slot(slot) {}
		};

		struct EmbeddedString {
			std::array<uint32_t, 4> data;
			uint32_t count;
			uint32_t length;
		};

		std::vector<TaggedMetadata> m_metaData;
		std::vector<uint32_t> m_indexMap;
		std::vector<EmbeddedString> m_embeddedStrings;
		std::vector<Resource> m_resources;
		const sld::StringRefMap *m_strings;
	};

	// class containing "optimized" (for some definition of) metadata
	class Optimized : protected OptimizedData
	{
		friend class MetadataManager;
	public:
		// returns the "new" metadata index for the given metadata reference
		uint32_t getIndex(const MetadataRef &aRef) const;

		// appends the embedded string to the block string
		void appendBlockString(const MetadataRef &aRef, std::wstring &aString) const;

		// returns the total 'packed' metadata count
		size_t metadataCount() const { return m_metaData.size(); }
		// returns the total resource count
		uint32_t resourceCount() const { return uint32_t(m_resources.size()); }

		// generates the 'main' metadata header resource binary "blob"
		MemoryBuffer generateHeader() const;
		// generates the binary "blob" for the metadata resource of the given index
		MemoryBuffer generateResource(uint32_t index) const;
	};

	// returns "optimized" metadata
	Optimized optimized() const;

private:

	uint32_t addMetadata(ESldStyleMetaTypeEnum aType, const void *aData, const std::wstring &aArticleId);

	const TaggedMetadata& findMetadata(uint32_t index, ESldStyleMetaTypeEnum type) const;

	// data for managing metadata strings
	sld::StringRefMap m_strings;

	// metadata
	std::vector<TaggedMetadata> m_metaData;
	// the mapping from article id to metadata index
	std::unordered_map<std::wstring, std::vector<uint32_t>> m_metaDataArticleId;

	sld::BumpPtrAllocator allocator;
};

// Checks if the external key can be marked resolved
static inline bool checkExtKey(const TMetadataExtKey &aKey, const MetadataManager *aMgr)
{
	return aMgr->getString(aKey.DictId).size() && aMgr->getString(aKey.Key).size();
}
