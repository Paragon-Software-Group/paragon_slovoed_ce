#include "CSSDataManager.h"

#include "Engine/SldCSSPropertyType.h"
#include "Engine/SldMacros.h"
#include "Engine/SldTypeDefs.h"

#include "Log.h"
#include "SlovoEdProject.h"
#include "Tools.h"

// the soft cap of a single resource after which we start outputting a notice (for now)
enum : size_t { SoftResourceSizeCap = 1 << 16 }; // 64kb

CSSDataManager::~CSSDataManager() {}

/**
 * Generates a string store with all of the css strings.
 */
StringStore CSSDataManager::generateStringStore(CompressionConfig aCompression, ThreadPool &aThreadPool) const
{
	if (m_strings.empty())
		return StringStore();

	// create a non indexable store
	StringStore stringStore = StringStore::compress(m_strings.strings(), aCompression, aThreadPool, 1 << 15, 0);

	// rewrite all of the string indexes
	const unsigned offsetBits = stringStore.maxOffsetIndexBits();
	const unsigned resourceIndexBits = stringStore.resourceIndexBits();
	// XXX: Add some checking here

	const auto indexMap = stringStore.buildDirectOffsetsTable();
	for (TSldCSSProperty *prop : m_data.props)
	{
		for (int i : xrange(prop->count))
		{
			TSldCSSValue &value = prop->values[i];
			if (value.type == TSldCSSValue::Url ||
				value.type == TSldCSSValue::String ||
				value.type == TSldCSSValue::SldStringLiteral)
			{
				const auto idx = indexMap[value.string.index];
				value.string.resource = idx.first;
				value.string.index = idx.second;
			}
		}
	}

	return stringStore;
}

/**
 * Returns resource type used by the string store associated with css or 0
 * if there are no strings
 */
uint32_t CSSDataManager::stringsResourceType() const
{
	return m_strings.size() ? SLD_RESOURCE_CSS_DATA_STRINGS : 0;
}

// struct used as a css block during css style parsing
struct CSSDataManager::CSSBlock
{
	Allocator &allocator;
	std::vector<TSldCSSProperty*> props;

	CSSBlock(Allocator &allocator) : allocator(allocator) {}

	// adds a new css property to the block
	void addProperty(ESldCSSPropertyType type, const std::vector<TSldCSSValue> &values, bool important = false)
	{
		TSldCSSProperty *prop = createProperty(type, values.data(), values.size(), important);
		addProperty(prop);
	}

	// adds a new css property to the block
	void addProperty(ESldCSSPropertyType type, const TSldCSSValue &value, bool important = false)
	{
		TSldCSSProperty *prop = createProperty(type, &value, 1, important);
		addProperty(prop);
	}

	// adds a new css property to the block
	// if we already have a property of the same type erases it
	void addProperty(TSldCSSProperty *prop)
	{
		if (prop == nullptr)
			return;

		// look through already present properties to find if we already have it
		// the properties are unique and we can't have 2 of the same type, the last one wins
		auto it = std::find_if(props.begin(), props.end(),
		                       [prop](const TSldCSSProperty *aProp) { return prop->type == aProp->type; });
		if (it != props.end())
			props.erase(it);

		props.push_back(prop);
	}

	// creates a new property with the given values
	TSldCSSProperty* createProperty(ESldCSSPropertyType type, const TSldCSSValue *values, size_t valueCount, bool important)
	{
		if (valueCount == 0)
			return nullptr;

		// create a property struct and set the value count
		const size_t size = sizeof(TSldCSSProperty) + sizeof(TSldCSSValue) * valueCount;
		TSldCSSProperty *prop = (TSldCSSProperty*)allocator.allocate(size);
		prop->type = type;
		prop->important = important;
		prop->count = static_cast<uint8_t>(valueCount);

		// copy the values in
		for (size_t i : xrange(valueCount))
			prop->values[i] = values[i];

		return prop;
	}
};

// css block / data helpers
namespace {
// local typedef for css blocks
typedef CSSDataManager::CSSBlock CSSBlock;

// stores a struct inside the relevant data structures (or not if we already have it's copy)
// returns an index in the data store
template <typename T>
static uint32_t store(T *val, std::vector<T*> &vec, CSSDataManager::ProxyHashMap<T> &map)
{
	const sld::HashProxy<T> hashVal(val);
	const auto &fit = map.find(hashVal);
	if (fit != map.end())
		return fit->second;

	const uint32_t index = static_cast<uint32_t>(vec.size());
	vec.push_back(val);
	map.emplace(sld::HashProxy<T>(val), index);

	return index;
}

// adds a new css block adding all of the properties from it
static uint32_t addCSSBlock(CSSBlock &aBlock, CSSDataManager::Data &aData)
{
	// create a property block struct with "real" property indexes
	const size_t size = sizeof(TSldCSSPropertyBlock) + sizeof(uint32_t) * aBlock.props.size();
	TSldCSSPropertyBlock *block = (TSldCSSPropertyBlock*)aBlock.allocator.allocate(size);
	block->count = static_cast<uint32_t>(aBlock.props.size());
	for (const auto prop : enumerate(aBlock.props))
		block->props[prop.index] = store(*prop, aData.props, aData.propsMap);

	return store(block, aData.blocks, aData.blocksMap);
}

// merges a css block at @aIndex in @aData into @aBlock
static void mergeCSSBlock(CSSBlock &aBlock, CSSDataManager::Data &aData, uint32_t aIndex)
{
	assert(aIndex < aData.blocks.size());
	TSldCSSPropertyBlock *block = aData.blocks[aIndex];

	aBlock.props.reserve(aBlock.props.size() + block->count);
	for (uint16_t i : xrange(block->count))
		aBlock.addProperty(aData.props[block->props[i]]);
}

} // anon namespace

// context used for parsing css styles
class CSSDataManager::ParseContext
{
public:
	ParseContext(CSSDataManager &mgr, sld::wstring_ref str, CImageContainer &imageContainer, sld::wstring_ref file = L"[inline]")
		: mgr(mgr), images(imageContainer), str(str), pos(0), len(0), line_(1), column_(1), file_(file)
	{}

	ParseContext(ParseContext &ctx, sld::wstring_ref str)
		: mgr(ctx.mgr), images(ctx.images), str(str), pos(0), len(0)
	{}

	inline uint32_t addString(sld::wstring_ref aString) {
		return mgr.addString(aString);
	}

	inline uint32_t addImageUrl(sld::wstring_ref aString) {
		return images.AddImage(aString);
	}

	// parsing functions
	wchar_t current() const
	{
		return get_char(pos + len);
	}

	wchar_t peek() const
	{
		return get_char(pos + len + 1);
	}

