#include "SldHTMLBuilder.h"

#include "SldDictionary.h"
#include "SldIntFormatter.h"
#include "SldMacros.h"
#include "SldMetadataParser.h"

// local macro to save on the typing
#define U16(_X) SldU16StringRefLiteral(_X)

#ifdef SLD_HAS_CONSTEXPR
#  define ARRAY_SIZE(_X) (sld2::array_size)(_X)
#else
#  define ARRAY_SIZE(_X) (sizeof(_X) / sizeof((_X)[0]))
#endif

/**
 * ISldHTMLBuilder
 */

namespace {

template <UInt32 N>
inline SldU16StringRef to_string(const SldU16StringRef(&aData)[N], UInt32 aValue, SldU16StringRef aDefault = nullptr)
{
	return aValue < N ? aData[aValue] : aDefault;
}

} // anon namespace

namespace sld2 {
namespace html {

SldU16StringRef toString(ESldStyleFontNameEnum aName)
{
	static const SldU16StringRef data[] = {
		U16("DejaVu Sans"),
		U16("Lucida Sans"),
		U16("Verdana"),
		U16("Georgia"),
		U16("Arial Rounded MT Bold"),
		U16("DejaVu Serif"),
		U16("Helvetica"),
		U16("Source Sans Pro"),
		U16("Gentium"),
		U16("Merriweather"),
		U16("Merriweather Sans"),
		U16("Noto Sans"),
		U16("Noto Serif"),
		U16("Trajectum"),
		U16("CombiNumerals Ltd"),
		U16("Charis SIL"),
		U16("Helvetica Neue"),
		U16("Times New Roman"),
		U16("Lyon Text"),
		U16("Atlas Grotesk"),
		U16("1234 Sans"),
		U16("Augean"),
		U16("Courier New"),
		U16("Wittenberger"),
		U16("Kruti Dev 010"),
		U16("Win Innwa"),
		U16("Myriad Pro Cond"),
		U16("PhoneticTM"),
		U16("Symbol"),
	};
	static_assert(ARRAY_SIZE(data) == eFontName_Symbol + 1, "Needs an update!");
	return ::to_string(data, aName, U16("DejaVu Sans"));
}

SldU16StringRef toString(EBorderStyle aStyle)
{
	static const SldU16StringRef data[] = {
		U16("none"),
		U16("hidden"),
		U16("dotted"),
		U16("dashed"),
		U16("solid"),
		U16("double"),
		U16("groove"),
		U16("ridge"),
		U16("inset"),
		U16("outset")
	};
	static_assert(ARRAY_SIZE(data) == eBorderStyleOutset + 1, "Needs an update!");
	return ::to_string(data, aStyle, U16("none"));
}

SldU16StringRef toString(ESldTextAlignEnum aAlign)
{
	static const SldU16StringRef data[] = {
		U16("left"),
		U16("center"),
		U16("right"),
		U16("justify")
	};
	static_assert(ARRAY_SIZE(data) == eTextAlign_Justify + 1, "Needs an update!");
	return ::to_string(data, aAlign, U16("inherit"));
}

SldU16StringRef toString(ESldVerticalTextAlignEnum aVAlign)
{
	static const SldU16StringRef data[] = {
		U16("middle"),
		U16("top"),
		U16("bottom")
	};
	static_assert(ARRAY_SIZE(data) == eVerticalTextAlign_Bottom + 1, "Needs an update!");
	return ::to_string(data, aVAlign, U16("inherit"));
}

SldU16StringRef toString(ESldMetaSwitchThematicTypeEnum aType)
{
	static const SldU16StringRef data[] = {
		U16("default"),
		U16("examples"),
		U16("etymology"),
		U16("phrase"),
		U16("idioms"),
		U16("phonetics"),
		U16("morphology"),
		U16("grammatics"),
		U16("menu"),
		U16("images"),
		U16("links"),
		U16("synonyms"),
		U16("antonyms"),
		U16("help-notes"),
		U16("usage-notes"),
		U16("radio-button"),
		U16("abbreviations"),
    U16("test-hint")
	};
	static_assert(ARRAY_SIZE(data) == eSwitchThematicsNumber, "Needs an update!");
	return ::to_string(data, aType, data[eSwitchThematicDefault]);
}

} // namespace html
} // namespace sld2

/**
 * sld2::html::StyleFormatter
 */

SldU16StringRef sld2::html::StyleFormatter::fontWeight(UInt32 aWeight) const
{
	static const SldU16StringRef named[] = {
		U16("normal"),
		U16("bold"),
		U16("bolder"),
		U16("lighter")
	};
	if (aWeight < sld2::array_size(named))
		return named[aWeight];

	static const SldU16StringRef values[] = {
		SldU16StringRef(),
		U16("100"),
		U16("200"),
		U16("300"),
		U16("400"),
		U16("500"),
		U16("600"),
		U16("700"),
		U16("800"),
		U16("900"),
	};
	aWeight = aWeight % 100 == 0 ? aWeight / 100 : ~0u;
	return ::to_string(values, aWeight, U16("normal"));
}

SldU16StringRef sld2::html::StyleFormatter::fontSize(UInt32 aSize)
{
	static const SldU16StringRef data[] = {
		U16("medium"),
		U16("x-small"),
		U16("large"),
		U16("small")
	};
	if (aSize < sld2::array_size(data))
		return data[aSize];

	UInt16 *ptr = start();
	ptr = pappend(ptr, U16("pt"));
	ptr = format(ptr, aSize, 10);
	return finalize(ptr);
}

SldU16StringRef sld2::html::StyleFormatter::color(UInt32 aColor)
{
	// the longest string possible is rgba(255,255,255,0.01) - 22 chars
	// which perfectly fits into 32 (31 because of the nul-term really) char formatter buffer

	union {
		struct { UInt8 a, b, g, r; };
		UInt32 u32;
	} color;
	color.u32 = aColor;

	if (color.a == 0)
		return U16("transparent");

	UInt16 *ptr = start();
	*--ptr = ')';

	if (color.a == 0xff)
	{
		*--ptr = '1';
	}
	else
	{
		// XXX: untested
		const UInt32 alpha = color.a * 100 / 0xff;
		if (alpha < 10)
		{
			*--ptr = (UInt16)('0' + alpha);
			*--ptr = '0';
		}
		else
		{
			if (alpha % 10 != 0)
				*--ptr = (UInt16)('0' + alpha % 10);
			*--ptr = (UInt16)('0' + alpha / 10);
		}
		ptr = pappend(ptr, U16("0."));
	}

	*--ptr = ',';
	ptr = format(ptr, color.b, 10);
	*--ptr = ',';
	ptr = format(ptr, color.g, 10);
	*--ptr = ',';
	ptr = format(ptr, color.r, 10);

	ptr = pappend(ptr, U16("rgba("));

	return finalize(ptr);
}

SldU16StringRef sld2::html::StyleFormatter::lineHeight(UInt32 aHeight, ESldStyleLevelEnum aLevel)
{
	// для строк с индексами устанавливаем большую высоту строки
	// (иначе текст строки будет накладываться на предыдущую/последующую)

	// table is indexed by height first level second (the 2 last levels are the same)
	static const SldU16StringRef data[][3] = {
		{ U16("160%"), U16("260%"), U16("260%") },
		{ U16("110%"), U16("180%"), U16("180%") },
		{ U16("220%"), U16("360%"), U16("360%") }
	};
	if (aHeight < sld2::array_size(data))
		return ::to_string(data[aHeight], aLevel, U16("inherit"));

	UInt16 *ptr = start();
	ptr = pappend(ptr, U16("pt"));
	ptr = format(ptr, aHeight, 10);
	return finalize(ptr);
}

/**
* StringBuilder
*/
namespace {

// helper for integers
static const UInt32 int_digits_counts[] = { 0, 3, 5, 0, 10 };
template <typename T> struct int_digits {
	static_assert(sld2::is_integral<T>::value && sizeof(T) <= 4, "only 32 bit integers supported");
	static SLD_CONSTEXPR UInt32 ndigits() {
		return int_digits_counts[sizeof(T)] + sld2::is_signed<T>::value;
	}
};

struct BaseFormatter
{
	struct Color { UInt32 v; };

	// "base" SldU16StringRef
	inline UInt32 size(SldU16StringRef str) const { return str.size(); }

	template <typename Buffer>
	void append(SldU16StringRef str, Buffer&& buf) {
		if (str.size())
			buf.append(str);
	}

	// TSizeValue
	inline UInt32 size(const TSizeValue&) const { return 15; }

	template <typename Buffer>
	void append(const TSizeValue &v, Buffer&& buf) {
		if (v.IsValid())
			append(fmt.format(v), sld2::forward<Buffer>(buf));
	}

	// Integers
	// We had a nice generic enable_if driven template for them but infortunately
	// unupdated vs13 dies with an ice on it... so we have to do it old-style with macros
	// (updated vs13 compiled the enable_if driven template just fine btw)
#define FORMAT_INT(type_) \
	UInt32 size( type_ ) const { return int_digits< type_ >::ndigits(); } \
	template <typename Buffer> \
	void append( type_ v, Buffer&& buf) { append(fmt.format(v), sld2::forward<Buffer>(buf)); }

	FORMAT_INT(Int8)
	FORMAT_INT(Int16)
	FORMAT_INT(Int32)
	FORMAT_INT(UInt8)
	FORMAT_INT(UInt16)
	FORMAT_INT(UInt32)

#undef FORMAT_INT

	// Color
	inline UInt32 size(const Color&) const { return 24; }

	template <typename Buffer>
	void append(const Color &v, Buffer&& buf) { append(fmt.color(v.v), sld2::forward<Buffer>(buf)); }

protected:
	sld2::html::StyleFormatter fmt;
};

BaseFormatter::Color asColor(UInt32 v) { return{ v }; }

// the buffer should have an interface of:
//  void ::reserve(UInt32 count)
//  void ::append(SldU16StringRef s)
//  UInt32 size() const
//  SldU16StringRef ::flush()

template <typename Buffer, typename Fmt = BaseFormatter>
struct StringBuilder
{
	template <typename... Args>
	StringBuilder(Args&&... args) : buf_(sld2::forward<Args>(args)...) {}

