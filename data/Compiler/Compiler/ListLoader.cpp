#include "ListLoader.h"

#include "Log.h"
#include "StringFunctions.h"
#include "sld_Platform.h"
#include "Tools.h"
#include "XmlTagContent.h"

static void PrintCommonError(int error, pugi::xml_node node, sld::wstring_ref aListId)
{
	if (error == ERROR_WRONG_TAG_CONTENT)
		sldXLog("Error! List: `%s`. Invalid <%s> node text value: '%s'\n",
				aListId, sld::as_ref(node.name()), sld::as_ref(node.child_value()));
	else if (error == ERROR_UNKNOWN_TAG)
		sldXLog("Error! List: `%s`. Unknown tag: <%s>, parent: <%s>\n",
				aListId, sld::as_ref(node.name()), sld::as_ref(node.parent().name()));
}

static const sld::enums::Descriptor<EListVariantTypeEnum> kVariantTypes[] = {
	{ L"Show",                      eVariantShow },
	{ L"ShowSecondary",             eVariantShowSecondary },
	{ L"SortKey",                   eVariantSortKey },
	{ L"PartOfSpeech",              eVariantPartOfSpeech },
	{ L"Label",                     eVariantLabel },
	{ L"SourceSortKey",             eVariantSourceSortKey },
	{ L"LanguageCode",              eVariantLanguageCode },
	{ L"LockFlag",                  eVariantLockFlag },
	{ L"Depth",                     eVariantDepth },
	{ L"Phrase",                    eVariantPhrase },
	{ L"Numeration",                eVariantNumeration },
	{ L"Localization",              eVariantLocalizationPreferences },
	{ L"TextBookLevelType",         eVariantTextBookLevelType },
	{ L"TextBookLevelNumber",       eVariantTextBookLevelNumber },
	{ L"TextBookLevelDescription",  eVariantTextBookLevelDescription },
	{ L"TypeOfContent",             eVariantTypeOfContent },
	{ L"ExternArticleLink",         eVariantExternArticleLink },
	{ L"ExternalKey",               eVariantExternalKey },
	{ L"InternalKey",               eVariantInternalKey },
	{ L"HtmlSourceName",            eVariantHtmlSourceName },
	{ L"AuthorName",                eVariantAuthorName },
	{ L"KesID",                     eVariantKesID },
};

static EListVariantTypeEnum ParseVariantType(sld::wstring_ref value)
{
	EListVariantTypeEnum type = sld::enums::findValue(kVariantTypes, value, eVariantUnknown);
	if (type == eVariantUnknown && value.starts_with(L"GrammarInfo_"))
	{
		value.remove_prefix(wcslen(L"GrammarInfo_"));
		const long index = std::stol(to_string(value));
		if (index >= 0 && index <= (eVariantGrammarInfoEnd - eVariantGrammarInfoBegin))
			type = static_cast<EListVariantTypeEnum>(eVariantGrammarInfoBegin + index);
	}
	return type;
}

static std::wstring to_wstring(EListVariantTypeEnum type)
{
	for (auto&& v : kVariantTypes)
	{
		if (v.value == type)
			return to_string(v.string);
	}
	if (type >= eVariantGrammarInfoBegin && type <= eVariantGrammarInfoEnd)
		return L"GrammarInfo_" + std::to_wstring(type - eVariantGrammarInfoBegin);
	return L"UNKNOWN";
}

// processes localization strings (mostly regarding default "0000" localization)
static int processStrings(std::vector<TListLocalizedNames> &aStrings, UInt32 aLangCode, sld::wstring_ref aListId)
{
	const auto defaultCode = SldLanguage::Default;

	const TListLocalizedNames *names = nullptr;
	for (auto&& strings : aStrings)
	{
		if (strings.LanguageCode == defaultCode)
			return ERROR_NO;

		if (strings.LanguageCode == aLangCode)
			names = &strings;
	}

	char langCode[sizeof(aLangCode) + 1] = {};
	memcpy(langCode, &aLangCode, sizeof(aLangCode));
	if (names)
	{
		sldILog("Notice. Autogenerating default localization strings for list '%s' from '%s' locale.\n",
				aListId, langCode);
	}
	else
	{
		sld::printf(eLogStatus_Error, "\nError! No default localization strings for list '%s'! See log file for details.", aListId);

		if (aStrings.empty())
		{
			sldILog("Error! Autogenerating default localization strings for list '%s' failed because there are no list strings!\n",
					aListId);
		}
		else
		{
			sldILog("Error! Autogenerating default localization strings for list '%s' failed because no '%s' locale strings exist!\n",
					aListId, langCode);
		}

		return ERROR_EMPTY_STRING;
	}

	aStrings.insert(aStrings.cbegin(), *names)->LanguageCode = defaultCode;
	return ERROR_NO;
}

