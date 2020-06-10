#ifndef _SLD_HTML_BUILDER_
#define _SLD_HTML_BUILDER_

#include "SldCSSDataManager.h"
#include "SldIntFormatter.h"
#include "SldStack.h"
#include "SldString.h"
#include "SldMetadataParser.h"

// a bunch of generic helper classes
namespace sld2 {

// compile time sized bitset
template <UInt32 Count>
class BitSet
{
public:
	BitSet() { sld2::memzero(data); }

	// sets all of the bits to 0 (false)
	void zero() { sld2::memzero(data); }

	// sets all of the bits to 1 (true)
	void ones() { sldMemSet(data, 0xff, sizeof(data)); }

	// returns the 'state' of the bit at the given position
	inline bool operator[](UInt32 pos) const { return bitset::test(data, pos); }

	// XXX: doing a mutable operator[] requires a crapton of boilerplate with proxy
	//      structs and other stuff; do we really need it?

	// sets a bit at the given position @pos
	inline void set(UInt32 pos) { bitset::set(data, pos); }

	// clears a bit at the given posisiton @pos
	inline void clear(UInt32 pos) { bitset::clear(data, pos); }

private:
	bitset::Word data[(Count + sizeof(bitset::Word) * 8 - 1) / (sizeof(bitset::Word) * 8)];
};

} // namespace sld2

class CSldStyleInfo;

namespace sld2 {

namespace html {

// return canonical string representations for given enums
SldU16StringRef toString(ESldStyleFontNameEnum aName);
SldU16StringRef toString(EBorderStyle aStyle);
SldU16StringRef toString(ESldTextAlignEnum aAlign);
SldU16StringRef toString(ESldVerticalTextAlignEnum aVAlign);
SldU16StringRef toString(ESldMetaSwitchThematicTypeEnum aType);

// can be used to get "canonical" string representations for given style attributes
class StyleFormatter : public fmt::FormatterBase<64>
{
public:
	// formats font weight
	SldU16StringRef fontWeight(UInt32 aWeight) const;

	// formats font size for enum-based or old-style sizes
	SldU16StringRef fontSize(UInt32 aSize);

	// formats color in 'rgba(r,g,b,a)' form (or "transparent" for fully transparent colors)
	// WARNING:
	//  This function expects the color to be in a "natural" little endian order with alpha
	//  in the lowest 'logical' byte (so first byte in a uint, abgr in memory).
	//  BUT! CSldStyleInfo ::Get[Background]Color() return the color with *blue* in the
	//  lowest byte (effectively making it bgr0 on le). So you *CAN'T* pass it as is even
	//  if you don't care about alpha. Fixing it up requires shifting the color to the left
	//  by a byte and oring the alpha byte in. Smth like
	//    `color = (color << 8) | (alpha & 0xff);`
	SldU16StringRef color(UInt32 aColor);

	// formats line height
	SldU16StringRef lineHeight(UInt32 aHeight, ESldStyleLevelEnum aLevel);
};

/**
 * StyleSerializer
 *
 * For all functions 'touches' denotes what css property may be added by default impl.
 */
class StyleSerializer
{
public:
	virtual ~StyleSerializer() {}

	// handles CSldStyleInfo::GetStyleFontName()
	//  touches: 'font-family'
	virtual void fontName(SldU16String &aString);

	// handles CSldStyleInfo::GetTextSize[Value]()
	//  touches: 'font-size'
	virtual void size(SldU16String &aString);

	// handles CSldStyleInfo::GetBoldValue()
	//  touches: 'font-weight'
	virtual void weight(SldU16String &aString);

	// handles CSldStyleInfo::GetLineHeight[Value]()
	//  touches: 'line-height'
	virtual void lineHeight(SldU16String &aString);

	// handles CSldStyleInfo::IsItalic()
	//  touches: 'font-style'
	virtual void italic(SldU16String &aString);

	// handles CSldStyleInfo ::IsUnderline() ::IsStrikethrough() ::IsOverline()
	//  touches: 'text-decoration'
	virtual void decoration(SldU16String &aString);

	// handles CSldStyleInfo::GetLevel()
	//  touches: 'vertical-align'
	virtual void level(SldU16String &aString);

	// handles CSldStyleInfo::GetColor()
	//  touches: 'color'
	virtual void color(SldU16String &aString);

	// handles CSldStyleInfo::GetBackgroundColor()
	//  touches: 'background-color'
	virtual void bgColor(SldU16String &aString);

