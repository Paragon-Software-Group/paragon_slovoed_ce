#include "ArticlesLoader.h"

#include <algorithm>
#include <initializer_list>
#include <map>
#include <sstream>
#include <type_traits>

#include "Articles.h"
#include "Log.h"
#include "sld_Platform.h"
#include "SlovoEdProject.h"
#include "StringFunctions.h"
#include "Tools.h"
#include "XmlTagContent.h"

map<wstring, UInt32> SlovoEdProject::m_ExternContent[eExternContentTypeMax];

map<UInt32, UInt32> CArticle::m_ThematicToStatesNumberMap;

// small utility helpers
namespace {

static std::wstring lowercase(sld::wstring_ref aStr) {
	std::wstring text(aStr);
	transform(text.begin(), text.end(), text.begin(), towlower);
	return text;
}

// returns if the 2 strings are equal - case insensitive
// obviously only works for ascii only strings
static bool iequals(const sld::wstring_ref &lhs, const sld::wstring_ref &rhs)
{
	if (lhs.size() != rhs.size())
		return false;
	for (size_t i : indices(lhs))
	{
		if (towlower(lhs[i]) != towlower(rhs[i]))
			return false;
	}
	return true;
}

// returns if the 2 strings are equal with '-' being equal to '_'
static bool anameequals(const sld::wstring_ref &lhs, const sld::wstring_ref &rhs)
{
	if (lhs.size() != rhs.size())
		return false;
	for (size_t i : indices(lhs))
	{
		const wchar_t l = lhs[i];
		const wchar_t r = rhs[i];
		if (l != r && !((l == '-' && r == '_') || (l == '_' && r == '-')))
			return false;
	}
	return true;
}

/**
 * Выполняем следующие два преобразования:
 * CurrentTag   -> current_tag (Преобразование прописных букв в строчные + разрыв слов нижним подчеркиванием)
 * current-tag  -> current_tag (замена дефиса на нижнее подчеркивание)
 */
static std::wstring normalizeAttrName(sld::wstring_ref aTag)
{
	std::wstring result;
	result.reserve(aTag.size());

	for (size_t letter : indices(aTag))
	{
		if (L'A' <= aTag[letter] && aTag[letter] <= L'Z')
		{
			if (letter != 0 && aTag[letter - 1] != L'_' && aTag[letter - 1] != L'-')
				result += L'_';

			result += L'a' + (aTag[letter] - L'A');
		}
		else if (aTag[letter] == L'-')
			result += L'_';
		else
			result += aTag[letter];
	}

	return result;
}

} // anon namespace

// switch blocks "state" struct
namespace {

struct SwitchInfo {
	// соответствие label'а блока switch к числовому id (для постпроцесса)
	std::map<std::wstring, UInt32> labelToId;
	// вектор id блоков switch, которые управляются настройками приложения (для проверок)
	std::vector<UInt32> applicationManaged;
	// вектор соответствия id блока switch (просто индекс в массив) к количеству состояний в нем
	// также используется для определения id текущего switch блока
	std::vector<unsigned> stateCounts;
	// лейблы managed switch блоков
	std::vector<std::wstring> managedSwitchLabels;

	void clear()
	{
		labelToId.clear();
		applicationManaged.clear();
		stateCounts.clear();
		managedSwitchLabels.clear();
	}
};

} // anon namespace

struct ArticlesLoader::ParserContext
{
	ArticlesLoader &loader;
	sld::ArenaAllocator<BlockType> &blockAllocator;
	sld::ArenaAllocator<BlockType::Metadata> &metadataAllocator;

	// current article pointer
	CArticle *article;
	// switch block related data, reset for every article
	SwitchInfo switchInfo;
	// per article error count
	unsigned errorCount;
	// total count of empty articles
	unsigned emptyArticlesCount;
	// the stack of tags
	struct {
		std::vector<ESldStyleMetaTypeEnum> data;

		void clear() { data.clear(); }
		NOINLINE void push(ESldStyleMetaTypeEnum v) { data.push_back(v); }
		void pop() { data.pop_back(); }
		ESldStyleMetaTypeEnum top() const { return data.back(); }
	} tagsStack;

	ParserContext(ArticlesLoader &loader) :
		loader(loader), blockAllocator(loader.m_articles.blockAllocator),
		metadataAllocator(loader.m_articles.metadataAllocator),
		article(nullptr), errorCount(0), emptyArticlesCount(0)
	{}

	// this resets all of the per article context state to defaults
	void setArticle(CArticle *article_)
	{
		article = article_;
		errorCount = 0;
		switchInfo.clear();
		tagsStack.clear();
	}

	TMetadataString addMetadataString(sld::wstring_ref aString) {
		return loader.m_metadata.addString(aString);
	}

	template <typename T>
	MetadataRef addMetadata(const T &aData) {
		return loader.m_metadata.addMetadata(aData, article->GetID());
	}

	void addAtomicArticle(sld::wstring_ref id, const std::vector<std::wstring> &refs) {
		loader.m_atomicList.AddAtomicArticle(id, refs);
	}

	uint32_t addAbstractItem(sld::wstring_ref aString) {
		return loader.m_abstractItemContainer.AddItem(aString);
	}

	uint32_t addImage(sld::wstring_ref aString) {
		return loader.m_imageContainer.AddImage(aString);
	}

	uint32_t addScene(sld::wstring_ref aString) {
		return loader.m_sceneContainer.AddScene(aString);
	}

	uint32_t addSound(sld::wstring_ref aString) {
		return loader.m_soundContainer.AddSound(aString);
	}

	uint32_t addVideo(sld::wstring_ref aString) {
		return loader.m_videoContainer.AddVideo(aString);
	}

	uint32_t parseCSSAttribs(sld::wstring_ref aClass, sld::wstring_ref aStyle) {
		auto ret = loader.m_css.parseCSSAttribs(aClass, aStyle, loader.m_imageContainer);
		if (ret.second)
		{
			sldXLog("Warning! Article `%s`: Unknown css class name: '%s'\n",
					sld::as_ref(article->GetID()), aClass);
			errorCount++;
		}
		return ret.first;
	}

	void addPage(CArticle::PageIndex&& page) {
		article->m_Pages.push_back(std::move(page));
	}

	void addBlock(BlockType *block) {
		article->m_blocks.push_back(block);
	}

	uint32_t getStyleIndex(sld::wstring_ref aStyle) const {
		return loader.m_styles.GetStyleNumberByName(aStyle);
	}

	uint32_t getStyleIndex(ESldStyleMetaTypeEnum aType) const {
		return loader.m_styles.GetMetadataStyleNumber(aType);
	}

	BlockType* createMetaBlock(ESldStyleMetaTypeEnum aMetaType)
	{
		BlockType *block = blockAllocator.create(aMetaType, getStyleIndex(aMetaType), std::wstring(), false);
		block->metadata_ = metadataAllocator.create();
		return block;
	}

	// block adding functions
	int addTextBlock(sld::wstring_ref aName, sld::wstring_ref aText);

	void addClosingMetaBlock(ESldStyleMetaTypeEnum aMetaType) {
		addBlock(blockAllocator.create(aMetaType, getStyleIndex(aMetaType), L"close", true));
	}

	template <typename T>
	void addOpeningMetaBlock(const T &aData, uint32_t cssBlockIndex = InvalidCSSBlockIndex)
	{
		BlockType *block = createMetaBlock(aData.metaType);
		block->metadata_->ref = addMetadata(aData);
		block->metadata_->cssBlockIndex = cssBlockIndex;
		addBlock(block);
	}
};

// local typedef
namespace { typedef ArticlesLoader::ParserContext ParserContext; }

// Special named return for when a given tag/attribute was not handled.
// So it's not an error, we simply could not do anything with it.
enum : int { NotHandled = -1 };

// enum traits
namespace {

namespace detail {

// the base template for all defined enums
template <typename T, T Unknown = T(~0), bool CaseSensitive = false>
struct enum_traits_base {
	typedef T enum_type;
	struct descriptor_type {
		const wchar_t *const string;
		enum_type value;
	};
	static const enum_type unknown = Unknown;
	static const bool is_case_sensitive = CaseSensitive;
};
enum class InvalidEnum { Nope };

} // namespace detail

// the base template for enum_traits implementations
template <typename T> struct enum_traits_impl {
	typedef detail::InvalidEnum enum_type;
};

// hell yeah macro magic
#define DECLARE_ENUM_TRAITS(_ENUM_, ...) \
template <> struct enum_traits_impl<_ENUM_> : public detail::enum_traits_base<_ENUM_, ##__VA_ARGS__> { \
	static const descriptor_type data[]; \
}; \
const enum_traits_impl<_ENUM_>::descriptor_type enum_traits_impl<_ENUM_>::data[] =

DECLARE_ENUM_TRAITS(EBorderStyle, eBorderStyleUnknown)
{
	{ L"none",   eBorderStyleNone },
	{ L"hidden", eBorderStyleHidden },
	{ L"dotted", eBorderStyleDotted },
	{ L"dashed", eBorderStyleDashed },
	{ L"solid",  eBorderStyleSolid },
	{ L"double", eBorderStyleDouble },
	{ L"groove", eBorderStyleGroove },
	{ L"ridge",  eBorderStyleRidge },
	{ L"inset",  eBorderStyleInset },
	{ L"outset", eBorderStyleOutset },
};

DECLARE_ENUM_TRAITS(ESldTextAlignEnum, eTextAlign_Unknown)
{
	{ L"left",    eTextAlign_Left },
	{ L"center",  eTextAlign_Center },
	{ L"right",   eTextAlign_Right },
	{ L"justify", eTextAlign_Justify },
};

DECLARE_ENUM_TRAITS(ESldVerticalTextAlignEnum, eVerticalTextAlign_Unknown)
{
	{ L"top",    eVerticalTextAlign_Top },
	{ L"center", eVerticalTextAlign_Center },
	{ L"bottom", eVerticalTextAlign_Bottom },
};

DECLARE_ENUM_TRAITS(ESldAtomicObjectContentType, eContentType_Unknown)
{
	{ L"text",           eContentType_Text },
	{ L"image",          eContentType_Image },
	{ L"video",          eContentType_Video },
	{ L"sound",          eContentType_Sound },
	{ L"3d",             eContentType_3D },
	{ L"slideshow",      eContentType_Slideshow },
	{ L"table",          eContentType_Table },
	{ L"drawing_object", eContentType_DrawingObject },
	{ L"object",         eContentType_DrawingObject },
	{ L"sidenote",       eContentType_Sidenote },
	{ L"diagram",        eContentType_Diagram },
	{ L"test",           eContentType_Test },
};

DECLARE_ENUM_TRAITS(ESldLogicalType, eLogicalType_Unknown)
{
	{ L"problem",     eLogicalType_Problem },
	{ L"task",        eLogicalType_Task },
	{ L"example",     eLogicalType_Example },
	{ L"explanation", eLogicalType_Explanation },
	{ L"fact",        eLogicalType_Fact },
	{ L"additional",  eLogicalType_Additional },
};

DECLARE_ENUM_TRAITS(ESldActivityType, eActivityType_Unknown)
{
	{ L"know",               eActivityType_Know },
	{ L"name",               eActivityType_Name },
	{ L"drawup",             eActivityType_DrawUp },
	{ L"draw_up",            eActivityType_DrawUp },
	{ L"explain",            eActivityType_Explain },
	{ L"characterise",       eActivityType_Characterise },
	{ L"characterize",       eActivityType_Characterise },
	{ L"determine",          eActivityType_Determine },
	{ L"classify",           eActivityType_Classify },
	{ L"display",            eActivityType_Display },
	{ L"conduct_experiment", eActivityType_ConductExperiment },
	{ L"calculate",          eActivityType_Calculate },
	{ L"understand",         eActivityType_Understand },
	{ L"simulate",           eActivityType_Simulate },
	{ L"analyze",            eActivityType_Analyze },
	{ L"compare_differ",     eActivityType_CompareDiffer },
	{ L"describe",           eActivityType_Describe },
	{ L"plan",               eActivityType_Plan },
	{ L"collate",            eActivityType_Collate },
	{ L"prove",              eActivityType_Prove },
	{ L"search_information", eActivityType_SearchInformation },
	{ L"report",             eActivityType_Report },
	{ L"generalize",         eActivityType_Generalize },
	{ L"translate",          eActivityType_Translate },
	{ L"apply_knowledge",    eActivityType_ApplyKnowledge },
	{ L"solve",              eActivityType_Solve },
	{ L"compose",            eActivityType_Compose },
	{ L"dialog",             eActivityType_Dialog },
	{ L"read",               eActivityType_Read },
	{ L"write",              eActivityType_Write },
	{ L"talk",               eActivityType_Talk },
	{ L"listen",             eActivityType_Listen },
	{ L"interpret",          eActivityType_Interpret },
	{ L"evaluate",           eActivityType_Evaluate },
	{ L"define_type",        eActivityType_DefineType },
	{ L"formulate",          eActivityType_Formulate },
	{ L"argue_justify",      eActivityType_ArgueJustify },
	{ L"explore",            eActivityType_Explore },
	{ L"measure",            eActivityType_Measure },
	{ L"perform_tasks",      eActivityType_PerfromTasks },
	{ L"design",             eActivityType_Design },
	{ L"show_on_map",        eActivityType_ShowOnMap },
};