	template <typename... Args>
	inline StringBuilder& append(const Args&... args)
	{
		const UInt32 sz = _size(args...);
		buf_.reserve(sz);
		_append(args...);
		return *this;
	}

	UInt32 size() const { return buf_.size(); }

	SldU16StringRef flush() { return buf_.flush(); }

private:
	template <typename T>
	UInt32 _size(const T &arg) const { return fmt.size(arg); }

	template <typename T, typename... Args>
	UInt32 _size(const T &arg, const Args&... args) const { return _size(arg) + _size(args...); }

	template <typename T>
	void _append(const T &arg) { fmt.append(arg, buf_); }

	template <typename T, typename... Args>
	void _append(const T &arg, const Args&... args) { _append(arg); _append(args...); }

	Buffer buf_;
	Fmt fmt;
};

struct StringBuf
{
	StringBuf(SldU16String &str) : str_(str) {}

	inline void reserve(UInt32 count) { str_.reserve(str_.size() + count); }
	inline void append(SldU16StringRef str) { str_.append(str); }
	inline SldU16StringRef flush() const { return str_; }
	inline UInt32 size() const { return str_.size(); }

	SldU16String &str_;
};

} // anon namespace


/**
 * sld2::html::StyleSerializer
 */

void sld2::html::StyleSerializer::fontName(SldU16String &aString)
{
    aString.append(U16(" font-family: '"));
	aString.append(sld2::html::toString(style->GetStyleFontName(variantIndex)));
	aString.append(U16("';"));
}

void sld2::html::StyleSerializer::size(SldU16String &aString)
{
	aString.append(U16(" font-size: "));
	const TSizeValue size = style->GetTextSizeValue(variantIndex);
	if (size.IsValid())
		aString.append(fmt.format(size));
	else
		aString.append(fmt.fontSize(style->GetTextSize(variantIndex)));
	aString.push_back(';');
}

void sld2::html::StyleSerializer::weight(SldU16String &aString)
{
	aString.append(U16(" font-weight: "));
	aString.append(fmt.fontWeight(style->GetBoldValue(variantIndex)));
	aString.push_back(';');
}

void sld2::html::StyleSerializer::lineHeight(SldU16String &aString)
{
	aString.append(U16(" line-height: "));
	const TSizeValue size = style->GetLineHeightValue(variantIndex);
	if (size.IsValid())
		aString.append(fmt.format(size));
	else
		aString.append(fmt.lineHeight(style->GetLineHeight(variantIndex), style->GetLevel(variantIndex)));
	aString.push_back(';');
}

void sld2::html::StyleSerializer::italic(SldU16String &aString)
{
	static const SldU16StringRef data[] = {
		U16(" font-style: normal;"),
		U16(" font-style: italic;")
	};
	aString.append(data[style->IsItalic(variantIndex) ? 1 : 0]);
}

void sld2::html::StyleSerializer::decoration(SldU16String &aString)
{
	const UInt32 underline = style->IsUnderline(variantIndex);
	const UInt32 strikethrough = style->IsStrikethrough(variantIndex);
	const UInt32 overline = style->IsOverline(variantIndex);
	if (underline == 0 && strikethrough == 0 && overline == 0)
	{
		aString.append(U16(" text-decoration: none;"));
	}
	else
	{
		aString.append(U16(" text-decoration:"));
		if (underline)
			aString.append(U16(" underline"));

		if (strikethrough)
			aString.append(U16(" line-through"));

		if (overline)
			aString.append(U16(" overline"));

		aString.push_back(';');
	}
}

void sld2::html::StyleSerializer::level(SldU16String &aString)
{
	static const SldU16StringRef data[] = {
		U16(" vertical-align: baseline;"),
		U16(" vertical-align: sub;"),
		U16(" vertical-align: super;")
	};
	const ESldStyleLevelEnum level = style->GetLevel(variantIndex);
	if ((UInt16)level < sld2::array_size(data))
		aString.append(data[level]);
}

void sld2::html::StyleSerializer::color(SldU16String &aString)
{
	const UInt32 alpha = style->GetColorAlpha(variantIndex);
	const UInt32 color = (style->GetColor(variantIndex) << 8) | (alpha & 0xff);

	aString.append(U16(" color: "));
	aString.append(fmt.color(color));
	aString.push_back(';');
}

void sld2::html::StyleSerializer::bgColor(SldU16String &aString)
{
	const UInt32 alpha = style->GetBackgroundColorAlpha(variantIndex);
	if (alpha)
	{
		const UInt32 color = (style->GetBackgroundColor(variantIndex) << 8) | (alpha & 0xff);

		aString.append(U16(" background-color: "));
		aString.append(fmt.color(color));
		aString.push_back(';');
	}
}

void sld2::html::StyleSerializer::custom(SldU16String &aString)
{
	aString.append(U16(" white-space: pre-wrap; word-wrap: break-word; unicode-bidi: embed;"));
	const auto langCode = style->GetLanguage();
	/// Направление написания текста: для арабского и иврита - справа налево
	if (langCode != SldLanguage::Arabic && langCode != SldLanguage::Hebrew && langCode != SldLanguage::Urdu && langCode != SldLanguage::Farsi)
		aString.append(U16(" direction: ltr;"));
	else
		aString.append(U16(" direction: rtl;"));
}

/**
 * sld2::html:: serialization helpers
 */

void sld2::html::CreateCSSStyle(SldU16String &aString, const CSldMetadataProxy<eMetaMediaContainer> &aContainer, CSldCSSUrlResolver &aResolver)
{
	StringBuilder<StringBuf> b(aString);

	// XXX: this completely breaks formatting of all media containers...
	// as eDisplay_Inline really means 'block' and what _Block means is a mistery
	// or it's all dependant on some other undocumented crap
	//static const SldU16StringRef display[] = {
	//	U16(" display: inline;"),
	//	U16(" display: block;")
	//};
	//b.append(::to_string(display, aContainer->Display));

	if (aContainer->Type != eMedia_Line)
	{
		// добавление вертикального выравнивания
		static const SldU16StringRef align[] = {
			SldU16StringRef(),
			U16(" vertical-align: top;"),
			U16(" vertical-align: middle;"),
			U16(" vertical-align: bottom;")
		};
		b.append(::to_string(align, aContainer->AlignVertical),
		// добавление внутренних отступов
				 U16(" padding: "), aContainer->PaddingTop, U16(" "),
				 aContainer->PaddingRight, U16(" "),
				 aContainer->PaddingBottom, U16(" "),
				 aContainer->PaddingLeft,
		// добавление внешних отступов
				 U16("; margin: "),
				 aContainer->MarginTop, U16(" "),
				 aContainer->MarginRight, U16(" "),
				 aContainer->MarginBottom, U16(" "),
				 aContainer->MarginLeft, U16(";"));
	}

	if (aContainer->Colour)
		b.append(U16(" background-color: "), asColor(aContainer->Colour), U16(";"));

	/// Добавление градиента в стиль
	if (aContainer->Gradient)
	{
		static const SldU16StringRef gradients[] = {
			SldU16StringRef(),
			U16("to top"),
			U16("to left"),
			U16("to bottom"),
			U16("to right"),
			U16("to top left"),
			U16("to top right"),
			U16("to bottom left"),
			U16("to bottom right")
		};

		if (aContainer->Gradient < sld2::array_size(gradients))
		{
			b.append(U16(" background: linear-gradient("), gradients[aContainer->Gradient]);

			if (aContainer->ColourBegin)
				b.append(U16(","), asColor(aContainer->ColourBegin));

			if (aContainer->ColourEnd)
				b.append(U16(","), asColor(aContainer->ColourEnd));

			b.append(U16(");"));
		}
	}

	/// добавление в стиль обтекания
	static const SldU16StringRef floats[] = {
		U16(" float: none;"),
		U16(" float: left;"),
		U16(" float: right;")
	};
	b.append(::to_string(floats, aContainer->Float));

	static const SldU16StringRef clear[] = {
		U16(" clear: none;"),
		U16(" clear: left;"),
		U16(" clear: right;"),
		U16(" clear: both;")
	};
	b.append(::to_string(clear, aContainer->ClearType));

	if (aContainer->Expansion == eExtansionFull)
		b.append(U16(" width: 100%;"));
	else if (aContainer->Width.IsValid())
		b.append(U16(" width: "), aContainer->Width, U16(";"));

	if (aContainer->Height.IsValid())
		b.append(U16(" height: "), aContainer->Height, U16(";"));

	/// добавление всего связанного с фоновым изображением
	if (aContainer->Background != SLD_INDEX_PICTURE_NO)
	{
		const SldU16String fileName = aResolver.ResolveImageUrl(aContainer->Background);
		if (fileName.size())
		{
			/// тайлинг для фонового изображения
			static const SldU16StringRef tiling[] = {
				U16(" background-repeat: no-repeat;"),
				U16(" background-repeat: repeat-x;"),
				U16(" background-repeat: repeat-y;"),
				U16(" background-repeat: repeat;")
			};

			b.append(U16(" background-image: url('"), fileName, U16("');"),
					 ::to_string(tiling, aContainer->Tiling));
		}
	}

	if (aContainer->TextIndent.IsValid())
		b.append(U16(" text-indent: "), aContainer->TextIndent, U16(";"));

	if (aContainer->Expansion == eExtansionLeft)
		b.append(U16(" margin-left: -20;"));
}

/**
 * sld2::html::StateTracker
 */

sld2::html::StateTracker::StateTracker()
	: prevBlockType(eMetaUnknown)
{
	sld2::memzero(switchThematicCounts);
	sld2::memzero(idCounters);
}