	wchar_t get()
	{
		size_t cur = pos + len;
		if (cur >= str.size())
			return '\0';

		if (str[cur] == '\n')
		{
			line_++;
			column_ = 1;
		}
		else
		{
			column_++;
		}

		len++;
		return get_char(cur);
	}

	sld::wstring_ref consume()
	{
		sld::wstring_ref ret = str.substr(pos, len);
		pos += len;
		len = 0;
		return ret;
	}

	sld::wstring_ref peek(size_t num) const
	{
		return str.substr(pos + len, num);
	}

	sld::wstring_ref get(size_t num)
	{
		size_t lpos = pos + len;
		for (size_t i = 0; i < num; i++)
			get();
		return str.substr(lpos, pos + len - lpos);
	}

	sld::wstring_ref consume(size_t num)
	{
		get(num);
		return consume();
	}

	sld::wstring_ref string() const { return str.substr(pos + len); }

	unsigned line() const { return line_; }
	unsigned column() const { return column_; }
	const std::wstring& file() const { return file_; }

	// returns a "context" - source line for the given line & column pair
	sld::wstring_ref context(unsigned line, unsigned column) const
	{
		assert(line <= line_ && column <= column_);
		if (line == 0 || column == 0 || line > line_ || column > column_)
			return sld::wstring_ref();

		size_t next_nl = str.substr(pos + len).find('\n');
		if (next_nl != sld::wstring_ref::npos)
			next_nl += pos + len;

		sld::wstring_ref ctx = str.substr(0, next_nl);

		size_t last_nl = ctx.find_last_of('\n');
		for (unsigned real_line = line_; real_line != line; real_line--)
		{
			ctx = ctx.substr(0, last_nl);
			last_nl = ctx.find_last_of('\n');
		}

		return ctx.substr(last_nl + 1);
	}

private:
	inline wchar_t get_char(size_t pos) const
	{
		return pos < str.size() ? str[pos] : '\0';
	}

	CSSDataManager &mgr;
	CImageContainer &images;

	// string
	const sld::wstring_ref str;
	// absolute position in a string
	size_t pos;
	// length of the 'current substring'
	unsigned len;
	// current line
	unsigned line_;
	// current column (not utf-aware)
	unsigned column_;
	// currently parsed file "name"
	std::wstring file_;
};

// css style parsing
namespace {

typedef CSSDataManager::ParseContext ParseContext;

// private errors
enum : int { InvalidCSSValue = -1, InvalidCSSPropName = -2 };

// supporting css parsing logic
namespace parser {

// parse table
namespace {

static const unsigned chr_class_wspace   = 1 << 0;
static const unsigned chr_class_alpha    = 1 << 1;
static const unsigned chr_class_number   = 1 << 2;
static const unsigned chr_class_nonascii = 1 << 3;
static const unsigned chr_class_newline  = 1 << 4;

static const uint8_t char_class[256] = {
       0,    0,    0,    0,    0,    0,    0,    0,    0,  0x1, 0x10,    0, 0x10, 0x10,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
     0x1,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
     0x4,  0x4,  0x4,  0x4,  0x4,  0x4,  0x4,  0x4,  0x4,  0x4,    0,    0,    0,    0,    0,    0,
       0,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,
     0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,    0,    0,    0,    0,    0,
       0,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,
     0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,  0x2,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
       0,    0,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,
     0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,
     0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,
     0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,
     0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,  0x8,
};

static inline bool test(wchar_t chr, unsigned flags)
{
	return ((static_cast<unsigned>(chr) < 128 ? char_class[static_cast<unsigned>(chr)] : char_class[128]) & flags) != 0;
}

} // anon namespace

static inline bool is_nmstart(wchar_t chr)
{
	// nmstart   [_a-z]|{nonascii}|{escape}
	// we dont support {escape} for now
	return chr == '_' || test(chr, chr_class_alpha | chr_class_nonascii);
}

static inline bool is_nmchar(wchar_t chr)
{
	// nmchar    [_a-z0-9-]|{nonascii}|{escape}
	// we dont support {escape} for now
	return chr == '-' || chr == '_' || test(chr, chr_class_alpha | chr_class_number | chr_class_nonascii);
}

static inline bool is_propend(wchar_t chr)
{
	return chr == ';' || chr == '}' || chr == '\n';
}

static inline bool is_selector_start(wchar_t chr)
{
	return is_nmstart(chr) || chr == '-' || chr == '.' || chr == '*' || chr == ':';
}

// a single token parsed from a css "stream"
struct Token
{
	sld::wstring_ref string;
	unsigned line;
	unsigned column;

	Token() : line(0), column(0) {}
	Token(const ParseContext &ctx) : line(ctx.line()), column(ctx.column()) {}