DECLARE_ENUM_TRAITS(ESldCrosswordItemDirection, eCrosswordItemDirectionUnknown)
{
	{ L"horizontal", eCrosswordItemDirectionHorizontal },
	{ L"vertical",   eCrosswordItemDirectionVertical },
};

DECLARE_ENUM_TRAITS(ESldImageAreaType, eImageAreaType_Unknown)
{
	{ L"link",  eImageAreaType_Link },
	{ L"drop",  eImageAreaType_Drop },
	{ L"slide", eImageAreaType_Slide },
	{ L"text",  eImageAreaType_Text },
	{ L"test",  eImageAreaType_Test },
	{ L"url",   eImageAreaType_Url },
	{ L"popup", eImageAreaType_Popup },
};

DECLARE_ENUM_TRAITS(ESldVideoType, eVideoType_Unknown)
{
	{ L"h.264", eVideoType_H264 },
	{ L"h264",  eVideoType_H264 },
};

DECLARE_ENUM_TRAITS(ESldFlow, eFlow_Unknown, true)
{
	{ L"InText",     eFlow_InText },
	{ L"Square",     eFlow_Square },
	{ L"Tight",      eFlow_Tight },
	{ L"Through",    eFlow_Through },
	{ L"TopBottom",  eFlow_TopBottom },
	{ L"FrontText",  eFlow_FrontText },
	{ L"BehindText", eFlow_BehindText },
};

DECLARE_ENUM_TRAITS(ESldPlace, ePlace_Unknown, true)
{
	{ L"InText",      ePlace_InText },
	{ L"UpLeft",      ePlace_UpLeft },
	{ L"UpMiddle",    ePlace_UpMiddle },
	{ L"UpRight",     ePlace_UpRight },
	{ L"MiddleLeft",  ePlace_MiddleLeft },
	{ L"Centre",      ePlace_Centre },
	{ L"Center",      ePlace_Centre },
	{ L"MiddleRight", ePlace_MiddleRight },
	{ L"DownLeft",    ePlace_DownLeft },
	{ L"DownMiddle",  ePlace_DownMiddle },
	{ L"DownRight",   ePlace_DownRight },
};

DECLARE_ENUM_TRAITS(ESldMoveEffect, eMoveEffect_Unknown, true)
{
	{ L"Base", eMoveEffect_Base },
};

DECLARE_ENUM_TRAITS(ESldShowEffect, eShowEffect_Unknown, true)
{
	{ L"Base", eShowEffect_Base },
};

DECLARE_ENUM_TRAITS(ESldStyleLevelEnum, ESldStyleLevelEnum(~0), true)
{
	{ L"Normal", eLevelNormal },
	{ L"Sub",    eLevelSub },
	{ L"Sup",    eLevelSup },
};

DECLARE_ENUM_TRAITS(ESldCaptionType, eCaptionTypeUnknown)
{
	{ L"index",       eCaptionTypeIndex },
	{ L"caption",     eCaptionTypeCaption },
	{ L"explanation", eCaptionTypeExplanation },
};

DECLARE_ENUM_TRAITS(ESldMediaType)
{
	{ L"3d",               eMedia_3d },
	{ L"image",            eMedia_Image },
	{ L"audio",            eMedia_Audio },
	{ L"table",            eMedia_Table },
	{ L"video",            eMedia_Video },
	{ L"slideshow",        eMedia_SlideShow },
	{ L"text",             eMedia_Text },
	{ L"line",             eMedia_Line },
	{ L"marginal",         eMedia_Marginal },
	{ L"creator",          eMedia_Creator },
	{ L"paint",            eMedia_Paint },
	{ L"task_block",       eMedia_TaskBlock },
	{ L"interactive_area", eMedia_InteractiveArea },
	{ L"footnote",         eMedia_Footnote },
	{ L"overlay",          eMedia_Overlay },
	{ L"block",            eMedia_Block },
};

DECLARE_ENUM_TRAITS(ESldTiling)
{
	{ L"horizontal", eTilingHorizontal },
	{ L"vertical",   eTilingVertical },
	{ L"both",       eTilingBoth },
	{ L"spread",     eTilingSpread },
	{ L"no",         eTilingNone },
};

DECLARE_ENUM_TRAITS(ESldDisplay, eDisplay_Unknown)
{
	{ L"inline", eDisplay_Inline },
	{ L"block",  eDisplay_Block },
	{ L"front",  eDisplay_Front },
	{ L"behind", eDisplay_Behind },
};

DECLARE_ENUM_TRAITS(ESldFloat, eFloat_Unknown)
{
	{ L"none",  eFloat_None },
	{ L"left",  eFloat_Left },
	{ L"right", eFloat_Right },
};

DECLARE_ENUM_TRAITS(ESldClearType, eClearType_Unknown)
{
	{ L"none",  eClearType_None },
	{ L"left",  eClearType_Left },
	{ L"right", eClearType_Right },
	{ L"both",  eClearType_Both },
};

DECLARE_ENUM_TRAITS(ESldContainerExtansion)
{
	{ L"yes",   eExtansionFull },
	{ L"no",    eExtansionNone },
	{ L"left",  eExtansionLeft },
	{ L"right", eExtansionRight },
};

DECLARE_ENUM_TRAITS(ESldGradient)
{
	{ L"to_top",          eGradientTop },
	{ L"to_left",         eGradientLeft },
	{ L"to_bottom",       eGradientBottom },
	{ L"to_right",        eGradientRight },
	{ L"to_top_left",     eGradientTopLeft },
	{ L"to_top_right",    eGradientTopRight },
	{ L"to_bottom_left",  eGradientBottomLeft },
	{ L"to_bottom_right", eGradientBottomRight },
};

DECLARE_ENUM_TRAITS(ESldAlignVertical)
{
	{ L"top",    eAlignVTop },
	{ L"centre", eAlignVCentre },
	{ L"center", eAlignVCentre },
	{ L"bottom", eAlignVBottom },
	{ L"no",     eAlignVNone },
};

DECLARE_ENUM_TRAITS(ESldAlignHorizont)
{
	{ L"left",   eAlignHLeft },
	{ L"centre", eAlignHCentre },
	{ L"center", eAlignHCentre },
	{ L"right",  eAlignHRight },
	{ L"no",     eAlignHNone },
};

DECLARE_ENUM_TRAITS(ESldTaskBlockType)
{
	{ L"given",      eTaskBlockGiven },
	{ L"find",       eTaskBlockFind },
	{ L"solution",   eTaskBlockSolvation },
	{ L"prove",      eTaskBlockProve },
	{ L"proof",      eTaskBlockProof },
	{ L"analysis",   eTaskBlockAnalysis },
	{ L"additional", eTaskBlockAdditional },
};

DECLARE_ENUM_TRAITS(ESldMapType)
{
	{ L"layer", eMap_Layer },
	{ L"slide", eMap_Slide },
};

DECLARE_ENUM_TRAITS(ESldLegendItemType)
{
	{ L"default", eLegendItemTypeDefault },
	{ L"fill",    eLegendItemTypeFill },
	{ L"line",    eLegendItemTypeLine },
	{ L"icon",    eLegendItemTypeIcon },
};

DECLARE_ENUM_TRAITS(ESldTextDirectionEnum, eTextDirection_Unknown)
{
	{ L"ltr", eTextDirection_LTR },
	{ L"rtl", eTextDirection_RTL },
};

DECLARE_ENUM_TRAITS(ESldLabelType)
{
	{ L"normal", eLabelNormal },
	{ L"page",   eLabelPage },
	{ L"fts",    eLabelFullTextSearch },
};

DECLARE_ENUM_TRAITS(ESldTextControl)
{
	{ L"sentence", eTextControlSentence },
	{ L"word",     eTextControlWord },
	{ L"free",     eTextControlFree },
};

DECLARE_ENUM_TRAITS(ESldListType)
{
	{ L"unordered",        eListTypeUnordered},
	{ L"ordered",          eListTypeOrdered },
	{ L"ordered_reversed", eListTypeOrderedReversed },
};

DECLARE_ENUM_TRAITS(ESldTestModeEnum)
{
	{ L"exam",      eTestModeExam},
	{ L"test",      eTestModeTesting },
	{ L"practical", eTestModePractical },
};

DECLARE_ENUM_TRAITS(ESldTestDemoEnum)
{ // really?
	{ L"true",  eTestDemoShow },
	{ L"false", eTestDemoHide },
};

DECLARE_ENUM_TRAITS(ESldTestDifficultyEnum)
{
	{ L"1", ESldTestDifficulty1 },
	{ L"2", ESldTestDifficulty2 },
	{ L"3", ESldTestDifficulty3 },
};

DECLARE_ENUM_TRAITS(ESldTestTypeEnum, eTestType_Unknown, true)
{
	{ L"MultipleChoice", eTestType_MultipleChoice },
	{ L"CloseDeletion",  eTestType_CloseDeletion },
	{ L"FreeAnswer",     eTestType_FreeAnswer },
	{ L"Reordering",     eTestType_Reordering },
	{ L"Mapping",        eTestType_Mapping },
	{ L"Highlighting",   eTestType_Highlighting },
	{ L"Handwriting",    eTestType_Handwriting },
	{ L"Crossword",      eTestType_Crossword },
	{ L"Area",           eTestType_Area },
	{ L"Drawing",        eTestType_Drawing },
};

DECLARE_ENUM_TRAITS(ESldTestOrderEnum)
{
	{ L"fixed",  eTestOrderFixed },
	{ L"random", eTestOrderRandom },
};

DECLARE_ENUM_TRAITS(ESldTestInputTypeEnum)
{
	{ L"checkbox", eTestInputType_Checkbox },
	{ L"radio",    eTestInputType_Radio },
	{ L"text",     eTestInputType_Text },
	{ L"file",     eTestInputType_File },
	{ L"link",     eTestInputType_Link },
	{ L"numeric",  eTestInputType_Numeric },
	{ L"one_word", eTestInputType_OneWord },
	{ L"slider",   eTestInputType_Slider },
};

DECLARE_ENUM_TRAITS(ESldMappingTestTypeEnum, eTestMappingTypeUnknown)
{
	{ L"default",    eMappingTestTypeDefault },
	{ L"simple",     eMappingTestTypeSimple },
	{ L"connection", eMappingTestTypeConnection }
};

DECLARE_ENUM_TRAITS(ESldTimeLinePosition)
{
	{ L"above", eTimeLineAbove },
	{ L"below", eTimeLineBelow },
	{ L"left",  eTimeLineLeft },
	{ L"right", eTimeLineRight },
};

DECLARE_ENUM_TRAITS(ESldInteractiveObjectShapeEnum)
{
	{ L"rect",   eInteractiveObjectShape_Rect },
	{ L"circle", eInteractiveObjectShape_Circle },
};

DECLARE_ENUM_TRAITS(ESldAbstractResourceType)
{
	{ L"lab_algebra",		eAbstractResourceAlgebraLab },
	{ L"lab_geometry",		eAbstractResourceGeometryLab },
	{ L"lab_map",			eAbstractResourceMapLab },
	{ L"html_site",			eAbstractResourceHtmlSite },
	{ L"flash",				eAbstractResourceFlash },
	{ L"lab_stereometry",	eAbstractResourceStereometryLab },
	{ L"lab_physics",		eAbstractResourcePhysicsLab },
};