// Производит загрузку параметров листа из xml узла файла проекта
int CListLoader::Load(pugi::xml_node aNode, UInt32 aDictLangCodeFrom)
{
	pugi::xml_attribute idAttr = aNode.attribute(L"id");
	if (idAttr)
		m_list.SetListId(idAttr.value());

	std::wstring listFile;
	VariantsMap listVariants;

	// pull the list variants node upfront as parsing of the "old-style" VariantProperty nodes depends on it
	pugi::xml_node listVariantsNode = aNode.child(L"VariantProperties");

	for (pugi::xml_node node : aNode)
	{
		if (node.type() != pugi::node_element)
			continue;

		int error = ERROR_NO;
		const sld::wstring_ref nodeName(node.name());
		const std::wstring nodeText(node.child_value());
		if (nodeName == L"ListType")
		{
			error = ParseType(nodeText);
		}
		else if (nodeName == L"FullTextSearchOptions")
		{
			error = LoadFullTextSearchOptions(node, true);
			if (error != ERROR_NO) // does its own error printing
				return error;
		}
		else if (nodeName == L"LinkOptions")
		{
			error = LoadFullTextSearchOptions(node, false);
			if (error != ERROR_NO) // does its own error printing
				return error;
		}
		else if (nodeName == L"isListSorted")
		{
			bool sorted;
			error = ParseBoolParamNode(node, sorted, BoolType::YesNo);
			if (error == ERROR_NO)
				m_list.SetSorted(sorted);
		}
		else if (nodeName == L"isListHasLocalizedEntries")
		{
			error = ParseBoolParamNode(node, m_list.m_isLocalizedEntriesList, BoolType::YesNo);
		}
		else if (nodeName == L"LanguageCodeFrom")
		{
			error = ParseLanguageCodeNode(node, &m_list.m_LanguageCodeFrom);
		}
		else if (nodeName == L"LanguageCodeTo")
		{
			error = ParseLanguageCodeNode(node, &m_list.m_LanguageCodeTo);
		}
		else if (nodeName == L"ListFile")
		{
			listFile = nodeText;
		}
		else if (nodeName == L"IconFolder")
		{
			m_ImageContainer.SetImageFolder(nodeText);
		}
		else if (nodeName == L"VideoFolder")
		{
			m_VideoContainer.SetVideoFolder(nodeText);
		}
		else if (nodeName == L"SoundFolder")
		{
			error = m_SoundContainer.SetSoundFolder(nodeText);
			if (error != ERROR_NO)
			{
				sldXLog("Error! Can't open sound folder: `%s`.\n", sld::as_ref(nodeText));
				return error;
			}
		}
		else if (nodeName == L"CompressionMethod")
		{
			error = ParseCompressionConfig(nodeText, m_list.m_compressionConfig);
			if (error != ERROR_NO)
				return error;
		}
		else if (nodeName == L"VariantProperty" && listVariantsNode.empty())
		{
			error = LoadVariantProperty(node, listVariants);
			if (error != ERROR_NO) // does its own error printing
				return error;
		}
		else if (nodeName == L"AlphabetType")
		{
			error = ParseAlphabetType(nodeText);
		}
		else if (nodeName == L"Strings")
		{
			error = LoadLocalizedListStrings(node);
			if (error != ERROR_NO) // does its own error printing
				return error;
		}
		else if (nodeName == L"AuxillaryListType")
		{
			sldXLog("Note. List: `%s`. <AuxillaryListType> is deprecated, use <FullTextSearchOptions> instead.\n",
					sld::as_ref(m_list.GetListId()));
		}
		else if (nodeName == L"ResourceFolder")
		{
			m_list.m_ResourceFolderName = nodeText;
			if (m_list.m_ResourceFolderName.size() && m_list.m_ResourceFolderName.back() != L'\\')
				m_list.m_ResourceFolderName.push_back(L'\\');
		}
		else if (nodeName == L"SearchRange")
		{
			error = ParseSearchRange(nodeText);
		}
		else if (nodeName == L"isListHasHierarchy" ||
				 nodeName == L"isListHasIcons" ||
				 nodeName == L"isListHasVideos" ||
				 nodeName == L"isListHasScenes" ||
				 nodeName == L"isListHasSounds" ||
				 nodeName == L"isListHasNumberOfArticlesPerItem")
		{
			// Флаги проставляются автоматически
			sldXLog("Note. List: `%s`. Deprecated tag: '%s', please remove.\n",
					sld::as_ref(m_list.GetListId()), nodeName);
		}
		else if (nodeName != L"VariantProperties")
		{
			error = ERROR_UNKNOWN_TAG;
		}

		if (error != ERROR_NO)
		{
			PrintCommonError(error, node, m_list.GetListId());
			return error;
		}
	}

	if (listFile.empty())
	{
		sldILog("Error! List: `%s`. No list file specified!\n", sld::as_ref(m_list.GetListId()));
		sld::printf(eLogStatus_Error, "\nError! List: `%s`. No list file specified!", sld::as_ref(m_list.GetListId()));
		return ERROR_EMPTY_STRING;
	}

	if (listVariantsNode)
	{
		int error = LoadVariantProperties(listVariantsNode, listVariants);
		if (error != ERROR_NO)
			return error;
	}

	if (listVariants.empty())
		sldXLog("Warning! List: `%s`. No list variants specified!\n", sld::as_ref(m_list.GetListId()));

	// check list variants integrity
	// * list variants numbers *must* be monotonically increasing and start from 0
	// * we can't have *any* type duplicates
	bool hasDuplicates = false;
	std::unordered_set<UInt32> variantTypes;
	for (auto&& it : enumerate(listVariants))
	{
		if (it->first != it.index)
		{
			sldXLog("Error! List: `%s`. Invalid list variant property number: %u, expected: %u.\n",
					sld::as_ref(m_list.GetListId()), it->first, it.index);
			return ERROR_WRONG_INDEX;
		}
		if (!variantTypes.insert(it->second.Type).second)
		{
			sldXLog("Error! List: `%s`. Duplicate list variant property type: '%s', number: %u.\n",
					sld::as_ref(m_list.GetListId()),
					sld::as_ref(to_wstring((EListVariantTypeEnum)it->second.Type)),
					it.index);
			hasDuplicates = true;
		}
	}

	if (hasDuplicates)
	{
		sld::printf(eLogStatus_Error, "\nError! List: `%s`. Duplicate variant types!", sld::as_ref(m_list.GetListId()));
		return ERROR_WRONG_INDEX;
	}

	// assign variants to the list
	for (auto&& variant : listVariants)
		m_list.m_VariantsProperty.push_back(variant.second);

	// check list localization strings
	int error = processStrings(m_list.m_LocalizedNames, aDictLangCodeFrom, m_list.GetListId());
	if (error != ERROR_NO)
		return error;

	return LoadFile(listFile);
}