void sld2::html::StateTracker::clear()
{
	prevBlockType = eMetaUnknown;

	usedCSSStyles.clear();
	usedStyles.clear();

	sldMemZero(BlocksStack.data(), BlocksStack.size() * sizeof(BlocksStack[0]));

	hideBlocks.clear();
	hideControlBlocks.clear();
	switchBlocks.clear();
	switchControlBlocks.clear();
	listBlocks.clear();

	sld2::memzero(switchThematicCounts);
	sld2::memzero(idCounters);

	script_OnLoad.clear();
	script_OnResize.clear();
}

void sld2::html::StateTracker::addTextBlock(ESldStyleMetaTypeEnum aType, UInt32 aStyle)
{
	prevBlockType = aType;
	usedStyles.insert(aStyle);
}

void sld2::html::StateTracker::addUsedCSSStyle(UInt32 aCSSStyleId)
{
	if (aCSSStyleId != CSldMetadataParser::InvalidDataIndex)
		usedCSSStyles.insert(aCSSStyleId);
}

void sld2::html::StateTracker::track(const CSldMetadataProxy<eMetaHide> &aData)
{
	if (aData.isClosing())
		hideBlocks.pop();
	else
		hideBlocks.emplace(idCounters.hide++, aData);
}

void sld2::html::StateTracker::track(const CSldMetadataProxy<eMetaHideControl> &aData)
{
	if (aData.isClosing())
		hideControlBlocks.pop();
	else
		hideControlBlocks.emplace(idCounters.hideControl++, aData);
}

void sld2::html::StateTracker::track(const CSldMetadataProxy<eMetaSwitch> &aData)
{
	if (aData.isClosing())
	{
		switchBlocks.pop();
	}
	else
	{
		idCounters.switch_++;
		switchBlocks.emplace(aData);
		if (aData->Thematic < eSwitchThematicsNumber)
			switchThematicCounts[aData->Thematic] = (UInt8)aData->NumStates;
	}
}

void sld2::html::StateTracker::track(const CSldMetadataProxy<eMetaSwitchControl> &aData)
{
	if (aData.isClosing())
		switchControlBlocks.pop();
	else
		switchControlBlocks.emplace(idCounters.switchControl++, aData);
}

void sld2::html::StateTracker::track(const CSldMetadataProxy<eMetaSwitchState> &aData)
{
	if (!aData.isClosing())
		return;

	if (isInside(eMetaSwitchControl))
		switchControlBlocks.top().stateIndex++;
	else if (isInside(eMetaSwitch) && !switchBlocks.empty())
		switchBlocks.top().stateIndex++;
}

void sld2::html::StateTracker::track(const CSldMetadataProxy<eMetaManagedSwitch> &aData)
{
	if (aData.isClosing())
		return;

	SldU16StringRef label = aData.string_ref(aData->Label);
	if (label.empty())
		return;

	switchControlBlocks.top().managedSwitches.push_back(to_string(label));
}

void sld2::html::StateTracker::track(const CSldMetadataProxy<eMetaArticleEventHandler> &aData)
{
	const SldU16StringRef string = aData.string_ref(aData->JSString);
	if (string.empty())
		return;

	switch (aData->Type)
	{
	case eSldMetadataDOMEvent_OnLoad: script_OnLoad.assign(string); break;
	case eSldMetadataDOMEvent_OnResize: script_OnResize.assign(string); break;
	default: break;
	}
}

sld2::html::StateTracker::ListBlock::ListBlock(UInt32 aId, const CSldMetadataProxy<eMetaList> &aData)
	: id(aId), style(aData->Style), itemsCount(aData->Number),
	  type((ESldListType)aData->Type), markerType(deflt), currentItemId(0)
{
	SldU16StringRef markerRef = aData.string_ref(aData->Marker);
	if (markerRef.empty())
		return;

	if (aData->Type != eListTypeUnordered)
	{
		static const SldU16StringRef markerTypes[] = {
			/* [none] = */           U16("none"),
			/* [decimal] = */        U16("decimal"),
			/* [lower_cyrillic] = */ U16("lower-cyrillic"),
			/* [lower_latin] = */    U16("lower-latin"),
			/* [lower_greek] = */    U16("lower-greek"),
			/* [lower_roman] = */    U16("lower-roman"),
			/* [upper_cyrillic] = */ U16("upper-cyrillic"),
			/* [upper_latin] = */    U16("upper-latin"),
			/* [upper_roman] = */    U16("upper-roman"),
		};
		// if it ever becomes perf critical we can safely rework it into
		// "prefix tree" if ladder
		for (UInt32 i = 0; i < sld2::array_size(markerTypes); i++)
		{
			if (markerRef == markerTypes[i])
			{
				markerType = MarkerType(i);
				break;
			}
		}
	}

	// copy the marker into our internal array
	_marker.assign(markerRef.data(), markerRef.size() + 1);

	auto marker = sld2::make_span(_marker.data(), _marker.size() - 1);
	if (markerType != deflt)
	{
		markers.resize(1);
		markers[0] = SldU16StringRef(marker.data(), marker.size());
	}
	else
	{
		UInt32 count = 0, start = 0;
		while (start < marker.size())
		{
			UInt32 end = start;
			while (end < marker.size() && marker[end] != '|')
				end++;

			auto chunk = marker.subspan(start, end - start);
			markers.resize(count + 1);
			markers[count++] = SldU16StringRef(chunk.data(), chunk.size());

			marker[end] = '\0';
			start = end + 1;
		}
		markerType = markers.size() == 1 ? custom_single : custom_multi;
	}
}

void sld2::html::StateTracker::track(const CSldMetadataProxy<eMetaList> &aData)
{
	if (aData.isClosing())
	{
		listBlocks.pop();
	}
	else
	{
		listBlocks.emplace(idCounters.list++, aData);
		usedStyles.insert(aData->Style);
	}
}

void sld2::html::StateTracker::track(const CSldMetadataProxy<eMetaLi> &aData)
{
	if (aData.isClosing())
		listBlocks.top().currentItemId++;
}

/**
 * CSldBasicHTMLBuilder
 */

bool CSldBasicHTMLBuilder::IDataAccess::canHaveImageLink(const sld2::html::StateTracker &aState)
{
	return !aState.isInside(eMetaLink) && !aState.isInside(eMetaHideControl) &&
	       !aState.isInside(eMetaPopupImage) && !aState.isInside(eMetaUrl) &&
	       !aState.isInside(eMetaPopupArticle) && !aState.isInside(eMetaSwitchControl) &&
		   !aState.isInside(eMetaSound);
}

void CSldBasicHTMLBuilder::Clear()
{
	m_state.clear();
}

SldU16StringRef CSldBasicHTMLBuilder::GetHideControlScript()
{
	// XXX: we don't really nead headID here
	return U16("\n\
function sld2_hideControl_onClick(headID, bodyID)\n\
{\n\
	var div = document.getElementById(bodyID);\n\
	div.style.display = div.style.display == \"none\" ? \"inline\" : \"none\";\n\
}\n");
}

SldU16StringRef CSldBasicHTMLBuilder::GetSwitchControlScript()
{
	return U16("\n\
function sld2_switchState_onClick(blockId, numStates)\n\
{\n\
	var firstBlock = document.querySelector(\'div[id=\"\' + blockId + \'-state0\"][title=\"radio-button\"]\');\n\
	if(firstBlock)\n\
	{\n\
		var radioButtons = document.querySelectorAll(\'div[class=\"\' + firstBlock.className + \'\"][title=\"radio-button\"]\');\n\
		var blockDisplay;\n\
		for(var blockIndex = 0; blockIndex < radioButtons.length; blockIndex++)\n\
		{\n\
			if(radioButtons[blockIndex].style.display != \'none\'){\n\
				blockDisplay = radioButtons[blockIndex].style.display;\n\
				break;}\n\
		}\n\
		\n\
		for(var blockIndex = 0; blockIndex < radioButtons.length; blockIndex++)\n\
		{\n\
			if(radioButtons[blockIndex] == firstBlock)\n\
			{\n\
				radioButtons[blockIndex++].style.display = blockDisplay;\n\
				radioButtons[blockIndex].style.display = \'none\';\n\
			}\n\
			else if(blockIndex % 2 == 0)\n\
				radioButtons[blockIndex].style.display = \'none\';\n\
			else\n\
				radioButtons[blockIndex].style.display = blockDisplay;\n\
		}\n\
	}\n\
	else\n\
	{\n\
		function findSwitch(index)\n\
		{\n\
			var fullId = blockId + \"-state\" + index;\n\
			return document.getElementById(fullId);\n\
		}\n\
\n\
		for (var stateIndex = 0; stateIndex < numStates; stateIndex++)\n\
		{\n\
			var block = findSwitch(stateIndex);\n\
			if (block.style.display != \"none\")\n\
			{\n\
				var styleDisplay = block.style.display;\n\
				block.style.display = \"none\";\n\
				stateIndex++;\n\
				break;\n\
			}\n\
		}\n\
		if (stateIndex == numStates)\n\
			stateIndex = 0;\n\
		var block = findSwitch(stateIndex);\n\
		block.style.display = styleDisplay;\n\
	}\n\
}\n");
}

SldU16StringRef CSldBasicHTMLBuilder::GetRadioButtonInitScript()
{
	return U16("\n\
function sld2_radioButton_onInit(index)\n\
{\n\
	if(document.querySelector(\'div[class=\"sld-switch-control\"][title=\"radio-button\"]\'))\n\
		sld2_switchState_onClick(\'switch-control\' + index, 2);\n\
	if(document.querySelector(\'div[class=\"sld-switch-state\"][title=\"radio-button\"]\'))\n\
		sld2_switchState_onClick(\'switch\' + index, 2);\n\
}\n");
}