DECLARE_ENUM_TRAITS(ESldMetaSwitchThematicTypeEnum)
{
	{ L"default",		eSwitchThematicDefault },
	{ L"examples",		eSwitchThematicExamples },
	{ L"etymology",		eSwitchThematicEtymology },
	{ L"phrase",		eSwitchThematicPhrase },
	{ L"idioms",		eSwitchThematicIdioms },
	{ L"phonetics",		eSwitchThematicPhonetics },
	{ L"morph",			eSwitchThematicMorph },
	{ L"gram",			eSwitchThematicGram },
	{ L"menu",			eSwitchThematicMenu },
	{ L"radio-button",	eSwitchThematicRadioButton },
	{ L"images",		eSwitchThematicImages },
	{ L"links",			eSwitchThematicLinks },
	{ L"synonyms",		eSwitchThematicSynonyms },
	{ L"antonyms",		eSwitchThematicAntonyms },
	{ L"help-notes",	eSwitchThematicHelpNotes },
	{ L"usage-notes",	eSwitchThematicUsageNotes },
	{ L"abbreviations",	eSwitchThematicAbbreviations },
	{ L"test-hint",		eSwitchThematicTestHint },
};

#undef DECLARE_ENUM_TRAITS

// The main enum_traits template
template <typename T, typename traits = enum_traits_impl<T>>
struct enum_traits {
	typedef typename traits::enum_type enum_type;
	static_assert(!std::is_same<enum_type, detail::InvalidEnum>::value,
				  "Enum traits require an enum_traits_impl specialization declared for a given enum.");
	static const enum_type unknown = traits::unknown;
	static enum_type from_string(const sld::wstring_ref &aString) {
		for (const typename traits::descriptor_type &info : traits::data)
		{
			if (traits::is_case_sensitive ? info.string == aString : iequals(info.string, aString))
				return info.value;
		}
		return unknown;
	}
};

}

// attribute "parsers"
namespace {

template <typename T, T Min, T Max>
struct int_parse_helper_impl {
	static constexpr T min = Min;
	static constexpr T max = Max;
};

template <typename T> struct int_parse_helper {};
template <> struct int_parse_helper<long long> : public int_parse_helper_impl<long long, LLONG_MIN, LLONG_MAX> {
	static long long parse(const wchar_t *s, wchar_t **e, int r) { return wcstoll(s, e, r); }
};
template <> struct int_parse_helper<unsigned long long> : public int_parse_helper_impl<unsigned long long, 0, ULLONG_MAX> {
	static unsigned long long parse(const wchar_t *s, wchar_t **e, int r) { return wcstoull(s, e, r); }
};

template <typename T>
bool parseInt(sld::wstring_ref string, T *value, int radix = 10) {
	static_assert(std::is_integral<T>::value, "The value must be an integer!");

	using helper = int_parse_helper<typename std::conditional<std::is_signed<T>::value, long long, unsigned long long>::type>;

	const STString<128> str(string);
	wchar_t *end;
	const auto v = helper::parse(str.c_str(), &end, radix);
	if (end == str.c_str() || *end != '\0' || ((v == helper::min || v == helper::max) && errno == ERANGE))
		return false;
	if (v > (std::numeric_limits<T>::max)() || v < (std::numeric_limits<T>::min)())
		return false;
	*value = static_cast<T>(v);
	return true;
}

template <typename T>
static int ParseIntAttrib(sld::wstring_ref aAttribValue, T &aVal)
{
	return parseInt(aAttribValue, &aVal) ? ERROR_NO : ERROR_WRONG_STRING_ATTRIBUTE;
}

static int ParseSizeValueAttrib(sld::wstring_ref aAttribValue, TSizeValue &aSizeValue,
								EMetadataUnitType aDefaultUnitType = eMetadataUnitType_UNKNOWN)
{
	TSizeValue size = ParseSizeValue(aAttribValue, aDefaultUnitType);
	if (!size.IsValid())
		return ERROR_WRONG_STRING_ATTRIBUTE;
	aSizeValue = size;
	return ERROR_NO;
}

static int ParseHtmlColorAttrib(sld::wstring_ref aAttribValue, uint32_t &aColor)
{
	const size_t len = aAttribValue.length();
	if (len != 6 && len != 8)
		return ERROR_WRONG_STRING_ATTRIBUTE;

	uint32_t v;
	if (!parseInt(aAttribValue, &v, 16))
		return ERROR_WRONG_STRING_ATTRIBUTE;

	// by default make alpha channel fully opaque
	aColor = len == 8 ? v : (v << 8) | 0xff;
	return ERROR_NO;
}

template <typename T, typename U, typename traits = enum_traits<T>>
static int ParseEnumAttrib(sld::wstring_ref aAttribValue, U &aNum)
{
	static_assert(std::is_integral<U>::value, "The value must be an integer!");

	T eVal = traits::from_string(aAttribValue);
	if (eVal == traits::unknown)
		return ERROR_WRONG_STRING_ATTRIBUTE;

	aNum = eVal;
	return ERROR_NO;
}

/// Парсит ссылку на внешнюю SDC базу
static int ParseExtKey(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue,
					   TMetadataExtKey &aExtKey, ESldExternContentType aExternContentType,
					   const sld::wstring_ref &aDictIdName = L"dictid",
					   const sld::wstring_ref &aListIdxName = L"listidx",
					   const sld::wstring_ref &aKeyName = L"key")
{
	int ret = ERROR_NO;
	if (aName == aDictIdName)
	{
		if (aValue.size() != 4 && aValue.size() != 8)
			return ERROR_WRONG_STRING_ATTRIBUTE;
		SlovoEdProject::m_ExternContent[aExternContentType][to_string(aValue)]++;
		aExtKey.DictId = ctx.addMetadataString(aValue);
	}
	else if (aName == aListIdxName)
		ret = ParseIntAttrib(aValue, aExtKey.ListIdx);
	else if (aName == aKeyName)
		aExtKey.Key = ctx.addMetadataString(aValue);
	else
		ret = NotHandled; // tell the caller that it can't be parsed to an extlink
	return ret;
}

/**
 * Parses bool attributes. Case insensitive.
 *  'true', 'yes', '1' are treated as `true`
 *  'false', 'no', '0' are treated as `false`
 * everything else results in an error
 */
template <typename T>
static int ParseBoolAttrib(sld::wstring_ref aAttribValue, T &aBool)
{
	static_assert(std::is_integral<T>::value, "The value must be an integer!");

	const std::wstring val = lowercase(aAttribValue);
	if (val == L"true" || val == L"yes" || val == L"1")
		aBool = 1;
	else if (val == L"false" || val == L"no" || val == L"0")
		aBool = 0;
	else
		return ERROR_WRONG_STRING_ATTRIBUTE;
	return ERROR_NO;
}

template <typename T>
static int ParsePercentsAttrib(sld::wstring_ref aAttribValue, T &aValue)
{
	static_assert(std::is_integral<T>::value, "The value must be an integer!");

	const TSizeValue val = ParseSizeValue(aAttribValue);
	if (val.Units != eMetadataUnitType_percent || val.Value < 0 || val.Integer() > 100)
		return ERROR_WRONG_STRING_ATTRIBUTE;

	aValue = static_cast<T>(val.Integer());
	return ERROR_NO;
}

} // anon namespace

// fwd decl
static int loadNode(pugi::xml_node aNode, ParserContext &aCtx);

// tag handlers
namespace {
namespace handlers {

// logs a warning updating the error state
static bool logWarning(int err, ParserContext &ctx, pugi::xml_node node, pugi::xml_attribute attr)
{
	if (err == NotHandled)
	{
		sldXLog("Warning! Article `%s`: Unknown <%s> attribute: `%s='%s'`\n",
				sld::as_ref(ctx.article->GetID()), sld::as_ref(node.name()),
				sld::as_ref(attr.name()), sld::as_ref(attr.value()));
		ctx.errorCount++;
	}
	else if (err == ERROR_WRONG_STRING_ATTRIBUTE)
	{
		sldXLog("Warning! Article `%s`: Invalid <%s> `%s` attribute value: `%s`\n",
				sld::as_ref(ctx.article->GetID()), sld::as_ref(node.name()),
				sld::as_ref(attr.name()), sld::as_ref(attr.value()));
		ctx.errorCount++;
	}
	else
		return false;
	return true;
}

namespace detail {

template <typename T>
struct void_type { typedef void type; };

/* Base classes used by TagOp to implement conditional css support.
 */

struct NonStyleableTag {
	uint32_t cssBlockIndex(ParserContext&) const { return InvalidCSSBlockIndex; }
	int parseCSSAttribs(sld::wstring_ref, const wchar_t *aValue) const { return NotHandled; }
};

struct CSSStyleableTag {
	std::wstring _cssClass, _cssStyle;

	uint32_t cssBlockIndex(ParserContext &ctx) const {
		if (_cssClass.empty() && _cssStyle.empty())
			return InvalidCSSBlockIndex;
		return ctx.parseCSSAttribs(_cssClass, _cssStyle);
	}

	int parseCSSAttribs(sld::wstring_ref name, sld::wstring_ref value) {
		int ret = ERROR_NO;
		if (name == L"class")
			_cssClass.assign(value.data(), value.size());
		else if (name == L"style")
			_cssStyle.assign(value.data(), value.size());
		else
			ret = NotHandled;
		return ret;
	}
};

template <bool CSSStyleable>
using CSSTagOpBase = typename std::conditional<CSSStyleable, CSSStyleableTag, NonStyleableTag>::type;

/* Base classes used by TagOp as the main base class (conditional presence of metaData member).
 */

struct StructlessTag {
	BlockType *block = nullptr;
	MetadataRef addMetadata(const ParserContext&) const { return MetadataRef(); }
};

template <ESldStyleMetaTypeEnum Enum>
struct StructuredTag {
	BlockType *block = nullptr;
	typename SldMetadataTraits<Enum>::struct_type metaData;
	MetadataRef addMetadata(ParserContext &ctx) const { return ctx.addMetadata(metaData); }
};

template <ESldStyleMetaTypeEnum Enum>
using StructTagOpBase = typename std::conditional<std::is_same<TMetadataEmptyStruct, typename SldMetadataTraits<Enum>::struct_type>::value,
													StructlessTag, StructuredTag<Enum>>::type;

/* The base template from which the tag handlers are designed to be inherited from.
 * For structured metadata has a `metaData` memeber of the corresponding metadata struct type.
 * Driven by the not-really-designed-to-be-overridable ::load() which has 3 main
 * extension methods (that the inheriting struct may "override"):
 *
 *    int init(ParserContext &ctx, pugi::xml_node node)
 *  called right before the "main" xml attribute parsing "loop"; may effectively be
 *  treated as a "second" constructor; returns are treated as follows:
 *    ERROR_NO - continues normal execution
 *    anything else - immidiately returns to the caller
 *
 *    int attrib(ParserContext &ctx, sld::wstring_ref aName, const wchar_t *aValue)
 *  called for every xml atribute encountered; aValue can be any type constructible from const wchar_t*
 *  returns are treated as follows:
 *    ERROR_NO - continues normal execution
 *    anything else - logs a warning and continues parsing
 *
 *    int finish(ParserContext &ctx, pugi::xml_node node)
 *  called as a final "step" before returning; the return is passed to the caller as-is
 *
 * Implements CRTP (for the curious).
 */
template <typename Impl, ESldStyleMetaTypeEnum Enum, bool CSSStyleable = false>
struct OpenTagOpBase : public StructTagOpBase<Enum>, public CSSTagOpBase<CSSStyleable>
{
	int init(ParserContext &ctx, pugi::xml_node node) const {
		(void)ctx; (void)node; return ERROR_NO;
	};
	int attrib(ParserContext &ctx, sld::wstring_ref aName, const wchar_t *aValue) const {
		(void)ctx; (void)aName; (void)aValue; return NotHandled;
	};
	int finish(ParserContext &ctx, pugi::xml_node node) const {
		(void)ctx; (void)node; return ERROR_NO;
	}