/** 
	Производит загрузку католога/списка слов из файла.

	@param fileName - имя файла

	@return код ошибки.
*/
int CListLoader::LoadFile(const wstring &fileName)
{
	int error = ERROR_NO;

	sld::printf("\nLoading list from file `%s`...", sld::as_ref(fileName));

	//загрузка xml-файла
	pugi::xml_document doc;
	if (!doc.load_file(fileName.c_str()))
	{
		sldILog("Error! Can't open file: `%s`.\n", sld::as_ref(fileName));
		CLogW::PrintToConsole("[FAIL]", eLogStatus_Error);
		return ERROR_CANT_OPEN_FILE;
	}

	sldILog("Processing file `%s`.\n", sld::as_ref(fileName));

	pugi::xml_node RootNode = doc.first_child();
	wstring RootName(RootNode.name());

	if (RootName != L"List")
	{
		sldXLog("Error! Unknown tag: `%s`, expected `List`.\n", sld::as_ref(RootName));
		CLogW::PrintToConsole("[FAIL]", eLogStatus_Error);
		return ERROR_WRONG_TAG;
	}
	
	sld::printf("\nLoaded 0 nodes");
	error = LoadListElement(RootNode, 0);
	if (error != ERROR_NO)
	{
		CLogW::PrintToConsole("[FAIL]", eLogStatus_Error);
		return error;
	}
	CLogW::PrintToConsole("[OK]");

	return ERROR_NO;
}