	// called at the end of serialization, effectively letting you overwrite anything
	//  touches: 'white-space', 'word-wrap', 'direction', 'unicode-bidi'
	virtual void custom(SldU16String &aString);

	// serializes a given style + variant pair into the passed in string
	// NOTE: does not clear the string!
	void toString(SldU16String &aString, const CSldStyleInfo *aStyle, UInt32 aVariant = SLD_DEFAULT_STYLE_VARIANT_INDEX)
	{
		style = aStyle;
		variantIndex = aVariant;

		fontName(aString);
		size(aString);
		lineHeight(aString);
		italic(aString);
		weight(aString);
		decoration(aString);
		color(aString);
		level(aString);
		bgColor(aString);

		custom(aString);
	}

protected:
	// currently serialized style
	const CSldStyleInfo *style;
	// currently serialized style variant index
	UInt32 variantIndex;
	// embedded style formatter
	StyleFormatter fmt;
};

// creates a css style string for a given media container struct
// NOTE: does not clear the string!
void CreateCSSStyle(SldU16String &aString, const CSldMetadataProxy<eMetaMediaContainer> &aContainer, CSldCSSUrlResolver &aResolver);

// state tracker
class StateTracker
{
protected:

	// a very primitive set based off a sorted vector
	template <typename T>
	class Set
	{
		using Vec = CSldVector<T>;
	public:
		using value_type = typename Vec::value_type;
		using size_type = typename Vec::size_type;
		using const_reference = typename Vec::const_reference;
		using const_iterator = typename Vec::const_iterator;
		static SLD_CONSTEXPR_OR_CONST size_type	npos = Vec::npos;

		// element access
		const_reference operator[](size_type n) const { return data_[n]; }
		size_type find(const_reference v) const { return sld2::sorted_find(data_, v); }

		// capacity
		size_type size() const { return data_.size(); }
		bool empty() const { return data_.empty(); }

		// modifiers
		void clear() { data_.clear(); }
		const_iterator insert(const_reference v) { return sld2::sorted_insert(data_, v); }
		const_iterator insert(value_type&& v) { return sld2::sorted_insert(data_, sld2::move(v)); }

		// iterators
		const_iterator begin() const { return data_.begin(); }
		const_iterator end() const { return data_.end(); }

	private:
		Vec data_;
	};

public:
	// the type of the previous block encountered
	ESldStyleMetaTypeEnum prevBlockType;

	// sets (ordered) of used text & css styles
	Set<UInt32> usedStyles;
	Set<UInt32> usedCSSStyles;

	bool isInside(const ESldStyleMetaTypeEnum aType) const { return BlocksStack[static_cast<UInt8>(aType)] != 0; }

	// cached hide block metadata
	struct HideBlock
	{
		UInt32 id;
		bool hasControl;
		SldU16String label;

		HideBlock(UInt32 aId, const CSldMetadataProxy<eMetaHide> &aData)
			: id(aId), hasControl(aData->HasControl != 0), label(aData.string(aData->Label))
		{}
	};
	// a stack of hide blocks
	Stack<HideBlock> hideBlocks;

	// cached hide control block metadata
	struct HideControlBlock
	{
		UInt32 id;

		HideControlBlock(UInt32 aId, const CSldMetadataProxy<eMetaHideControl>&)
			: id(aId)
		{}
	};
	// a stack of hide control blocks
	Stack<HideControlBlock> hideControlBlocks;

	// cached switch block metadata
	struct SwitchBlock
	{
		bool isInline;
		UInt16 stateIndex;
		UInt16 numStates;
		ESldMetaSwitchManageTypeEnum manage;
		ESldMetaSwitchThematicTypeEnum thematic;
		SldU16String label;

		explicit SwitchBlock(const CSldMetadataProxy<eMetaSwitch> &aData)
			: isInline(aData->Inline != 0), stateIndex(0), numStates(aData->NumStates),
			  manage((ESldMetaSwitchManageTypeEnum)aData->Manage),
			  thematic((ESldMetaSwitchThematicTypeEnum)aData->Thematic),
			  label(aData.string(aData->Label))
		{}
	};
	// a stack of switch blocks
	Stack<SwitchBlock> switchBlocks;

	// cached switch control block metadata
	struct SwitchControlBlock
	{
		UInt32 id;
		UInt16 stateIndex;
		UInt16 numStates;
		ESldMetaSwitchThematicTypeEnum thematic;
		CSldVector<SldU16String> managedSwitches;

