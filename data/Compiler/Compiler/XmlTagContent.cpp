#include "XmlTagContent.h"

#include "Log.h"
#include "Tools.h"

using namespace sld::enums;

ESldStyleUsageEnum GetStyleUsageByXmlTagContent(wstring aStr)
{
	static const Descriptor<ESldStyleUsageEnum> data[] = {
		{ L"ThisHeadword", eStyleUsage_ThisHeadword },
		{ L"PairHeadword", eStyleUsage_PairHeadword },

		{ L"ThisHeadwordAlternative", eStyleUsage_ThisHeadwordAlternative },
		{ L"PairHeadwordAlternative", eStyleUsage_PairHeadwordAlternative },

		{ L"ThisTranslation", eStyleUsage_ThisTranslation },
		{ L"PairTranslation", eStyleUsage_PairTranslation },

		{ L"ThisPhonetic", eStyleUsage_ThisPhonetic },
		{ L"PairPhonetic", eStyleUsage_PairPhonetic },

		{ L"ThisGrammatic", eStyleUsage_ThisGrammatic },
		{ L"PairGrammatic", eStyleUsage_PairGrammatic },

		{ L"ThisExample", eStyleUsage_ThisExample },
		{ L"PairExample", eStyleUsage_PairExample },

		{ L"ThisDefinition", eStyleUsage_ThisDefinition },
		{ L"PairDefinition", eStyleUsage_PairDefinition },

		{ L"ThisComment", eStyleUsage_ThisComment },
		{ L"PairComment", eStyleUsage_PairComment },

		{ L"ThisClarification", eStyleUsage_ThisClarification },
		{ L"PairClarification", eStyleUsage_PairClarification },

		{ L"ThisNumeration", eStyleUsage_ThisNumeration },
		{ L"PairNumeration", eStyleUsage_PairNumeration },

		{ L"ThisNumerationHeadword", eStyleUsage_ThisNumerationHeadword },
		{ L"PairNumerationHeadword", eStyleUsage_PairNumerationHeadword },

		{ L"ThisNumerationMeaning", eStyleUsage_ThisNumerationMeaning },
		{ L"PairNumerationMeaning", eStyleUsage_PairNumerationMeaning },

		{ L"ThisStress", eStyleUsage_ThisStress },
		{ L"PairStress", eStyleUsage_PairStress },

		{ L"ThisSynonym", eStyleUsage_ThisSynonym },
		{ L"PairSynonym", eStyleUsage_PairSynonym },

		{ L"ThisAbbreviation", eStyleUsage_ThisAbbreviation },
		{ L"PairAbbreviation", eStyleUsage_PairAbbreviation },

		{ L"ThisEtymology", eStyleUsage_ThisEtymology },
		{ L"PairEtymology", eStyleUsage_PairEtymology },

		{ L"ThisIdiom", eStyleUsage_ThisIdiom },
		{ L"PairIdiom", eStyleUsage_PairIdiom },

		{ L"ThisSeparator", eStyleUsage_ThisSeparator },
		{ L"PairSeparator", eStyleUsage_PairSeparator },

		{ L"ThisStylisticLitter", eStyleUsage_ThisStylisticLitter },
		{ L"PairStylisticLitter", eStyleUsage_PairStylisticLitter },

		{ L"ThisReference", eStyleUsage_ThisReference },
		{ L"PairReference", eStyleUsage_PairReference },

		{ L"ThisPhrase", eStyleUsage_ThisPhrase },
		{ L"PairPhrase", eStyleUsage_PairPhrase },

		{ L"BothMetadata", eStyleUsage_BothMetadata },
	};
	return findValue(data, aStr, eStyleUsage_Unknown);
}