/** 
	Производит добавление элемента каталога/списка слов.

	@param aNode - кусок XML файла с очередным элементом
	@param aDepth - уровень вложенности на котором мы сейчас находимся

	@return код ошибки.
*/
int CListLoader::LoadListElement(pugi::xml_node aNode, UInt32 aDepth)
{
	UInt32 i = 0;
	RateLimit rlimit;
	for (pugi::xml_node node : aNode)
	{
		if (node.type() != pugi::node_element)
			continue;

		int error;
		const sld::wstring_ref nodeName = node.name();
		if (nodeName == L"Level")
		{
			error = LoadLevel(node, aDepth);
		}
		else if (nodeName == L"article")
		{
			error = LoadArticle(node, aDepth);
		}
		else
		{
			sldILog("Error! Wrong node name! Expected `Level` or `article`.\n");
			return ERROR_WRONG_TAG;
		}

		if (error != ERROR_NO)
			return error;

		if (rlimit.check())
			sld::printf("\rLoaded %d nodes", i);

		i++;
	}
	
	sld::printf("\rLoaded %d nodes", i);
	
	return ERROR_NO;
}

static int load(TListLocalizedNames &aStrings, sld::wstring_ref aName, const wchar_t *aValue)
{
	int ret = ERROR_NO;
	if (aName == L"ListName")
		sld::copy(aStrings.ListName, aValue);
	else if (aName == L"ListNameShort")
		sld::copy(aStrings.ListNameShort, aValue);
	else if (aName == L"ListClass")
		sld::copy(aStrings.ListClass, aValue);
	else if (aName == L"ListLanguagePair")
		sld::copy(aStrings.ListLanguagePair, aValue);
	else if (aName == L"ListLanguagePairShort")
		sld::copy(aStrings.ListLanguagePairShort, aValue);
	else if (aName == L"ListLanguageNameFrom")
		sld::copy(aStrings.ListLanguageNameFrom, aValue);
	else if (aName == L"ListLanguageNameTo")
		sld::copy(aStrings.ListLanguageNameTo, aValue);
	else
		ret = ERROR_UNKNOWN_TAG;
	return ret;
}

/***********************************************************************
* Добавляем блок локализованных строк для списка слов
*
* @param aNode - кусок XML файла, содержащий строки для списка слов
*
* @return код ошибки
************************************************************************/
int CListLoader::LoadLocalizedListStrings(pugi::xml_node aNode)
{
	TListLocalizedNames tmpNames;
	memset(&tmpNames, 0, sizeof(tmpNames));
	tmpNames.structSize = sizeof(tmpNames);
	
	int error = ParseLanguageCodeAttrib(aNode, &tmpNames.LanguageCode);
	if (error != ERROR_NO)
		return error;

	for (const auto &strings : m_list.m_LocalizedNames)
	{
		if (strings.LanguageCode == tmpNames.LanguageCode)
			return ERROR_NO;
	}

	// Локализованные строки
	for (pugi::xml_node node : aNode)
	{
		if (node.type() != pugi::node_element)
			continue;

		error = load(tmpNames, node.name(), node.child_value());
		if (error != ERROR_NO)
		{
			PrintCommonError(error, node, m_list.GetListId());
			return error;
		}
	}

	m_list.m_LocalizedNames.push_back(tmpNames);

	return ERROR_NO;
}

/**
 * Добавляем свойства варианта написания
 *
 * @param aNode - узел <VariantProperties>
 * @param aVariants - мапа свойств вариантов написания куда добавлять
 *
 * @return код ошибки
 */
int CListLoader::LoadVariantProperties(pugi::xml_node aNode, VariantsMap &aVariants)
{
	for (auto&& node : enumerate(aNode))
	{
		if (node->type() != pugi::node_element)
			continue;

		if (node->name() != sld::wstring_ref(L"VariantProperty"))
		{
			PrintCommonError(ERROR_UNKNOWN_TAG, *node, m_list.GetListId());
			return ERROR_UNKNOWN_TAG;
		}

		int error = LoadVariantProperty(*node, aVariants, node.index);
		if (error != ERROR_NO)
			return error;
	}
	return ERROR_NO;
}