		explicit SwitchControlBlock(UInt32 aId, const CSldMetadataProxy<eMetaSwitchControl> &aData)
			: id(aId), stateIndex(0), numStates(aData->NumStates),
			  thematic((ESldMetaSwitchThematicTypeEnum)aData->Thematic)
		{}
	};
	// a stack of switch-control blocks
	Stack<SwitchControlBlock> switchControlBlocks;

	// the counts of switch thematics
	UInt8 switchThematicCounts[eSwitchThematicsNumber];

	// cached list block metadata
	struct ListBlock
	{
		enum MarkerType {
			// these correspond 1-to-1 to named marker types supported by the compiler
			none = 0, decimal, lower_cyrillic, lower_latin, lower_greek, lower_roman,
			upper_cyrillic, upper_latin, upper_roman,
			// these are virtual
			custom_single, custom_multi, deflt
		};

		UInt32 id;
		UInt16 style;
		UInt16 itemsCount;
		ESldListType type;
		MarkerType markerType;
		UInt32 currentItemId;
		sld2::DynArray<SldU16StringRef> markers;

		ListBlock(UInt32 aId, const CSldMetadataProxy<eMetaList> &aData);

	private:
		sld2::DynArray<UInt16> _marker;
	};
	// a stack of list blocks
	Stack<ListBlock> listBlocks;

	// counters used for some metadata blocks' ids
	// can laso be used to effectively get the total number of relevant
	// metadata blocks in the currently tracked "unit"
	struct IDCounters {
		UInt32 hide, hideControl, switch_, switchControl, list;
	} idCounters;

	// script strings
	SldU16String script_OnLoad;
	SldU16String script_OnResize;

public:
	StateTracker();
	virtual ~StateTracker() {}

	// to be used with eMetaText and eMetaPhonetic blocks *after* they are handled
	void addTextBlock(ESldStyleMetaTypeEnum aType, UInt32 aStyle);

	// to be used with metadata blocks
	template <ESldStyleMetaTypeEnum Type, typename Callback>
	void addMetaBlock(const CSldMetadataProxy<Type> &aData, Callback&& cb)
	{
		if (!aData.isClosing())
		{
			addUsedCSSStyle(aData.cssStyleId());
			track(aData);
		}
		else
		{
			BlocksStack[static_cast<UInt8>(Type)]--;
		}

		cb(aData);

		if (!aData.isClosing())
			BlocksStack[static_cast<UInt8>(Type)]++;
		else
			track(aData);

		prevBlockType = Type;
	}

	void clear();

	// updates used css styles
	void addUsedCSSStyle(UInt32 aCSSStyleId);

	// generic catch-all track function
	void track(const CSldMetadataProxyBase&) {}

	// overloads for actually tracked metadata structs
	void track(const CSldMetadataProxy<eMetaHide>&);
	void track(const CSldMetadataProxy<eMetaHideControl>&);
	void track(const CSldMetadataProxy<eMetaSwitch>&);
	void track(const CSldMetadataProxy<eMetaSwitchControl>&);
	void track(const CSldMetadataProxy<eMetaSwitchState>&);
	void track(const CSldMetadataProxy<eMetaManagedSwitch>&);
	void track(const CSldMetadataProxy<eMetaArticleEventHandler>&);
	void track(const CSldMetadataProxy<eMetaList>&);
	void track(const CSldMetadataProxy<eMetaLi>&);

private:
	// a bitset of "are we inside?" flags indexed by the block type
	// XXX: the name is kinda meh...
	sld2::Array<UInt8, eMeta_Last> BlocksStack;
};

namespace detail {

class BuilderIface
{
public:
	virtual ~BuilderIface() {}

protected:
#define ADD_BLOCK(_enum) virtual void addBlock(const CSldMetadataProxy< _enum >&) {}
	SLD_FOREACH_METADATA_TYPE(ADD_BLOCK)
#undef ADD_BLOCK
};

} // namespace detail
} // namespace html
} // namespace sld2

// the builder itself...
class CSldBasicHTMLBuilder : protected sld2::html::detail::BuilderIface
{
public:

	// data access interface
	// XXX: names? naming is complete crap atm
	class IDataAccess : public CSldCSSUrlResolver
	{
	public:
		using CSldCSSUrlResolver::ResolveImageUrl;