// an array of all the known metadata tags
static const Descriptor<ESldStyleMetaTypeEnum> kTextTypes[] = {
	{ L"abstract_resource",   eMetaAbstractResource },
	{ L"area",                eMetaImageArea },
	{ L"atomic_object",       eMetaAtomicObject },
	{ L"background_img",      eMetaBackgroundImage },
	{ L"caption",             eMetaCaption },
	{ L"construction_set",    eMetaConstructionSet },
	{ L"crossword_hint",      eMetaCrosswordHint },
	{ L"crossword_item",      eMetaCrossword },
	{ L"demo_link",           eMetaDemoLink },
	{ L"div",                 eMetaDiv },
	{ L"drawing_block",       eMetaDrawingBlock },
	{ L"extern_article",      eMetaExternArticle },
	{ L"flash_cards_link",    eMetaFlashCardsLink },
	{ L"footnote_brief",      eMetaFootnoteBrief },
	{ L"footnote_total",      eMetaFootnoteTotal },
	{ L"formula",             eMetaFormula },
	{ L"hide",                eMetaHide },
	{ L"hide-control",        eMetaHideControl },
	{ L"img",                 eMetaImage },
	{ L"info_block",          eMetaInfoBlock },
	{ L"interactive_object",  eMetaInteractiveObject },
	{ L"item_time_line",      eMetaTimeLineItem },
	{ L"label",               eMetaLabel },
	{ L"legend_item",         eMetaLegendItem },
	{ L"li",                  eMetaLi },
	{ L"link",                eMetaLink },
	{ L"list",                eMetaList },
	{ L"managed-switch",      eMetaManagedSwitch },
	{ L"map",                 eMetaMap },
	{ L"map_element",         eMetaMapElement },
	{ L"media_container",     eMetaMediaContainer },
	{ L"nobr",                eMetaNoBrText },
	{ L"p",                   eMetaParagraph },
	{ L"popup_article",       eMetaPopupArticle },
	{ L"popup_img",           eMetaPopupImage },
	{ L"scene_3d",            eMetaScene },
	{ L"sidenote",            eMetaSidenote },
	{ L"slide_show",          eMetaSlideShow },
	{ L"sound",               eMetaSound },
	{ L"source",              eMetaVideoSource },
	{ L"switch",              eMetaSwitch },
	{ L"switch-control",      eMetaSwitchControl },
	{ L"switch-state",        eMetaSwitchState },
	{ L"table",               eMetaTable },
	{ L"task_block_entry",    eMetaTaskBlockEntry },
	{ L"td",                  eMetaTableCol },
	{ L"test",                eMetaTest },
	{ L"test_container",      eMetaTestContainer },
	{ L"test_control",        eMetaTestControl },
	{ L"test_input",          eMetaTestInput },
	{ L"test_result",         eMetaTestResult },
	{ L"test_result_element", eMetaTestResultElement },
	{ L"test_spear",          eMetaTestSpear },
	{ L"test_target",         eMetaTestTarget },
	{ L"test_token",          eMetaTestToken },
	{ L"text_control",        eMetaTextControl },
	{ L"time_line",           eMetaTimeLine },
	{ L"tr",                  eMetaTableRow },
	{ L"ui_element",          eMetaUiElement },
	{ L"url",                 eMetaUrl },
	{ L"video",               eMetaVideo },
};
// 2 are for simple text and phonetics
// 2 are there for the "unused" enum values
// 1 is for eMetaArticleEventHandler
static_assert(sld::array_size(kTextTypes) + 2 + 2 + 1 == eMeta_Last,
			  "The xml tag name to tag meta type table must be updated.");

ESldStyleMetaTypeEnum GetTextTypeByTagName(sld::wstring_ref aName)
{
	return findValueHashed(kTextTypes, aName, eMetaUnknown);
}

sld::wstring_ref GetTagNameByTextType(ESldStyleMetaTypeEnum aType)
{
	for (auto&& v : kTextTypes)
	{
		if (v.value == aType)
			return v.string;
	}
	return L"UNKNOWN";
}

ESlovoEdContainerDatabaseTypeEnum GetDatabaseTypeByXmlTagContent(wstring aStr)
{
	static const Descriptor<ESlovoEdContainerDatabaseTypeEnum> data[] = {
		{ L"Phrasebook",			eDatabaseType_Phrasebook },
		{ L"Dictionary",			eDatabaseType_Dictionary },
		{ L"Sound",					eDatabaseType_Sound },
		{ L"Morphology",			eDatabaseType_Morphology },
		{ L"Bundle",				eDatabaseType_Bundle },
		{ L"Games",					eDatabaseType_Games },
		{ L"Textbook",				eDatabaseType_TextBook },
		{ L"TextBook",				eDatabaseType_TextBook },
		{ L"Images",				eDatabaseType_Images },
		{ L"Book",					eDatabaseType_Book },
		{ L"PictureDictionary",		eDatabaseType_PictureDictionary },
		{ L"DisplayMorphology",		eDatabaseType_DisplayMorphology },
		{ L"InflectionMorphology",	eDatabaseType_InflectionMorphology },
	};
	return findValue(data, aStr, eDatabaseType_Unknown);
}