/***********************************************************************
* Добавляем свойства варианта написания
*
* @param aNode     - кусок XML файла с данными
* @param aVariants - мапа свойств вариантов написания куда добавлять
* @param aNumber   - номер варианта написания (ULONG_MAX для "старого формата")
*
* @return код ошибки
************************************************************************/
int CListLoader::LoadVariantProperty(pugi::xml_node aNode, VariantsMap &aVariants, unsigned long aNumber)
{
	ListVariantProperty variant(eVariantUnknown, ~0u);
	for (pugi::xml_node node : aNode)
	{
		if (node.type() != pugi::node_element)
			continue;

		int error = ERROR_NO;
		const sld::wstring_ref nodeName(node.name());
		if (nodeName == L"Number" && aNumber == ULONG_MAX)
		{
			aNumber = wcstoul(node.child_value(), 0, 10);
			if (aNumber == ULONG_MAX)
				error = ERROR_WRONG_TAG_CONTENT;
		}
		else if (nodeName == L"Type")
		{
			variant.Type = ParseVariantType(node.child_value());
			if (variant.Type == eVariantUnknown)
				error = ERROR_WRONG_TAG_CONTENT;
		}
		else if (nodeName == L"LanguageCode")
		{
			error = ParseLanguageCodeNode(node, &variant.LangCode);
		}
		else
		{
			error = ERROR_UNKNOWN_TAG;
		}

		if (error != ERROR_NO)
		{
			PrintCommonError(error, node, m_list.GetListId());
			return error;
		}
	}

	if (aNumber == ULONG_MAX || variant.Type == eVariantUnknown || variant.LangCode == ~0u)
	{
		sldXLog("Error! List: `%s`. Insufficient list variant parameters.\n", sld::as_ref(m_list.GetListId()));
		return ERROR_WRONG_TAG_CONTENT;
	}

	auto it = aVariants.emplace(static_cast<UInt32>(aNumber), variant);
	if (!it.second)
	{
		sldILog("Error! List: `%s`. Duplicate list variant property at number: %lu!\n",
				sld::as_ref(m_list.GetListId()), aNumber);
		sld::printf(eLogStatus_Error, "\nError! List: `%s`. Duplicate list variant property!",
					sld::as_ref(m_list.GetListId()));
		return ERROR_WRONG_INDEX;
	}

	return ERROR_NO;
}

/** 
	Производит обработку установки уровня вложенности в каталоге/списке слов.

	@param aNode - кусок XML файла с очередным элементом
	@param aDepth - уровень вложенности на котором мы сейчас находимся

	@return код ошибки.
*/
int CListLoader::LoadLevel(pugi::xml_node aNode, UInt32 aDepth)
{
	int error;

	pugi::xml_attribute attribute = aNode.attribute(L"sort");
	if (attribute)
	{
		bool childrenIsSorted = attribute.as_bool();
		if (!m_list.m_List.empty())
			m_list.m_List.back().childrenIsSorted = childrenIsSorted;
		else
			m_list.m_ZeroLevelSorted = childrenIsSorted;
	}

	attribute = aNode.attribute(L"type");
	if (attribute)
	{
		static const sld::enums::Descriptor<EHierarchyLevelType> data[] = {
			{ L"normal",           eLevelTypeNormal },
			{ L"covered",          eLevelTypeCovered },
			{ L"uncovered",        eLevelTypeUncovered },
			{ L"static uncovered", eLevelTypeStaticUncovered },
		};
		EHierarchyLevelType levelType = sld::enums::findValue(data, attribute.value(), EHierarchyLevelType(~0u));
		if (levelType == EHierarchyLevelType(~0u))
		{
			sldXLog("Error! Wrong level attribute: type=%s\n", sld::as_ref(attribute.value()));
			levelType = eLevelTypeNormal;
		}
		m_list.m_List.back().HierarchyLevelType = levelType;
	}

	attribute = aNode.attribute(L"depth");
	if (attribute)
	{
		UInt32 depth = wcstoul(attribute.value(), NULL, 10);

		if (depth != 0 && !m_list.GetHierarchy())
			m_list.SetHierarchy(true);

		error = LoadListElement(aNode, depth);
		if (error != ERROR_NO)
			return error;
	}

	return ERROR_NO;
}