SldU16StringRef CSldBasicHTMLBuilder::GetCrossRefScript()
{
	return U16("\n\
function sld2_crossRef_onClick(e)\n\
{\n\
	var selection = 0;\n\
	if('getSelection' in window)\n\
		var selection = window.getSelection();\n\
	else\n\
		return;\n\
\n\
	var node = selection.anchorNode.parentNode;\n\
	if(!node || node != e.target || !node.hasAttribute('sld-ref'))\n\
		return;\n\
\n\
	var langCode = node.getAttribute('lang');\n\
	while(node.previousElementSibling && node.previousElementSibling.hasAttribute('sld-ref') && node.previousElementSibling.getAttribute('lang') == langCode){\n\
		node = node.previousElementSibling;\n\
	}\n\
\n\
	var textValue = '';\n\
	var offset = selection.anchorOffset;\n\
	var afterTargetNode = false;\n\
	while(node && node.hasAttribute('sld-ref') && node.getAttribute('lang') == langCode){\n\
		if(!afterTargetNode){\n\
			if(node === selection.anchorNode.parentNode)\n\
				afterTargetNode = true;\n\
			else\n\
				offset += node.textContent.length;\n\
		}\n\
		textValue += node.textContent;\n\
		node = node.nextElementSibling;\n\
	}\n\
\n\
	window.location.href = 'sld-ref:'+ langCode + ':' + offset + ':' + textValue;\n\
}\n\
\n");
}

SldU16StringRef CSldBasicHTMLBuilder::GetCrossRefInitScript()
{
	return U16("\n\
function sld2_crossRef_onInit()\n\
{\n\
	document.addEventListener('click', sld2_crossRef_onClick, false);\n\
	document.addEventListener('touchend', sld2_crossRef_onClick, false);\n\
}\n\
\n");
}

SldU16StringRef CSldBasicHTMLBuilder::GetSearchHighlightScript()
{
	return U16("\n\
function sld2_getTextForLabel(label)\n\
{\n\
	let label_node = document.querySelector('a[name=\"' + label + '\"]');\n\
	if (!label_node)\n\
		return '';\n\
	\n\
	function getTextNodesFrom(node)\n\
	{\n\
		if(!node)\n\
			 return [];\n\
\n\
		let nodes = [];\n\
		if (node.nodeType == Node.ELEMENT_NODE && window.getComputedStyle(node).display == 'block')\n\
		{\n\
			nodes = getTextNodesFrom(node.firstChild);\n\
			if (nodes.length > 0 && nodes[nodes.length - 1] != null)\n\
				nodes = nodes.concat([null]);\n\
		}\n\
		else\n\
		{\n\
			for (let n = node; n; n = n.nextSibling)\n\
			{\n\
				if (n.nodeType == Node.TEXT_NODE)\n\
				{\n\
					nodes = nodes.concat([n]);\n\
					continue;\n\
				}\n\
			\n\
				if (n.nodeType == Node.ELEMENT_NODE && window.getComputedStyle(n).display == 'block')\n\
				{\n\
					nodes = nodes.concat([null]);\n\
					break;\n\
				}\n\
				\n\
				nodes = nodes.concat(getTextNodesFrom(n.firstChild));\n\
				if (nodes.length > 0 && nodes[nodes.length - 1] == null)\n\
					break;\n\
			}\n\
		}\n\
\n\
		return nodes;\n\
	}\n\
		\n\
	function getTextAfter(node)\n\
	{\n\
		let text = \"\";		\n\
		let text_nodes = getTextNodesFrom(node.nextSibling);\n\
		for (let i = 0; i < text_nodes.length && text_nodes[i]; i++)\n\
			text += text_nodes[i].wholeText;\n\
			\n\
		return text;\n\
	}\n\
	\n\
	return getTextAfter(label_node);\n\
}\n\
\n\
function sld2_highlightTextForLabel(label, bounds, phrase_color = \"khaki\", words_color = \"tomato\")\n\
{\n\
	if (typeof(sld2_highlightTextForLabel.highlighted) == 'undefined')\n\
		sld2_highlightTextForLabel.highlighted = [];\n\
\n\
	let label_node = document.querySelector('a[name=\"' + label + '\"]');\n\
\n\
	for (let i = 0; i < sld2_highlightTextForLabel.highlighted.length; i++)\n\
		sld2_highlightTextForLabel.highlighted[i].outerHTML = sld2_highlightTextForLabel.highlighted[i].innerHTML;\n\
		\n\
	sld2_highlightTextForLabel.highlighted = [];\n\
	\n\
	function getSubTextNodes(node)\n\
	{\n\
		if (node.nodeType == Node.TEXT_NODE)\n\
			return [node];\n\
	\n\
		let nodes = [];\n\
		for (let c = node.firstChild; c; c = c.nextSibling)\n\
			nodes = nodes.concat(getSubTextNodes(c));\n\
			\n\
		return nodes;\n\
	}\n\
	\n\
	function getTextNodesAfter(node)\n\
	{\n\
		let nodes = [];\n\
		for (let n = node.nextSibling; n; n = n.nextSibling)\n\
			nodes = nodes.concat(getSubTextNodes(n));\n\
			\n\
		return nodes;\n\
	}\n\
	 \n\
	const text_nodes = getTextNodesAfter(label_node);\n\
	\n\
	let accumulated_offset = 0;\n\
	for (let i = 0; i < text_nodes.length && accumulated_offset < bounds[1]; i++)\n\
	{\n\
		const text_length = text_nodes[i].length;\n\
		if (accumulated_offset + text_length < bounds[0])\n\
		{\n\
			accumulated_offset += text_length;\n\
			continue;\n\
		}\n\
			\n\
		let prange = new Range();\n\
		if (accumulated_offset < bounds[0])\n\
		{\n\
			const hl_start = bounds[0] - accumulated_offset;\n\
			prange.setStart(text_nodes[i], hl_start);\n\
		}\n\
		else\n\
		{\n\
			prange.setStartBefore(text_nodes[i]);\n\
		}\n\
		\n\
		if (accumulated_offset + text_length <= bounds[1])\n\
		{\n\
			prange.setEndAfter(text_nodes[i]);\n\
		}\n\
		else\n\
		{\n\
			const hl_end = bounds[1] - accumulated_offset;\n\
			prange.setEnd(text_nodes[i], hl_end);\n\
		}\n\
		\n\
		let bg = document.createElement(\"span\");\n\
		bg.style.background = phrase_color;\n\
	\n\
		prange.surroundContents(bg);\n\
		\n\
		sld2_highlightTextForLabel.highlighted.push(bg);\n\
		\n\
		accumulated_offset += text_length;\n\
	}\n\
	\n\
	const highlight_begin = '<span style=\"background: ' + words_color + '; \">';\n\
	const highlight_end = '</span>';\n\
	let offset = bounds[0];\n\
	\n\
	let additional_highlighted = [];\n\
	for (let i = 0; i < sld2_highlightTextForLabel.highlighted.length; i++)\n\
	{\n\
		let length = sld2_highlightTextForLabel.highlighted[i].innerHTML.length;\n\
		\n\
		for (let j = bounds.length - 2; j > 1; j -= 2)\n\
		{\n\
			if (bounds[j+1] <= offset)\n\
			{\n\
				break;\n\
			}\n\
			else if (bounds[j+1] >= offset + length && bounds[j] >= offset + length)\n\
			{\n\
				continue;\n\
			}	\n\
			else if (bounds[j+1] > offset + length && bounds[j] > offset)\n\
			{\n\
				let new_inner = sld2_highlightTextForLabel.highlighted[i].innerHTML.substr(0, bounds[j] - offset) + highlight_begin + \n\
					sld2_highlightTextForLabel.highlighted[i].innerHTML.substr(bounds[j] - offset) + highlight_end;\n\
				sld2_highlightTextForLabel.highlighted[i].innerHTML = new_inner;\n\
			}\n\
			else if (bounds[j+1] > offset + length && bounds[j] <= offset)\n\
			{\n\
				sld2_highlightTextForLabel.highlighted[i].innerHTML = highlight_begin + sld2_highlightTextForLabel.highlighted[i].innerHTML + highlight_end;\n\
			}\n\
			else if (bounds[j] > offset)\n\
			{\n\
				let new_inner = sld2_highlightTextForLabel.highlighted[i].innerHTML.substr(0, bounds[j] - offset) + highlight_begin + \n\
					sld2_highlightTextForLabel.highlighted[i].innerHTML.substr(bounds[j] - offset, bounds[j+1]-bounds[j]) + highlight_end + \n\
					sld2_highlightTextForLabel.highlighted[i].innerHTML.substr(bounds[j+1] - offset);\n\
				sld2_highlightTextForLabel.highlighted[i].innerHTML = new_inner;\n\
			}\n\
			else\n\
			{\n\
				let new_inner = highlight_begin + \n\
						sld2_highlightTextForLabel.highlighted[i].innerHTML.substr(0, bounds[j+1] - offset) + highlight_end + \n\
						sld2_highlightTextForLabel.highlighted[i].innerHTML.substr(bounds[j+1] - offset);\n\
				sld2_highlightTextForLabel.highlighted[i].innerHTML = new_inner;\n\
			}\n\
		}\n\
		\n\
		for (let j = 0; j < sld2_highlightTextForLabel.highlighted[i].childNodes.length; j++)\n\
		{\n\
			if (sld2_highlightTextForLabel.highlighted[i].childNodes[j].nodeType != Node.TEXT_NODE)\n\
				additional_highlighted.push(sld2_highlightTextForLabel.highlighted[i].childNodes[j]);\n\
		}\n\
		\n\
		offset += length;\n\
	}\n\
	\n\
	sld2_highlightTextForLabel.highlighted = additional_highlighted.concat(sld2_highlightTextForLabel.highlighted);\n\
}");
}