EMediaSourceTypeEnum GetMediaSourceTypeByXmlTagContent(wstring aStr)
{
	static const Descriptor<EMediaSourceTypeEnum> data[] = {
		{ L"Database",       eMediaSourceType_Database },
		{ L"InternetServer", eMediaSourceType_InternetServer },
	};
	return findValue(data, aStr, eMediaSourceType_Unknown);
}

ESldStyleFontFamilyEnum GetStyleFontFamilyByXmlTagContent(wstring aStr)
{
	static const Descriptor<ESldStyleFontFamilyEnum> data[] = {
		{ L"SansSerif", eFontFamily_SansSerif },
		{ L"Serif", eFontFamily_Serif },
		{ L"Fantasy", eFontFamily_Fantasy },
		{ L"Monospace", eFontFamily_Monospace },
	};
	return findValue(data, aStr, eFontFamily_Unknown);
}

ESldStyleFontNameEnum GetStyleFontNameByXmlTagContent(wstring aStr)
{
	static const Descriptor<ESldStyleFontNameEnum> data[] = {
		{ L"DejaVu Sans",         eFontName_DejaVu_Sans },
		{ L"Lucida Sans",         eFontName_Lucida_Sans },
		{ L"Verdana",             eFontName_Verdana },
		{ L"Georgia",             eFontName_Georgia },
		{ L"HelveticaNeueLT Std", eFontName_HelveticaNeueLT_Std },
		{ L"DejaVu Serif",        eFontName_DejaVu_Serif },
		{ L"Helvetica",           eFontName_Helvetica },
		{ L"Source Sans Pro",     eFontName_Source_Sans_Pro },
		{ L"Gentium",             eFontName_Gentium },
		{ L"Merriweather",        eFontName_Merriweather },
		{ L"Merriweather Sans",   eFontName_Merriweather_Sans },
		{ L"Noto Sans",           eFontName_Noto_Sans },
		{ L"Noto Serif",          eFontName_Noto_Serif },
		{ L"Trajectum",           eFontName_Trajectum },
		{ L"Combi Numerals",      eFontName_Combi_Numerals },
		{ L"Charis SIL",          eFontName_Charis_SIL },
		{ L"Times New Roman",     eFontName_TimesNewRoman },
		{ L"Helvetica Neue",      eFontName_HelveticaNeue },
		{ L"Lyon Text",           eFontName_Lyon_Text },
		{ L"Atlas Grotesk",       eFontName_Atlas_Grotesk },
		{ L"1234 Sans",           eFontName_1234_Sans },
		{ L"Augean",              eFontName_Augean },
		{ L"Courier New",		      eFontName_Courier_New },
		{ L"Wittenberger",		    eFontName_Wittenberger },
		{ L"Kruti Dev",           eFontName_Kruti_Dev },
		{ L"Win Innwa",           eFontName_Win_Innwa },
		{ L"Myriad Pro Cond",     eFontName_Myriad_Pro_Cond },
		{ L"PhoneticTM",          eFontName_Phonetic_TM },
		{ L"Symbol",              eFontName_Symbol },
	};
	return findValue(data, aStr, eFontName_Unknown);
}

EFullTextSearchLinkType GetFullTextSearchLinkTypeByXmlTagContent(wstring aStr)
{
	if (aStr == L"ArticleId")
		return eLinkType_ArticleId;
	else if (aStr == L"ListEntryId")
		return eLinkType_ListEntryId;

	return eLinkType_Unknown;
}

EFullTextSearchShiftType GetFullTextSearchShiftTypeByXmlTagContent(wstring aStr)
{
	if (aStr == L"None")
		return eShiftType_None;
	else if (aStr == L"SymbolsFromArticleBegin")
		return eShiftType_SymbolsFromArticleBegin;

	return eShiftType_Unknown;
}