/** 
	Производит добавление элемента каталога/списка слов.

	@param aNode - кусок XML файла с очередным элементом
	@param aDepth - уровень вложенности на котором мы сейчас находимся

	@return код ошибки.
*/
int CListLoader::LoadArticle(pugi::xml_node aNode, UInt32 aDepth)
{
	ListElement	newElement;
	
	// Читаем атрибуты тега <article>
	pugi::xml_attribute idAttr = aNode.attribute(L"id");
	if (idAttr)
		newElement.Id = idAttr.value();
	
	UInt32 VariantIndex = 0;
	for (pugi::xml_node node : aNode)
	{
		if (node.type() != pugi::node_element)
			continue;

		const sld::wstring_ref nodeName = node.name();
		if (nodeName == L"wl-name")
		{
			wstring name;
			if (node.first_child() && node.first_child().type() != pugi::node_pcdata)
			{
				name = GetNotFormattedNodeValue(node);

				if (name.size())
				{
					const std::wstring index = L"StylizedVariant_" + m_list.m_ListId + L"_" + std::to_wstring(m_list.m_StylizedVariantsArticlesCount);
					newElement.m_StylizedVariantsArticleIndexes[VariantIndex] = index;

					pugi::xml_node articleNode = m_StylizedVariants.root().first_child().append_child(L"article");
					articleNode = articleNode.append_child(L"id");
					articleNode.append_child(pugi::node_pcdata).set_value(index.c_str());

					for (pugi::xml_node child : node)
						articleNode = articleNode.parent().insert_copy_after(child, articleNode);
					m_list.m_StylizedVariantsArticlesCount++;
				}
				else 
				{
					wstring sound = node.first_child().name();

					const ESldStyleMetaTypeEnum metaType = GetTextTypeByTagName(sound);

					if (metaType == eMetaSound)
					{						
						const std::wstring index = L"StylizedVariant_" + m_list.m_ListId + L"_" + std::to_wstring(m_list.m_StylizedVariantsArticlesCount);
						newElement.m_StylizedVariantsArticleIndexes[VariantIndex] = index;

						pugi::xml_node articleNode = m_StylizedVariants.root().first_child().append_child(L"article");
						articleNode = articleNode.append_child(L"id");
						articleNode.append_child(pugi::node_pcdata).set_value(index.c_str());

						for (pugi::xml_node child : node)
							articleNode = articleNode.parent().insert_copy_after(child, articleNode);
						m_list.m_StylizedVariantsArticlesCount++;

					}
				}
			}
			else
			{
				name = node.child_value();
			}

			newElement.Name.push_back(std::move(name));

			pugi::xml_attribute attribute = node.attribute(L"style");
			if (attribute)
				newElement.Styles[VariantIndex] = attribute.value();

			VariantIndex++;
		}
		else if (nodeName == L"id")
		{
			ArticleRefStruct tmpRef;

			tmpRef.ID = node.child_value();

			// Читаем атрибуты тега <id>
			for (pugi::xml_attribute attribute : node.attributes())
			{
				if (wcscmp(attribute.name(), L"shift") == 0)
				{
					unsigned long val = wcstoul(attribute.value(), 0, 10);
					if (val == ULONG_MAX)
					{
						sldXLog("Error! Wrong attribute content: AttributeName=%s\n", sld::as_ref(attribute.name()));
						return ERROR_WRONG_TAG_CONTENT;
					}

					tmpRef.shift = static_cast<UInt32>(val);
					break;
				}
				else
				{
					sldXLog("Error! CListLoader::AddArticle : Wrong attribute name: %s\n", sld::as_ref(attribute.name()));
					return ERROR_WRONG_TAG_ATTRIBUTE;
				}
			}

			newElement.ArticleRefs.push_back(std::move(tmpRef));
		}
		else if (nodeName == L"img")
		{
			// Читаем атрибуты
			pugi::xml_attribute attribute = node.attribute(L"url");
			if (attribute)
			{
				const sld::wstring_ref url = attribute.value();
				if (url.find(L'.') == sld::wstring_ref::npos)
				{
					sldXLog("Error! CListLoader::AddArticle : Wrong img url: '%s' in tag <%s>", url, nodeName);
					return ERROR_WRONG_TAG_CONTENT;
				}

				UInt32 Index = m_ImageContainer.AddImage(url);
				if (Index)
					newElement.PictureIndex = Index;

				if (!m_list.GetPicture())
					m_list.SetPicture(true);
			}
		}
		else if (nodeName == L"video")
		{
			// Читаем атрибуты
			pugi::xml_attribute attribute = node.attribute(L"url");
			if (attribute)
			{
				UInt32 iVideoIndex = m_VideoContainer.AddVideo(attribute.value());
				if (iVideoIndex)
					newElement.VideoIndex = iVideoIndex;

				if (!m_list.GetVideo())
					m_list.SetVideo(true);
			}
		}
		else if (nodeName == L"scene_3d")
		{
			// Читаем атрибуты
			pugi::xml_attribute attribute = node.attribute(L"url");
			if (attribute)
			{
				UInt32 Index = m_SceneContainer.AddScene(attribute.value());
				if (Index)
					newElement.SceneIndex = Index;

				if (!m_list.GetScene())
					m_list.SetScene(true);
			}
		}
		else if (nodeName == L"sound")
		{
			// Читаем атрибуты
			pugi::xml_attribute attribute = node.attribute(L"url");
			if (attribute)
			{
				UInt32 Index = m_SoundContainer.AddSound(attribute.value());
				if (Index)
					newElement.SoundIndex = Index;

				if (!m_list.GetSound())
					m_list.SetSound(true);
			}
		}
		else if (nodeName == L"alt-name")
		{
			const sld::wstring_ref nodeText = node.child_value();
			if (nodeText.size())
			{
				newElement.AltName.insert(to_string(nodeText));
				m_list.m_HasAlternativeHeadwords = true;
			}
		}
		else
		{
			sldXLog("Warning! Unknown list element <%s> in list id: '%s', element id: '%s'\n",
					nodeName, sld::as_ref(m_list.m_ListId), sld::as_ref(newElement.Id));
		}
	}

	if (newElement.Name.empty())
	{
		STString<1024> string(L"Warning! No wl-name(s) for list element id: '%s', list id: '%s'. Aborting.",
							  newElement.Id, m_list.m_ListId);
		sldXLog("%s\n", string);
		sld::printf(eLogStatus_Warning, "\n%s", string);
		return ERROR_EMPTY_WL_NAME;
	}

	newElement.Level = aDepth;
	newElement.Shift2Next = LIST_END;

	return m_list.AddListElement(std::move(newElement));
}