	bool empty() const { return string.empty(); }
	explicit operator bool() const { return !empty(); }
	operator sld::wstring_ref() const { return string; }
};

static inline bool operator==(const Token &lhs, const wchar_t *rhs) { return lhs.string == rhs; }
static inline bool operator!=(const Token &lhs, const wchar_t *rhs) { return lhs.string != rhs; }

/**
 * parse_ident: tries to parse a valid css ident from @ctx
 * Returns a string reference containing a valid css ident or an empty one otherwise.
 */
static Token parse_ident(ParseContext &ctx)
{
	//     ident     [-]?{nmstart}{nmchar}*
	//     nmstart   [_a-z]|{nonascii}|{escape}
	//     nmchar    [_a-z0-9-]|{nonascii}|{escape}
	Token token(ctx);

	if (ctx.current() == '-' && is_nmstart(ctx.peek()))
		ctx.get();

	if (!is_nmstart(ctx.current()))
		return token;
	ctx.get();

	while (is_nmchar(ctx.current()))
		ctx.get();

	token.string = ctx.consume();
	return token;
}

static void skip_comment_block(ParseContext &ctx)
{
	if (ctx.peek(2) != L"/*")
		return;

	ctx.get(2);
	while (ctx.current() && ctx.peek(2) != L"*/")
		ctx.get();
	ctx.get(2);
}

/**
 * consume_whitespace: consumes css whitespace and comments
 *
 * @ctx: parse context reference
 * @consume_newlines: flag if we should treat newlines as whitespace
 */
static void consume_whitespace(ParseContext &ctx, bool consume_newlines = true)
{
	const unsigned flags = consume_newlines ? chr_class_wspace | chr_class_newline : chr_class_wspace;

	while (ctx.current())
	{
		skip_comment_block(ctx);

		if (!test(ctx.current(), flags))
			break;

		ctx.get();
	}

	ctx.consume();
}

// consumes 'current' character if it's equal to @chr
static bool consume_char(ParseContext &ctx, wchar_t chr)
{
	if (ctx.current() == chr)
	{
		ctx.consume(1);
		return true;
	}
	return false;
}

// parses a single string argument
static void parse_proparg_string(ParseContext &ctx)
{
	const wchar_t quote = ctx.current();
	do
	{
		if (ctx.current() == '\\' && ctx.peek() == quote)
			ctx.get();
		ctx.get();
	} while (ctx.current() && ctx.current() != quote);

	if (ctx.current() == quote)
		ctx.get();
}

// parses a single function argument
static void parse_proparg_func(ParseContext &ctx)
{
	assert(ctx.current() == '(');
	while (ctx.current() && ctx.current() != ')')
	{
		if (ctx.current() == '"' || ctx.current() == '\'')
			parse_proparg_string(ctx);
		else
			ctx.get();
	}
}

static inline bool is_propargend(wchar_t chr, wchar_t separator)
{
	return is_propend(chr) || test(chr, chr_class_wspace) || chr == separator;
}

// parses a single argument
static Token parse_prop_argument(ParseContext &ctx, wchar_t separator = ' ')
{
	Token token(ctx);
	while (ctx.current() && !is_propargend(ctx.current(), separator))
	{
		if (ctx.current() == '"' || ctx.current() == '\'')
			parse_proparg_string(ctx);
		else if (ctx.current() == '(')
			parse_proparg_func(ctx);
		else
			ctx.get();
	}
	token.string = ctx.consume();
	return token;
}

// parses a property arguments string
// stops calling callback after a first error return but tries to parse all of the
// property values *always*
template <typename Callback>
static int parse_prop_arguments(ParseContext &ctx, Callback &cb, wchar_t separator = ' ')
{
	consume_whitespace(ctx, false);

	int err = ERROR_NO;
	while (ctx.current() && !is_propend(ctx.current()))
	{
		const Token value = parse_prop_argument(ctx, separator);
		if (value.empty())
			err = InvalidCSSValue;

		consume_whitespace(ctx, false);

		if (err == ERROR_NO)
			err = cb(ctx, value);
	}
	return err;
}

static void skip_prop_arguments(ParseContext &ctx)
{
	static const auto dummy = [](ParseContext&, parser::Token) { return ERROR_NO; };
	parser::parse_prop_arguments(ctx, dummy);
}

// parses a selector name
static Token parse_selector_name(ParseContext &ctx)
{
	Token token(ctx);
	// NOTE: support only universal selectors with classes (for now)
	if (ctx.current() == '*' || ctx.current() == '.')
	{
		// universal selector or a start of simple selector
		consume_char(ctx, '*');
		token.string = L"*";
	}
	return token;
}

// small struct representing a css selector - very barebones
// and conceptually it's a `SelectorToken`
struct Selector
{
	Token name;
	Token class_name;
};

static Selector parse_selector(ParseContext &ctx)
{
	Selector selector;

	// skip leading whitespace
	consume_whitespace(ctx);

	// selector name
	selector.name = parse_selector_name(ctx);

	// selector class
	if (parser::consume_char(ctx, '.'))
		selector.class_name = parser::parse_ident(ctx);

	// skip trailing whitespace
	consume_whitespace(ctx);

	return selector;
}

static void parse_selectors_group(ParseContext &ctx, std::vector<Selector> &selectors)
{
	// skip leading whitespace
	consume_whitespace(ctx);

	while (ctx.current() && ctx.current() != '{')
	{
		// eat a comma if we have one
		consume_char(ctx, ',');

		if (ctx.current() == '\0')
			return;

		const Selector selector = parse_selector(ctx);
		if (selector.name.empty() || selector.class_name.empty())
		{
			// skip till the next selector or the block
			while (ctx.current() && ctx.current() != '{' && ctx.current() != ',')
				ctx.get();
		}

		// push the selector in any case the caller will figure it out
		selectors.push_back(selector);
	}

	// skip trailing whitespace
	consume_whitespace(ctx);
}

} // namespace parser

// the main css logging function
static void logWarning(const ParseContext &ctx, const parser::Token &token, fmt::CStringRef fmt, fmt::ArgList args)
{
	sldXLog("Warning! [CSS] %s:%u:%u: ", sld::as_ref(ctx.file()), token.line, token.column);
	CLogW::InstanceXmlLog().Log(fmt, std::move(args));

	const sld::wstring_ref context = ctx.context(token.line, token.column);
	if (!context.empty())
	{
		// XXX: We may "trim" very long context strings a bit
		sldXLog("\n    `%s`", context);
		sldXLog("\n     %*s\n", token.column, "^");
	}
	else
	{
		sldXLog("\n");
	}
}
FMT_VARIADIC(void, logWarning, const ParseContext&, const parser::Token&, fmt::CStringRef)

// valid values mask
enum ValidValuesFlags : unsigned {
	NonNegative = 0,
	IntOnly
};

// descriptor for a single "simple" css property
struct PropertyDescriptor
{
	// css property name
	const wchar_t *name;
	// type
	ESldCSSPropertyType type;
	// the mask of valid css values for this property
	uint32_t validValuesMask;
	// valid value flags (#ValidValuesFlags)
	uint32_t validValuesFlags;
	// flag if property can have multiple "simple" values
	bool multiValue;
	// the count of enums in the next field
	uint16_t enumCount;
	// an array of supported enum strings
	const wchar_t *const *enums;
};

// desciptor for a property that declares 4 'sides' of a property at once
struct FourSidedPropertyDescriptor
{
	// the name of the property
	const wchar_t* name;
	// the 4 'sides' of a property in [ top, right, bottom, left ] order
	const PropertyDescriptor* sides[4];
};

// css property argument (value) helpers/parsers
namespace values {

// One note about the usage of ParseContext here. Ideally accessing the string through string()
// is a no-no. But the current setup passes already parsed css property arguments to value
// handler[s]. So the context contains *only* the prop argument string.
// So all this "raw" access is fine (if a little bit fugly)

// small value parsing helpers
namespace {

/**
 * custom stoul
 *
 * @str: the reference to the parsed string
 * @base: number base [only <=10 and 16 are supported]
 * @length: number of characters consumed from @str
 *
 * Returns a number parsed from str.
 * Compared to stoul (and friends):
 * - does not skip whitespace
 * - does not handle '-' at the beginning
 * - does not handle different `0x` and the like specifiers
 */
template <typename charT>
static uint64_t stou64(sld::basic_string_ref<charT> str, unsigned base = 10, size_t *length = nullptr)
{
	assert(base <= 10 || base == 16);

	uint64_t number = 0;
	if (length != nullptr)
		*length = 0;

	while (str.size() > 0)
	{
		if (static_cast<uint64_t>(str[0] - '0') <= 9)
			number = base * number + (str[0] - '0');
		else if (static_cast<uint64_t>((str[0] | ' ') - 'a') <= 5 && base == 16)
			number = base * number + ((str[0] | ' ') - 'a' + 10);
		else // return the number on first non digit
			return number;

		str.remove_prefix(1);
		if (length != nullptr)
			(*length)++;
	}

	return number;
}

static sld::wstring_ref parseString(sld::wstring_ref str, bool stripQuotes = false)
{
	if (str.size() < 2)
		return sld::wstring_ref();

	const wchar_t quote = str.front();
	if ((quote == '"' || quote == '\'') && str.ends_with(quote))
		return stripQuotes ? str.substr(1, str.size() - 2) : str;
	return sld::wstring_ref();
}

static sld::wstring_ref parseFunc(sld::wstring_ref str, sld::wstring_ref func)
{
	if (!str.starts_with(func))
		return sld::wstring_ref();

	str.remove_prefix(func.size());
	if (!(str.starts_with('(') && str.ends_with(')')))
		return sld::wstring_ref();

	return str.substr(1, str.size() - 2);
}

static bool parseColorValue(ParseContext &ctx, uint8_t *value)
{
	bool negative = false;
	if (ctx.current() == '-')
	{
		negative = true;
		ctx.consume(1);
	}

	size_t len;
	uint64_t val = stou64(ctx.string(), 10, &len);
	if (len == 0)
		return false;

	if (negative)
		val = 0;
	else if (val > 0xff)
		val = 0xff;

	ctx.consume(len);

	if (ctx.current() == '%')
	{
		val = val < 100 ? static_cast<uint64_t>(val * 2.55f) : 255;
		ctx.consume(1);
	}
	*value = static_cast<uint8_t>(val);

	parser::consume_whitespace(ctx);

	if (!parser::consume_char(ctx, ',') && ctx.current() != ')')
		return false;

	parser::consume_whitespace(ctx);
	return true;
}

// scales value to be used as a "fractional" value inside the engine
static uint64_t scaleFract(uint64_t value, size_t valueLength, uint32_t valueStep, uint32_t valueMax)
{
	const uint32_t maxPow10 = valueStep * valueMax;
	static const uint32_t pow10s[] = { 1, 10, 100, 1000, 10 * 1000, 100 * 1000, 1000 * 1000 };
	size_t pow10Index = 0;
	for (const auto pow10 : enumerate(pow10s))
	{
		if (*pow10 == maxPow10)
		{
			pow10Index = pow10.index;
			break;
		}
	}
	if (pow10Index == 0)
		return 0;

	if (valueLength < pow10Index)
	{
		value *= pow10s[pow10Index - valueLength];
	}
	else if (valueLength > pow10Index)
	{
		value /= pow10s[valueLength - pow10Index];
	}
	return value / valueStep;
}
struct Number {
	uint32_t sign : 1;
	uint32_t fractional : 31;
	uint32_t integer;
};
static bool parseNumber(sld::wstring_ref str, Number *num, uint32_t step, uint32_t max)
{
	uint32_t sign = 0;
	if (str.starts_with('-'))
	{
		sign = 1;
		str.remove_prefix(1);
	}

	size_t len;
	uint64_t integer = stou64(str, 10, &len);
	if (len == 0 && !str.starts_with('.'))
		return false;

	str.remove_prefix(len);
	// parse fractional part
	uint64_t fractional = 0;
	if (str.starts_with('.'))
	{
		str.remove_prefix(1);
		if (str.empty())
			return false; // treat "n." or "n.-" as malformed

		fractional = stou64(str, 10, &len);
		if (len == 0)
			return false;

		str.remove_prefix(len);
		fractional = scaleFract(fractional, len, step, max);
	}

	num->sign = sign;
	num->integer = static_cast<uint32_t>(integer);
	num->fractional = static_cast<uint32_t>(fractional);
	return true;
}

static inline bool isImportant(const parser::Token &token) { return token == L"!important"; }

}

static inline bool supports(const PropertyDescriptor &aProp, TSldCSSValue::Type aType)
{
	return (aProp.validValuesMask & (1 << aType)) != 0;
}

static inline bool checkFlags(const PropertyDescriptor &aProp, ValidValuesFlags aFlag)
{
	return (aProp.validValuesFlags & (1 << aFlag)) != 0;
}

static bool color(ParseContext &ctx, const PropertyDescriptor &aProp, TSldCSSValue &aValue)
{
	if (!supports(aProp, TSldCSSValue::Color))
		return false;

	if (ctx.string() == L"transparent")
	{
		aValue.type = TSldCSSValue::Color;
		aValue.color.u32 = 0;
		return true;
	}

	union {
		struct { uint8_t a, b, g, r; };
		uint32_t u32;
	} color = {};
	color.a = TSldCSSValue::Color_AlphaMax;

	bool parsed = false;
	if (ctx.current() == '#')
	{
		// #abc or #aabbcc
		ctx.consume(1);
		if (ctx.string().size() != 3 && ctx.string().size() != 6)
			return false;

		const size_t numLen = ctx.string().size() == 6 ? 2 : 1;
		// XXX: Check for invalid values?
		color.r = static_cast<uint8_t>(stou64(ctx.consume(numLen), 16));
		color.g = static_cast<uint8_t>(stou64(ctx.consume(numLen), 16));
		color.b = static_cast<uint8_t>(stou64(ctx.consume(numLen), 16));

		if (numLen == 1)
		{
			color.r += color.r << 4;
			color.g += color.g << 4;
			color.b += color.b << 4;
		}

		parsed = true;
	}
	else if (ctx.string().starts_with(L"rgb") && ctx.string().ends_with(')'))
	{
		// rgb([0..255], [0..255], [0..255]) or rgb([0..100]%, [0..100]%, [0..100]%)
		// rgba([0..255], [0..255], [0..255], [0..1]) or rgb([0..100]%, [0..100]%, [0..100]%, [0..1])
		ctx.consume(wcslen(L"rgb"));
		const bool hasAlpha = parser::consume_char(ctx, 'a');
		if (!parser::consume_char(ctx, '('))
			return false;

		parsed = parseColorValue(ctx, &color.r) &&
		         parseColorValue(ctx, &color.g) &&
		         parseColorValue(ctx, &color.b);

		if (parsed && hasAlpha)
		{
			Number number;
			if (!parseNumber(ctx.string(), &number, TSldCSSValue::Color_AlphaStep, TSldCSSValue::Color_AlphaMax))
				return false;

			// clamp the number to [0..1]
			if (number.sign) // values < 0
				number.fractional = 0;
			if (number.integer >= 1) // values >= 1
				number.fractional = TSldCSSValue::Color_AlphaMax;
			color.a = static_cast<uint8_t>(number.fractional);
		}
	}

	if (parsed)
	{
		aValue.type = TSldCSSValue::Color;
		aValue.color.u32 = color.u32;
	}
	return parsed;
}

static bool enumeration(ParseContext &ctx, const PropertyDescriptor &aProp, TSldCSSValue &aValue)
{
	if (!supports(aProp, TSldCSSValue::Enum))
		return false;

	for (uint16_t i : xrange(aProp.enumCount))
	{
		if (aProp.enums[i] == ctx.string())
		{
			aValue.type = TSldCSSValue::Enum;
			aValue.enumeration.u32 = i;
			return true;
		}
	}
	return false;
}

static bool lengths(ParseContext &ctx, const PropertyDescriptor &aProp, TSldCSSValue &aValue)
{
	if (!(supports(aProp, TSldCSSValue::Length) || supports(aProp, TSldCSSValue::Percentage)))
		return false;

	static const auto checkValue = [](const PropertyDescriptor &prop, const TSizeValue &size) {
		return checkFlags(prop, ValidValuesFlags::NonNegative) ? size.Value >= 0 : true;
	};

	const TSizeValue size = ParseSizeValue(ctx.string());
	if (size.Units == eMetadataUnitType_percent)
	{
		if (supports(aProp, TSldCSSValue::Percentage) && checkValue(aProp, size))
		{
			aValue.type = TSldCSSValue::Percentage;
			aValue.percentage.value = size.Value;
			return true;
		}
	}
	else if (supports(aProp, TSldCSSValue::Length))
	{
		if (size.Value == 0)
		{
			aValue.type = TSldCSSValue::Number;
			// as the values a zero initialzied we dont have to do anything
			return true;
		}
		else if (size.IsValid() && checkValue(aProp, size))
		{
			aValue.type = TSldCSSValue::Length;
			aValue.length.units = size.Units;
			aValue.length.value = size.Value;
			return true;
		}
	}
	return false;
}

static bool named(const PropertyDescriptor &aProp, TSldCSSValue &aValue, TSldCSSValue::Type aType)
{
	if (!supports(aProp, aType))
		return false;
	aValue.type = aType;
	return true;
}

static bool number(ParseContext &ctx, const PropertyDescriptor &aProp, TSldCSSValue &aValue)
{
	if (!supports(aProp, TSldCSSValue::Number))
		return false;

	Number number;
	if (!parseNumber(ctx.string(), &number, TSldCSSValue::Number_FracStep, TSldCSSValue::Number_FracMax))
		return false;

	if ((checkFlags(aProp, ValidValuesFlags::NonNegative) && number.sign) ||
		 (checkFlags(aProp, ValidValuesFlags::IntOnly) && number.fractional))
		return false;

	aValue.type = TSldCSSValue::Number;
	aValue.number.sign = number.sign;
	aValue.number.integer = static_cast<uint32_t>(number.integer);
	aValue.number.fractional = static_cast<uint32_t>(number.fractional);

	return true;
}

static bool string(ParseContext &ctx, const PropertyDescriptor &aProp, TSldCSSValue &aValue)
{
	if (!supports(aProp, TSldCSSValue::String))
		return false;

	sld::wstring_ref string = parseString(ctx.string());
	if (string.empty())
		return false;

	aValue.type = TSldCSSValue::String;
	aValue.string.index = ctx.addString(string);
	return true;
}

static bool url(ParseContext &ctx, const PropertyDescriptor &aProp, TSldCSSValue &aValue)
{
	if (!supports(aProp, TSldCSSValue::Url))
		return false;

	sld::wstring_ref url = parseFunc(ctx.string(), L"url");
	if (url.empty())
		return false;

	aValue.type = TSldCSSValue::Url;
	aValue.url.index = ctx.addString(url);
	return true;
}

static bool imageUrl(ParseContext &ctx, const PropertyDescriptor &aProp, TSldCSSValue &aValue)
{
	if (!supports(aProp, TSldCSSValue::SldImageUrl))
		return false;

	sld::wstring_ref url = parseString(parseFunc(ctx.string(), L"sld-image-url"), true);
	if (url.empty())
		return false;

	aValue.type = TSldCSSValue::SldImageUrl;
	aValue.imageUrl.index = ctx.addImageUrl(url);
	return true;
}

// the main value parsing function
// expects the context to contain *only* the property argument string
static bool parse(ParseContext ctx, const PropertyDescriptor &aProp, TSldCSSValue &aValue)
{
	// initial and inherit are supported by all css properties
	const sld::wstring_ref value = ctx.string();
	if (value == L"initial")
	{
		aValue.type = TSldCSSValue::Initial;
		return true;
	}
	else if (value == L"inherit")
	{
		aValue.type = TSldCSSValue::Inherit;
		return true;
	}
	// check 'named' css values with conditional support
	else if (value == L"auto")
		return named(aProp, aValue, TSldCSSValue::Auto);
	else if (value == L"none")
		return named(aProp, aValue, TSldCSSValue::None);

	// check different property value types (the order *DOES* matter)
	if (enumeration(ctx, aProp, aValue) ||
	       imageUrl(ctx, aProp, aValue) ||
	            url(ctx, aProp, aValue) ||
	        lengths(ctx, aProp, aValue) ||
	         number(ctx, aProp, aValue) ||
	          color(ctx, aProp, aValue) ||
	         string(ctx, aProp, aValue))
	{
		return true;
	}

	// as a last resort check if the prop supports string "literals"
	if (supports(aProp, TSldCSSValue::SldStringLiteral))
	{
		aValue.type = TSldCSSValue::SldStringLiteral;
		aValue.string.index = ctx.addString(value);
		return true;
	}

	return false;
}

} // namespace values

// Include the autogenerated properties descriptors
namespace props {

// the autogenerated file uses this macro for defining valid value masks
#define VMASK(_TYPE) (1 << TSldCSSValue::_TYPE)
#define FMASK(_TYPE) (1 << ValidValuesFlags::_TYPE)

#include "CSSProperties.incl"

static_assert(sld::array_size(simpleProps) == eSldCSSPropertyType_Count,
			  "Mismatched property descriptors table.");

#undef VMASK
#undef FMASK

} // namespace props

namespace props {

// small helper to log 'invalid css value' messages and return the relevant error
static int logInvalidCSSValue(const ParseContext &ctx, const PropertyDescriptor &aProp, const parser::Token &aValue, const char *info = nullptr)
{
	logWarning(ctx, aValue, "unsupported/invalid `%s` property value: %s %s",
			   sld::as_ref(aProp.name),
			   aValue.empty() ? "" : ("`" + sld::as_utf8(aValue) + "`"),
			   info ? info : "");
	return InvalidCSSValue;
}

// parses a "simple" css property (with a single possible value)
static int simpleSingle(ParseContext &ctx, const PropertyDescriptor &aProp, CSSBlock &aBlock)
{
	const parser::Token propValue = parser::parse_prop_argument(ctx);
	if (propValue.empty())
		return logInvalidCSSValue(ctx, aProp, propValue, "[no value given]");

	parser::consume_whitespace(ctx, false);

	const parser::Token important = parser::parse_prop_argument(ctx);
	if (important)
	{
		parser::consume_whitespace(ctx, false);
		if (!values::isImportant(important) || (ctx.current() && !parser::is_propend(ctx.current())))
			return logInvalidCSSValue(ctx, aProp, important, "[the property supports only a single value]");
	}

	TSldCSSValue value = {};
	if (!values::parse(ParseContext(ctx, propValue), aProp, value))
		return logInvalidCSSValue(ctx, aProp, propValue);

	aBlock.addProperty(aProp.type, value, bool(important));
	return ERROR_NO;
}

// parses a "simple" css property (with multiple possible values)
static int simpleMulti(ParseContext &ctx, const PropertyDescriptor &aProp, CSSBlock &aBlock)
{
	struct Callback {
		const PropertyDescriptor &prop;
		std::vector<TSldCSSValue> values;
		bool important;

		Callback(const PropertyDescriptor &aProp) : prop(aProp), important(false) {}
		int operator()(ParseContext &ctx, const parser::Token &propValue)
		{
			if (values::isImportant(propValue))
			{
				if (important)
					return logInvalidCSSValue(ctx, prop, propValue, "[duplicate '!important']");
				important = true;
				return ERROR_NO;
			}

			TSldCSSValue value = {};
			if (!values::parse(ParseContext(ctx, propValue), prop, value))
				return logInvalidCSSValue(ctx, prop, propValue);
			values.push_back(value);
			return ERROR_NO;
		}
	} callback(aProp);

	parser::Token dummy(ctx);
	int err = parser::parse_prop_arguments(ctx, callback);
	if (err != ERROR_NO)
		return err;
	if (callback.values.empty())
		return logInvalidCSSValue(ctx, aProp, dummy, "[no values given]");

	aBlock.addProperty(aProp.type, callback.values, callback.important);
	return ERROR_NO;
}

// parses a property that declares 4 'sides' of a property at once
static int fourSided(ParseContext &ctx, const FourSidedPropertyDescriptor &aProp, CSSBlock &aBlock)
{
	// make a local prop descriptor copy to pass around value parsing functions
	PropertyDescriptor prop = *aProp.sides[0];
	prop.name = aProp.name;

	struct Callback {
		const PropertyDescriptor &prop;
		TSldCSSValue values[4];
		int count;
		bool important;

		Callback(const PropertyDescriptor &aProp)
			: prop(aProp), count(0), important(false) { memset(values, 0, sizeof(values)); }

		int operator()(ParseContext &ctx, const parser::Token &propValue)
		{
			if (values::isImportant(propValue))
			{
				if (important)
					return logInvalidCSSValue(ctx, prop, propValue, "[duplicate '!important']");
				important = true;
				return ERROR_NO;
			}

			if (count < 4)
			{
				if (!values::parse(ParseContext(ctx, propValue), prop, values[count]))
					return logInvalidCSSValue(ctx, prop, propValue);
			}
			count++;
			return ERROR_NO;
		}
	} callback(prop);

	parser::Token dummy(ctx);
	int err = parser::parse_prop_arguments(ctx, callback);
	if (err != ERROR_NO)
		return err;
	if (callback.count == 0)
		return logInvalidCSSValue(ctx, prop, dummy, "[no values given]");
	if (callback.count > 4)
		return logInvalidCSSValue(ctx, prop, dummy, "[more than 4 values given]");

	// the mapping from 'side' to value index per number of values
	// the sides in each row are 'top', 'right', 'bottom', 'left'
	static const uint32_t mapping[4][4] = {
		{ 0, 0, 0, 0 },
		{ 0, 1, 0, 1 },
		{ 0, 1, 2, 1 },
		{ 0, 1, 2, 3 },
	};
	const int index = callback.count - 1;
	for (int i : xrange(4))
	{
		const TSldCSSValue &value = callback.values[mapping[index][i]];
		aBlock.addProperty(aProp.sides[i]->type, value, callback.important);
	}

	return ERROR_NO;
}

// an array of properties that need truly custom handling (like 'border' and the like)
static const struct {
	const wchar_t* name;
	int(*parse)(ParseContext &ctx, CSSDataManager::CSSBlock &aBlock);
} customProps[] = {
	{ nullptr, nullptr }
};

// the main property parsing function
static int parse(ParseContext &ctx, const parser::Token &aName, CSSBlock &aBlock)
{
	// check 'custom' props first
	for (const auto &prop : customProps)
	{
		if (aName == prop.name)
			return prop.parse(ctx, aBlock);
	}
	// check 'four sided' props next
	for (const FourSidedPropertyDescriptor *prop : fourSidedProps)
	{
		if (aName == prop->name)
			return fourSided(ctx, *prop, aBlock);
	}
	// check 'simple' props last
	for (const PropertyDescriptor *prop : simpleProps)
	{
		if (prop && aName == prop->name)
			return prop->multiValue ? simpleMulti(ctx, *prop, aBlock) : simpleSingle(ctx, *prop, aBlock);
	}

	// log an error and consume all of the properties
	logWarning(ctx, aName, "unsupported css property: `%s`", sld::as_ref(aName));
	parser::skip_prop_arguments(ctx);
	return InvalidCSSPropName;
}

} // namespace props

// normalizes the css style string by removing all of the whitespace and
// semicolons from the beginning and end
static sld::wstring_ref normalizeStyleString(sld::wstring_ref string)
{
	static const auto is_junk = [](wchar_t chr) {
		return parser::test(chr, parser::chr_class_wspace) || parser::is_propend(chr);
	};

	while (string.size() && is_junk(string.front()))
		string.remove_prefix(1);

	while (string.size() && is_junk(string.back()))
		string.remove_suffix(1);

	return string;
}

// normalizes the css class string by removing all of the whitespace from begin & end
static sld::wstring_ref normalizeClassString(sld::wstring_ref string)
{
	while (string.size() && parser::test(string.front(), parser::chr_class_wspace))
		string.remove_prefix(1);

	while (string.size() && parser::test(string.back(), parser::chr_class_wspace))
		string.remove_suffix(1);

	return string;
}

// parses a single css property block (either inline or in a file)
static void parsePropertyBlock(ParseContext &ctx, CSSBlock &aBlock)
{
	parser::consume_whitespace(ctx);
	if (parser::consume_char(ctx, '{'))
		parser::consume_whitespace(ctx);

	while (ctx.current() && ctx.current() != '}')
	{
		const char *error = nullptr;

		const parser::Token propName = parser::parse_ident(ctx);
		if (propName.empty())
		{
			error = "invalid property name";
		}
		else
		{
			parser::consume_whitespace(ctx, false);
			if (parser::consume_char(ctx, ':'))
			{
				parser::consume_whitespace(ctx, false);
				// don't bother checking the return (for now)
				// simply skip "invalid" properties adding all of the valid ones to the blocks
				props::parse(ctx, propName, aBlock);
			}
			else
			{
				error = "expected ':'";
			}
		}

		if (error)
		{
			// log the error and skip everything 'till the property end
			logWarning(ctx, propName, "malformed property declaration: %s", error);
			parser::skip_prop_arguments(ctx);
		}

		// skip the trailing ';'
		parser::consume_char(ctx, ';');

		// skip trailing whitespace
		parser::consume_whitespace(ctx);
	}

	// skip the trailing '}' if any
	parser::consume_char(ctx, '}');
}

// parses a single css class name
static parser::Token parseClassName(ParseContext &ctx)
{
	const parser::Token className = parser::parse_ident(ctx);
	if (className.empty())
	{
		logWarning(ctx, className, "unexpected character, expected css class name");
		return className;
	}

	parser::consume_whitespace(ctx);
	return className;
}

} // anon namespace