		/**
		 * Должна резолвить ссылки на "внешние" картинки
		 *
		 * @param[in] aDictId  - id внешнего словаря
		 * @param[in] aListId  - индекс списка во внешнем словаре
		 * @param[in] aKey     - слово во внешнем словаре
		 *
		 * При невозможности создать валидный путь (вне зависимости от причины)
		 * обязана вернуть пустую строку.
		 *
		 * Полный аналог CSldCSSUrlResolver::ResolveImageUrl() для внешних картинок
		 *
		 * @return строка с путем к картинке (или пустая строка при ошибке)
		 */
		virtual SldU16String ResolveImageUrl(SldU16StringRef aDictId, UInt32 aListId, SldU16StringRef aKey) = 0;

		/**
		 * Должна вернуть css класс стиля для переданного индекса
		 *
		 * @param[in] aStyleId  - индекс стиля
		 *
		 * NOTE: используется дефолтными обработчиками eMetaList, eMetaHideControl,
		 *       eMetaText (::addText()) и eMetaPhonetic (::addPhonetics())
		 *
		 * @return строка для использования в качестве css класса
		 */
		virtual SldU16StringRef StyleName(UInt32 aStyleId) = 0;

		/**
		 * Должна вставить переданный css стиль в генерируемый html документ
		 *
		 * @param[in] aStyleString  - строка с css стилем
		 *
		 * NOTE: на данный момент используется только листами
		 */
		virtual void InjectCSSStyle(SldU16StringRef aStyleString) = 0;

		/**
		 * Должна вернуть css класс стиля для структуры метаданных
		 *
		 * @param[in] aData  - ссылка на структуру метаданных
		 *
		 * Также должна добавить сам стиль в используемые текущим генерируемым
		 * html документом.
		 *
		 * @return строка для использования в качестве css класса
		 */
		virtual SldU16StringRef CSSClassName(const CSldMetadataProxyBase &aData) = 0;

		/**
		 * Должна вернуть css класс стиля для медиа контейнера
		 *
		 * @param[in] aData  - ссылка на структуру метаданных медиа контейнера
		 *
		 * Также должна добавить сам стиль в используемые текущим генерируемым
		 * html документом.
		 * NOTE: используется *только* дефолтным обработчиком eMetaMediaContainer
		 *
		 * @return строка для использования в качестве css класса
		 */
		virtual SldU16StringRef CSSClassName(const CSldMetadataProxy<eMetaMediaContainer> &aContainer) = 0;

		/**
		 * Должна вернуть css класс стиля для фоновой картинки
		 *
		 * @param[in] aData  - ссылка на структуру метаданных медиа контейнера
		 *
		 * Также должна добавить сам стиль в используемые текущим генерируемым
		 * html документом.
		 * NOTE: используется *только* дефолтным обработчиком eMetaBackgroundImage
		 *
		 * @return строка для использования в качестве css класса
		 */
		virtual SldU16StringRef CSSClassName(const CSldMetadataProxy<eMetaBackgroundImage> &aImage) = 0;

		// XXX: all of these names are pretty bad... better naming welcome!

		/**
		 * Должна вернуть дефольное состояние хайд блоков
		 *
		 * @param[in] aData  - ссылка на state билдера
		 *
		 * NOTE: используется *только* дефолтными обработчиками eMetaHide & eMetaHideControl
		 */
		virtual bool isShowHideBlocks(const sld2::html::StateTracker&) { return false; }

		/**
		 * Должна вернуть индекс дефолтного состояние свич блока
		 *
		 * @param[in] aData  - ссылка на state билдера
		 * @param[in] aType  - тип тематик текущего свич блока
		 *
		 * NOTE: используется *только* дефолтными обработчиками eMetaSwitchState
		 */
		virtual UInt32 getDefaultSwitchState(const sld2::html::StateTracker&, ESldMetaSwitchThematicTypeEnum) { return 0; }

		/**
		 * Должна вернуть можно ли генерировать ссылку на полную картинку
		 *
		 * @param[in] aData  - ссылка на state билдера
		 *
		 * NOTE: используется *только* дефолтными обработчиками eMetaImage
		 */
		virtual bool canHaveImageLink(const sld2::html::StateTracker&);
	};

	CSldBasicHTMLBuilder(IDataAccess &aObj) : m_data(aObj) {}
	virtual ~CSldBasicHTMLBuilder() {}

	CSldBasicHTMLBuilder(const CSldBasicHTMLBuilder&) = delete;
	CSldBasicHTMLBuilder& operator=(const CSldBasicHTMLBuilder&) = delete;