// an array of "canonical names" for all known brands
static const Descriptor<EDictionaryBrandName> kBrandNames[] = {
	{ L"Slovoed",								eBrand_SlovoEd },
	{ L"Merriam-Webster",						eBrand_Merriam_Webster },
	{ L"Oxford",								eBrand_Oxford },
	{ L"Duden",									eBrand_Duden },
	{ L"PONS",									eBrand_PONS },
	{ L"VOX",									eBrand_VOX },
	{ L"Van Dale",								eBrand_Van_Dale },
	{ L"AL-MAWRID",								eBrand_AL_MAWRID },
	{ L"Harrap",								eBrand_Harrap },
	{ L"AKADEMIAI KIADO",						eBrand_AKADEMIAI_KIADO },
	{ L"MultiLex",								eBrand_MultiLex },
	{ L"Berlitz",								eBrand_Berlitz },
	{ L"Langenscheidt",							eBrand_Langenscheidt },
	{ L"Britannica",							eBrand_Britannica },
	{ L"Mondadori",								eBrand_Mondadori },
	{ L"Slovari XXI veka",						eBrand_Slovari_XXI_veka },
	{ L"Enciclopedia Catalana",					eBrand_Enciclopedia_Catalana },
	{ L"Collins",								eBrand_Collins },
	{ L"WAHRIG",								eBrand_WAHRIG },
	{ L"Wat&Hoe",								eBrand_Wat_N_Hoe },
	{ L"Le Robert",								eBrand_Le_Robert },
	{ L"AUP PONS",								eBrand_AUP_PONS },
	{ L"Independent Publishing",				eBrand_Independent_Publishing },
	{ L"Chambers",								eBrand_Chambers },
	{ L"Barron's",								eBrand_Barrons },
	{ L"Librairie Orientale",					eBrand_Librairie_Orientale },
	{ L"Cambridge",								eBrand_Cambridge },
	{ L"Hoepli",								eBrand_Hoepli },
	{ L"Drofa",									eBrand_Drofa },
	{ L"RedHouse",								eBrand_Red_House },
	{ L"Living Language",						eBrand_Living_Language },
	{ L"PASSWORD",								eBrand_PASSWORD },
	{ L"Richmond",								eBrand_Richmond },
	{ L"Magnus",								eBrand_Magnus },
	{ L"Ecovit Kiado",							eBrand_Ecovit_Kiado },
	{ L"Priroda",								eBrand_Priroda },
	{ L"Operator's Dictionary",					eBrand_Operators_Dictionary },
	{ L"Lexikon 2K",							eBrand_Lexikon_2K },
	{ L"Lexicology Centre",						eBrand_Lexicology_Centre },
	{ L"Turover",								eBrand_Turover },
	{ L"International Relations",				eBrand_International_Relations },
	{ L"MYJMK",									eBrand_MYJMK },
	{ L"TransLegal",							eBrand_TransLegal },
	{ L"Focalbeo",								eBrand_Focalbeo },
	{ L"Insight Guides",						eBrand_Insight_Guides },
	{ L"Editura Litera",						eBrand_Editura_Litera },
	{ L"ETB Lab",								eBrand_EtbLab },
	{ L"ELOKONT",								eBrand_ELOKONT },
	{ L"Prosveschenie",							eBrand_Prosveschenie },
	{ L"Gorodskoy Metodologicheskiy Centr",		eBrand_Gorodskoy_Metodologicheskiy_Centr },
	{ L"Ventana-Graf",							eBrand_Ventana_Graf },
	{ L"Astrel",								eBrand_Astrel },
	{ L"Kuznetsov",								eBrand_Kuznetsov },
	{ L"The Kosciuszko Foundation",				eBrand_The_Kosciuszko_Foundation },
	{ L"Druid",									eBrand_Druid },
	{ L"Academia International",				eBrand_AcademiaInternational },
	{ L"Zanichelli Editore",					eBrand_Zanichelli_Editore },
	
};
static_assert(sld::array_size(kBrandNames) == eBrandName_Count, "The brand name table is out of date.");

EDictionaryBrandName GetDictionaryBrandNameByXmlTagContent(wstring aStr)
{
	EDictionaryBrandName brand = findValue(kBrandNames, aStr, eBrand_Unknown);
	if (brand != eBrand_Unknown)
		return brand;

	// auxilliary table with non-canonical names (mostly etb-lab legacy)
	static const Descriptor<EDictionaryBrandName> data[] = {
		{ L"ETB LAB",								eBrand_EtbLab },
		{ L"Etb Lab",								eBrand_EtbLab },
		{ L"ETB-LAB",								eBrand_EtbLab },
		{ L"ETB-Lab",								eBrand_EtbLab },
		{ L"etb-lab",								eBrand_EtbLab },
		{ L"elokont",								eBrand_ELOKONT },
		{ L"elocont",								eBrand_ELOKONT },
		{ L"prosveschenie",							eBrand_Prosveschenie },
		{ L"gorodskoy metodologicheskiy centr",		eBrand_Gorodskoy_Metodologicheskiy_Centr },
		{ L"ventana-graf",							eBrand_Ventana_Graf },
		{ L"astrel",								eBrand_Astrel },
	};
	return findValue(data, aStr, eBrand_Unknown);
}