/**
 * parses css style attributes (class & style) returning their css block index
 *
 * @param [in] aClass          - css 'class' attribute string
 * @param [in] aStyle          - css 'style' attribute string
 * @param [in] aImageContainer - image container, d'uh, for... images
 *
 * @return a pair of:
 *    index of the css block (or InvalidCSSBlockIndex on error)
 *    flag if there were unknown css classes inside the class string
 */
std::pair<uint32_t, bool> CSSDataManager::parseCSSAttribs(sld::wstring_ref aClass, sld::wstring_ref aStyle,
										 CImageContainer &aImageContainer)
{
	CSSBlock cssBlock(allocator);

	aStyle = normalizeStyleString(aStyle);
	aClass = normalizeClassString(aClass);

	bool unknownClassName = false;
	if (aClass.size())
	{
		const auto fit = m_classStyles.find(aClass);
		if (fit == m_classStyles.end())
		{
			ParseContext ctx(*this, aClass, aImageContainer, L"[class]");
			do
			{
				const parser::Token className = parseClassName(ctx);
				if (className.empty())
					break;

				auto idx = m_classIndex.find(className);
				if (idx == m_classIndex.end())
				{
					unknownClassName |= true;
				}
				else
				{
					idx->second.useCount++;
					if (idx->second.index != InvalidCSSBlockIndex)
						m_cssIndexesBuffer.push_back(idx->second.index);
				}
			} while (ctx.current());

			/// Браузеры, при слиянии стилей ориентируются на последовательность стилей, заданную при их объявлении,
			/// а не на порядок в атрибуте "class"
			std::sort(m_cssIndexesBuffer.begin(), m_cssIndexesBuffer.end());
			for (const auto index : m_cssIndexesBuffer)
			{
				mergeCSSBlock(cssBlock, m_classData, index);
			}
			m_cssIndexesBuffer.clear();
		}
		else
		{
			if (aStyle.empty())
				return{ fit->second, unknownClassName };
			mergeCSSBlock(cssBlock, m_data, fit->second);
		}
	}
	const bool hasClassProps = !cssBlock.props.empty();

	if (aStyle.size())
	{
		auto fit = m_inlineStyles.find(aStyle);
		if (fit == m_inlineStyles.end())
		{
			ParseContext ctx(*this, aStyle, aImageContainer, L"[inline]");
			parsePropertyBlock(ctx, cssBlock);
		}
		else
		{
			if (!hasClassProps)
				return{ fit->second, unknownClassName };
			mergeCSSBlock(cssBlock, m_data, fit->second);
		}
	}

	if (cssBlock.props.empty())
		return{ InvalidCSSBlockIndex, unknownClassName };

	const uint32_t index = addCSSBlock(cssBlock, m_data);
	if (!hasClassProps)
		m_inlineStyles.emplace(strdup(aStyle), index);
	else if (hasClassProps && aStyle.empty())
		m_classStyles.emplace(strdup(aClass), index);
	return{ index, unknownClassName };
}