/** 
	Добавляет блок свойств списка слов полнотекстового поиска по куску XML-файла

	@param aNode - кусок XML файла

	@return код ошибки 
*/
int CListLoader::LoadFullTextSearchOptions(pugi::xml_node aNode, bool aFTSList)
{
	// Устанавливаем флаг, что в файле проекта для списка слов есть блок с настройками полнотекстового поиска.
	// Это означает, что даже если тип списка слов не находится в пределах диапазона полнотекстовых поисковых списков,
	// этот список все равно будет обрабатываться как список слов полнотекстовго поиска.
	if (aFTSList)
		m_list.m_FullTextSearchOptions.IsOptionsPresent = 1;
	else
		m_list.SetLinkedList(true);

	for (pugi::xml_node node : aNode)
	{
		if (node.type() != pugi::node_element)
			continue;

		int error = ERROR_NO;
		const sld::wstring_ref nodeName = node.name();
		const std::wstring nodeText(node.child_value());
		if (nodeName == L"LinkType")
		{
			EFullTextSearchLinkType LinkType = GetFullTextSearchLinkTypeByXmlTagContent(nodeText);
			if (LinkType != eLinkType_Unknown)
				m_list.m_FullTextSearchOptions.LinkType = LinkType;
			else
				error = ERROR_WRONG_TAG_CONTENT;
		}
		else if (nodeName == L"ShiftType")
		{
			EFullTextSearchShiftType ShiftType = GetFullTextSearchShiftTypeByXmlTagContent(nodeText);
			if (ShiftType != eShiftType_Unknown)
				m_list.m_FullTextSearchOptions.ShiftType = ShiftType;
			else
				error = ERROR_WRONG_TAG_CONTENT;
		}
		else if (nodeName == L"Priority")
		{
			unsigned long val = wcstoul(nodeText.c_str(), 0, 10);
			if (val != ULONG_MAX)
			{
				// Приоритет не может менять диапазон полнотекста, а значит превышать 15
				m_list.m_FullTextSearchOptions.Priority = static_cast<UInt32>(std::min(val, 0xful));
				if (m_list.IsFullTextSearchListType())
					m_list.m_Usage += m_list.m_FullTextSearchOptions.Priority;
			}
			else
				error = ERROR_WRONG_TAG_CONTENT;
		}
		else if (nodeName == L"isListSuffixSearch")
		{
			error = ParseBoolParamNode(node, m_list.m_FullTextSearchOptions.isSuffixSearch, BoolType::YesNo);
		}
		else if (nodeName == L"isListFullMorphologyWordForms")
		{
			error = ParseBoolParamNode(node, m_list.m_FullTextSearchOptions.isFullMorphologyWordForms, BoolType::YesNo);
		}
		else if (nodeName == L"AllowSourceListId")
		{
			if (!nodeText.empty() && nodeText != m_list.m_ListId)
				m_list.m_FullTextSearchOptions.mAllowedSourceListId.insert(nodeText);
		}
		else if (nodeName == L"MorphologyId")
		{
			int err = ParseLanguageCodeNode(node, &m_list.m_FullTextSearchOptions.MorphologyId);
			PrintCommonError(err, node, m_list.GetListId()); // for some reason it's not a hard error...
		}
		else
		{
			error = ERROR_UNKNOWN_TAG;
		}

		if (error != ERROR_NO)
		{
			PrintCommonError(error, node, m_list.GetListId());
			return error;
		}
	}

	return ERROR_NO;
}