wstring GetDictionaryXmlBrandNameByBrandId(UInt32 aBrand)
{
	const EDictionaryBrandName brand = static_cast<EDictionaryBrandName>(aBrand);
	for (auto&& v : kBrandNames)
	{
		if (v.value == brand)
			return to_string(v.string);
	}
	return L"Unknown brand";
}

UInt8 GetClassLevelMajor(const wstring aLevel)
{
	const Int32 iLevel = wcstoul(aLevel.c_str(), 0, 10);
	if (iLevel < 1 || iLevel > 11)
		return 0;
	else
		return (UInt8)iLevel;
}

UInt8 GetClassLevelMinor(const wstring aLevel)
{
	const Int32 iLevel = wcstoul(aLevel.c_str(), 0, 10);

	if (iLevel < 0 || iLevel > 3)
		return 0;
	else
		return (UInt8)iLevel;
}

UInt8 GetUInt8(const wstring aValue)
{
	const Int32 iValue = wcstoul(aValue.c_str(), 0, 10);

	if (iValue < 0x00 || iValue > 0xFF)
		return 0;
	else
		return (UInt8)iValue;
}

UInt8 GetRevision(const wstring aRevision)
{
	return GetUInt8(aRevision);
}

UInt8 GetBookPart(const wstring aBookPart)
{
	return GetUInt8(aBookPart);
}

UInt16 GetPublishYear(const wstring aPublishYear)
{
	const Int32 iPublishYear = wcstoul(aPublishYear.c_str(), 0, 10);

	if (iPublishYear < 1900 || iPublishYear > 2100)
		return 0;
	else
		return (UInt16)iPublishYear;
}

ESldContentType GetContentType(const wstring aContentType)
{
	static const Descriptor<ESldContentType> data[] = {
		{ L"Theory",			eDataBaseContentType_Theory },
		{ L"Practice",			eDataBaseContentType_Practice },
		{ L"MethodBook",		eDataBaseContentType_MethodBook },
		{ L"ProblemBook",		eDataBaseContentType_ProblemBook },
		{ L"Corps",				eDataBaseContentType_Corps },
		{ L"Video",				eDataBaseContentType_Video },
		{ L"Animation",			eDataBaseContentType_Animation },
		{ L"Image",				eDataBaseContentType_Image },
		{ L"Sound",				eDataBaseContentType_Sound },
		{ L"3DScene",			eDataBaseContentType_3DScene },
		{ L"TestSimulator",		eDataBaseContentType_TestSimulator },
		{ L"TestExaminer",		eDataBaseContentType_TestExaminer },
		{ L"TestPractical",		eDataBaseContentType_TestPractical },
		{ L"Slideshow",			eDataBaseContentType_Slideshow },
		{ L"Scenario",			eDataBaseContentType_Scenario },
		{ L"Fiction",			eDataBaseContentType_Fiction },
		{ L"ExternalResource",	eDataBaseContentType_ExternalResource },
		{ L"Other",				eDataBaseContentType_Other },
	};
	return findValue(data, aContentType, eDataBaseContentType_Wrong);
}

EEducationalLevel GetEducationalLevel(const wstring& aEducationalLevel)
{
	static const Descriptor<EEducationalLevel> data[] = {
		{ L"PreschoolEducation",         eEducationalLevelPreschoolEducation },
		{ L"PrimaryEducation",           eEducationalLevelPrimaryEducation },
		{ L"BasicEducation",             eEducationalLevelBasicEducation },
		{ L"SecondaryEducation",         eEducationalLevelSecondaryEducation },
		{ L"TechnicalSchoolFirstCycle",  eEducationalLevelTechnicalSchoolFirstCycle },
		{ L"TechnicalSchoolSecondCycle", eEducationalLevelTechnicalSchoolSecondCycle },
		{ L"HigherEducation",            eEducationalLevelHigherEducation },
		{ L"UniversityPostgraduate",     eEducationalLevelUniversityPostgraduate },
		{ L"VocationalTraining",         eEducationalLevelVocationalTraining },
		{ L"AdditionalEducation",        eEducationalLevelAdditionalEducation },
	};
	return findValue(data, aEducationalLevel, eEducationalLevelUnknown);
}