/**
 * parses a css *file* string
 *
 * @param [in] aContents       - a string containing the full *contents* of a css file
 * @param [in] aImageContainer - image container, d'uh, for... images
 * @param [in] aFileName       - file name, used only for logging purposes
 *
 * @return error code [always returns ERROR_NO for now]
 */
int CSSDataManager::parseCSSFile(sld::wstring_ref aContents, CImageContainer &aImageContainer, sld::wstring_ref aFileName)
{
	ParseContext ctx(*this, aContents, aImageContainer, aFileName);
	while (ctx.current())
	{
		parser::consume_whitespace(ctx);

		std::vector<parser::Selector> selectors;
		parser::parse_selectors_group(ctx, selectors);
		if (selectors.empty())
			logWarning(ctx, parser::Token(ctx), "unexpected character, expected selector start");

		if (ctx.current() != '{')
		{
			logWarning(ctx, parser::Token(ctx), "unexpected character, expected '{'");
			break;
		}

		for (const parser::Selector &selector : selectors)
		{
			if (selector.name.empty() || selector.class_name.empty())
				logWarning(ctx, selector.name, "unsupported selector name & pseudoclass combination");
		}

		// parse properties
		CSSBlock cssBlock(allocator);
		parsePropertyBlock(ctx, cssBlock);

		uint32_t index = InvalidCSSBlockIndex;
		for (const parser::Selector &selector : selectors)
		{
			if (selector.name.empty() || selector.class_name.empty())
				continue;

			auto it = m_classIndex.find(selector.class_name);

			// special case for empty css blocks
			// if we don't add them to the map we will be spammed with "unknown css classes"
			// later on if we encounter them as classes in articles
			// use InvalidCSSBlockIndex as a "flag" for empty css blocks
			if (cssBlock.props.empty())
			{
				if (it == m_classIndex.end())
					m_classIndex.emplace(strdup(selector.class_name), InvalidCSSBlockIndex);
				continue;
			}

			if (it == m_classIndex.end())
			{
				if (index == InvalidCSSBlockIndex)
					index = addCSSBlock(cssBlock, m_classData);
				m_classIndex.emplace(strdup(selector.class_name), index);
			}
			else if (it->second.index == InvalidCSSBlockIndex)
			{
				it->second.index = addCSSBlock(cssBlock, m_classData);
			}
			else
			{
				CSSBlock block(allocator);
				mergeCSSBlock(block, m_classData, it->second.index);
				for (TSldCSSProperty *prop : cssBlock.props)
					block.addProperty(prop);
				it->second.index = addCSSBlock(block, m_classData);
			}
		}
	}

	return ERROR_NO;
}