/// Устанавливает тип назначения списка.
int CListLoader::ParseType(const wstring &aText)
{
	const uint32_t listType = GetListTypeFromString(aText);
	if (listType == eWordListType_Unknown)
	{
		sldILog("Error! Unknown list type: `%s`.\n", sld::as_ref(aText));
		return ERROR_WRONG_LIST_TYPE;
	}

	m_list.SetType(listType);
	return ERROR_NO;
}

/**
Склеивает все текстовое содержимое xml-узла в одну строку

@param[in] aNode	- xml-узел, содержимое которого нам нужно получить

@return строка с содержимым узла
*/
wstring CListLoader::GetNotFormattedNodeValue(pugi::xml_node aNode)
{
	wstring result = L"";
	pugi::xml_node node = aNode.first_child();
	while (node)
	{
		if (wcsncmp(node.name(), L"whitespace", 10) == 0)
		{
			result += L" ";
		}
		else
		{
			result += node.value();
		}

		if (node.first_child())
		{
			node = node.first_child();
		}
		else if (node.next_sibling())
		{
			node = node.next_sibling();
		}
		else
		{
			while (!node.next_sibling() && node != aNode && node.parent())
			{
				node = node.parent();
			}

			if (node != aNode)
				node = node.next_sibling();
			else
				break;
		}

	}

	return result;
}

/// Устанавливает уровни каталога, на которых будет производиться поиск
int CListLoader::ParseSearchRange(const wstring &aSearchRange)
{
	static const sld::enums::Descriptor<ESldSearchRange> data[] = {
		{ L"FullRecursive",           eSearchRangeFullRecursive },
		{ L"Root",                    eSearchRangeRoot },
		{ L"CurrentLevel",            eSearchRangeCurrentLevel },
		{ L"CurrentLevelRecursive;",  eSearchRangeCurrentLevelRecursive },
	};
	ESldSearchRange range = sld::enums::findValue(data, aSearchRange, eSearchRangeDefault);
	if (range == eSearchRangeDefault)
	{
		sldILog("Error! CListLoaser::ParseSearchRange : wrong search range: '%s'!\n",
				sld::as_ref(aSearchRange));
		return ERROR_WRONG_SEARCH_RANGE;
	}

	m_list.SetSearchRange(range);
	return ERROR_NO;
}

int CListLoader::ParseAlphabetType(const wstring &aAlphabetType)
{
	// TODO: Проверять, соответствует ли алфавит основному языку списка.
	EAlphabetType type = GetAlphabetTypeFromString(aAlphabetType);
	if (type == EAlphabet_Unknown)
	{
		sldILog("Error! CListLoader::ParseAlphabetType : wtong alphabet type: '%s'!\n",
				sld::as_ref(aAlphabetType));
		return ERROR_WRONG_ALPHABET_TYPE;
	}

	if (type != EAlphabet_Standard)
		m_list.SetAlphabetType(type);
	return ERROR_NO;
}
