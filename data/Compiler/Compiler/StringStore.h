#pragma once

#include <cstddef>
#include <cstdint>
#include <exception>
#include <unordered_map>
#include <vector>

#include "sld_Types.h"
#include "string_ref.h"
#include "util.h"

class ThreadPool;

// to be moved somewhere generic
namespace sld {

class exception : public std::exception
{
public:
	explicit exception(int error) : error_(error) {}
	int error() const { return error_; }
private:
	int error_;
};

} // namespace sld

class StringStore
{
public:
	// string store building flags
	enum Flags : unsigned {
		// indexable string store where each string gets an index (position inside
		// the source strings vector) and can be accessed through it; otherwise strings
		// can be accessed only directly through the resource index + offset pair
		Indexable = 1u << 0,
	};

	StringStore() = default;

	// returns the total generated resource count
	uint32_t resourceCount() const;
	// returns a memory blob for the given resource index
	MemoryBuffer getResourceBlob(uint32_t aIndex) const;

	// returns the bit count needed to encode a resource index for any string
	unsigned resourceIndexBits() const;
	// returns the bit count needed to encode an offset for any string
	unsigned maxOffsetIndexBits() const;
	// returns a table containg <resource index, offset> for all encoded string
	std::vector<std::pair<uint32_t, uint32_t>> buildDirectOffsetsTable() const;

	// creates a new string store from the source string array
	static StringStore compress(const std::vector<sld::wstring_ref> &aStrings,
								CompressionConfig aCompression, ThreadPool &aThreadPool,
								size_t aMaxResourceSize, unsigned aFlags = Indexable);

	// creates a new simple string store from a source string
	static MemoryBuffer compress(sld::wstring_ref aString, CompressionConfig aCompression, ThreadPool &aThreadPool);

	struct Resource {
		MemoryBuffer data;
		std::vector<uint32_t> offsets;
	};
private:
	std::vector<Resource> m_data;
	MemoryBuffer m_header;
};