	int load(ParserContext &ctx, pugi::xml_node aNode)
	{
#define DISPATCH(call) static_cast<Impl*>(this)->call
		int err = DISPATCH(init(ctx, aNode));
		if (err != ERROR_NO)
			return err;

		// parse attributes
		for (const pugi::xml_attribute &attr : aNode.attributes())
		{
			const sld::wstring_ref name = attr.name();
			const wchar_t *value = attr.value();
			err = DISPATCH(attrib(ctx, name, value));
			if (err == NotHandled)
				err = DISPATCH(parseCSSAttribs(name, value));
			if (sld2_unlikely(err != ERROR_NO) && !logWarning(err, ctx, aNode, attr))
				return err;
		}

		return DISPATCH(finish(ctx, aNode));
#undef DISPATCH
	}
};

// Base metadata node children handler (to de-templatize it)
struct TagOpBase
{
	// default impl of children handler
	int children(ParserContext &ctx, pugi::xml_node node) const
	{
		for (pugi::xml_node child : node.children())
		{
			// check for text nodes inside metadata tags
			if (sld2_unlikely(child.type() == pugi::node_pcdata))
			{
				sldXLog("Warning! Article `%s`: Text content '%s' inside metadata tag <%s>\n",
						sld::as_ref(ctx.article->GetID()),
						sld::as_ref(child.value()),
						sld::as_ref(node.name()));
				ctx.errorCount++;
				continue;
			}

			int error = ::loadNode(child, ctx);
			if (error != ERROR_NO)
				return error;
		}
		return ERROR_NO;
	}

	// special handler with a list of allowed children metadata types
	int children(ParserContext &ctx, pugi::xml_node node, std::initializer_list<ESldStyleMetaTypeEnum> allowed) const
	{
		for (pugi::xml_node child : node.children())
		{
			// check for text nodes inside metadata tags
			if (sld2_unlikely(child.type() == pugi::node_pcdata))
			{
				sldXLog("Warning! Article `%s`: Text content '%s' inside metadata tag <%s>\n",
						  sld::as_ref(ctx.article->GetID()),
						  sld::as_ref(child.value()),
						  sld::as_ref(node.name()));
				ctx.errorCount++;
				continue;
			}

			const sld::wstring_ref childName = child.name();
			const auto childType = GetTextTypeByTagName(childName);
			if (sld2_unlikely(std::find(allowed.begin(), allowed.end(), childType) == allowed.end()))
			{
				// we can print out allowed tags here if needed
				sldXLog("Error! Article `%s`: non-allowed block (<%s>) inside metadata tag <%s>\n",
						sld::as_ref(ctx.article->GetID()), childName, sld::as_ref(node.name()));
				ctx.errorCount++;
				return ERROR_WRONG_TAG;
			}

			int error = ::loadNode(child, ctx);
			if (error != ERROR_NO)
				return error;
		}
		return ERROR_NO;
	}

	void log_invalidParent(ParserContext &ctx, pugi::xml_node node, const std::initializer_list<ESldStyleMetaTypeEnum> &allowed) const
	{
		std::string allowedParents, u8type;
		for (ESldStyleMetaTypeEnum type : allowed)
		{
			sld::as_utf8(GetTagNameByTextType(type), u8type);
			allowedParents += "<" + u8type + ">/";
		}
		allowedParents.pop_back();
		sldXLog("Error! Article `%s`: <%s> tag encountered outside of allowed parents (%s)\n",
				sld::as_ref(ctx.article->GetID()), sld::as_ref(node.name()), allowedParents);
		ctx.errorCount++;
	}
};

} // namespace detail

template <typename Impl, ESldStyleMetaTypeEnum Enum, bool CSSStyleable = false>
class TagOp : protected detail::TagOpBase
{
protected:
	using Super = TagOp;

public:
	template <typename OpenImpl>
	struct OpenBase : public detail::OpenTagOpBase<OpenImpl, Enum, CSSStyleable>
	{
		using Super = OpenBase;
		Impl *p = nullptr;
	};

private:
	template <typename T, class U = void>
	struct OpenOp { struct S : public OpenBase<S> {}; using type = S; };

	template <typename T>
	struct OpenOp<T, typename detail::void_type<typename T::Open>::type> { using type = typename T::Open; };

public:

	int open(ParserContext &ctx, pugi::xml_node node)
	{
		BlockType *const block = ctx.createMetaBlock(Enum);

		typename OpenOp<Impl>::type op;
		op.p = static_cast<Impl*>(this);
		op.block = block;

		int err = op.load(ctx, node);
		if (sld2_likely(err == ERROR_NO))
		{
			block->metadata_->ref = op.addMetadata(ctx);
			block->metadata_->cssBlockIndex = op.cssBlockIndex(ctx);
			ctx.addBlock(block);
		}
		return err;
	}

	int close(ParserContext &ctx, pugi::xml_node node) const
	{
		ctx.addClosingMetaBlock(Enum);
		return ERROR_NO;
	}

	int load(ParserContext &ctx, pugi::xml_node node)
	{
		int err = static_cast<Impl*>(this)->open(ctx, node);
		if (sld2_unlikely(err != ERROR_NO && err != NotHandled))
			return err;
		const bool shouldAddClosingMetaBlock = err == ERROR_NO;

		ctx.tagsStack.push(Enum);

		err = static_cast<Impl*>(this)->children(ctx, node);
		if (sld2_unlikely(err != ERROR_NO))
			return err;

		ctx.tagsStack.pop();

		if (shouldAddClosingMetaBlock)
			err = static_cast<Impl*>(this)->close(ctx, node);
		return err;
	}

	int open(ParserContext &ctx, pugi::xml_node node, std::initializer_list<ESldStyleMetaTypeEnum> allowed)
	{
		if (sld2_likely(std::find(allowed.begin(), allowed.end(), ctx.tagsStack.top()) != allowed.end()))
			return this->open(ctx, node);

		log_invalidParent(ctx, node, allowed);
		return ERROR_WRONG_TAG;
	}
};

struct AbstractResource : public TagOp<AbstractResource, eMetaAbstractResource>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"url")
				metaData.ItemIdx = ctx.addAbstractItem(aValue);
			else if (aName == L"type")
				ret = ParseEnumAttrib<ESldAbstractResourceType>(aValue, metaData.Type);
			else // eExternContentTypeImage ? really?
				ret = ParseExtKey(ctx, aName, aValue, metaData.ExtKey, eExternContentTypeImage);
			return ret;
		}
	};
};

struct AtomicObject : public TagOp<AtomicObject, eMetaAtomicObject>
{
	struct Open : public OpenBase<Open> {
		sld::wstring_ref atomicId;
		std::vector<std::wstring> atomicRef;

		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"id")
				atomicId = aValue;
			else if (aName.find(L"KES_code") != aName.npos || aName.find(L"FC_code") != aName.npos || aName.find(L"list_entry_id") != aName.npos)
				split(aValue, L'|', atomicRef);
			else if (aName == L"license") // XXX: apparently unused !!!
				ret = NotHandled; /* __swprintf(wbuf, L"%s=\"%s\";\0", aAtribute->name.c_str(), EncodeMetadataTextContent(aAtribute->value).c_str()); */
			else if (aName == L"content_type")
				ret = ParseEnumAttrib<ESldAtomicObjectContentType>(aValue, metaData.ContentType);
			else if (aName == L"logical_type")
				ret = ParseEnumAttrib<ESldLogicalType>(aValue, metaData.LogicalType);
			else if (aName == L"activity_type")
				ret = ParseEnumAttrib<ESldActivityType>(aValue, metaData.ActivityType);
			else
				ret = NotHandled;
			return ret;
		}

		int finish(ParserContext &ctx, pugi::xml_node)
		{
			if (atomicId.empty())
			{
				sldXLog("CArticle::AddBlock : Error! Empty atomic id!\n");
				return ERROR_EMPTY_ARTICLE_ID;
			}
			block->AddText(BlockType::AtomicObjectId, atomicId);
			ctx.addAtomicArticle(atomicId, atomicRef);
			return ERROR_NO;
		}
	};
};

struct BackgroundImage : public TagOp<BackgroundImage, eMetaBackgroundImage>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"url")
				metaData.PictureIndex = ctx.addImage(aValue);
			else if (aName == L"show_width")
				ret = ParseSizeValueAttrib(aValue, metaData.ShowWidth, eMetadataUnitType_px);
			else if (aName == L"show_height")
				ret = ParseSizeValueAttrib(aValue, metaData.ShowHeight, eMetadataUnitType_px);
			else if (aName == L"level")
				ret = ParseEnumAttrib<ESldStyleLevelEnum>(aValue, metaData.Level);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct Caption : public TagOp<Caption, eMetaCaption>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"hide")
				ret = ParseBoolAttrib(aValue, metaData.Hide);
			else if (aName == L"editable")
				ret = ParseBoolAttrib(aValue, metaData.Editable);
			else if (aName == L"type")
				ret = ParseEnumAttrib<ESldCaptionType>(aValue, metaData.Type);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct CrosswordItem : public TagOp<CrosswordItem, eMetaCrossword>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"x")
				ret = ParseIntAttrib(aValue, metaData.X);
			else if (aName == L"y")
				ret = ParseIntAttrib(aValue, metaData.Y);
			else if (aName == L"answer")
				metaData.Answer = ctx.addMetadataString(lowercase(aValue));
			else if (aName == L"init")
				metaData.Init = ctx.addMetadataString(lowercase(aValue));
			else if (aName == L"direction")
				ret = ParseEnumAttrib<ESldCrosswordItemDirection>(aValue, metaData.Direction);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct Div : public TagOp<Div, eMetaDiv, true> {};

struct ExternArticle : public TagOp<ExternArticle, eMetaExternArticle>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"article_id")
				block->AddText(BlockType::ArticleId, aValue, true);
			else if (aName == L"list_entry_id")
				block->AddText(BlockType::ListEntryId, aValue, true);
			else if (aName == L"block_start")
				ret = ParseIntAttrib(aValue, metaData.BlockStart);
			else if (aName == L"block_end")
				ret = ParseIntAttrib(aValue, metaData.BlockEnd);
			else
				ret = ParseExtKey(ctx, aName, aValue, metaData.ExtKey, eExternContentTypeExternArticle);
			return ret;
		}
	};
};

struct FlashCardsLink : public TagOp<FlashCardsLink, eMetaFlashCardsLink>
{
	struct Open : public OpenBase<Open> {
		sld::wstring_ref listEntryIdFront;
		sld::wstring_ref listEntryIdBack;

		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"list_entry_id_front")
				listEntryIdFront = aValue;
			else if (aName == L"list_entry_id_back")
				listEntryIdBack = aValue;
			else
				ret = NotHandled;
			return ret;
		}

		int finish(ParserContext &ctx, pugi::xml_node)
		{
			if (!listEntryIdFront.empty())
				block->AddText(BlockType::FlashCardListEntryIdFront, listEntryIdFront, true);
			if (!listEntryIdBack.empty())
				block->AddText(BlockType::FlashCardListEntryIdBack, listEntryIdBack, true);
			return ERROR_NO;
		}
	};
};

struct Formula : public TagOp<Formula, eMetaFormula>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"src")
				metaData.AbstractItemIdx = ctx.addAbstractItem(aValue);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct Hide : public TagOp<Hide, eMetaHide>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"label")
				metaData.Label = ctx.addMetadataString(aValue);
			else if (aName == L"control")
				ret = ParseBoolAttrib(aValue, metaData.HasControl);
			else
				ret = NotHandled;
			return ret;
		}

		int finish(ParserContext &ctx, pugi::xml_node node)
		{
			// TODO: we can automatically set HasControl depending on the presence of the nested <hide-control>
			if (metaData.HasControl && GetTextTypeByTagName(node.first_child().name()) != eMetaHideControl)
			{
				sldXLog("Warning! Article `%s`: <hide> with control=true does not have a <hide-control>\n",
						sld::as_ref(ctx.article->GetID()));
				ctx.errorCount++;
				return NotHandled;
			}
			return Super::finish(ctx, node);
		}
	};
};

struct HideControl : public TagOp<HideControl, eMetaHideControl>
{
	int open(ParserContext &ctx, pugi::xml_node node)
	{
		if (ctx.tagsStack.top() != eMetaHide)
		{
			log_invalidParent(ctx, node, { eMetaHide });
			return ERROR_WRONG_TAG;
		}

		// so we don't get <hide-control>s outside of hides, the warning is emitted from eMetaHide handler
		if (node != node.parent().first_child())
			return NotHandled;

		return Super::open(ctx, node);
	}
};