SldU16StringRef CSldBasicHTMLBuilder::GetSwitchUncoverScript()
{
	return U16("function sld2_tryExpandAsSwitchControl(node)\n\
{\n\
	if (node.tagName != 'DIV' || !node.classList.contains('sld-switch-control'))\n\
		return;\n\
\n\
	if (window.getComputedStyle(node).display != 'none')\n\
		return;\n\
\n\
	const active_state = node.dataset.stateId;\n\
	\n\
	let controls = document.evaluate('//div[@data-control-id=\"' + node.dataset.controlId + '\"]', document, null, XPathResult.UNORDERED_NODE_SNAPSHOT_TYPE, null);\n\
	\n\
	for (let i = 0; i < controls.snapshotLength; i++)\n\
	{\n\
		if (controls.snapshotItem(i) == node)\n\
			controls.snapshotItem(i).style.display = 'inline';\n\
		else\n\
			controls.snapshotItem(i).style.display = 'none';\n\
	}\n\
	\n\
	const children = node.childNodes;\n\
	for (let i = 0; i < children.length; ++i)\n\
	{\n\
		if (children[i].tagName != 'SWITCHMARKER')\n\
			continue;\n\
		\n\
		let switch_label = children[i].dataset.label;\n\
		let switches = document.evaluate('//div[@class=\"sld-switch-state\" and @data-label=\"' + switch_label + '\"]', document, null, XPathResult.UNORDERED_NODE_SNAPSHOT_TYPE, null);\n\
		for (let i = 0; i < switches.snapshotLength; i++)\n\
		{\n\
			if (switches.snapshotItem(i).dataset.stateId != active_state)\n\
				switches.snapshotItem(i).style.display = 'none';\n\
			else if (switches.snapshotItem(i).dataset.isInline != '0')\n\
				switches.snapshotItem(i).style.display = 'inline';\n\
			else\n\
				switches.snapshotItem(i).style.display = 'block';\n\
		}\n\
	}\n\
}\n\
\n\
function sld2_activateAppSettingsSwitch(thematic, state)\n\
{\n\
	let app_switches = document.evaluate('//div[@class=\"sld-switch-state\" and @title=\"' + thematic + '\"]', document, null, XPathResult.UNORDERED_NODE_SNAPSHOT_TYPE, null);\n\
	for (let i = 0; i < app_switches.snapshotLength; i++)\n\
	{\n\
		if (app_switches.snapshotItem(i).dataset.stateId != state)\n\
			app_switches.snapshotItem(i).style.display = 'none';\n\
		else if (app_switches.snapshotItem(i).dataset.isInline != '0')\n\
			app_switches.snapshotItem(i).style.display = 'inline';\n\
		else\n\
			app_switches.snapshotItem(i).style.display = 'block';\n\
	}\n\
}\n\
\n\
function sld2_tryExpandAsSwitch(node)\n\
{\n\
	if (node.tagName != 'DIV' || !node.classList.contains('sld-switch-state'))\n\
		return [];\n\
		\n\
	if (window.getComputedStyle(node).display != 'none')\n\
		return [];\n\
		\n\
	if (node.id)\n\
	{\n\
		let markers = document.querySelectorAll('switchmarker[data-label=\"' + node.dataset.label + '\"]')\n\
		for (let i = 0; i < markers.length; ++i)\n\
		{\n\
			if (node.dataset.stateId == markers[i].parentNode.dataset.stateId)\n\
			{\n\
				sld2_tryExpandAsSwitchControl(markers[i].parentNode);\n\
				break;\n\
			}\n\
		}		\n\
		return [];\n\
	}\n\
	else\n\
	{\n\
		let thematic = node.getAttribute('title');\n\
		let state = parseInt(node.dataset.stateId, 10);\n\
		sld2_activateAppSettingsSwitch(thematic, state)\n\
		\n\
		return [thematic, state];\n\
	}\n\
}\n\
\n\
function sld2_expandParentSwitchBlock(node)\n\
{\n\
	let app_switches_states = new Map([]);\n\
	for (; node; node = node.parentNode)\n\
	{\n\
		let new_values = sld2_tryExpandAsSwitch(node);\n\
		if (Array.isArray(new_values) && new_values.length == 2)\n\
			app_switches_states.set(new_values[0], new_values[1]);\n\
		\n\
		sld2_tryExpandAsSwitchControl(node);\n\
	}\n\
	\n\
	return [...app_switches_states];\n\
}\n\
\n\
function sld2_changeBackgroundColoroBody(color)\n\
{\n\
    document.body.style.background = color;\n\
}");
}

SldU16StringRef CSldBasicHTMLBuilder::GetISOLanguageCode(const ESldLanguage aSldLangCode)
{
	switch (aSldLangCode)
	{
	case SldLanguage::Afrikaans:	return U16("af");
	case SldLanguage::Albanian:		return U16("sq");
	case SldLanguage::AmericanEnglish:		return U16("en");
	case SldLanguage::Arabic:		return U16("ar");
	case SldLanguage::Argentinian:	return U16("es");
	case SldLanguage::Basque:		return U16("eu");
	case SldLanguage::Brazilian:	return U16("pt");
	case SldLanguage::BritishEnglish:		return U16("en");
	case SldLanguage::Bulgarian:	return U16("bg");
	case SldLanguage::Burmese:		return U16("my");
	case SldLanguage::Cantonese:	return U16("zh");
	case SldLanguage::Catalan:		return U16("ca");
	case SldLanguage::Chinese:		return U16("zh");
	case SldLanguage::Croatian:		return U16("hr");
	case SldLanguage::Czech:		return U16("cs");
	case SldLanguage::Danish:		return U16("da");
	case SldLanguage::Dutch:		return U16("nl");
	case SldLanguage::English:		return U16("en");
	case SldLanguage::Estonian:		return U16("et");
	case SldLanguage::Farsi:		return U16("fa");
	case SldLanguage::Filipino:		return U16("tl");
	case SldLanguage::Finnish:		return U16("fi");
	case SldLanguage::French:		return U16("fr");
	case SldLanguage::German:		return U16("de");
	case SldLanguage::Greek:		return U16("el");
	case SldLanguage::Hebrew:		return U16("he");
	case SldLanguage::Hindi:		return U16("hi");
	case SldLanguage::Hungarian:	return U16("hu");
	case SldLanguage::Icelandic:	return U16("is");
	case SldLanguage::Indonesian:	return U16("id");
	case SldLanguage::Irish:		return U16("ga");
	case SldLanguage::Italian:		return U16("it");
	case SldLanguage::Japanese:		return U16("ja");
	case SldLanguage::Korean:		return U16("ko");
	case SldLanguage::Latin:		return U16("la");
	case SldLanguage::Latvian:		return U16("lv");
	case SldLanguage::Lithuanian:	return U16("lt");
	case SldLanguage::Malay:		return U16("ms");
	case SldLanguage::Norwegian:	return U16("no");
	case SldLanguage::Polish:		return U16("pl");
	case SldLanguage::Portuguese:	return U16("pt");
	case SldLanguage::Romanian:		return U16("ro");
	case SldLanguage::Russian:		return U16("ru");
	case SldLanguage::Serbian:		return U16("sr");
	case SldLanguage::Shona:		return U16("sn");
	case SldLanguage::Slovak:		return U16("sk");
	case SldLanguage::Slovenian:	return U16("sl");
	case SldLanguage::Spanish:		return U16("es");
	case SldLanguage::Swahili:		return U16("sw");
	case SldLanguage::Swedish:		return U16("sv");
	case SldLanguage::Thai:			return U16("th");
	case SldLanguage::Tswana:			return U16("tn");
	case SldLanguage::Turkish:		return U16("tr");
	case SldLanguage::Ukrainian:	return U16("uk");
	case SldLanguage::Urdu:			return U16("ur");
	case SldLanguage::Uzbek:		return U16("uz");
	case SldLanguage::Vietnamese:	return U16("vi");
	case SldLanguage::WorldEnglish:		return U16("en");
	default:						return U16("");
	}
}