UInt32 GetListTypeFromString(const std::wstring &aStr)
{
	// list type that should have a number appended
	// *important* - they should have an underscore appended!
	static const struct {
		const wchar_t *const prefix;
		uint32_t baseIndex;
		uint32_t lastIndex;
	} numeratedListTypes[] = {
		{ L"SpecialAdditionalInfo_", eWordListType_SpecialAdditionalInfoBase, eWordListType_SpecialAdditionalInfoLast },
		{ L"SpecialAdditionalInteractiveInfo_", eWordListType_SpecialAdditionalInteractiveInfoBase, eWordListType_SpecialAdditionalInteractiveInfoLast },
		{ L"ExternResourcePriority_", eWordListType_ExternResourcePriorityFirst, eWordListType_ExternResourcePriorityLast },
	};

	// normal list types
	static const Descriptor<EWordListTypeEnum> listTypes[] = {
		{ L"Dictionary",                 eWordListType_Dictionary },
		{ L"Catalog",                    eWordListType_Catalog },
		{ L"FullTextSearch",             eWordListType_FullTextSearchBase },
		{ L"FullTextSearch_Headword",    eWordListType_FullTextSearchHeadword },
		{ L"FullTextSearch_Content",     eWordListType_FullTextSearchContent },
		{ L"FullTextSearch_Translation", eWordListType_FullTextSearchTranslation },
		{ L"FullTextSearch_Example",     eWordListType_FullTextSearchExample },
		{ L"FullTextSearch_Definition",  eWordListType_FullTextSearchDefinition },
		{ L"FullTextSearch_Phrase",      eWordListType_FullTextSearchPhrase },
		{ L"FullTextSearch_Idiom",       eWordListType_FullTextSearchIdiom },
		{ L"AdditionalInfo",             eWordListType_AdditionalInfo },
		{ L"RegularSearch",              eWordListType_RegularSearch },
		{ L"Sound",                      eWordListType_Sound },
		{ L"Hidden",                     eWordListType_Hidden },
		{ L"DictionaryForSearch",        eWordListType_DictionaryForSearch },
		{ L"MorphologyArticles",         eWordListType_MorphologyArticles },
		{ L"MorphologyBaseForm",         eWordListType_MorphologyBaseForm },
		{ L"MorphologyInflectionForm",   eWordListType_MorphologyInflectionForm },
		{ L"GrammaticTest",              eWordListType_GrammaticTest },
		{ L"ArticlesHideInfo",           eWordListType_ArticlesHideInfo },
		{ L"GameArticles",               eWordListType_GameArticles },
		{ L"FlashCardsFront",            eWordListType_FlashCardsFront },
		{ L"FlashCardsBack",             eWordListType_FlashCardsBack },
		{ L"KES",                        eWordListType_KES },
		{ L"FC",                         eWordListType_FC },
		{ L"MergedDictionary",           eWordListType_MergedDictionary },
		{ L"InApp",                      eWordListType_InApp },
		{ L"FullTextAuxiliary",          eWordListType_FullTextAuxiliary },
		{ L"TextBook",                   eWordListType_TextBook },
		{ L"Tests",                      eWordListType_Tests },
		{ L"SubjectIndex",               eWordListType_SubjectIndex },
		{ L"PopupArticles",              eWordListType_PopupArticles },
		{ L"SimpleSearch",               eWordListType_SimpleSearch },
		{ L"DictionaryUsageInfo",        eWordListType_DictionaryUsageInfo },
		{ L"SlideShow",                  eWordListType_SlideShow },
		{ L"Map",                        eWordListType_Map },
		{ L"Atomic",                     eWordListType_Atomic },
		{ L"PageNumerationIndex",        eWordListType_PageNumerationIndex },
		{ L"BinaryResource",             eWordListType_BinaryResource },
		{ L"ExternBaseName",			 eWordListType_ExternBaseName },
		{ L"ArticleTemplates",			 eWordListType_ArticleTemplates },
		{ L"AuxiliarySearchList",		 eWordListType_AuxiliarySearchList },
		{ L"Enchiridion",				 eWordListType_Enchiridion },
		{ L"WordOfTheDay",				 eWordlistType_WordOfTheDay },
		{ L"PreloadedFavourites",		 eWordlistType_PreloadedFavourites }
	};

	// first check "numerated" list types
	for (size_t i = 0; i < sld::array_size(numeratedListTypes); i++)
	{
		const wchar_t *const prefix = numeratedListTypes[i].prefix;
		if (aStr.compare(0, wcslen(prefix), prefix) == 0)
		{
			long index = wcstol(aStr.c_str() + wcslen(prefix), nullptr, 10);
			if (index < 0 || index >(long)(numeratedListTypes[i].lastIndex - numeratedListTypes[i].baseIndex))
				return eWordListType_Unknown;
			return numeratedListTypes[i].baseIndex + index;
		}
	}

	// now check "normal" list types
	return findValue(listTypes, aStr, eWordListType_Unknown);
}