struct Image : public TagOp<Image, eMetaImage, true>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"url")
				metaData.PictureIndex = ctx.addImage(aValue);
			else if (aName == L"show_width")
				ret = ParseSizeValueAttrib(aValue, metaData.ShowWidth, eMetadataUnitType_px);
			else if (aName == L"show_height")
				ret = ParseSizeValueAttrib(aValue, metaData.ShowHeight, eMetadataUnitType_px);
			else if (aName == L"full_url" || aName == L"url_f")
				metaData.FullPictureIndex = ctx.addImage(aValue);
			else if (aName == L"show_width_f")
				ret = ParseSizeValueAttrib(aValue, metaData.FullShowWidth, eMetadataUnitType_px);
			else if (aName == L"show_height_f")
				ret = ParseSizeValueAttrib(aValue, metaData.FullShowHeight, eMetadataUnitType_px);
			else if (aName == L"scalable")
				ret = ParseBoolAttrib(aValue, metaData.Scalable);
			else if (aName == L"system")
				ret = ParseBoolAttrib(aValue, metaData.System);
			else
			{
				ret = ParseExtKey(ctx, aName, aValue, metaData.ExtKey, eExternContentTypeImage);
				ret = ret == NotHandled ? ParseExtKey(ctx, aName, aValue, metaData.FullExtKey, eExternContentTypeImage, L"dictid_f", L"listidx_f", L"key_f") : ret;
			}
			return ret;
		}
	};
};

struct ImageArea : public TagOp<ImageArea, eMetaImageArea>
{
	struct Open : public OpenBase<Open> {
		sld::wstring_ref action;

		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"action")
				action = aValue;
			else if (aName == L"type")
				ret = ParseEnumAttrib<ESldImageAreaType>(aValue, metaData.Type);
			else if (aName == L"percent")
				ret = ParseBoolAttrib(aValue, metaData.Percent);
			else if (aName == L"mask")
				metaData.Mask = ctx.addImage(aValue);
			else if (aName == L"id")
				metaData.Id = ctx.addMetadataString(aValue);
			else if (aName == L"width")
				ret = ParseIntAttrib(aValue, metaData.Width);
			else if (aName == L"height")
				ret = ParseIntAttrib(aValue, metaData.Height);
			else if (aName == L"top")
				ret = ParseIntAttrib(aValue, metaData.Top);
			else if (aName == L"left")
				ret = ParseIntAttrib(aValue, metaData.Left);
			else if (aName == L"coords")
				metaData.Coords = ctx.addMetadataString(aValue);
			else
				ret = NotHandled;
			return ret;
		}

		int finish(ParserContext &ctx, pugi::xml_node)
		{
			if (action.empty())
				return ERROR_NO;

			metaData.ActionScript = ctx.addMetadataString(action);
			if ((metaData.Type == eImageAreaType_Link || metaData.Type == eImageAreaType_Slide) && action.size())
				block->AddText(BlockType::ListEntryId, action, true);
			return ERROR_NO;
		}
	};
};

struct InfoBlock : public TagOp<InfoBlock, eMetaInfoBlock>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"type")
				metaData.Type = ctx.addMetadataString(aValue);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct InteractiveObject : public TagOp<InteractiveObject, eMetaInteractiveObject>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			const std::wstring tag = normalizeAttrName(aName);
			if (aName == L"id")
				metaData.Id = ctx.addMetadataString(aValue);
			else if (tag == L"shape")
				ret = ParseEnumAttrib<ESldInteractiveObjectShapeEnum>(aValue, metaData.Shape);
			else if (tag == L"starting_pos")
				metaData.StartingPos = ctx.addMetadataString(aValue);
			else if (tag == L"starting_size")
				metaData.StartingSize = ctx.addMetadataString(aValue);
			else if (tag == L"starting_angle")
			{
				int32_t val;
				ret = ParseIntAttrib(aValue, val);
				if (ret != ERROR_NO || val < 0 || val > 359)
					return ERROR_WRONG_STRING_ATTRIBUTE;
				metaData.StartingAngle = static_cast<uint32_t>(val);
			}
			else if (aName == L"draggable")
				ret = ParseBoolAttrib(aValue, metaData.Draggable);
			else if (aName == L"scalable")
				ret = ParseBoolAttrib(aValue, metaData.Scalable);
			else if (aName == L"rotatable")
				ret = ParseBoolAttrib(aValue, metaData.Rotatable);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct Label : public TagOp<Label, eMetaLabel>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"data")
				metaData.Data = ctx.addMetadataString(aValue);
			else if (aName == L"type")
				ret = ParseEnumAttrib<ESldLabelType>(aValue, metaData.Type);
			else
				ret = NotHandled;
			return ret;
		}

		int finish(ParserContext &ctx, pugi::xml_node node)
		{
			if (metaData.Type == eLabelPage)
			{
				// we expect page numeration labels to have a <text> child that contains the page number
				int index = node.child(L"text").text().as_int(-1);
				if (index > 0)
				{
					CArticle::PageIndex page = { (uint32_t)index, node.attribute(L"data").value() };
					ctx.addPage(std::move(page));
				}
			}
			return ERROR_NO;
		}
	};
};

struct LegendItem : TagOp<LegendItem, eMetaLegendItem>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"type")
				ret = ParseEnumAttrib<ESldLegendItemType>(aValue, metaData.Type);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct Link : public TagOp<Link, eMetaLink>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"title")
				metaData.Title = ctx.addMetadataString(aValue);
			else if (aName == L"label")
				metaData.Label = ctx.addMetadataString(aValue);
			else if (aName == L"link_type")
				ret = ParseIntAttrib(aValue, metaData.Type);
			else if (aName == L"list_entry_id")
				block->AddText(BlockType::ListEntryId, aValue, true);
			else
				ret = ParseExtKey(ctx, aName, aValue, metaData.ExtKey, eExternContentTypeLink);
			return ret;
		}
	};
};

struct List : public TagOp<List, eMetaList>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"type")
				ret = ParseEnumAttrib<ESldListType>(aValue, metaData.Type);
			else if (aName == L"style")
				metaData.Style = ctx.getStyleIndex(aValue);
			else if (aName == L"marker")
			{
				block->AddText(BlockType::ListMarkers, aValue, true);

				std::vector<sld::wstring_ref> markers;
				split(aValue, L'|', markers);
				for (sld::wstring_ref marker : markers)
				{
					if (marker.starts_with(L"img://"))
						ctx.addImage(marker.substr(wcslen(L"img://")));
				}
			}
			else if (aName == L"number")
				ret = ParseIntAttrib(aValue, metaData.Number);
			else
				ret = NotHandled;
			return ret;
		}

		int finish(ParserContext &ctx, pugi::xml_node node)
		{
			auto elems = node.children(L"li");
			auto elemsCount = std::distance(elems.begin(), elems.end());
			if (metaData.Number != UInt16(~0) && metaData.Number != elemsCount)
			{
				sldXLog("Warning! Mismatched number of specified and actual list elements"
						" (specified: %u, actual: %u). Using actual!\n",
						metaData.Number, elemsCount);
			}
			// always set the list elements count to the actual number of them
			metaData.Number = static_cast<decltype(metaData.Number)>(elemsCount);
			return ERROR_NO;
		}
	};

	int children(ParserContext &ctx, pugi::xml_node node) const
	{
		return Super::children(ctx, node, { eMetaLi });
	}
};

struct ListElement : public TagOp<ListElement, eMetaLi>
{
	int open(ParserContext &ctx, pugi::xml_node node)
	{
		return Super::open(ctx, node, { eMetaList });
	}
};

struct ManagedSwitch : public TagOp<ManagedSwitch, eMetaManagedSwitch>
{
	struct Open : public OpenBase<Open> {
		int init(ParserContext &ctx, pugi::xml_node node)
		{
			// instead of adding a string to the metadata manager that will get replaced
			// 100% of the time stuff the string into a "local" cache, which will later
			// be resolved in fixupSwitchBlocks()
			metaData.Label._u32 = static_cast<UInt32>(ctx.switchInfo.managedSwitchLabels.size());
			ctx.switchInfo.managedSwitchLabels.push_back(node.child_value());
			return ERROR_NO;
		}
	};

	int open(ParserContext &ctx, pugi::xml_node node)
	{
		return Super::open(ctx, node, { eMetaSwitchControl });
	}

	// skip all children so we get a single managed-switch added
	int children(ParserContext&, pugi::xml_node) { return ERROR_NO; }
};

struct Map : public TagOp<Map, eMetaMap>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"type")
				ret = ParseEnumAttrib<ESldMapType>(aValue, metaData.Type);
			else if (aName == L"image")
				metaData.PictureIndex = ctx.addImage(aValue);
			else if (aName == L"top")
				metaData.TopPictureIndex = ctx.addImage(aValue);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct MapElement : public TagOp<MapElement, eMetaMapElement>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"name")
				metaData.Name = ctx.addMetadataString(aValue);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct MediaContainer : public TagOp<MediaContainer, eMetaMediaContainer>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			const wstring tag = normalizeAttrName(aName);
			if (tag == L"type")
				ret = ParseEnumAttrib<ESldMediaType>(aValue, metaData.Type);
			else if (tag == L"colour_begin" || tag == L"color_begin")
				ret = ParseHtmlColorAttrib(aValue, metaData.ColourBegin);
			else if (tag == L"colour_end" || tag == L"color_end")
				ret = ParseHtmlColorAttrib(aValue, metaData.ColourEnd);
			else if (tag == L"colour" || tag == L"color")
				ret = ParseHtmlColorAttrib(aValue, metaData.Colour);
			else if (tag == L"background")
				metaData.Background = ctx.addImage(aValue);
			else if (tag == L"tiling")
				ret = ParseEnumAttrib<ESldTiling>(aValue, metaData.Tiling);
			else if (tag == L"display")
				ret = ParseEnumAttrib<ESldDisplay>(aValue, metaData.Display);
			else if (tag == L"float")
				ret = ParseEnumAttrib<ESldFloat>(aValue, metaData.Float);
			else if (tag == L"clear")
				ret = ParseEnumAttrib<ESldClearType>(aValue, metaData.ClearType);
			else if (tag == L"width")
				ret = ParseSizeValueAttrib(aValue, metaData.Width, eMetadataUnitType_px);
			else if (tag == L"height")
				ret = ParseSizeValueAttrib(aValue, metaData.Height, eMetadataUnitType_px);
			else if (tag == L"expansion")
				ret = ParseEnumAttrib<ESldContainerExtansion>(aValue, metaData.Expansion);
			else if (tag == L"interactivity")
				ret = ParseBoolAttrib(aValue, metaData.Interactivity);
			else if (tag == L"zoom_to_full")
				ret = ParseBoolAttrib(aValue, metaData.ZoomToFull);
			else if (tag == L"align_vertical")
				ret = ParseEnumAttrib<ESldAlignVertical>(aValue, metaData.AlignVertical);
			else if (tag == L"align_horizontal")
				ret = ParseEnumAttrib<ESldAlignHorizont>(aValue, metaData.AlignHorizont);
			else if (tag == L"gradient")
				ret = ParseEnumAttrib<ESldGradient>(aValue, metaData.Gradient);
			else if (tag == L"margin_top" && aValue != L"auto")
				ret = ParseSizeValueAttrib(aValue, metaData.MarginTop, eMetadataUnitType_px);
			else if (tag == L"margin_right" && aValue != L"auto")
				ret = ParseSizeValueAttrib(aValue, metaData.MarginRight, eMetadataUnitType_px);
			else if (tag == L"margin_bottom" && aValue != L"auto")
				ret = ParseSizeValueAttrib(aValue, metaData.MarginBottom, eMetadataUnitType_px);
			else if (tag == L"margin_left" && aValue != L"auto")
				ret = ParseSizeValueAttrib(aValue, metaData.MarginLeft, eMetadataUnitType_px);
			else if (tag == L"padding_top" && aValue != L"auto")
				ret = ParseSizeValueAttrib(aValue, metaData.PaddingTop, eMetadataUnitType_px);
			else if (tag == L"padding_right" && aValue != L"auto")
				ret = ParseSizeValueAttrib(aValue, metaData.PaddingRight, eMetadataUnitType_px);
			else if (tag == L"padding_bottom" && aValue != L"auto")
				ret = ParseSizeValueAttrib(aValue, metaData.PaddingBottom, eMetadataUnitType_px);
			else if (tag == L"padding_left" && aValue != L"auto")
				ret = ParseSizeValueAttrib(aValue, metaData.PaddingLeft, eMetadataUnitType_px);
			else if (tag == L"shadow")
				// bullshit
				metaData.Shadow = aValue == L"no" ? eShadowNone : eShadowTopBottom;
			else if (tag == L"overlay_src")
				metaData.OverlaySrc = ctx.addImage(aValue);
			else if (tag == L"overlay_pos")
				ret = ParseEnumAttrib<ESldPlace>(aValue, metaData.OverlayPos);
			else if (tag == L"baseline")
				ret = ParseIntAttrib(aValue, metaData.Baseline);
			else if (tag == L"border_style")
				ret = ParseEnumAttrib<EBorderStyle>(aValue, metaData.BorderStyle);
			else if (tag == L"border_size")
				ret = ParseSizeValueAttrib(aValue, metaData.BorderSize, eMetadataUnitType_px);
			else if (tag == L"border_color")
				ret = ParseHtmlColorAttrib(aValue, metaData.BorderColor);
			else if (tag == L"flow")
				flow(aValue); // deprecated
			else if (aName == L"text-indent")
				ret = ParseSizeValueAttrib(aValue, metaData.TextIndent);
			else
				ret = NotHandled;
			return ret;
		}

		void flow(sld::wstring_ref aValue)
		{
			sld::printf(eLogStatus_Warning, "\nWarning! Using the deprecated attribute \"flow\" detected!");

			if (aValue == L"right")
			{
				metaData.Flow = eFlow_Right;
				metaData.Display = eDisplay_Inline;
				metaData.Float = eFloat_Left;
			}
			else if (aValue == L"left")
			{
				metaData.Flow = eFlow_Left;
				metaData.Display = eDisplay_Inline;
				metaData.Float = eFloat_Right;
			}
			else if (aValue == L"in_text")
			{
				metaData.Flow = eFlow_InText;
				metaData.Display = eDisplay_Inline;
				metaData.Float = eFloat_None;
			}
			else if (aValue == L"top_bottom")
			{
				metaData.Flow = eFlow_TopBottom;
				metaData.Display = eDisplay_Block;
				metaData.Float = eFloat_None;
			}
		}
	};
};