ESldLanguage CSldBasicHTMLBuilder::GetLanguageCodeByISO(SldU16StringRef aISOCode)
{
	UInt32 isoCode = CSldCompare::UInt16StrToUInt32Code(aISOCode);
	switch (isoCode)
	{
	case EnsureNativeByteOrder_16('af'):	return SldLanguage::Afrikaans;
	case EnsureNativeByteOrder_16('sq'):	return SldLanguage::Albanian;
	case EnsureNativeByteOrder_16('ar'):	return SldLanguage::Arabic;
	case EnsureNativeByteOrder_16('eu'):	return SldLanguage::Basque;
	case EnsureNativeByteOrder_16('bg'):	return SldLanguage::Bulgarian;
	case EnsureNativeByteOrder_16('my'):	return SldLanguage::Burmese;
	case EnsureNativeByteOrder_16('ca'):	return SldLanguage::Catalan;
	case EnsureNativeByteOrder_16('zn'):	return SldLanguage::Chinese;
	case EnsureNativeByteOrder_16('hr'):	return SldLanguage::Croatian;
	case EnsureNativeByteOrder_16('cs'):	return SldLanguage::Czech;
	case EnsureNativeByteOrder_16('da'):	return SldLanguage::Danish;
	case EnsureNativeByteOrder_16('nl'):	return SldLanguage::Dutch;
	case EnsureNativeByteOrder_16('en'):	return SldLanguage::English;
	case EnsureNativeByteOrder_16('et'):	return SldLanguage::Estonian;
	case EnsureNativeByteOrder_16('fa'):	return SldLanguage::Farsi;
	case EnsureNativeByteOrder_16('tl'):	return SldLanguage::Filipino;
	case EnsureNativeByteOrder_16('fi'):	return SldLanguage::Finnish;
	case EnsureNativeByteOrder_16('fr'):	return SldLanguage::French;
	case EnsureNativeByteOrder_16('de'):	return SldLanguage::German;
	case EnsureNativeByteOrder_16('el'):	return SldLanguage::Greek;
	case EnsureNativeByteOrder_16('he'):	return SldLanguage::Hebrew;
	case EnsureNativeByteOrder_16('hi'):	return SldLanguage::Hindi;
	case EnsureNativeByteOrder_16('hu'):	return SldLanguage::Hungarian;
	case EnsureNativeByteOrder_16('is'):	return SldLanguage::Icelandic;
	case EnsureNativeByteOrder_16('id'):	return SldLanguage::Indonesian;
	case EnsureNativeByteOrder_16('ga'):	return SldLanguage::Irish;
	case EnsureNativeByteOrder_16('it'):	return SldLanguage::Italian;
	case EnsureNativeByteOrder_16('ja'):	return SldLanguage::Japanese;
	case EnsureNativeByteOrder_16('ko'):	return SldLanguage::Korean;
	case EnsureNativeByteOrder_16('la'):	return SldLanguage::Latin;
	case EnsureNativeByteOrder_16('lv'):	return SldLanguage::Latvian;
	case EnsureNativeByteOrder_16('lt'):	return SldLanguage::Lithuanian;
	case EnsureNativeByteOrder_16('ms'):	return SldLanguage::Malay;
	case EnsureNativeByteOrder_16('no'):	return SldLanguage::Norwegian;
	case EnsureNativeByteOrder_16('pl'):	return SldLanguage::Polish;
	case EnsureNativeByteOrder_16('pt'):	return SldLanguage::Portuguese;
	case EnsureNativeByteOrder_16('ro'):	return SldLanguage::Romanian;
	case EnsureNativeByteOrder_16('ru'):	return SldLanguage::Russian;
	case EnsureNativeByteOrder_16('sr'):	return SldLanguage::Serbian;
	case EnsureNativeByteOrder_16('sn'):	return SldLanguage::Shona;
	case EnsureNativeByteOrder_16('sk'):	return SldLanguage::Slovak;
	case EnsureNativeByteOrder_16('sl'):	return SldLanguage::Slovenian;
	case EnsureNativeByteOrder_16('es'):	return SldLanguage::Spanish;
	case EnsureNativeByteOrder_16('sw'):	return SldLanguage::Swahili;
	case EnsureNativeByteOrder_16('sv'):	return SldLanguage::Swedish;
	case EnsureNativeByteOrder_16('th'):	return SldLanguage::Thai;
	case EnsureNativeByteOrder_16('tn'):	return SldLanguage::Tswana;
	case EnsureNativeByteOrder_16('tr'):	return SldLanguage::Turkish;
	case EnsureNativeByteOrder_16('uk'):	return SldLanguage::Ukrainian;
	case EnsureNativeByteOrder_16('ur'):	return SldLanguage::Urdu;
	case EnsureNativeByteOrder_16('uz'):	return SldLanguage::Uzbek;
	case EnsureNativeByteOrder_16('vi'):	return SldLanguage::Vietnamese;
	default:								return SldLanguage::Unknown;
	}
}

namespace {
namespace handlers {
namespace detail {

struct CSSClass { SldU16StringRef name; };

struct AddBlockFormatter : public BaseFormatter
{
	using BaseFormatter::size;
	using BaseFormatter::append;

	inline UInt32 size(const CSSClass &cls) const {
		return cls.name.empty() ? 0 : cls.name.size() + 9;
	}

	template <typename Buffer>
	void append(const CSSClass &cls, Buffer&& buf) {
		if (cls.name.size())
		{
			buf.append(U16(" class='"));
			buf.append(cls.name);
			buf.append(U16("'"));
		}
	}
};

struct DynArrayBuf
{
	DynArrayBuf(sld2::DynArray<UInt16> &arr) : arr_(arr), size_(0) {}

	inline void reserve(UInt32 count) {
		const UInt32 s = size() + count + 1;
		if (arr_.size() < s)
			arr_.resize(sld2::default_init, s);
	}

	inline void append(SldU16StringRef str) {
		sld2::memcopy_n(arr_.data() + size(), str.data(), str.size());
		size_ += str.size();
	}

	inline SldU16StringRef flush() {
		arr_[size()] = 0;
		return SldU16StringRef(arr_.data(), size());
	}

	inline UInt32 size() const { return size_; }