EAlphabetType GetAlphabetTypeFromString(const std::wstring &aStr)
{
	static const Descriptor<EAlphabetType> data[] = {
		{ L"chin_hierogliph", EAlphabet_Chin_Hierogliph },
		{ L"chin_pinyin",     EAlphabet_Chin_Pinyin },
		{ L"japa_kana",       EAlphabet_Japa_Kana },
		{ L"japa_kanji",      EAlphabet_Japa_Kanji },
		{ L"japa_romanji",    EAlphabet_Japa_Romanji },
		{ L"kore_hangul",     EAlphabet_Kore_Hangul },
		{ L"kore_pinyin",     EAlphabet_Kore_Pinyin },
		{ L"standard",        EAlphabet_Standard },
	};
	return findValue(data, aStr, EAlphabet_Unknown);
}

static inline bool parseLanguageString(sld::wstring_ref string, UInt32 *value)
{
	if (string.size() != sizeof(UInt32))
		return false;

	// if we used narrow srings we could simply memcpy this over the uint...
	UIntUnion code;
	for (size_t n : indices(string))
		code.ui_8[n] = static_cast<UInt8>(string[n]);

	*value = code.ui_32;
	return true;
}

int ParseLanguageCodeAttrib(pugi::xml_node aNode, UInt32 *aLangCode)
{
	pugi::xml_attribute langCode = aNode.attribute(L"Language");
	if (langCode.empty())
	{
		sldXLog("Error! Empty \"Language\" attribute in tag: %s\n", sld::as_ref(aNode.name()));
		return ERROR_WRONG_TAG_ATTRIBUTE;
	}

	const sld::wstring_ref langValue = langCode.value();
	if (!parseLanguageString(langValue, aLangCode))
	{
		sldXLog("Error! Wrong language attribute: Tag=%s, Language=%s\n",
				sld::as_ref(aNode.name()), langValue);
		return ERROR_WRONG_TAG_ATTRIBUTE;
	}
	return ERROR_NO;
}

int ParseLanguageCodeNode(pugi::xml_node aNode, UInt32 *aLangCode)
{
	return parseLanguageString(aNode.child_value(), aLangCode) ? ERROR_NO : ERROR_WRONG_TAG_CONTENT;
}

int ParseBoolParamNode(pugi::xml_node aNode, bool &aValue, BoolType aType)
{
	static const sld::wstring_ref data[][2] = {
		{ L"YES", L"NO" }, { L"ON", L"OFF" }
	};
	static_assert(sld::array_size(data) == (size_t)BoolType::COUNT, "table needs an udate");
	const sld::wstring_ref value = aNode.child_value();
	const auto &values = data[static_cast<int>(aType)];
	if (value == values[0])
		aValue = true;
	else if (value == values[1])
		aValue = false;
	else
		return ERROR_WRONG_TAG_CONTENT;
	return ERROR_NO;
}

int ParseCompressionConfig(const sld::wstring_ref &aString, CompressionConfig &aConfig)
{
	if (aString == L"NoCompression")
	{
		aConfig.type = eCompressionTypeNoCompression;
	}
	else
	{
		sldILog("Error! Unknown compression type: %s\n", aString);
		return ERROR_WRONG_COMPRESSION_METHOD;
	}
	return ERROR_NO;
}