struct Paragraph : public TagOp<Paragraph, eMetaParagraph>
{
	struct Open : public OpenBase<Open> {
		int init(ParserContext&, pugi::xml_node) {
			metaData.Align = ~0;
			return ERROR_NO;
		}

		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"depth")
				ret = ParseSizeValueAttrib(aValue, metaData.Depth, eMetadataUnitType_em);
			else if (aName == L"indent")
				ret = ParseSizeValueAttrib(aValue, metaData.Indent, eMetadataUnitType_em);
			else if (anameequals(aName, L"margin-top"))
				ret = ParseSizeValueAttrib(aValue, metaData.MarginTop, eMetadataUnitType_em);
			else if (anameequals(aName, L"margin-bottom"))
				ret = ParseSizeValueAttrib(aValue, metaData.MarginBottom, eMetadataUnitType_em);
			else if (aName == L"align" && aValue != L"default")
				ret = ParseEnumAttrib<ESldTextAlignEnum>(aValue, metaData.Align);
			else if (aName == L"dir")
				ret = ParseEnumAttrib<ESldTextDirectionEnum>(aValue, metaData.TextDirection);
			else
				ret = NotHandled;
			return ret;
		}

		int finish(ParserContext&, pugi::xml_node) {
			if (metaData.Align == uint16_t(~0))
			{
				if (metaData.TextDirection == eTextDirection_RTL)
					metaData.Align = eTextAlign_Right;
				else
					metaData.Align = eTextAlign_Left;
			}
			return ERROR_NO;
		}
	};
};

struct PopupArticle : TagOp<PopupArticle, eMetaPopupArticle>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"title")
				metaData.Title = ctx.addMetadataString(aValue);
			else if (aName == L"list_entry_id")
				block->AddText(BlockType::ListEntryId, aValue, true);
			else if (aName == L"label")
				metaData.Label = ctx.addMetadataString(aValue);
			else
				ret = ParseExtKey(ctx, aName, aValue, metaData.ExtKey, eExternContentTypeLink);
			return ret;
		}
	};
};

struct PopupImage : public TagOp<PopupImage, eMetaPopupImage>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"url")
				metaData.PictureIndex = ctx.addImage(aValue);
			else if (aName == L"show_width")
				ret = ParseSizeValueAttrib(aValue, metaData.ShowWidth, eMetadataUnitType_px);
			else if (aName == L"show_height")
				ret = ParseSizeValueAttrib(aValue, metaData.ShowHeight, eMetadataUnitType_px);
			else
				ret = ParseExtKey(ctx, aName, aValue, metaData.ExtKey, eExternContentTypeImage);
			return ret;
		}
	};
};

struct Scene3D : public TagOp<Scene3D, eMetaScene>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"url")
				metaData.Id = ctx.addScene(aValue);
			else
				ret = ParseExtKey(ctx, aName, aValue, metaData.ExtKey, eExternContentTypeScene);
			return ret;
		}
	};
};

struct SlideShow : public TagOp<SlideShow, eMetaSlideShow>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"list_id")
				block->AddText(BlockType::SlideShowList, aValue, true);
			else if (aName == L"show_time")
				ret = ParseIntAttrib(aValue, metaData.Time);
			else if (aName == L"place")
				ret = ParseEnumAttrib<ESldPlace>(aValue, metaData.Place);
			else if (aName == L"flow")
				ret = ParseEnumAttrib<ESldFlow>(aValue, metaData.Flow);
			else if (aName == L"move_effect")
				ret = ParseEnumAttrib<ESldMoveEffect>(aValue, metaData.MoveEffect);
			else if (aName == L"show_effect")
				ret = ParseEnumAttrib<ESldShowEffect>(aValue, metaData.ShowEffect);
			else if (aName == L"time_step")
				metaData.TimeStep = ctx.addMetadataString(aValue);
			else if (aName == L"width")
				ret = ParseSizeValueAttrib(aValue, metaData.Width, eMetadataUnitType_px);
			else if (aName == L"height")
				ret = ParseSizeValueAttrib(aValue, metaData.Height, eMetadataUnitType_px);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct Sound : public TagOp<Sound, eMetaSound>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"url")
				metaData.Index = ctx.addSound(aValue);
			else if (aName == L"lang")
				metaData.Lang = ctx.addMetadataString(aValue);
			else
				ret = ParseExtKey(ctx, aName, aValue, metaData.ExtKey, eExternContentTypeSound);
			return ret;
		}
	};
};

static uint16_t switchStatesCount(const ParserContext &ctx, pugi::xml_node node)
{
	const auto states = node.children(L"switch-state");
	const auto numStates = static_cast<uint16_t>(std::distance(states.begin(), states.end()));
	if (numStates <= 1)
	{
		sldXLog("Warning! Article `%s`: <%s> block has %d switch-state%s.\n",
				sld::as_ref(ctx.article->GetID()), sld::as_ref(node.name()),
				numStates, numStates == 1 ? "" : "s");
	}
	return numStates;
}

struct Switch : public TagOp<Switch, eMetaSwitch>
{
	struct Open : public OpenBase<Open> {
		bool hasLabel = false;
		UInt32 switchId = 0;

		int init(ParserContext &ctx, pugi::xml_node node)
		{
			// reset to catch missing attributes
			metaData.Manage = uint16_t(~0u);

			metaData.NumStates = switchStatesCount(ctx, node);

			switchId = static_cast<UInt32>(ctx.switchInfo.stateCounts.size());
			ctx.switchInfo.stateCounts.push_back(metaData.NumStates);
			return ERROR_NO;
		}

		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"inline")
			{
				ret = ParseBoolAttrib(aValue, metaData.Inline);
			}
			else if (aName == L"manage")
			{
				if (aValue == L"switch-control")
				{
					metaData.Manage = eSwitchManagedBySwitchControl;
				}
				else if (aValue == L"application settings")
				{
					metaData.Manage = eSwitchManagedByApplicationSettings;
					ctx.switchInfo.applicationManaged.push_back(switchId);
				}
				else
					ret = ERROR_ILLEGAL_ATTRIBUTE_VALUE_IN_SWITCH_BLOCK;
			}
			else if (aName == L"label")
			{
				std::wstring label = to_string(aValue);
				if (ctx.switchInfo.labelToId.find(label) != ctx.switchInfo.labelToId.end())
					return ERROR_DUPLICATED_SWITCH_BLOCK_LABEL;
				ctx.switchInfo.labelToId.emplace(std::move(label), switchId);
				metaData.Label = ctx.addMetadataString(STString<64>(L"switch%u", switchId));
				hasLabel = true;
			}
			else if (aName == L"thematic")
			{
				ret = ParseEnumAttrib<ESldMetaSwitchThematicTypeEnum>(aValue, metaData.Thematic);
				if (ret == ERROR_NO)
				{
					UInt32 &globalCount = CArticle::m_ThematicToStatesNumberMap[metaData.Thematic];
					if (globalCount == 0)
						globalCount = metaData.NumStates;
					else if (globalCount != metaData.NumStates)
						return ERROR_WRONG_NUMBER_OF_STATES_IN_SWITCH_BLOCKS;
				}
			}
			else
				ret = NotHandled;
			return ret;
		}

		int finish(ParserContext &ctx, pugi::xml_node)
		{
			// проверяем инварианты атрибутов свитч блоков
			if (metaData.Manage == uint16_t(~0u) ||
				(metaData.Manage == eSwitchManagedBySwitchControl && !hasLabel) ||
				(metaData.Manage == eSwitchManagedByApplicationSettings && metaData.Thematic == uint16_t(~0u)))
			{
				return ERROR_NOT_ENOUGH_ATTRIBUTES_IN_SWITCH_BLOCK;
			}
			return ERROR_NO;
		}
	};

	int children(ParserContext &ctx, pugi::xml_node node) const
	{
		return Super::children(ctx, node, { eMetaSwitchState });
	}
};

struct SwitchControl : public TagOp<SwitchControl, eMetaSwitchControl>
{
	struct Open : public OpenBase<Open> {
		int init(ParserContext &ctx, pugi::xml_node node)
		{
			metaData.NumStates = switchStatesCount(ctx, node);
			return ERROR_NO;
		}

		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"thematic")
				ret = ParseEnumAttrib<ESldMetaSwitchThematicTypeEnum>(aValue, metaData.Thematic);
			else
				ret = NotHandled;
			return ret;
		}
	};

	int children(ParserContext &ctx, pugi::xml_node node) const
	{
		return Super::children(ctx, node, { eMetaSwitchState, eMetaManagedSwitch });
	}
};

struct SwitchState : public TagOp<SwitchState, eMetaSwitchState>
{
	int open(ParserContext &ctx, pugi::xml_node node)
	{
		return Super::open(ctx, node, { eMetaSwitch, eMetaSwitchControl });
	}
};

struct Table : public TagOp<Table, eMetaTable, true>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"width")
			{
				// we have to check for "old" named widths: full & auto
				TSizeValue size = ParseSizeValue(aValue);
				if (size.IsValid())
					metaData.Width = size;
				else if (iequals(aValue, L"full"))
					metaData.Width = TSizeValue(100 * SLD_SIZE_VALUE_SCALE, eMetadataUnitType_percent);
				else if (!iequals(aValue, L"auto"))
					return ERROR_WRONG_STRING_ATTRIBUTE;
			}
			else if (aName == L"cellpadding")
				ret = ParseSizeValueAttrib(aValue, metaData.CellPadding, eMetadataUnitType_em);
			else if (aName == L"cellspacing")
				ret = ParseSizeValueAttrib(aValue, metaData.CellSpacing, eMetadataUnitType_em);
			else
				ret = NotHandled;
			return ret;
		}
	};

	int children(ParserContext &ctx, pugi::xml_node node) const
	{
		return Super::children(ctx, node, { eMetaTableRow });
	}
};