	void AddText(UInt32 aStyle, SldU16StringRef aText);
	void AddPhonetics(UInt32 aStyle, SldU16StringRef aText);

	template <ESldStyleMetaTypeEnum Type>
	void AddBlock(const CSldMetadataProxy<Type> &aData)
	{
		m_state.addMetaBlock(aData, AddBlockDispatcher{ *this });
	}

	// the main entry point for dispatching into the builder
	ESldError Translate(CSldMetadataParser &aParser, ESldStyleMetaTypeEnum aType, UInt32 aStyle, SldU16StringRef aText);

	void Clear();

	// returns the default js function used to drive hide controls
	static SldU16StringRef GetHideControlScript();

	// returns the default js function used to drive switch controls
	static SldU16StringRef GetSwitchControlScript();

	// returns the default js function used to init radio-button switch-blocks
	static SldU16StringRef GetRadioButtonInitScript();

	// returns the default js function used to cross reference
	static SldU16StringRef GetCrossRefScript();

	// returns the default js function used to init cross reference
	static SldU16StringRef GetCrossRefInitScript();

	// returns the default js functions used to highlight searched query
	static SldU16StringRef GetSearchHighlightScript();

	// returns the default js functions used to activate disabled ancestor switches
	static SldU16StringRef GetSwitchUncoverScript();

	/// Возвращает строку с двухсимвольным ISO-кодом языка по Sld-коду
	static SldU16StringRef GetISOLanguageCode(const ESldLanguage aSldLangCode);

	/// Возвращает Sld-код языка по строке с ISO-кодом языка
	static ESldLanguage GetLanguageCodeByISO(SldU16StringRef aISOCode);

protected:
	virtual void addText(UInt32, SldU16StringRef);
	virtual void addPhonetics(UInt32, SldU16StringRef);

	using BuilderIface::addBlock;
	void addBlock(const CSldMetadataProxy<eMetaImage>&) override;
	void addBlock(const CSldMetadataProxy<eMetaTable>&) override;
	void addBlock(const CSldMetadataProxy<eMetaTableRow>&) override;
	void addBlock(const CSldMetadataProxy<eMetaTableCol>&) override;
	void addBlock(const CSldMetadataProxy<eMetaParagraph>&) override;
	void addBlock(const CSldMetadataProxy<eMetaLabel>&) override;
	void addBlock(const CSldMetadataProxy<eMetaLink>&) override;
	void addBlock(const CSldMetadataProxy<eMetaHide>&) override;
	void addBlock(const CSldMetadataProxy<eMetaHideControl>&) override;
	void addBlock(const CSldMetadataProxy<eMetaPopupImage>&) override;
	void addBlock(const CSldMetadataProxy<eMetaUrl>&) override;
	void addBlock(const CSldMetadataProxy<eMetaPopupArticle>&) override;
	void addBlock(const CSldMetadataProxy<eMetaNoBrText>&) override;
	void addBlock(const CSldMetadataProxy<eMetaBackgroundImage>&) override;
	void addBlock(const CSldMetadataProxy<eMetaMediaContainer>&) override;
	void addBlock(const CSldMetadataProxy<eMetaSwitchState>&) override;
	void addBlock(const CSldMetadataProxy<eMetaDiv>&) override;
	void addBlock(const CSldMetadataProxy<eMetaList>&) override;
	void addBlock(const CSldMetadataProxy<eMetaLi>&) override;

	/**
	 * Должна добавить строку к генерируемому html документу
	 *
	 * @param[in] aString  - строка
	 *
	 * Билдер старается вызывать этот метод как можно реже
	 * NOTE: строка может не иметь nul-терминатор
	 */
	virtual void append(SldU16StringRef aString) = 0;

	inline bool isInsideMetaBlock(ESldStyleMetaTypeEnum aType) const { return m_state.isInside(aType); }

protected:
	// reference to the data accessor
	IDataAccess &m_data;

	// builder state
	sld2::html::StateTracker m_state;

private:
	// temporary buffer for string building
	sld2::DynArray<UInt16> m_appendBuf;

	struct AddBlockDispatcher {
		CSldBasicHTMLBuilder &builder;

		template <ESldStyleMetaTypeEnum Type>
		void operator()(const CSldMetadataProxy<Type> &aData) { builder.addBlock(aData); }
	};

	template <ESldStyleMetaTypeEnum Type>
	void addBlockImpl(const CSldMetadataProxy<Type> &aData);
};

#endif
