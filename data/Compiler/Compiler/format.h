#pragma once

#define FMT_USE_WINDOWS_H 0
#include "fmt/format.h"
#include "fmt/printf.h"

#include "string_ref.h"

namespace sld {

inline string_ref as_ref(string_ref s) { return s; }
inline wstring_ref as_ref(wstring_ref s) { return s; }

namespace fmt {

using namespace ::fmt;

struct MemSize { size_t size; };
void format_arg(BasicFormatter<char> &f, const char *&format_str, const MemSize &sz);

} // namespace fmt

inline void format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const string_ref &string)
{
	const fmt::StringRef str(string.data(), string.size());
	f.format(format_str, fmt::internal::MakeArg<std::decay<decltype(f)>::type>(str));
}

void format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const wstring_ref &string);

} // namespace sld