struct TableCell : public TagOp<TableCell, eMetaTableCol, true>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"rowspan")
				ret = ParseIntAttrib(aValue, metaData.RowSpan);
			else if (aName == L"colspan")
				ret = ParseIntAttrib(aValue, metaData.ColSpan);
			else if (aName == L"bgcolor")
				ret = ParseHtmlColorAttrib(aValue, metaData.BgColor);
			else if (anameequals(aName, L"border-style"))
				ret = ParseEnumAttrib<EBorderStyle>(aValue, metaData.BorderStyle);
			else if (anameequals(aName, L"border-size"))
				ret = ParseSizeValueAttrib(aValue, metaData.BorderSize, eMetadataUnitType_px);
			else if (anameequals(aName, L"border-color"))
				ret = ParseHtmlColorAttrib(aValue, metaData.BorderColor);
			else if (aName == L"width")
				ret = ParseSizeValueAttrib(aValue, metaData.Width);
			else if (anameequals(aName, L"text-align"))
				ret = ParseEnumAttrib<ESldTextAlignEnum>(aValue, metaData.TextAlign);
			else if (anameequals(aName, L"text-valign"))
				ret = ParseEnumAttrib<ESldVerticalTextAlignEnum>(aValue, metaData.TextVertAlign);
			else if (aName == L"border-top-width")
				ret = ParseSizeValueAttrib(aValue, metaData.BorderTopWidth, eMetadataUnitType_px);
			else if (aName == L"border-bottom-width")
				ret = ParseSizeValueAttrib(aValue, metaData.BorderBottomWidth, eMetadataUnitType_px);
			else if (aName == L"border-left-width")
				ret = ParseSizeValueAttrib(aValue, metaData.BorderLeftWidth, eMetadataUnitType_px);
			else if (aName == L"border-right-width")
				ret = ParseSizeValueAttrib(aValue, metaData.BorderRightWidth, eMetadataUnitType_px);
			else
				ret = NotHandled;
			return ret;
		}
	};

	int open(ParserContext &ctx, pugi::xml_node node)
	{
		return Super::open(ctx, node, { eMetaTableRow });
	}
};

struct TableRow : public TagOp<TableRow, eMetaTableRow, true>
{
	int open(ParserContext &ctx, pugi::xml_node node)
	{
		return Super::open(ctx, node, { eMetaTable });
	}

	int children(ParserContext &ctx, pugi::xml_node node) const
	{
		return Super::children(ctx, node, { eMetaTableCol });
	}
};

struct TaskBlockEntry : public TagOp<TaskBlockEntry, eMetaTaskBlockEntry>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"type")
				ret = ParseEnumAttrib<ESldTaskBlockType>(aValue, metaData.Type);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct Test : public TagOp<Test, eMetaTest>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"type")
				ret = ParseEnumAttrib<ESldTestTypeEnum>(aValue, metaData.Type);
			else if (aName == L"order")
				ret = ParseEnumAttrib<ESldTestOrderEnum>(aValue, metaData.Order);
			else if (aName == L"difficulty")
				ret = ParseEnumAttrib<ESldTestDifficultyEnum>(aValue, metaData.Difficulty);
			else if (aName == L"max_points")
				ret = ParseIntAttrib(aValue, metaData.MaxPoints);
			else if (aName == L"draft")
				ret = ParseBoolAttrib(aValue, metaData.HasDraft);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct TestContainer : public TagOp<TestContainer, eMetaTestContainer>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"max_points")
				ret = ParseIntAttrib(aValue, metaData.MaxPoints);
			else if (aName == L"mode")
				ret = ParseEnumAttrib<ESldTestModeEnum>(aValue, metaData.Mode);
			else if (aName == L"show_in_demo")
				ret = ParseEnumAttrib<ESldTestDemoEnum>(aValue, metaData.Demo);
			else if (aName == L"difficulty")
				ret = ParseEnumAttrib<ESldTestDifficultyEnum>(aValue, metaData.Difficulty);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct TestControl : public TagOp<TestControl, eMetaTestControl>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"checklist")
				metaData.Checklist = ctx.addMetadataString(aValue);
			else if (aName == L"unique")
				ret = ParseBoolAttrib(aValue, metaData.Unique);
			else if (aName == L"type")
			{
				ret = ParseEnumAttrib<ESldMappingTestTypeEnum>(aValue, metaData.Type);
				if (ret != ERROR_NO)
				{
					sld::printf(eLogStatus_Warning, "\nUndefined type of mapping test. Used 'default'");
					ret = ERROR_NO;
				}
			}
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct TestInput : public TagOp<TestInput, eMetaTestInput>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"type")
				ret = ParseEnumAttrib<ESldTestInputTypeEnum>(aValue, metaData.Type);
			else if (aName == L"group")
				metaData.Group = ctx.addMetadataString(aValue);
			else if (aName == L"answer")
				metaData.Answers = ctx.addMetadataString(aValue);
			else if (aName == L"init")
				metaData.Initial = ctx.addMetadataString(aValue);
			else if (aName == L"cols")
				ret = ParseIntAttrib(aValue, metaData.Cols);
			else if (aName == L"rows")
				ret = ParseIntAttrib(aValue, metaData.Rows);
			else if (aName == L"level")
				ret = ParseEnumAttrib<ESldStyleLevelEnum>(aValue, metaData.Level);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct TestResultElement : public TagOp<TestResultElement, eMetaTestResultElement>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"color" || aName == L"colour")
				ret = ParseHtmlColorAttrib(aValue, metaData.Color);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct TestSpear : public TagOp<TestSpear, eMetaTestSpear>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"id")
				metaData.Id = ctx.addMetadataString(aValue);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct TestTarget : public TagOp<TestTarget, eMetaTestTarget>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"id")
				metaData.Id = ctx.addMetadataString(aValue);
			else if (aName == L"link")
				metaData.Links = ctx.addMetadataString(aValue);
			else if (aName == L"static")
				ret = ParseBoolAttrib(aValue, metaData.Static);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct TestToken : public TagOp<TestToken, eMetaTestToken>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"order")
				ret = ParseIntAttrib(aValue, metaData.Order);
			else if (aName == L"group")
				metaData.Group = ctx.addMetadataString(aValue);
			else if (aName == L"text")
				metaData.Text = ctx.addMetadataString(aValue);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct TextControl : public TagOp<TextControl, eMetaTextControl>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"scope")
				ret = ParseEnumAttrib<ESldTextControl>(aValue, metaData.Scope);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct TimeLine : public TagOp<TimeLine, eMetaTimeLine>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			const std::wstring tag = normalizeAttrName(aName);
			if (aName == L"width")
				ret = ParseSizeValueAttrib(aValue, metaData.Width);
			else if (aName == L"height")
				ret = ParseSizeValueAttrib(aValue, metaData.Height);
			else if (tag == L"width_line")
				ret = ParseSizeValueAttrib(aValue, metaData.LineWidth);
			else if (tag == L"height_line")
				ret = ParseSizeValueAttrib(aValue, metaData.LineHeight);
			else if (tag == L"position_line")
				ret = ParseEnumAttrib<ESldTimeLinePosition>(aValue, metaData.Position);
			else if (tag == L"default_time")
				ret = ParsePercentsAttrib(aValue, metaData.DefaultTime);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct TimeLineItem : public TagOp<TimeLineItem, eMetaTimeLineItem>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			const std::wstring tag = normalizeAttrName(aName);
			if (tag == L"internal_list_index")
				ret = ParseIntAttrib(aValue, metaData.ListIndex);
			else if (tag == L"internal_word_index")
				ret = ParseIntAttrib(aValue, metaData.EntryIndex);
			else if (aName == L"id")
				metaData.Id = ctx.addMetadataString(aValue);
			else if (tag == L"start_time")
				ret = ParsePercentsAttrib(aValue, metaData.StartTime);
			else if (tag == L"end_time")
				ret = ParsePercentsAttrib(aValue, metaData.EndTime);
			else
				ret = ParseExtKey(ctx, aName, aValue, metaData.ExtKey, eExternContentTypeExternArticle,
								  L"external_dict_id", L"external_list_index", L"external_key");
			return ret;
		}
	};
};

struct UiElement : public TagOp<UiElement, eMetaUiElement>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"type")
				metaData.Type = ctx.addMetadataString(aValue);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct Url : public TagOp<Url, eMetaUrl>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"src")
				metaData.Src = ctx.addMetadataString(aValue);
			else
				ret = NotHandled;
			return ret;
		}
	};
};

struct VideoSource : public TagOp<VideoSource, eMetaVideoSource>
{
	struct Open : public OpenBase<Open> {
		int attrib(ParserContext &ctx, sld::wstring_ref aName, sld::wstring_ref aValue)
		{
			int ret = ERROR_NO;
			if (aName == L"url")
				metaData.Id = ctx.addVideo(aValue);
			else if (aName == L"codec")
				ret = ParseEnumAttrib<ESldVideoType>(aValue, metaData.Codec);
			else if (aName == L"resolution")
				ret = ParseIntAttrib(aValue, metaData.Resolution);
			else
				ret = ParseExtKey(ctx, aName, aValue, metaData.ExtKey, eExternContentTypeVideo);
			return ret;
		}
	};
};

// "dummy" handler for structless metadata
template <ESldStyleMetaTypeEnum Enum>
struct Structless : public TagOp<Structless<Enum>, Enum> {};

template <typename Op>
int parse(ParserContext &ctx, pugi::xml_node aNode)
{
	Op op;
	return op.load(ctx, aNode);
}

typedef int(*HandlerFunc)(ParserContext &ctx, pugi::xml_node aNode);
static HandlerFunc find(ESldStyleMetaTypeEnum aType)
{
	static const HandlerFunc handlers[] = {
		/* [eMetaText] = */              nullptr,
		/* [eMetaPhonetics] = */         nullptr,
		/* [eMetaImage] = */             parse<Image>,
		/* [eMetaSound] = */             parse<Sound>,
		/* [eMetaTable] = */             parse<Table>,
		/* [eMetaTableRow] = */          parse<TableRow>,
		/* [eMetaTableCol] = */          parse<TableCell>,
		/* [eMetaParagraph] = */         parse<Paragraph>,
		/* [eMetaLabel] = */             parse<Label>,
		/* [eMetaLink] = */              parse<Link>,
		/* [eMetaHide] = */              parse<Hide>,
		/* [eMetaHideControl] = */       parse<HideControl>,
		/* [eMetaTest] = */              parse<Test>,
		/* [eMetaTestInput] = */         parse<TestInput>,
		/* [eMetaTestToken] = */         parse<TestToken>,
		/* [eMetaPopupImage] = */        parse<PopupImage>,
		/* [eMetaUrl] = */               parse<Url>,
		/* [eMetaUiElement] = */         parse<UiElement>,
		/* [eMetaPopupArticle] = */      parse<PopupArticle>,
		/* [eMetaNoBrText] = */          parse<Structless<eMetaNoBrText>>,
		/* [eMetaInfoBlock] = */         parse<InfoBlock>,
		/* [eMetaBackgroundImage] = */   parse<BackgroundImage>,
		/* [eMetaFlashCardsLink] = */    parse<FlashCardsLink>,
		/* [eMetaVideo] = */             parse<Structless<eMetaVideo>>,
		/* [eMetaScene] = */             parse<Scene3D>,
		/* [eMetaImageArea] = */         parse<ImageArea>,
		/* [eMetaSlideShow] = */         parse<SlideShow>,
		/* [eMetaVideoSource] = */       parse<VideoSource>,
		/* [eMetaMediaContainer] = */    parse<MediaContainer>,
		/* [eMetaTestSpear] = */         parse<TestSpear>,
		/* [eMetaTestTarget] = */        parse<TestTarget>,
		/* [eMetaTestControl] = */       parse<TestControl>,
		/* [eMetaSwitch] = */            parse<Switch>,
		/* [eMetaSwitchControl] = */     parse<SwitchControl>,
		/* [eMetaSwitchState] = */       parse<SwitchState>,
		/* [eMetaManagedSwitch] = */     parse<ManagedSwitch>,
		/* [eMetaDiv] = */               parse<Div>,
		/* [eMetaMap] = */               parse<Map>,
		/* [eMetaMapElement] = */        parse<MapElement>,
		/* [eMetaCaption] = */           parse<Caption>,
		/* [eMetaTestResult] = */        parse<Structless<eMetaTestResult>>,
		/* [eMetaTestResultElement] = */ parse<TestResultElement>,
		/* [eMetaTextControl] = */       parse<TextControl>,
		/* [eMetaTaskBlockEntry] = */    parse<TaskBlockEntry>,
		/* [eMetaSidenote] = */          parse<Structless<eMetaSidenote>>,
		/* [eMetaConstructionSet] = */   parse<Structless<eMetaConstructionSet>>,
		/* [eMetaDrawingBlock] = */      parse<Structless<eMetaDrawingBlock>>,
		/* [eMetaArticleEventHandler] =*/nullptr,
		/* [eMetaDemoLink] = */          parse<Structless<eMetaDemoLink>>,
		/* [eMeta_UnusedBroken] = */     nullptr,
		/* [eMetaTestContainer] = */     parse<TestContainer>,
		/* [eMetaLegendItem] = */        parse<LegendItem>,
		/* [eMetaAtomicObject] = */      parse<AtomicObject>,
		/* [eMetaCrossword] = */         parse<CrosswordItem>,
		/* [eMetaExternArticle] = */     parse<ExternArticle>,
		/* [eMetaList] = */              parse<List>,
		/* [eMetaLi] = */                parse<ListElement>,
		/* [eMetaInteractiveObject] = */ parse<InteractiveObject>,
		/* [eMeta_Unused0] = */          nullptr,
		/* [eMetaTimeLine] = */          parse<TimeLine>,
		/* [eMetaTimeLineItem] = */      parse<TimeLineItem>,
		/* [eMetaAbstractResource] = */  parse<AbstractResource>,
		/* [eMetaFormula] = */           parse<Formula>,
		/* [eMetaCrosswordHint] = */     parse<Structless<eMetaCrosswordHint>>,
		/* [eMetaFootnoteBrief] = */     parse<Structless<eMetaFootnoteBrief>>,
		/* [eMetaFootnoteTotal] = */     parse<Structless<eMetaFootnoteTotal>>,
	};
	static_assert(sld::array_size(handlers) == eMeta_Last,
				  "Stale metadata handlers table.");
	return aType < sld::array_size(handlers) ? handlers[aType] : nullptr;
}

} // namespace handlers
} // anon namespace