// rewrites image urls in 'internalUrl' values
void CSSDataManager::rewriteImageUrls(const AddedFileResourceMap &aImageMap)
{
	for (TSldCSSProperty *prop : m_data.props)
	{
		for (int i : xrange(prop->count))
		{
			TSldCSSValue &value = prop->values[i];
			if (value.type == TSldCSSValue::SldImageUrl)
				value.imageUrl.index = aImageMap[value.imageUrl.index];
		}
	}
}

// returns unused css classes
std::vector<std::wstring> CSSDataManager::unusedCSSClasses() const
{
	std::vector<std::wstring> ret;
	for (const auto &it : m_classIndex)
	{
		if (it.second.useCount == 0)
			ret.push_back(to_string(it.first));
	}
	return ret;
}

// returns the total number of css properties resources
uint32_t CSSDataManager::propsResourceCount() const
{
	return m_data.props.empty() ? 0 : 1;
}

// returns the total number of css blocks resources
uint32_t CSSDataManager::blocksResourceCount() const
{
	return m_data.blocks.empty() ? 0 : 1;
}

// generates the 'main' css data header resource binary "blob"
MemoryBuffer CSSDataManager::generateHeader() const
{
	const size_t size = sizeof(TSldCSSDataHeader) + // main resource header
		sizeof(TSldCSSResourceDescriptor) * propsResourceCount() + // props descriptors
		sizeof(TSldCSSResourceDescriptor) * blocksResourceCount(); // blocks descriptors

	MemoryBuffer blob(size);
	uint8_t *mem = blob.data();

	TSldCSSDataHeader *header = (TSldCSSDataHeader*)mem;
	header->_size = sizeof(*header);

	header->resourceDescriptorSize = sizeof(TSldCSSResourceDescriptor);
	header->resourceHeaderSize = sizeof(TSldCSSResourceHeader);
	header->propsResourceCount = propsResourceCount();
	header->blocksResourceCount = blocksResourceCount();
	header->propsIndexBits = GetCodeLen(m_data.props.size());
	header->stringsResourceType = stringsResourceType();

	mem += header->_size;

	// setup props descriptors "array"
	TSldCSSResourceDescriptor *props = (TSldCSSResourceDescriptor*)mem;
	props[0].endIndex = static_cast<uint32_t>(m_data.props.size());

	mem += sizeof(*props) * header->propsResourceCount;

	// setup blocks descriptors "array"
	TSldCSSResourceDescriptor *blocks = (TSldCSSResourceDescriptor*)mem;
	blocks[0].endIndex = static_cast<uint32_t>(m_data.blocks.size());

	// Spew this info here
	sldILog("'Primary' css data header size: %s\n", sld::fmt::MemSize{ size });
	sldILog("  unique css props count: %lu; unique css blocks count: %lu\n",
			m_data.props.size(), m_data.blocks.size());

	return blob;
}