	sld2::DynArray<UInt16> &arr_;
	UInt32 size_;
};

} // namespace detail

static detail::CSSClass cssClass(const CSldMetadataProxyBase &aData, CSldBasicHTMLBuilder::IDataAccess &access)
{
	if (aData.cssStyleId() != CSldMetadataParser::InvalidDataIndex)
		return{ access.CSSClassName(aData) };
	else
		return{ nullptr };
}

using IDataAccess = CSldBasicHTMLBuilder::IDataAccess;
using StateTracker = sld2::html::StateTracker;
using StringBuilder = ::StringBuilder<detail::DynArrayBuf, detail::AddBlockFormatter>;

static SldU16String resolveImageUrl(UInt32 aIndex, const CSldMetadataProxyBase &aData, const TMetadataExtKey &aKey, IDataAccess &access)
{
	// XXX: dictionary ids are *really* UInt32 ids in string form...
	// the fact they are stored in a string is *very* unfortunate
	SldU16StringRef dictId = aData.string_ref(aKey.DictId);
	if (dictId.empty())
		return access.ResolveImageUrl(aIndex);

	// make a copy of it so we can pass it as a ref
	UInt16 buf[16] = {};
	sld2::memcopy_n(buf, dictId.data(), dictId.length());
	return access.ResolveImageUrl(SldU16StringRef(buf, dictId.size()), aKey.ListIdx, aData.string_ref(aKey.Key));
}

void addBlock(const CSldMetadataProxy<eMetaImage> &aData, IDataAccess &access, const StateTracker &state, StringBuilder &b)
{
	if (aData.isClosing())
		return;

	const SldU16String fileName = resolveImageUrl(aData->PictureIndex, aData, aData->ExtKey, access);
	if (fileName.empty())
		return;

	const bool needsLink = access.canHaveImageLink(state);
	if (needsLink)
	{
		const SldU16String fullFileName = resolveImageUrl(aData->FullPictureIndex, aData, aData->FullExtKey, access);
		// "sld-full-image:file_name"
		b.append(U16("<a href=\"sld-full-image:"), fullFileName.empty() ? fileName : fullFileName, U16("\">"));
	}

	b.append(U16("<img src=\""), fileName, U16("\""));
	if (aData->ShowWidth.IsValid() || aData->ShowHeight.IsValid())
	{
		b.append(U16(" style='"));
		if (aData->ShowWidth.IsValid())
			b.append(U16("width:"), aData->ShowWidth, U16(";"));
		if (aData->ShowHeight.IsValid())
			b.append(U16("height:"), aData->ShowHeight);
		b.append(U16("'"));
	}
	b.append(cssClass(aData, access), U16("/>"));

	if (needsLink)
		b.append(U16("</a>"));
}

void addBlock(const CSldMetadataProxy<eMetaTable> &aData, IDataAccess &access, const StateTracker&, StringBuilder &b)
{
	if (aData.isClosing())
	{
		b.append(U16("</table>"));
		return;
	}

	if (aData->CellPadding.IsValid())
		b.append(U16("<table cellpadding='"), aData->CellPadding, U16("'"));
	else
		b.append(U16("<table cellpadding='2%'"));

	if (aData->CellSpacing.IsValid())
		b.append(U16(" cellspacing='"), aData->CellSpacing, U16("'"));

	b.append(cssClass(aData, access), U16(" style='border-collapse:collapse;"));
	if (aData->Width.IsValid())
		b.append(U16("width:"), aData->Width);
	b.append(U16("'>"));
}

void addBlock(const CSldMetadataProxy<eMetaTableRow> &aData, IDataAccess &access, const StateTracker&, StringBuilder &b)
{
	if (aData.isClosing())
		b.append(U16("</tr>"));
	else
		b.append(U16("<tr"), cssClass(aData, access), U16(">"));
}

void addBlock(const CSldMetadataProxy<eMetaTableCol> &aData, IDataAccess &access, const StateTracker&, StringBuilder &b)
{
	if (aData.isClosing())
	{
		b.append(U16("</td>"));
		return;
	}

	b.append(U16("<td"));

	if (aData->Width.IsValid() && aData->Width.Value != 0)
		b.append(U16(" width='"), aData->Width, U16("'"));

	if (aData->RowSpan)
		b.append(U16(" rowspan='"), aData->RowSpan, U16("'"));

	if (aData->ColSpan)
		b.append(U16(" colspan='"), aData->ColSpan, U16("'"));

	b.append(cssClass(aData, access), U16(" style=\"border-style:"), sld2::html::toString((EBorderStyle)aData->BorderStyle));

	// дефолтное значение не добавляем из-за конфликта с BorderStyle
	if (aData->BorderStyle != eBorderStyleNone && aData->BorderSize.IsValid())
		b.append(U16(";border-width:"), aData->BorderSize);

	b.append(U16(";border-color:"), asColor(aData->BorderColor),
			 U16(";background:"), asColor(aData->BgColor),
			 U16(";text-align:"), sld2::html::toString((ESldTextAlignEnum)aData->TextAlign),
			 U16(";vertical-align:"), sld2::html::toString((ESldVerticalTextAlignEnum)aData->TextVertAlign));

	if (aData->BorderTopWidth.IsValid() && aData->BorderTopWidth != aData->BorderSize)
		b.append(U16(";border-top-width:"), aData->BorderTopWidth);

	if (aData->BorderBottomWidth.IsValid() && aData->BorderBottomWidth != aData->BorderSize)
		b.append(U16(";border-bottom-width:"), aData->BorderBottomWidth);

	if (aData->BorderLeftWidth.IsValid() && aData->BorderLeftWidth != aData->BorderSize)
		b.append(U16(";border-left-width:"), aData->BorderLeftWidth);

	if (aData->BorderRightWidth.IsValid() && aData->BorderRightWidth != aData->BorderSize)
		b.append(U16(";border-right-width:"), aData->BorderRightWidth);

	b.append(U16("\">"));
}

void addBlock(const CSldMetadataProxy<eMetaParagraph> &aData, IDataAccess&, const StateTracker&, StringBuilder &b)
{
	if (aData.isClosing())
	{
		b.append(U16("</div>"));
		return;
	}

	b.append(U16("<div style='"));
	if (aData->TextDirection == eTextDirection_RTL)
		b.append(U16("direction:rtl;unicode-bidi:embed;margin-right:"), aData->Depth);
	else
		b.append(U16("direction:ltr;margin-left:"), aData->Depth);

	b.append(U16(";text-indent:"), aData->Indent,
			 U16(";margin-top:"), aData->MarginTop,
			 U16(";margin-bottom:"), aData->MarginBottom,
			 U16(";text-align:"), sld2::html::toString((ESldTextAlignEnum)aData->Align), U16("'>"));
}

void addBlock(const CSldMetadataProxy<eMetaLabel> &aData, IDataAccess&, const StateTracker&, StringBuilder &b)
{
	if (aData.isClosing())
		b.append(U16("</a>"));
	else
		b.append(U16("<a name=\""), aData.string_ref(aData->Data), U16("\">"));
}

void addBlock(const CSldMetadataProxy<eMetaLink> &aData, IDataAccess&, const StateTracker&, StringBuilder &b)
{
	if (aData.isClosing())
	{
		b.append(U16("</a>"));
	}
	else
	{
		// "sld-link:type:list_index:entry_index:label"
		b.append(U16("<a href=\"sld-link:"), aData->Type, U16(":"), aData->ListIndex, U16(":"),
				 aData->EntryIndex, U16(":"), aData.string_ref(aData->Label), U16("\">"));
	}
}

void addBlock(const StateTracker::HideBlock &aData, IDataAccess &access, const StateTracker &state, StringBuilder &b)
{
	const bool show = access.isShowHideBlocks(state);
	b.append(U16("<span id='hide-body-"), aData.id, U16("' class=\""), aData.label,
			 show ? U16("\" style='display:inline'>") : U16("\" style='display:none'>"));
}

void addBlock(const CSldMetadataProxy<eMetaHide> &aData, IDataAccess &access, const StateTracker &state, StringBuilder &b)
{
	if (aData.isClosing())
		b.append(U16("</span>"));
	else if (!aData->HasControl)
		addBlock(state.hideBlocks.top(), access, state, b);
}

void addBlock(const CSldMetadataProxy<eMetaHideControl> &aData, IDataAccess &access, const StateTracker &state, StringBuilder &b)
{
	const auto& hideBlock = state.hideBlocks.top();
	if (aData.isClosing())
	{
		b.append(U16("</span>"));
		addBlock(hideBlock, access, state, b);
	}
	else
	{
		sld2::IntFormatter fmt;
		const auto& hideControlBlock = state.hideControlBlocks.top();
		const SldU16StringRef hideControlId = fmt.format(hideControlBlock.id);
		b.append(U16("<span id='hide-head-"), hideControlId,
				 U16("' style='cursor:pointer;display:inline;' onclick=\"sld2_hideControl_onClick('hide-head-"),
				 hideControlId, U16("','hide-body-"), hideBlock.id, U16("');\">"));
	}
}

void addBlock(const CSldMetadataProxy<eMetaPopupImage> &aData, IDataAccess &access, const StateTracker&, StringBuilder &b)
{
	// "sld-popup-image:path"
	if (aData.isClosing())
	{
		b.append(U16("</a>"));
	}
	else
	{
		const SldU16String url = resolveImageUrl(aData->PictureIndex, aData, aData->ExtKey, access);
		b.append(U16("<a href=\"sld-popup-image:"), url, U16("\">"));
	}
}

void addBlock(const CSldMetadataProxy<eMetaUrl> &aData, IDataAccess&, const StateTracker&, StringBuilder &b)
{
	if (aData.isClosing())
		b.append(U16("</a>"));
	else
		b.append(U16("<a href=\""), aData.string_ref(aData->Src), U16("\">"));
}

void addBlock(const CSldMetadataProxy<eMetaPopupArticle> &aData, IDataAccess&, const StateTracker&, StringBuilder &b)
{
	if (aData.isClosing())
	{
		b.append(U16("</a>"));
	}
	else
	{
		// "sld-popup-article:list_index:entry_index:label"
		b.append(U16("<a href=\"sld-popup-article:"), aData->ListIndex, U16(":"),
				 aData->EntryIndex, U16(":"), aData.string_ref(aData->Label), U16("\">"));
	}
}

void addBlock(const CSldMetadataProxy<eMetaNoBrText> &aData, IDataAccess&, const StateTracker&, StringBuilder &b)
{
	b.append(aData.isClosing() ? U16("</nobr>") : U16("<nobr>"));
}

void addBlock(const CSldMetadataProxy<eMetaBackgroundImage> &aData, IDataAccess &access, const StateTracker&, StringBuilder &b)
{
	if (aData.isClosing())
	{
		b.append(U16("</span>"));
		return;
	}

	const SldU16StringRef className = access.CSSClassName(aData);
	if (className.empty())
		b.append(U16("<span>"));
	else
		b.append(U16("<span class='"), className, U16("'>"));
}

void addBlock(const CSldMetadataProxy<eMetaMediaContainer> &aData, IDataAccess &access, const StateTracker&, StringBuilder &b)
{
	if (aData.isClosing())
		b.append(U16("</div></div>"));
	else
		b.append(U16("<div><div class='"), access.CSSClassName(aData), U16("' style='display:inline-table'>"));
}

void addBlock(const CSldMetadataProxy<eMetaSwitchState> &aData, IDataAccess &access, const StateTracker &state, StringBuilder &b)
{
	if (aData.isClosing())
	{
		b.append(U16("</div>"));
		return;
	}

	if (state.isInside(eMetaSwitchControl))
	{
		const auto& switchControl = state.switchControlBlocks.top();
		auto thematic = switchControl.thematic < eSwitchThematicsNumber ? switchControl.thematic : eSwitchThematicDefault;
		const UInt32 shownState = access.getDefaultSwitchState(state, thematic);

		static const SldU16StringRef display[] = {
			U16("display:inline'>"),
			U16("display:none'>")
		};

		sld2::IntFormatter fmtId;
		sld2::IntFormatter fmtNumStates;

		const SldU16StringRef id = fmtId.format(switchControl.id);
		const SldU16StringRef numStates = fmtNumStates.format(switchControl.numStates);

		b.append(U16("<div class='sld-switch-control' id='switch-control"), id,
				 U16("-state"), switchControl.stateIndex,
				 U16("' data-control-id='"), id, 
				 U16("' data-state-id='"), switchControl.stateIndex,
		/// при нажатии на контрол вызываем скрипт для каждого управляемого этом контролом блока switch
				 U16("' onclick=\"sld2_switchState_onClick('switch-control"), id,
				 U16("',"), numStates, U16(");"));

		for (SldU16StringRef label : switchControl.managedSwitches)
			b.append(U16(" sld2_switchState_onClick('"), label, U16("',"), numStates, U16(");"));

		b.append(U16("\" title='"), sld2::html::toString(switchControl.thematic),
				 U16("' style='cursor:pointer;"),
				 display[switchControl.stateIndex == shownState ? 0 : 1]);

		for (SldU16StringRef label : switchControl.managedSwitches)
			b.append(U16("<switchmarker data-label='"), label, U16("'></switchmarker>"));
	}
	else
	{
		// Unfortunately we *can* get switch-state blocks outside of switch[-control] ones...
		// At least some versions of OALD9 exhibit this problem.
		if (sld2_unlikely(state.switchBlocks.empty()))
		{
			b.append(U16("</div>"));
			return;
		}

		const auto& switch_ = state.switchBlocks.top();
		auto thematic = switch_.thematic < eSwitchThematicsNumber ? switch_.thematic : eSwitchThematicDefault;
		const UInt32 shownState = access.getDefaultSwitchState(state, thematic);

		static const SldU16StringRef display[] = {
			U16(" style='display:block'>"),
			U16(" style='display:inline'>"),
			U16(" style='display:none'>")
		};

		b.append(U16("<div class='sld-switch-state'"));
		b.append(U16(" data-is-inline='"), switch_.isInline ? U16("1'") : U16("0'"));
		b.append(U16(" data-label='"), switch_.label, U16("'"));
		b.append(U16(" data-state-id='"), switch_.stateIndex, U16("'"));
		if (switch_.manage == eSwitchManagedBySwitchControl)
			b.append(U16(" id='"), switch_.label, U16("-state"), switch_.stateIndex, U16("'"));
		b.append(U16(" title='"), sld2::html::toString(switch_.thematic), U16("'"),
				 display[switch_.stateIndex == shownState ? switch_.isInline : 2]);
	}
}

void addBlock(const CSldMetadataProxy<eMetaDiv> &aData, IDataAccess &access, const StateTracker&, StringBuilder &b)
{
	if (aData.isClosing())
		b.append(U16("</div>"));
	else
		b.append(U16("<div"), cssClass(aData, access), U16(">"));
}

static void generateListItemStyle(SldU16StringRef listItemId, IDataAccess &access, SldU16StringRef marker)
{
	sld2::DynArray<UInt16> buf;
	StringBuilder style(buf);

	style.append(U16("ol li#"), listItemId, U16(":before { content:"));

	static const SldU16StringRef imgScheme = U16("img://");
	if (marker.size() > imgScheme.size() && marker.substr(0, imgScheme.size()) == imgScheme)
	{
		SldU16StringRef indexRef = marker.substr(imgScheme.size());
		if (indexRef.empty())
			return;

		// XXX: we should eventually add StringRef -> integer conversion functions
		// for now rely on the fact that markers are *guaranteed* to be nul-terminated
		UInt32 index;
		if (CSldCompare::StrToUInt32(indexRef.data(), 10, &index) != eOK)
			return;

		SldU16String url = access.ResolveImageUrl(index);
		if (url.empty())
			return;

		style.append(U16("url('"), url, U16("')"));
	}
	else
	{
		style.append(U16("\""), marker, U16("\""));
	}
	style.append(U16(";padding-right:8px;padding-left:4px}"));
	access.InjectCSSStyle(style.flush());
}

void addBlock(const CSldMetadataProxy<eMetaList> &aData, IDataAccess &access, const StateTracker &state, StringBuilder &b)
{
	if (aData.isClosing())
	{
		b.append(U16("</ol>"));
		return;
	}

	const auto& list = state.listBlocks.top();
	if (list.markerType == StateTracker::ListBlock::custom_single)
	{
		sld2::DynArray<UInt16> buf;
		SldU16StringRef itemId = StringBuilder(buf).append(U16("sld-list"), list.id, U16("-item")).flush();
		generateListItemStyle(itemId, access, list.markers.front());
	}

	b.append(U16("<ol id='sld-list"), list.id, U16("' style='"));

	switch (list.markerType)
	{
	case StateTracker::ListBlock::none:
	case StateTracker::ListBlock::lower_cyrillic:
	case StateTracker::ListBlock::upper_cyrillic:
	case StateTracker::ListBlock::custom_single:
	case StateTracker::ListBlock::custom_multi:
		b.append(U16("list-style-type:none;padding-left:0px;'"));
		break;
	case StateTracker::ListBlock::decimal:
	case StateTracker::ListBlock::lower_latin:
	case StateTracker::ListBlock::lower_greek:
	case StateTracker::ListBlock::lower_roman:
	case StateTracker::ListBlock::upper_latin:
	case StateTracker::ListBlock::upper_roman:
		b.append(U16("list-style-type:"), list.markers.front(), U16(";padding-left:20px;'"));
		break;
	case StateTracker::ListBlock::deflt:
	default:
		if (aData->Type == eListTypeUnordered)
			b.append(U16("list-style-type:disc;padding-left:20px;'"));
		else
			b.append(U16("list-style-type:decimal;padding-left:20px;'"));
		break;
	}

	if (aData->Type == eListTypeOrderedReversed)
		b.append(U16(" reversed='true' start='"), aData->Number, U16("'"));

	b.append(U16(">"));
}

void addBlock(const CSldMetadataProxy<eMetaLi> &aData, IDataAccess &access, const StateTracker &state, StringBuilder &b)
{
	if (aData.isClosing())
	{
		b.append(U16("</li>"));
	}
	else
	{
		const auto& list = state.listBlocks.top();
		const auto listStyle = detail::CSSClass{ access.StyleName(list.style) };
		if (list.markerType == StateTracker::ListBlock::custom_single)
		{
			b.append(U16("<li id='sld-list"), list.id, U16("-item'"), listStyle, U16(">"));
		}
		else if (list.markerType == StateTracker::ListBlock::custom_multi ||
				 list.markerType == StateTracker::ListBlock::upper_cyrillic ||
				 list.markerType == StateTracker::ListBlock::lower_cyrillic)
		{
			const UInt32 itemIdx = list.currentItemId;
			UInt32 markerIdx = itemIdx;
			if (list.type == eListTypeOrderedReversed && list.itemsCount != (UInt16)(~0u))
				markerIdx = list.itemsCount - itemIdx - 1;

			sld2::DynArray<UInt16> buf; // XXX: StackBuf for the builder?
			StringBuilder itemId_(buf);

			SldU16StringRef itemId;
			if (list.markerType == StateTracker::ListBlock::custom_multi)
			{
				if (itemIdx >= list.markers.size())
				{
					itemId = itemId_.append(U16("sld-list"), list.id, U16("-item"), itemIdx % list.markers.size()).flush();
				}
				else
				{
					itemId = itemId_.append(U16("sld-list"), list.id, U16("-item"), itemIdx).flush();
					generateListItemStyle(itemId, access, list.markers[markerIdx % list.markers.size()]);
				}
			}
			else // lower_cyrillic || upper_cyrillic
			{
				itemId = itemId_.append(U16("sld-list"), list.id, U16("-item"), itemIdx).flush();
				const UInt16 start = list.markerType == StateTracker::ListBlock::upper_cyrillic ? 0x410 : 0x430;
				const UInt16 temp[] = { static_cast<UInt16>(start + markerIdx % 32), '.', 0 };
				generateListItemStyle(itemId, access, SldU16StringRef(temp, 2));
			}
			b.append(U16("<li id='"), itemId, U16("'"), listStyle, U16(">"));
		}
		else
		{
			b.append(U16("<li"), listStyle, U16(">"));
		}
	}
}

} // namespace handlers
} // anon namespace