// generic article loader
// basically calls @load for every "article" child of the passed in node
// spewing the relevant warnings/errors and handling return values
// Loader is passed an article node and must return an error code
template <typename Loader>
static int loadArticles(pugi::xml_node aNode, Loader load)
{
	for (pugi::xml_node node : aNode)
	{
		if (node.type() != pugi::node_element)
			continue;

		const sld::wstring_ref nodeName = node.name();
		if (nodeName != L"article")
		{
			sldXLog("Error! Unknown tag in articles: <%s>\n", nodeName);
			CLogW::PrintToConsole("[FAIL]", eLogStatus_Error);
			return ERROR_UNKNOWN_TAG;
		}

		if (node.first_child().empty())
		{
			sldXLog("Warning! Empty article tag\n");
			continue;
		}

		int error = load(node);
		if (error != ERROR_NO)
		{
			CLogW::PrintToConsole("[FAIL]", eLogStatus_Error);
			return error;
		}
	}
	return ERROR_NO;
}

/**
 * Производит загрузку статей из файла
 *
 * @param[in] aFileName - имя файла со статьями
 *
 * @return код ошибки
 */
int ArticlesLoader::Load(const std::wstring &aFileName)
{
	m_error = 0;

	sldILog("Processing file `%s`\n", sld::as_ref(aFileName));
	sld::printf("\nLoading articles from file `%s`", sld::as_ref(aFileName));

	// Загрузка xml-файла
	m_timer.Start();
	pugi::xml_document XmlDoc;
	const auto result = XmlDoc.load_file(aFileName.c_str());
	if (!result)
	{
		sldILog("Error! Can't load xml file `%s`: %s\n", sld::as_ref(aFileName), result.description());
		CLogW::PrintToConsole("[FAIL]", eLogStatus_Error);
		return ERROR_CANT_OPEN_FILE;
	}

	m_timer.Stop();
	m_timer.PrintTimeStringW(L"Loading xml file \"" + aFileName + L"\" by pugi parser");
	m_timer.PrintMemoryString("Memory Usage");

	// Парсинг
	m_timer.Start();

	pugi::xml_node RootNode = XmlDoc.first_child();
	wstring RootNodeName(RootNode.name());

	if (RootNodeName != L"Articles")
	{
		sldXLog("Error! Wrong articles tag: %s\n", sld::as_ref(RootNodeName));
		CLogW::PrintToConsole("[FAIL]", eLogStatus_Error);
		return ERROR_WRONG_TAG;
	}

	if (!XmlDoc.first_child().first_child())
	{
		sldXLog("Error! Empty Articles tag!\n");
		CLogW::PrintToConsole("[FAIL]", eLogStatus_Error);
		return ERROR_WRONG_TAG;
	}

	struct ErrorStats {
		unsigned errorCount;
		unsigned articleCount;
	} errorStats = { 0, 0 };

	const auto printCount = [](UInt32 i, const ErrorStats &errorStats)
	{
		sld::printf("\rLoaded %d articles", i);
		if (errorStats.errorCount > 0)
		{
			sld::printf(eLogStatus_Warning, " [%u warning%s]", errorStats.errorCount,
											errorStats.errorCount == 1 ? "" : "s");
		}
	};

	ParserContext ctx(*this);

	sld::printf("\nLoaded 0 articles");
	UInt32 i = 0;
	RateLimit rlimit;
	int error = loadArticles(RootNode, [&](pugi::xml_node node) {
		int error = LoadArticle(node, ctx);
		if (error != ERROR_NO)
			return error;

		if (ctx.errorCount > 0)
		{
			errorStats.errorCount += ctx.errorCount;
			errorStats.articleCount++;
		}

		if (rlimit.check())
			printCount(i, errorStats);
		i++;

		return ERROR_NO;
	});
	if (error != ERROR_NO)
		return error;

	printCount(i, errorStats);

	if (errorStats.articleCount == 0 && ctx.emptyArticlesCount == 0)
	{
		CLogW::PrintToConsole("[OK]");
	}
	else
	{
		CLogW::PrintToConsole("[OK]", eLogStatus_Warning);
		if (errorStats.articleCount > 0)
		{
			sld::printf(eLogStatus_Warning,
						"\nWarning! %u article%s generated %u warning%s! Check the logs please!",
						errorStats.articleCount, errorStats.articleCount == 1 ? "" : "s",
						errorStats.errorCount, errorStats.errorCount == 1 ? "" : "s");
		}
		if (ctx.emptyArticlesCount > 0)
		{
			sld::printf(eLogStatus_Warning, "\nWarning! %u empty article%s! Check the logs for ids!",
						ctx.emptyArticlesCount, ctx.emptyArticlesCount == 1 ? "" : "s");
		}
	}

	m_timer.Stop();
	m_timer.PrintTimeStringW(L"Parsing and creating articles from file \"" + aFileName + L"\" by Compiler");
	m_timer.PrintMemoryString("Memory Usage");

	return ERROR_NO;
}

/**
 * Производит загрузку статей из xml-дерева
 *
 * @param[in] aNode - корень xml-дерева со статьями
 *
 * @return код ошибки
 */
int ArticlesLoader::Load(pugi::xml_node aNode)
{
	ParserContext ctx(*this);
	return loadArticles(aNode, [&](pugi::xml_node node) { return LoadArticle(node, ctx); });
}

// block adding functions
int ParserContext::addTextBlock(sld::wstring_ref aName, sld::wstring_ref aText)
{
	std::wstring text(aText);
	// Перевод строки
	if (iequals(aName, L"br"))
		text.append(L"\\%0A");
	// Пробельный тег
	if (aName.starts_with(L"whitespace"))
		text.push_back(L' ');

	article->m_blocks.push_back(blockAllocator.create(eMetaText, getStyleIndex(aName), std::move(text), false));
	return ERROR_NO;
}

// loads a single article's node
static int loadNode(pugi::xml_node aNode, ParserContext &aCtx)
{
	// данная функция занимается только "элементами"
	if (aNode.type() != pugi::node_element)
		return ERROR_NO;

	const sld::wstring_ref nodeName(aNode.name());

	// handled by the main Load() function
	if (nodeName == L"id")
		return ERROR_NO;

	const ESldStyleMetaTypeEnum metaType = GetTextTypeByTagName(nodeName);
	if (metaType != eMetaUnknown)
	{
		auto handler = handlers::find(metaType);
		assert(handler);
		return handler(aCtx, aNode);
	}

	if (aNode.first_child().empty())
		return aCtx.addTextBlock(nodeName, L"");

	aCtx.tagsStack.push(eMetaUnknown);
	for (pugi::xml_node child : aNode.children())
	{
		int error = ERROR_NO;
		if (child.type() == pugi::node_pcdata)
			error = aCtx.addTextBlock(nodeName, child.value());
		else
			error = loadNode(child, aCtx);
		if (error != ERROR_NO)
			return error;
	}
	aCtx.tagsStack.pop();
	return ERROR_NO;
}

// "fixes up" switch blocks after the main article load
static int fixupSwitchBlocks(CArticle *aArticle, SwitchInfo &aInfo, MetadataManager &aMetaMgr)
{
	STString<128> wbuf;
	unsigned stateCount = 0;
	for (BlockType &block : aArticle->blocks())
	{
		if (block.isClosing)
			continue;

		switch (block.metaType)
		{
		case eMetaManagedSwitch:
		{
			auto managedSwitch = aMetaMgr.findMetadata<eMetaManagedSwitch>(block.metadata());
			if (managedSwitch)
			{
				assert(managedSwitch->Label._u32 < aInfo.managedSwitchLabels.size());
				const std::wstring &label = aInfo.managedSwitchLabels[managedSwitch->Label._u32];

				const auto switchIdIt = aInfo.labelToId.find(label);
				if (switchIdIt == aInfo.labelToId.end())
					return ERROR_NON_EXISTING_SWITCH_BLOCK_LABEL;

				const UInt32 switchId = switchIdIt->second;

				wbuf.clear();
				managedSwitch->Label = aMetaMgr.addString(wbuf.appendf(L"switch%u", switchId));

				const auto &switchIds = aInfo.applicationManaged;
				if (find(switchIds.begin(), switchIds.end(), switchId) != switchIds.end())
					return ERROR_WRONG_MANAGE_ATTRIBUTE_VALUE_IN_SWITCH_BLOCK;
				if (stateCount != aInfo.stateCounts[switchId])
					return ERROR_WRONG_NUMBER_OF_STATES_IN_SWITCH_BLOCKS;
			}
			break;
		}
		case eMetaSwitchControl:
		{
			auto switchControl = aMetaMgr.findMetadata<eMetaSwitchControl>(block.metadata());
			if (switchControl)
				stateCount = switchControl->NumStates;
			break;
		}
		default:
			break;
		}
	}
	return ERROR_NO;
}

static void parseJSHandler(ParserContext &aCtx, pugi::xml_node aNode, const wchar_t *aName,
						   ESldMetadataDOMEventType aType)
{
	pugi::xml_attribute attrib = aNode.attribute(aName);
	if (attrib.empty())
		return;

	const sld::wstring_ref text = attrib.value();
	if (text.empty())
		return;

	TMetadataArticleEventHandler handler;
	handler.Type = aType;
	handler.JSString = aCtx.addMetadataString(text);

	aCtx.addOpeningMetaBlock(handler);
}

/**
 * Загружает одну статью из ветви XML-файла и добавляет ее в список статей
 *
 * @param[in] aNode - ветвь XML-файла
 * @param[in] aCtx  - ссылка на объект необходимый для распарсивания статей
 *
 * @return код ошибки
 */
int ArticlesLoader::LoadArticle(pugi::xml_node aNode, ParserContext &aCtx)
{
	assert(aNode.type() == pugi::node_element);

	CArticle *article = m_articles.articleAllocator.create();

	// check id before loading anything else as we need it for metadata
	pugi::xml_node idNode = aNode.child(L"id");
	if (idNode)
	{
		const std::wstring text = idNode.child_value();
		if (text.empty())
		{
			sldXLog("Error! Empty article id!\n");
			return ERROR_EMPTY_ARTICLE_ID;
		}

		article->setID(text);
	}

	aCtx.setArticle(article);

	// парсим обработчики js событий
	parseJSHandler(aCtx, aNode, L"onload", eSldMetadataDOMEvent_OnLoad);
	parseJSHandler(aCtx, aNode, L"onresize", eSldMetadataDOMEvent_OnResize);

	int error = loadNode(aNode, aCtx);
	if (error != ERROR_NO)
		return error;

	error = fixupSwitchBlocks(article, aCtx.switchInfo, m_metadata);
	if (error != ERROR_NO)
		return error;

	error = m_articles.addArticle(article);
	if (error == ERROR_NO_ARTICLE)
		aCtx.emptyArticlesCount++;
	else if (error != ERROR_NO)
		m_error = error;

	return ERROR_NO;
}