// helper for generating resource blobs
template <typename Writer>
MemoryBuffer generateResourceBlob(size_t start, size_t end, const Writer &writer)
{
	// calculate resource sizes
	const size_t headerSize = sizeof(TSldCSSResourceHeader);

	size_t dataSize = 0;
	for (size_t i : xrange(start, end))
		dataSize += writer.structSize(i);

	size_t offsetsSize = 0;
	unsigned offsetBits = 0;
	do
	{
		offsetBits = GetCodeLen(dataSize + headerSize + offsetsSize);
		offsetsSize = BitStore::calcDataSize(offsetBits, end - start);
	} while (offsetBits != GetCodeLen(dataSize + headerSize + offsetsSize));

	const size_t totalResourceSize = headerSize + offsetsSize + dataSize;

	MemoryBuffer blob(totalResourceSize);
	uint8_t *data = blob.data();

	TSldCSSResourceHeader *header = (TSldCSSResourceHeader*)data;
	header->offsetBits = offsetBits;

	BitStore offsets((uint32_t*)(data + headerSize), offsetBits);
	uint8_t *blobs = data + headerSize + offsetsSize;
	for (size_t i : xrange(end - start))
	{
		offsets.store(i, static_cast<uint32_t>(blobs - data));
		blobs += writer.write(blobs, start + i);
	}

	return blob;
}