template <ESldStyleMetaTypeEnum Type>
void CSldBasicHTMLBuilder::addBlockImpl(const CSldMetadataProxy<Type> &aData)
{
	// XXX:
	// we could try to slightly optimize closing blocks handling as most of them simply
	// unconditionally push a single string which can go straight to CSldBasicHTMLBuilder
	// ::append()
	// doing it in a pretty way is slightly complicated though...
	// one possibility is to leave closing blocks handling inside CSldBasicHTMLBuilder::AddBlock()

	handlers::StringBuilder b(m_appendBuf);
	const sld2::html::StateTracker &state = m_state;
	handlers::addBlock(aData, m_data, state, b);
	if (b.size())
		append(b.flush());
}

void CSldBasicHTMLBuilder::AddText(UInt32 aStyle, SldU16StringRef aText)
{
	addText(aStyle, aText);
	m_state.addTextBlock(eMetaText, aStyle);
}

void CSldBasicHTMLBuilder::AddPhonetics(UInt32 aStyle, SldU16StringRef aText)
{
	addPhonetics(aStyle, aText);
	m_state.addTextBlock(eMetaPhonetics, aStyle);
}

void CSldBasicHTMLBuilder::addText(UInt32 aStyle, SldU16StringRef aText)
{
	if (aText.empty())
		return;

	handlers::StringBuilder b(m_appendBuf);
	const auto style = handlers::detail::CSSClass{ m_data.StyleName(aStyle) };
	append(b.append(U16("<span"), style, U16(">"), aText, U16("</span>")).flush());
}

void CSldBasicHTMLBuilder::addPhonetics(UInt32 aStyle, SldU16StringRef aText)
{
	if (aText.empty())
		return;

	handlers::StringBuilder b(m_appendBuf);
	const auto style = handlers::detail::CSSClass{ m_data.StyleName(aStyle) };
	append(b.append(U16("<nobr><span"), style, U16(">"), aText, U16("</span></nobr>")).flush());
}

void CSldBasicHTMLBuilder::addBlock(const CSldMetadataProxy<eMetaImage> &aData) { addBlockImpl(aData); }
void CSldBasicHTMLBuilder::addBlock(const CSldMetadataProxy<eMetaTable> &aData) { addBlockImpl(aData); }
void CSldBasicHTMLBuilder::addBlock(const CSldMetadataProxy<eMetaTableRow> &aData) { addBlockImpl(aData); }
void CSldBasicHTMLBuilder::addBlock(const CSldMetadataProxy<eMetaTableCol> &aData) { addBlockImpl(aData); }
void CSldBasicHTMLBuilder::addBlock(const CSldMetadataProxy<eMetaParagraph> &aData) { addBlockImpl(aData); }
void CSldBasicHTMLBuilder::addBlock(const CSldMetadataProxy<eMetaLabel> &aData) { addBlockImpl(aData); }
void CSldBasicHTMLBuilder::addBlock(const CSldMetadataProxy<eMetaLink> &aData) { addBlockImpl(aData); }
void CSldBasicHTMLBuilder::addBlock(const CSldMetadataProxy<eMetaHide> &aData) { addBlockImpl(aData); }
void CSldBasicHTMLBuilder::addBlock(const CSldMetadataProxy<eMetaHideControl> &aData) { addBlockImpl(aData); }
void CSldBasicHTMLBuilder::addBlock(const CSldMetadataProxy<eMetaPopupImage> &aData) { addBlockImpl(aData); }
void CSldBasicHTMLBuilder::addBlock(const CSldMetadataProxy<eMetaUrl> &aData) { addBlockImpl(aData); }
void CSldBasicHTMLBuilder::addBlock(const CSldMetadataProxy<eMetaPopupArticle> &aData) { addBlockImpl(aData); }
void CSldBasicHTMLBuilder::addBlock(const CSldMetadataProxy<eMetaNoBrText>& aData) { addBlockImpl(aData); }
void CSldBasicHTMLBuilder::addBlock(const CSldMetadataProxy<eMetaBackgroundImage> &aData) { addBlockImpl(aData); }
void CSldBasicHTMLBuilder::addBlock(const CSldMetadataProxy<eMetaMediaContainer> &aData) { addBlockImpl(aData); }
void CSldBasicHTMLBuilder::addBlock(const CSldMetadataProxy<eMetaSwitchState> &aData) { addBlockImpl(aData); }
void CSldBasicHTMLBuilder::addBlock(const CSldMetadataProxy<eMetaDiv> &aData) { addBlockImpl(aData); }
void CSldBasicHTMLBuilder::addBlock(const CSldMetadataProxy<eMetaList> &aData) { addBlockImpl(aData); }
void CSldBasicHTMLBuilder::addBlock(const CSldMetadataProxy<eMetaLi> &aData) { addBlockImpl(aData); }

namespace {

struct Dispatcher {
	CSldBasicHTMLBuilder &builder;

	template <ESldStyleMetaTypeEnum Type>
	void operator()(const CSldMetadataProxy<Type> &aData) { builder.AddBlock(aData); }
};

} // anon namespace

ESldError CSldBasicHTMLBuilder::Translate(CSldMetadataParser &aParser, ESldStyleMetaTypeEnum aType,
										  UInt32 aStyle, SldU16StringRef aText)
{
	switch (aType)
	{
	case eMetaText:      AddText(aStyle, aText); break;
	case eMetaPhonetics: AddPhonetics(aStyle, aText); break;
	default: return aParser.ParseBlockString(aText, aType, Dispatcher{ *this });
	}
	return eOK;
}