static void logResource(const char *name, uint32_t index, size_t count, size_t size)
{
	sldILog("  css %s resource[%u]: count: %lu, size: %s\n",
			name, index, count, sld::fmt::MemSize{ size });
	if (size > SoftResourceSizeCap)
		sldILog("  NOTE: please contact engine team (as it's >%s)\n",
				sld::fmt::MemSize{ SoftResourceSizeCap });
}

// generates the binary "blob" for the css property resource of the given index
MemoryBuffer CSSDataManager::generatePropsResource(uint32_t index) const
{
	assert(index < propsResourceCount());
	if (index >= propsResourceCount())
		return MemoryBuffer();

	// fow now write everything inside a single resource

	struct PropWriter
	{
		const decltype(m_data.props) &props;

		size_t structSize(size_t index) const { return struct_size(props[index]); }

		size_t write(uint8_t *data, size_t index) const
		{
			const TSldCSSProperty* prop = props[index];
			const size_t size = struct_size(prop);
			memcpy(data, prop, size);
			return size;
		}
	} writer = { m_data.props };

	MemoryBuffer ret = generateResourceBlob(0, m_data.props.size(), writer);

	logResource("props", index, m_data.props.size(), ret.size());

	return ret;
}

// generates the binary "blob" for the css blocks resource of the given index
MemoryBuffer CSSDataManager::generateBlocksResource(uint32_t index) const
{
	assert(index < blocksResourceCount());
	if (index >= blocksResourceCount())
		return MemoryBuffer();

	// fow now write everything inside a single resource

	struct BlockWriter
	{
		const decltype(m_data.blocks) &blocks;
		unsigned propsIndexBits;

		size_t structSize(size_t index) const
		{
			const TSldCSSPropertyBlock *block = blocks[index];
			return sizeof(*block) + BitStore::calcDataSize(propsIndexBits, block->count);
		}

		size_t write(uint8_t *data, size_t index) const
		{
			const TSldCSSPropertyBlock* src = blocks[index];
			TSldCSSPropertyBlock* dst = (TSldCSSPropertyBlock*)data;
			dst->count = src->count;

			BitStore props(dst->props, propsIndexBits);
			for (uint16_t i : xrange(src->count))
				props.store(i, src->props[i]);

			return structSize(index);
		}
	} writer = { m_data.blocks, GetCodeLen(m_data.props.size()) };

	MemoryBuffer ret = generateResourceBlob(0, m_data.blocks.size(), writer);

	logResource("blocks", index, m_data.blocks.size(), ret.size());

	return ret;
}
