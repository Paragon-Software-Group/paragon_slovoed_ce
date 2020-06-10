#include "SlovoEdProject.h"

#include <unordered_set>

#include "ArticlesLoader.h"
#include "ListLoader.h"
#include "sld_Platform.h"
#include "Tools.h"

using namespace std;

/** 
	Конструктор, производит инициализацию по-умолчанию
*/
SlovoEdProject::SlovoEdProject(void)
{
	// Очищаем структуру заголовка
	memset(&m_header, 0, sizeof(m_header));
	// Очищаем структуру с информацией о версии
	memset(&m_VersionInfo, 0, sizeof(m_VersionInfo));

	CLogW::InstanceLog().SetLogFileName(L"PreViewLog.log");
	CLogW::InstanceXmlLog().SetLogFileName(L"PreViewXmlLog.log");

	CLogW::InstanceXmlLog().PrintStartLine();
	CLogW::InstanceLog().PrintStartLine();

	// Тип содержимого проекта по-умолчанию
	m_DatabaseType = eDatabaseType_Dictionary;
	m_SoundSourceType = eMediaSourceType_Database;
	m_ImageSourceType = eMediaSourceType_Database;
	m_VideoSourceType = eMediaSourceType_Database;
	m_SceneSourceType = eMediaSourceType_Database;
	m_MeshSourceType = eMediaSourceType_Database;
	m_MaterialSourceType = eMediaSourceType_Database;

	m_pAdditionalInfo = 0;

	m_IsHasSwitchThematics = 0;
	m_IsInApp = 0;

	m_IsMultiLanguageBase = false;
}

/** 
	Деструктор
*/
SlovoEdProject::~SlovoEdProject(void)
{
	delete m_pAdditionalInfo;
}

/***********************************************************************
* Добавляем список слов
*
* @param aNode              - ветвь XML-дерева с нужным разделом
* @param aSimpleSortedLists - вектор сортированых списков
* @param aCtx               - объект с данными используемыми для распарсивания статей
*
* @return код ошибки
************************************************************************/
int SlovoEdProject::AddList(pugi::xml_node aNode, std::vector<CWordList> &aSimpleSortedLists, ArticlesLoader &aArticlesLoader)
{
	CListLoader List;

	int error = List.Load(aNode, m_header.LanguageCodeFrom);
	if (error != ERROR_NO)
		return error;

	if (List->IsFullTextSearchAuxillaryList() && !List->IsFullTextSearchList())
	{
		sldXLog("Error! Missing FTS options for auxiliary list: `%s`\n", sld::as_ref(List->GetListId()));
		return ERROR_WRONG_AUXILIARY_LINK_TYPE;
	}

	error = PostProcessList(List, aSimpleSortedLists, aArticlesLoader);
	if (error != ERROR_NO)
		return error;

	return AddWordList(std::move(List.list()));
}

int SlovoEdProject::PostProcessList(CListLoader &aList, std::vector<CWordList> &aSimpleSortedLists,
									ArticlesLoader &aArticlesLoader)
{
	/// Проверяем, является ли первый вариант написания - Show-вариантом
	/// В случае, если вариант всего один (например это вспомогательный список из одного сорткея)
	/// SimpleSorted так же не нужен
	bool firstShowVariant = true;
	if (aList->GetVariantPropertyCount() != 0 && aList->GetType() != eWordListType_FullTextAuxiliary)
		firstShowVariant = aList->GetVariantProperty(0)->Type == eVariantShow;

	if (!aList->GetSorted() || aList->GetHierarchy() || (!firstShowVariant && aList->GetVariantPropertyCount() > 1))
	{
		CWordList ListForSearch = aList.list();

		int error = ListForSearch.ConvertListToSimpleSortedList();
		if (error != ERROR_NO)
			return error;

		aList->SetSimpleSortedListIndex((Int32)aSimpleSortedLists.size());
		/// альтернативные заголовки будут учтены в Simple Sorted List-e
		aList->SetAlternativeHeadwordsFlag(false);

		aSimpleSortedLists.push_back(std::move(ListForSearch));
	}

	if (aList->HasStylizedVariants())
	{
		int error = aArticlesLoader.Load(aList.GetStylizedVariantsNode().first_child());
		if (error != ERROR_NO)
			return error;
	}

	uint32_t variantIndex = 0;
	for (; variantIndex < aList->GetVariantPropertyCount(); variantIndex++)
	{
		if (aList->GetVariantProperty(variantIndex)->Type == eVariantHtmlSourceName)
			break;
	}

	if (variantIndex != aList->GetVariantPropertyCount())
	{
		for (const ListElement& elem : aList->elements())
		{
			const sld::wstring_ref htmlName = elem.GetName(variantIndex);
			if (htmlName.size())
				m_ListEntryId_HtmlSourceName.emplace(elem.Id, to_string(htmlName));
		}
	}

	// adding file resources
	const bool hasImages = aList->GetPicture();
	AddedFileResourceMap imageMap;
	if (hasImages)
		imageMap = m_ImageManager.AddImageContainer(aList.GetImageContainer());

	const bool hasVideos = aList->GetVideo();
	AddedFileResourceMap videoMap;
	if (hasVideos)
		videoMap = m_VideoManager.AddVideoContainer(aList.GetVideoContainer());

	const bool hasSounds = aList->GetSound();
	AddedFileResourceMap soundMap;
	if (hasSounds)
		soundMap = m_SoundManager.AddSoundContainer(aList.GetSoundContainer());

	const bool hasScenes = aList->GetScene();
	AddedFileResourceMap sceneMap;
	if (hasScenes)
		sceneMap = m_SceneManager.AddSceneContainer(aList.GetSceneContainer(), m_MaterialManager, m_MeshManager);

	for (ListElement &listElement : aList->elements())
	{
		if (hasImages && listElement.PictureIndex != SLD_INDEX_PICTURE_NO)
			listElement.PictureIndex = imageMap[listElement.PictureIndex];

		if (hasVideos && listElement.VideoIndex != SLD_INDEX_VIDEO_NO)
			listElement.VideoIndex = videoMap[listElement.VideoIndex];

		if (hasSounds && listElement.SoundIndex != SLD_INDEX_SOUND_NO)
			listElement.SoundIndex = soundMap[listElement.SoundIndex];

		if (hasScenes && listElement.SceneIndex != SLD_INDEX_SCENE_NO)
			listElement.SceneIndex = sceneMap[listElement.SceneIndex];
	}

	return ERROR_NO;
}

// style parsing helpers
namespace {

static int ParseColorParamNode(TRGBColor &aColor, pugi::xml_node aNode, bool *aPresent = nullptr)
{
	struct Color {
		const sld::wstring_ref name;
		UInt8 value;
	} colors[] = {
		{ L"Red",   aColor.R() },
		{ L"Green", aColor.G() },
		{ L"Blue",  aColor.B() },
		{ L"Alpha", aColor.Alpha() },
	};

	for (pugi::xml_attribute attrib : aNode.attributes())
	{
		const sld::wstring_ref name = attrib.name();
		auto color = std::find_if(std::begin(colors), std::end(colors),
								  [name](const Color &c) { return c.name == name; });
		if (color == std::end(colors))
		{
			sldXLog("Error! Unknown color node attribute! tag: '%s', attribute: '%s'\n",
					sld::as_ref(aNode.name()), name);
			return ERROR_WRONG_TAG_ATTRIBUTE;
		}

		auto value = attrib.as_llong();
		if (value < 0 || value > 0xff)
		{
			sldXLog("Warning! Color attribute value out of range! tag: '%s', attribute: '%s', value: '%d'\n",
					sld::as_ref(aNode.name()), name, value);
			value = value < 0 ? 0 : value > 0xff ? 0xff : value;
		}
		color->value = static_cast<UInt8>(value);

		if (aPresent)
			*aPresent = true;
	}

	aColor.SetColor(colors[0].value, colors[1].value, colors[2].value, colors[3].value);

	return ERROR_NO;
}

static void logCommonError(int error, pugi::xml_node node)
{
	if (error == ERROR_WRONG_TAG_CONTENT)
		sldXLog("Error! Invalid <%s> node text value: '%s'\n",
				sld::as_ref(node.name()), sld::as_ref(node.child_value()));
	else if (error == ERROR_UNKNOWN_TAG)
		sldXLog("Error! Unknown tag : <%s>\n", sld::as_ref(node.name()));
}

}

/***********************************************************************
* Устанавливаем данные варианта стиля
*
* @param aStyleVariant	- указатель на вариант стиля, данные для которого нужно установить
* @param aNode			- xml узел, в котором содержатся данные
* @param aImages		- контейнер с картинками
*
* @return код ошибки
************************************************************************/
int SlovoEdProject::SetStyleVariantData(TSlovoedStyleVariant* aStyleVariant, pugi::xml_node aNode, CImageContainer &aImages)
{
	assert(aStyleVariant);

	wstring TagName(aNode.name());
	wstring Text(aNode.child_value());

	if (TagName == L"Color")
	{
		TRGBColor color(0, 0, 0, 255);
		int error = ParseColorParamNode(color, aNode);
		if (error != ERROR_NO)
			return error;
		aStyleVariant->SetColor(color);
	}
	else if (TagName == L"BackgroundColor")
	{
		TRGBColor color(255, 255, 255, 0);
		int error = ParseColorParamNode(color, aNode);
		if (error != ERROR_NO)
			return error;
		aStyleVariant->SetBackgroundColor(color);
	}
	else if (TagName == L"Visible")
	{
		bool flag;
		int error = ParseBoolParamNode(aNode, flag, BoolType::OnOff);
		if (error != ERROR_NO)
			return error;
		aStyleVariant->SetVisible(flag);
	}
	else if (TagName == L"Bold")
	{
		if (Text == L"ON")
			aStyleVariant->SetBoldValue(eBoldValueBold);
		else if (Text == L"OFF")
			aStyleVariant->SetBoldValue(eBoldValueNormal);
		else if (Text == L"HEAVY")
			aStyleVariant->SetBoldValue(eBoldValueBolder);
		else if (Text == L"LIGHT")
			aStyleVariant->SetBoldValue(eBoldValueLighter);
		else
		{
			UInt32 boldValue = wcstoul(Text.c_str(), 0, 10);
			if (boldValue < 100 || boldValue > 900 || (boldValue % 100 != 0))
				return ERROR_WRONG_TAG_CONTENT;
			aStyleVariant->SetBoldValue(boldValue);
		}
	}
	else if (TagName == L"Italic")
	{
		bool flag;
		int error = ParseBoolParamNode(aNode, flag, BoolType::OnOff);
		if (error != ERROR_NO)
			return error;
		aStyleVariant->SetItalic(flag);
	}
	else if (TagName == L"Underline")
	{
		bool flag;
		int error = ParseBoolParamNode(aNode, flag, BoolType::OnOff);
		if (error == ERROR_NO)
		{
			aStyleVariant->SetUnderline(flag);
		}
		else
		{
			UInt32 underlineValue = wcstoul(Text.c_str(), 0, 10);
			if (underlineValue < eUnderlineMin || underlineValue > eUnderlineMax)
				return ERROR_WRONG_TAG_CONTENT;

			TRGBColor color(255, 255, 255, 0);
			bool present;
			error = ParseColorParamNode(color, aNode, &present);
			if (error != ERROR_NO)
				return error;
			aStyleVariant->SetUnderlineStyle(underlineValue, present ? 1 : 0, color);
		}
	}
	else if (TagName == L"LineHeight")
	{
		if (Text == L"Standard")
			aStyleVariant->SetLineHeight(eSizeNormal);
		else if (Text == L"Small")
			aStyleVariant->SetLineHeight(eSizeSmall);
		else if (Text == L"Big")
			aStyleVariant->SetLineHeight(eSizeLarge);
		else
		{
			TSizeValue size = ParseSizeValue(Text);
			if (!size.IsValid() && size.Integer() > 5)
				size.Units = eMetadataUnitType_pt;

			if (!size.IsValid())
				return ERROR_WRONG_TAG_CONTENT;

			aStyleVariant->SetLineHeight(SIZE_VALUE_UNITS);
			aStyleVariant->SetLineHeight(size);
		}
	}
	else if (TagName == L"Strikethrough")
	{
		bool flag;
		int err = ParseBoolParamNode(aNode, flag, BoolType::OnOff);
		if (err != ERROR_NO)
			return err;
		aStyleVariant->SetStrikethrough(flag);
	}
	else if (TagName == L"Overline")
	{
		bool flag;
		int err = ParseBoolParamNode(aNode, flag, BoolType::OnOff);
		if (err != ERROR_NO)
			return err;
		aStyleVariant->SetOverline(flag);
	}
	else if (TagName == L"Size")
	{
		if (Text == L"Standard")
			aStyleVariant->SetFontSize(eSizeNormal);
		else if (Text == L"Small")
			aStyleVariant->SetFontSize(eSizeSmall);
		else if (Text == L"Big")
			aStyleVariant->SetFontSize(eSizeLarge);
		else if (Text == L"SmallCaps")
			aStyleVariant->SetFontSize(eSizeSmallCaps);
		else
		{
			TSizeValue size = ParseSizeValue(Text);
			if (!size.IsValid() && size.Integer() > 5)
				size.Units = eMetadataUnitType_pt;

			if (!size.IsValid())
				return ERROR_WRONG_TAG_CONTENT;

			aStyleVariant->SetFontSize(SIZE_VALUE_UNITS);
			aStyleVariant->SetFontSize(size);
		}
	}
	else if (TagName == L"Level")
	{
		if (Text == L"Normal")
			aStyleVariant->SetFontLevel(eLevelNormal);
		else if (Text == L"Sub")
			aStyleVariant->SetFontLevel(eLevelSub);
		else if (Text == L"Sup")
			aStyleVariant->SetFontLevel(eLevelSup);
		else
			return ERROR_WRONG_TAG_CONTENT;
	}
	else if (TagName == L"FontFamily")
	{
		ESldStyleFontFamilyEnum FontFamily = GetStyleFontFamilyByXmlTagContent(Text);
		if (FontFamily == eFontFamily_Unknown)
			return ERROR_WRONG_TAG_CONTENT;
		aStyleVariant->SetFontFamily(FontFamily);
	}
	else if (TagName == L"FontName")
	{
		ESldStyleFontNameEnum FontName = GetStyleFontNameByXmlTagContent(Text);
		if (FontName == eFontName_Unknown)
			return ERROR_WRONG_TAG_CONTENT;
		aStyleVariant->SetFontName(FontName);
	}
	else if (TagName == L"Prefix")
	{
		UInt32 len = (UInt32)Text.size();
		if (len > SLD_MAX_STYLE_PREFIX_SIZE)
		{
			sldXLog("Error! To large style prefix '%s'. Max prefix len = %d\n",
					sld::as_ref(Text), SLD_MAX_STYLE_PREFIX_SIZE);
			return ERROR_TO_LARGE_STYLE_PREFIX;
		}
		aStyleVariant->SetPrefix(Text);
	}
	else if (TagName == L"Postfix")
	{
		UInt32 len = (UInt32)Text.size();
		if (len > SLD_MAX_STYLE_POSTFIX_SIZE)
		{
			sldXLog("Error! To large style postfix '%s'. Max postfix len = %d\n",
					sld::as_ref(Text), SLD_MAX_STYLE_POSTFIX_SIZE);
			return ERROR_TO_LARGE_STYLE_POSTFIX;
		}
		aStyleVariant->SetPostfix(Text);
	}
	else if (TagName == L"BackgroundImageTop")
	{
		if (!Text.empty())
		{
			UInt32 index = aImages.AddImage(Text);
			auto mapping = m_ImageManager.AddImageContainer(aImages);
			aStyleVariant->SetBackgroundImageTop(mapping[index]);
		}
	}
	else if (TagName == L"BackgroundImageBottom")
	{
		if (!Text.empty())
		{
			UInt32 index = aImages.AddImage(Text);
			auto mapping = m_ImageManager.AddImageContainer(aImages);
			aStyleVariant->SetBackgroundImageBottom(mapping[index]);
		}
	}
	else if (TagName == L"Unclickable")
	{
		bool flag;
		int err = ParseBoolParamNode(aNode, flag, BoolType::OnOff);
		if (err != ERROR_NO)
			return err;
		aStyleVariant->SetUnclickable(flag ? 1 : 0);
	}
	else if (TagName == L"Comment" || TagName == L"TextType")
	{
		// TextType is deprecated for individual variants and we don't really care about comments
	}
	else
	{
		return ERROR_UNKNOWN_TAG;
	}
	
	return ERROR_NO;
}

/***********************************************************************
* Добавляем описание стиля
*
* @param aNode   - ветвь XML-дерева с нужным разделом
* @param aImages - контейнер с картинками
*
* @return код ошибки
************************************************************************/
int SlovoEdProject::AddStyle(pugi::xml_node aNode, CImageContainer &aImages)
{
	int error;

	const std::wstring styleName = aNode.child(L"Tag").child_value();
	if (styleName.empty())
	{
		sldXLog("Warning! Style without a tag name. Skipping.\n");
		return ERROR_NO;
	}

	if (isMetadataTag(styleName))
	{
		sldXLog("Note. Specifying metadata styles in the project file is deprecated: '%s'. Please remove.\n",
				sld::as_ref(styleName));
		return ERROR_NO;
	}

	TSlovoedStyle Style;
	TSlovoedStyleVariant UnTypedStyleVariant;

	Style.SetTag(styleName);

	for (pugi::xml_node Node = aNode.first_child(); Node; Node = Node.next_sibling())
	{
		if (Node.type() != pugi::node_element)
			continue;

		wstring TagName(Node.name());
		wstring Text(Node.child_value());

		if (TagName == L"TextType")
		{
			static const sld::enums::Descriptor<ESldStyleMetaTypeEnum> types[] = {
				{ L"Normal",   eMetaText },
				{ L"Phonetic", eMetaPhonetics }
			};
			ESldStyleMetaTypeEnum TextType = findValue(types, Text, eMetaUnknown);
			if (TextType != eMetaText && TextType != eMetaPhonetics)
			{
				// this will also result in spamming the console from PostProcessStyles()
				sldXLog("Warning! Unknown TextType ('%s') for style: '%s'. Using default style.\n",
						sld::as_ref(Text), sld::as_ref(styleName));
				return ERROR_NO;
			}
			Style.SetTextType(TextType);
		}
		else if (TagName == L"StyleVariant")
		{
			TSlovoedStyleVariant StyleVariant;
			for (pugi::xml_node vaNode : Node)
			{
				if (vaNode.type() != pugi::node_element)
					continue;

				error = SetStyleVariantData(&StyleVariant, vaNode, aImages);
				if (error != ERROR_NO)
				{
					logCommonError(error, vaNode);
					return error;
				}
			}

			if (Node.attribute(L"default").as_uint(0u) != 0u)
				Style.SetDefaultVariantIndex(Style.VariantsCount());

			Style.AddStyleVariant(StyleVariant);
		}
		else if (TagName == L"Tag") { /* handled above */ }
		else if (TagName == L"Language")
		{
			UInt32 langCode;
			error = ParseLanguageCodeNode(Node, &langCode);
			if (error != ERROR_NO)
			{
				logCommonError(ERROR_WRONG_TAG_CONTENT, Node);
				return ERROR_WRONG_TAG_CONTENT;
			}
			Style.SetLanguage(langCode);
		}
		else if (TagName == L"Usage")
		{
			ESldStyleUsageEnum Usage = GetStyleUsageByXmlTagContent(Text);
			if (Usage == eStyleUsage_Unknown)
				logCommonError(ERROR_WRONG_TAG_CONTENT, Node);
			else
				Style.AddUsage(Usage);
		}
		else
		{
			error = SetStyleVariantData(&UnTypedStyleVariant, Node, aImages);
			if (error != ERROR_NO)
			{
				logCommonError(error, Node);
				return error;
			}
		}
	}

	if (!Style.VariantsCount())
		Style.AddStyleVariant(UnTypedStyleVariant);

	if (!Style.UsageCount())
		Style.AddUsage(eStyleUsage_Unknown);

	m_Styles.AddStyle(Style);

	return ERROR_NO;
}

/** 
	Добавляем свойства словаря

	@param	aNode - ветвь XML-дерева

	@return код ошибки.
*/
int SlovoEdProject::AddDictionaryFeatures(pugi::xml_node aNode)
{
	for (pugi::xml_node Node = aNode.first_child(); Node; Node = Node.next_sibling())
	{
		if (Node.type() != pugi::node_element)
			continue;
		
		wstring Name(Node.name());
		wstring Text(Node.child_value());

		if (Name == L"SkeletalMode")
		{
			SetSkeletalMode(wcstoul(Text.c_str(), 0, 10));
		}
		else if (Name == L"Taboo")
		{
			SetTaboo(wcstoul(Text.c_str(), 0, 10));
		}
		else if (Name == L"Stress")
		{
			SetStress(wcstoul(Text.c_str(), 0, 10));
		}
		else if (Name == L"DarkTheme")
		{
			SetDarkTheme(wcstoul(Text.c_str(), 0, 10));
		}
		else
		{
			sldXLog("Error! Unknown tag: '%s'\n", sld::as_ref(Name));
			return ERROR_UNKNOWN_TAG;
		}
	}

	return ERROR_NO;
}

/** 
	Устанавливает свойство словаря SkeletalMode

	@param	aValue - значение свойства

	@return код ошибки.
*/
int SlovoEdProject::SetSkeletalMode(UInt32 aValue)
{
	return aValue ? SetFeature(eFeature_SkeletalMode) : ERROR_NO;
}

/** 
	Устанавливает свойство словаря Taboo

	@param	aValue - значение свойства

	@return код ошибки.
*/
int SlovoEdProject::SetTaboo(UInt32 aValue)
{
	return aValue ? SetFeature(eFeature_Taboo) : ERROR_NO;
}

/** 
	Устанавливает свойство словаря Stress

	@param	aValue - значение свойства

	@return код ошибки.
*/
int SlovoEdProject::SetStress(UInt32 aValue)
{
	return aValue ? SetFeature(eFeature_Stress) : ERROR_NO;
}

/**
	Устанавливает свойство словаря DarkTheme

	@param	aValue - значение свойства

	@return код ошибки.
*/
int SlovoEdProject::SetDarkTheme(UInt32 aValue)
{
	return aValue ? SetFeature(eFeature_DarkTheme) : ERROR_NO;
}

/** 
Устанавливает произвольное свойство словаря

	@param	aFeature - значение свойства

	@return код ошибки.
*/
int SlovoEdProject::SetFeature(const ESldFeatures aFeature)
{
	m_header.DictionaryFeatures |= (0x01 << aFeature);

	return ERROR_NO;
}

/***********************************************************************
* Устанавливает тип содержимого учебника
*
* @param	aType - тип содержимого
*
* @return код ошибки
************************************************************************/
int SlovoEdProject::SetContentType(const ESldContentType aType)
{
	AdditionalInfo()->ContentType |= aType;

	return ERROR_NO;
}

/** 
	Добавляем статьи из файла, путь к которому прописан в ветви дерева переданной в параметере

	@param	aNode - ветвь XML-дерева в которой находится путь к файлу с статьями.

	@return код ошибки.
*/
int SlovoEdProject::AddArticles(pugi::xml_node aNode, const wstring& aRoot, ArticlesLoader &aArticlesLoader)
{
	int error;

	std::vector<std::wstring> articleFiles;
	std::vector<std::wstring> cssFiles;
	std::wstring javaScriptFile;
	for (pugi::xml_node node : aNode)
	{
		if (node.type() != pugi::node_element)
			continue;

		const sld::wstring_ref nodeName = node.name();
		const std::wstring nodeText = node.child_value();
		if (nodeName == L"ArticlesFile")
		{
			articleFiles.push_back(nodeText);
		}
		else if (nodeName == L"ImageFolder")
		{
			aArticlesLoader.GetImageContainer().SetImageFolder(nodeText);
		}
		else if (nodeName == L"VideoFolder")
		{
			aArticlesLoader.GetVideoContainer().SetVideoFolder(nodeText);
		}
		else if (nodeName == L"SoundFolder")
		{
			error = aArticlesLoader.GetSoundContainer().SetSoundFolder(nodeText);
			if (error != ERROR_NO)
			{
				sldXLog("Error! Can't open sound folder: `%s`\n", sld::as_ref(nodeText));
				return error;
			}
		}
		else if (nodeName == L"CompressionMethod")
		{
			CompressionConfig config;
			int error = ParseCompressionConfig(nodeText, config);
			if (error != ERROR_NO)
				return error;
			m_Articles.SetCompressionMethod(config);
		}
		else if (nodeName == L"CSSFile")
		{
			cssFiles.push_back(nodeText);
		}
		else if (nodeName == L"JavaScriptFile")
		{
			if (javaScriptFile.size())
				sldXLog("Warning! Only a single js file currently supported!\n");
			else
				javaScriptFile = nodeText;
		}
		else
		{
			sldXLog("Error! Unknown <Articles> tag: `%s`\n", sld::as_ref(nodeName));
			return ERROR_UNKNOWN_TAG;
		}
	}

	// load css files
	bool cssError = false;
	for (const std::wstring &file : cssFiles)
	{
		const std::wstring path = aRoot + file;
		std::wstring contents = sld::read_text_file(path);
		if (contents.empty())
		{
			sldILog("Warning! Can't load css file `%s`. Check file presense and/or encoding. Skipping.\n",
					sld::as_ref(path));
			cssError = true;
			continue;
		}

		error = m_CSSDataManager.parseCSSFile(contents, aArticlesLoader.GetImageContainer(), path);
		if (error != ERROR_NO)
			return error;
	}
	if (cssError)
		sld::printf("\nWarning! There were some problems loading css file(s). Check the logs please!");

	// load articles
	for (const std::wstring &file : articleFiles)
	{
		error = aArticlesLoader.Load(aRoot + file);
		if (error != ERROR_NO)
			return error;
		// код ошибки получаем отдельно, т.к. сначала в лог нужно записать все однотипные ошибки,
		// не выходя из Load на первой встретившейся
		if (aArticlesLoader.GetError() != ERROR_NO)
			return error;
	}

	// load js file
	if (javaScriptFile.size())
	{
		m_javaScriptFileContents = sld::read_text_file(javaScriptFile);
		if (m_javaScriptFileContents.empty())
		{
			sldILog("Warning! Error loading js file (or it's compeltely empty): '%s'\n",
					sld::as_ref(javaScriptFile));
		}
	}

	return ERROR_NO;
}

/***********************************************************************
* Добавляем файл с таблицей сортировки
*
* @param aNode - ветвь XML-дерева с нужным разделом
* @param aRoot - папка, в которой лежит файл таблицы сортировки
*
* @return код ошибки
************************************************************************/
int SlovoEdProject::AddSortFile(pugi::xml_node aNode, wstring aRoot)
{
	UInt32 languageCode = 0;
	int error = ParseLanguageCodeAttrib(aNode, &languageCode);
	if (error != ERROR_NO)
		return error;

	return m_SortTablesManager.AddMainSortTable(languageCode, aRoot + aNode.child_value());
}

/***********************************************************************
* Добавляем файл с вспомогательной таблицей сортировки
*
* @param aNode - ветвь XML-дерева с нужным разделом
*
* @return код ошибки
************************************************************************/
int SlovoEdProject::AddSecondarySortFile(pugi::xml_node aNode, const wstring& aRoot)
{
	UInt32 languageCode = 0;
	int error = ParseLanguageCodeAttrib(aNode, &languageCode);
	if (error != ERROR_NO)
		return error;

	return m_SortTablesManager.AddSecondarySortTable(languageCode, aRoot + aNode.child_value());
}

static int load(TLocalizedNames &aStrings, sld::wstring_ref aName, const wchar_t *aString)
{
	int ret = ERROR_NO;
	if (aName == L"ProductName")
		sld::copy(aStrings.ProductName, aString);
	else if (aName == L"DictionaryName")
		sld::copy(aStrings.DictionaryName, aString);
	else if (aName == L"DictionaryNameShort")
		sld::copy(aStrings.DictionaryNameShort, aString);
	else if (aName == L"DictionaryClass")
		sld::copy(aStrings.DictionaryClass, aString);
	else if (aName == L"DictionaryLanguagePair")
		sld::copy(aStrings.DictionaryLanguagePair, aString);
	else if (aName == L"DictionaryLanguagePairShort")
		sld::copy(aStrings.DictionaryLanguagePairShort, aString);
	else if (aName == L"DictionaryLanguagePairReverse")
		sld::copy(aStrings.DictionaryLanguagePairReverse, aString);
	else if (aName == L"DictionaryLanguagePairShortReverse")
		sld::copy(aStrings.DictionaryLanguagePairShortReverse, aString);
	else if (aName == L"LanguageNameFrom")
		sld::copy(aStrings.LanguageNameFrom, aString);
	else if (aName == L"LanguageNameTo")
		sld::copy(aStrings.LanguageNameTo, aString);
	else if (aName == L"AuthorName")
		sld::copy(aStrings.AuthorName, aString);
	else if (aName == L"AuthorWeb")
		sld::copy(aStrings.AuthorWeb, aString);
	else
		ret = ERROR_UNKNOWN_TAG;
	return ret;
}

/***********************************************************************
* Добавляем локализованные строки
*
* @param aNode - ветвь XML-дерева с нужным разделом
*
* @return код ошибки
************************************************************************/
int SlovoEdProject::AddStrings(pugi::xml_node aNode)
{
	TLocalizedNames tmpNames;
	memset(&tmpNames, 0, sizeof(tmpNames));
	tmpNames.structSize = sizeof(tmpNames);
	
	int error = ParseLanguageCodeAttrib(aNode, &tmpNames.LanguageCode);
	if (error != ERROR_NO)
		return error;

	for (const auto &strings : m_names)
	{
		if (strings.LanguageCode == tmpNames.LanguageCode)
			return ERROR_NO;
	}

	// Локализованные строки
	for (pugi::xml_node node : aNode)
	{
		if (node.type() != pugi::node_element)
			continue;

		if (load(tmpNames, node.name(), node.child_value()) != ERROR_NO)
			sldXLog("Warning! Unknown tag: %s\n", sld::as_ref(node.name()));
	}

	m_names.push_back(tmpNames);

	return ERROR_NO;
}

/***********************************************************************
* Загружаем проект словаря из файла
* 
* @param aFileName	- имя файла
*
* @return код ошибки
************************************************************************/
int SlovoEdProject::Load(wstring aFileName)
{
	int error;
	CMyProcessTimer timerCommonXml("XmlParsingTimeCommon.time");
	timerCommonXml.PrintMemoryString("Memory Usage");
	
	// Загрузка xml-файла
	timerCommonXml.Start();
	pugi::xml_document XmlDoc;
	if (!XmlDoc.load_file(aFileName.c_str()))
	{
		sldXLog("Error! Can't load xml file: `%s`.\n", sld::as_ref(aFileName));
		return ERROR_CANT_OPEN_FILE;
	}

	timerCommonXml.Stop();
	timerCommonXml.PrintTimeString("Loading xml file by pugi parser");
	timerCommonXml.PrintMemoryString("Memory Usage");

	// парсинг xml-файла
	sldXLog("Processing file `%s`.\n", sld::as_ref(aFileName));
	timerCommonXml.Start();

	pugi::xml_node RootNode = XmlDoc.first_child();
	wstring RootNodeName(RootNode.name());
	wstring rootOfFile = aFileName.substr(0, aFileName.find_last_of(L'\\') + 1);

	if (RootNodeName != L"SlovoEd_Project")
	{
		sldXLog("Error! Wrong project tag: '%s', expected 'SlovoEd_Project'\n", sld::as_ref(RootNodeName));
		return ERROR_WRONG_PROJECT_TAG;
	}

	// узел <Articles>
	pugi::xml_node articlesNode;
	// все узлы <List> и <Style>
	std::vector<pugi::xml_node> listNodes, styleNodes;

	size_t IDLenght = 0;

	for (pugi::xml_node Node = RootNode.first_child(); Node; Node = Node.next_sibling())
	{
		if (Node.type() != pugi::node_element)
			continue;

		wstring nodeNameWstr(Node.name());
		wstring textWstr(Node.child_value());

		if (nodeNameWstr == L"LanguageCodeFrom")
		{
			error = ParseLanguageCodeNode(Node, &m_header.LanguageCodeFrom);
			if (error != ERROR_NO)
			{
				sldXLog("Error! Wrong language from: '%s'\n", sld::as_ref(textWstr));
				return ERROR_WRONG_LANGUAGE_FROM;
			}
		}
		else if (nodeNameWstr == L"LanguageCodeTo")
		{
			error = ParseLanguageCodeNode(Node, &m_header.LanguageCodeTo);
			if (error != ERROR_NO)
			{
				sldXLog("Error! Wrong language to: '%s'\n", sld::as_ref(textWstr));
				return ERROR_WRONG_LANGUAGE_TO;
			}
		}
		else if (nodeNameWstr == L"SortFile")
		{
			error = AddSortFile(Node, rootOfFile);
			if (error != ERROR_NO)
				return error;
		}
		else if (nodeNameWstr == L"SecondarySortFile")
		{
			error = AddSecondarySortFile(Node, rootOfFile);
			if (error != ERROR_NO)
				return error;
		}
		else if (nodeNameWstr == L"MorphoBase")
		{
			error = AddMorphoBase(Node);
			if (error != ERROR_NO)
				return error;
		}
		else if (nodeNameWstr == L"HASH")
		{
			m_header.HASH = wcstoul(textWstr.c_str(), NULL, 16);
		}
		else if (nodeNameWstr == L"DictID")
		{
			UInt32 size = sizeof(m_header.DictID);
			IDLenght = textWstr.length();
			if (IDLenght != 4 && IDLenght != 8)
			{
				sldXLog("Error! Wrong DICT ID: '%s'\n", sld::as_ref(textWstr));
				return ERROR_WRONG_DICT_ID;
			}
			UIntUnion code;
			code.ui_32 = 0;

			if (IDLenght == 4)
			{
				for (UInt32 n=0;n<size;n++)
					code.ui_8[n] = (UInt8)textWstr[n];
			}
			else
			{
				EncodeDictIdForETB(textWstr, code);
			}

			m_header.DictID = code.ui_32;
		}
		else if (nodeNameWstr == L"EngineVersion")
		{
			// Игнорируем это значение
			continue;
		}
		else if (nodeNameWstr == L"MarketingTotalWordsCount")
		{
			m_header.MarketingTotalWordsCount = wcstoul(textWstr.c_str(), NULL, 10);
		}
		else if (nodeNameWstr == L"DatabaseType")
		{
			SetDatabaseType(GetDatabaseTypeByXmlTagContent(textWstr));
		}
		else if (nodeNameWstr == L"IsInApp")
		{
			bool value;
			error = ParseBoolParamNode(Node, value, BoolType::YesNo);
			if (error != ERROR_NO)
			{
				sldXLog("Error! Invalid <IsInApp> value: '%s'.\n", sld::as_ref(textWstr));
				return ERROR_WRONG_TAG_CONTENT;
			}
			SetInApp(value ? 1 : 0);
		}
		else if (nodeNameWstr == L"SoundSourceType")
		{
			EMediaSourceTypeEnum Type = GetMediaSourceTypeByXmlTagContent(textWstr);
			if (Type == eMediaSourceType_Unknown)
			{
				sldXLog("Error! Unknown SoundSourceType: '%s'.\n", sld::as_ref(textWstr));
				return ERROR_UNKNOWN_MEDIA_SOURCE_TYPE;
			}
			SetSoundSourceType(Type);
		}
		else if (nodeNameWstr == L"ImageSourceType")
		{
			EMediaSourceTypeEnum Type = GetMediaSourceTypeByXmlTagContent(textWstr);
			if (Type == eMediaSourceType_Unknown)
			{
				sldXLog("Error! Unknown ImageSourceType: '%s'.\n", sld::as_ref(textWstr));
				return ERROR_UNKNOWN_MEDIA_SOURCE_TYPE;
			}
			SetImageSourceType(Type);
		}
		else if (nodeNameWstr == L"VideoSourceType")
		{
			EMediaSourceTypeEnum Type = GetMediaSourceTypeByXmlTagContent(textWstr);
			if (Type == eMediaSourceType_Unknown)
			{
				sldXLog("Error! Unknown VideoSourceType: '%s'.\n", sld::as_ref(textWstr));
				return ERROR_UNKNOWN_MEDIA_SOURCE_TYPE;
			}
			SetVideoSourceType(Type);
		}
		else if (nodeNameWstr == L"SceneSourceType")
		{
			EMediaSourceTypeEnum Type = GetMediaSourceTypeByXmlTagContent(textWstr);
			if (Type == eMediaSourceType_Unknown)
			{
				sldXLog("Error! Unknown SceneSourceType: '%s'.\n", sld::as_ref(textWstr));
				return ERROR_UNKNOWN_MEDIA_SOURCE_TYPE;
			}
			SetSceneSourceType(Type);
		}
		else if (nodeNameWstr == L"DictionaryFeatures")
		{
			error = AddDictionaryFeatures(Node);
			if (error != ERROR_NO)
				return error;
		}
		else if (nodeNameWstr == L"Strings")
		{
			error = AddStrings(Node);
			if (error != ERROR_NO)
				return error;
		}
		else if (nodeNameWstr == L"Articles")
		{
			if (articlesNode)
			{
				sldILog("Error! There can be only one <Articles> node!\n");
				sld::printf(eLogStatus_Error, "\nError! There can be only one <Articles> node!");
				return ERROR_WRONG_TAG;
			}
			articlesNode = Node;
		}
		else if (nodeNameWstr == L"List")
		{
			listNodes.push_back(Node);
		}
		else if (nodeNameWstr == L"Style")
		{
			styleNodes.push_back(Node);
		}
		else if (nodeNameWstr == L"ClassLevel_FromMajor" ||
				 nodeNameWstr == L"ClassLevel_Major" ||
				 nodeNameWstr == L"ClassLevel_From" ||
				 nodeNameWstr == L"ClassLevel")
		{
			AdditionalInfo()->ClassLevel_FromMajor = GetClassLevelMajor(textWstr);
			if (!AdditionalInfo()->ClassLevel_FromMajor)
			{
				sldXLog("Error! Wrong ClassLevel_FromMajor value\n");
				return ERROR_WRONG_TAG_CONTENT;
			}
		}
		else if (nodeNameWstr == L"ClassLevel_FromMinor" ||
				 nodeNameWstr == L"ClassLevel_Minor")
		{
			AdditionalInfo()->ClassLevel_FromMinor = GetClassLevelMinor(textWstr);
			if (!AdditionalInfo()->ClassLevel_FromMinor)
			{
				sldXLog("Error! Wrong ClassLevel_FromMinor value\n");
				return ERROR_WRONG_TAG_CONTENT;
			}
		}
		else if (nodeNameWstr == L"ClassLevel_UpToMajor" ||
				 nodeNameWstr == L"ClassLevel_UpTo")
		{
			AdditionalInfo()->ClassLevel_UpToMajor = GetClassLevelMajor(textWstr);
			if (!AdditionalInfo()->ClassLevel_UpToMajor)
			{
				sldXLog("Error! Wrong ClassLevel_UpToMajor value\n");
				return ERROR_WRONG_TAG_CONTENT;
			}
		}
		else if (nodeNameWstr == L"ClassLevel_UpToMinor")
		{
			AdditionalInfo()->ClassLevel_UpToMinor = GetClassLevelMinor(textWstr);
			if (!AdditionalInfo()->ClassLevel_UpToMinor)
			{
				sldXLog("Error! Wrong ClassLevel_UpToMinor value\n");
				return ERROR_WRONG_TAG_CONTENT;
			}
		}
		else if (nodeNameWstr == L"PublisherName")
		{
			sld::copy(AdditionalInfo()->PublisherName, textWstr.c_str());
		}
		else if (nodeNameWstr == L"PublishYear")
		{
			AdditionalInfo()->PublishYear = GetPublishYear(textWstr);
			if (!AdditionalInfo()->PublishYear)
			{
				sldXLog("Error! Wrong PublishYear value\n");
				return ERROR_WRONG_TAG_CONTENT;
			}
		}
		else if (nodeNameWstr == L"Revision")
		{
			AdditionalInfo()->Revision = GetRevision(textWstr);
			if (!AdditionalInfo()->Revision)
			{
				sldXLog("Error! Wrong Revision value\n");
				return ERROR_WRONG_TAG_CONTENT;
			}
		}
		else if (nodeNameWstr == L"BookPart")
		{
			AdditionalInfo()->BookPart = GetBookPart(textWstr);
			if (!AdditionalInfo()->BookPart)
			{
				sldXLog("Error! Wrong BookPart value\n");
				return ERROR_WRONG_TAG_CONTENT;
			}
		}
		else if (nodeNameWstr == L"ContentType")
		{
			const ESldContentType eContentType = GetContentType(textWstr);
			if (eContentType == eDataBaseContentType_Wrong)
			{
				sldXLog("Error! Wrong ContentType value\n");
				return ERROR_WRONG_TAG_CONTENT;
			}

			SetContentType(eContentType);
		}
		else if (nodeNameWstr == L"CoverThumbnail")
		{
			CImageContainer imageContainer;
			const UInt32 index = imageContainer.AddImage(textWstr);
			const auto imageMap = m_ImageManager.AddImageContainer(imageContainer);
			AdditionalInfo()->CoverThumbnail = imageMap[index];
		}
		else if (nodeNameWstr == L"CoverFullImage")
		{
			CImageContainer imageContainer;
			const UInt32 index = imageContainer.AddImage(textWstr);
			const auto imageMap = m_ImageManager.AddImageContainer(imageContainer);
			AdditionalInfo()->CoverFullImage = imageMap[index];
		}
		else if (nodeNameWstr == L"ISBN")
		{
			string ISBN = sld::as_utf8(textWstr);

			if (ISBN.length() > 19)
			{
				sldXLog("Error! Wrong ISBN size\n");
				return ERROR_WRONG_TAG_CONTENT;
			}

			if (ISBN.size())
			{
				strcpy((char*)AdditionalInfo()->ISBN, ISBN.c_str());
			}
		}
		else if (nodeNameWstr == L"EducationalLevel")
		{
			EEducationalLevel newLevel = GetEducationalLevel(textWstr);
			if (newLevel == eEducationalLevelUnknown)
			{
				sldXLog("Error! Wrong EducationalLevel value: '%s'\n", sld::as_ref(textWstr));
				return ERROR_WRONG_TAG_CONTENT;
			}

			AdditionalInfo()->EducationalLevel |= newLevel;		
		}
		else if (nodeNameWstr == L"Annotation")
		{
			m_Annotation = textWstr;
		}
		else if (nodeNameWstr == L"IsMultiLanguageBase")
		{
			ParseBoolParamNode(Node, m_IsMultiLanguageBase, BoolType::YesNo);
		}
		else if (nodeNameWstr == L"LanguageSymbols" ||
				 nodeNameWstr == L"SymbolsDelimiters")
		{
			// we don't use these since a long time ago
			sldXLog("Deprecated tag: '%s'. Please remove.\n", sld::as_ref(nodeNameWstr));
		}
		else
		{
			sldXLog("Error! Unknown tag: '%s'\n", sld::as_ref(nodeNameWstr));
			return ERROR_WRONG_TAG;
		}
	}

	if ((IDLenght == 8 && m_DatabaseType != eDatabaseType_TextBook) || (IDLenght == 4 && m_DatabaseType == eDatabaseType_TextBook))
	{
		sldXLog("Error! DictID\n");
		return ERROR_WRONG_DICT_ID;
	}

	// Список с атомарными объектами (?)
	CWordList atomicList;

	// Загрузчик статей
	ArticlesLoader articlesLoader(m_Articles, m_MetadataManager, m_CSSDataManager, m_Styles, atomicList);

	for (pugi::xml_node styleNode : styleNodes)
	{
		error = AddStyle(styleNode, articlesLoader.GetImageContainer());
		if (error != ERROR_NO)
			return error;
	}

	if (articlesNode)
	{
		error = AddArticles(articlesNode, rootOfFile, articlesLoader);
		if (error != ERROR_NO)
			return error;
	}

	// вспомогательные списки сортированные по Show-варианту
	std::vector<CWordList> simpleSortedLists;

	for (pugi::xml_node listNode : listNodes)
	{
		error = AddList(listNode, simpleSortedLists, articlesLoader);
		if (error != ERROR_NO)
			return error;
	}

	error = ProcessAtomicReferenceLists(atomicList);
	if (error != ERROR_NO)
		return error;

	// Добавляем вспомогательные листы, для поиска по Show-варианту
	const UInt32 numberOfLists = m_WordLists.GetNumberOfLists();
	for (UInt32 currentListIndex : xrange(numberOfLists))
	{
		CWordList* currentList = m_WordLists.GetWordList(currentListIndex);
		Int32 simpleSortedIndex = currentList->GetSimpleSortedListIndex();
		if (simpleSortedIndex == SLD_DEFAULT_LIST_INDEX || simpleSortedIndex >= simpleSortedLists.size())
			continue;

		currentList->SetSimpleSortedListIndex(numberOfLists + currentList->GetSimpleSortedListIndex());
		error = AddWordList(std::move(simpleSortedLists[simpleSortedIndex]));
		if (error != ERROR_NO)
			return error;
	}

	timerCommonXml.Stop();
	timerCommonXml.PrintTimeString("Parsing and creating project by Compiler");
	timerCommonXml.PrintMemoryString("Memory Usage");

	sld::printf("\nLoading project complete");
	CLogW::PrintToConsole("[OK]");
	
	sld::printf("\n\nPost processing...");
	
	timerCommonXml.Start();
	// Постообработка проекта
	error = PostProcessProject(articlesLoader);
	if (error != ERROR_NO)
	{
		CLogW::PrintToConsole("[FAIL]", eLogStatus_Error);
		return error;
	}
	CLogW::PrintToConsole("[OK]");

	timerCommonXml.Stop();
	timerCommonXml.PrintTimeString("Postprocessing project by Compiler");
	timerCommonXml.PrintMemoryString("Memory Usage");

	return ERROR_NO;
}

/***********************************************************************
* Возвращает имя файла для чтения/сохранения информации о версии словарной базы
*
* @return имя файла
************************************************************************/
wstring SlovoEdProject::GetVersionInfoFilename(void)
{
	wchar_t wbuf[256] = {0};
	UIntUnion code;
	code.ui_32 = m_header.DictID;

	if (!IsETBDatabase())
	{
		wbuf[0] = code.ui_8[0];
		wbuf[1] = code.ui_8[1];
		wbuf[2] = code.ui_8[2];
		wbuf[3] = code.ui_8[3];
	}
	else
		swprintf(wbuf, sld::array_size(wbuf), L"%08X", code.ui_32);

	wcscat(wbuf, L".Version.xml");
	
	return wstring(wbuf);
}

/***********************************************************************
* Загружает информацию о версии словарной базы из отдельного файла
* 
* @param aFileName	- имя файла
*
* @return код ошибки
************************************************************************/
int SlovoEdProject::LoadVersionInfo(const wstring &aFileName)
{
	memset(&m_VersionInfo, 0, sizeof(m_VersionInfo));
	m_VersionInfo.structSize = sizeof(m_VersionInfo);

	pugi::xml_document doc;
	auto result = doc.load_file(aFileName.c_str());
	if (result.status == pugi::status_file_not_found)
	{
		SaveVersionInfo(aFileName);
		result = doc.load_file(aFileName.c_str());
	}

	if (result.status != pugi::status_ok)
	{
		sldILog("Error! Can't open file: '%s' - %s\n",
				sld::as_ref(aFileName), result.description());
		return ERROR_CANT_OPEN_FILE;
	}

	pugi::xml_node root = doc.first_child();
	if (root.name() != sld::wstring_ref(L"DictionaryVersionInfo"))
	{
		sldILog("Error! Unknown tag: %s\n", sld::as_ref(root.name()));
		return ERROR_WRONG_TAG;
	}

	for (pugi::xml_node node : root)
	{
		if (node.type() != pugi::node_element)
			continue;

		const sld::wstring_ref nodeName = node.name();
		const sld::wstring_ref nodeText = node.text().get();
		if (nodeName == L"MajorVersion" || nodeName == L"MinorVersion")
		{
			const unsigned long version = wcstoul(nodeText.data(), nullptr, 10);
			if (version == ULONG_MAX)
			{
				sldILog("Error! Wrong tag content: TagName=%s, TagContent=%s",
						sld::as_ref(nodeName), sld::as_ref(nodeText));
				return ERROR_WRONG_TAG_CONTENT;
			}

			if (nodeName == L"MajorVersion")
				m_VersionInfo.MajorVersion = version;
			else
				m_VersionInfo.MinorVersion = version;
		}
		else if (nodeName == L"BrandName")
		{
			EDictionaryBrandName brand = GetDictionaryBrandNameByXmlTagContent(to_string(nodeText));
			if (brand == eBrand_Unknown)
			{
				sldILog("Warning! Unknown brand name: '%s'. Default brand name \"Slovoed\" will be used.\n",
						sld::as_ref(nodeText));
			}
			else
			{
				m_VersionInfo.Brand = brand;
			}
		}
		else
		{
			sldILog("Error! Unknown tag: %s\n", sld::as_ref(nodeName));
			return ERROR_WRONG_TAG;
		}
	}

	if (!m_IsInApp)
		m_VersionInfo.MinorVersion++;

	return ERROR_NO;
}

/***********************************************************************
* Сохраняет информацию о версии словарной базы в отдельный файл
* 
* @param aFileName	- имя файла
*
* @return код ошибки
************************************************************************/
int SlovoEdProject::SaveVersionInfo(const wstring &aFileName)
{
	pugi::xml_document doc;

	const std::wstring brandName(GetDictionaryXmlBrandNameByBrandId(m_VersionInfo.Brand));

	pugi::xml_node root = doc.append_child(L"DictionaryVersionInfo");
	root.append_child(L"MajorVersion").text().set(m_VersionInfo.MajorVersion);
	root.append_child(L"MinorVersion").text().set(m_VersionInfo.MinorVersion);
	root.append_child(L"BrandName").text().set(brandName.c_str());

	const unsigned flags = pugi::format_indent | pugi::format_no_declaration | pugi::format_write_bom;
	if (!doc.save_file(aFileName.c_str(), L"\t", flags, pugi::encoding_utf16_le))
	{
		sldILog("Error! Can't save file: %s\n", sld::as_ref(aFileName));
		return ERROR_CANT_OPEN_FILE;
	}
	return ERROR_NO;
}

/***********************************************************************
* Возвращает указатель на структуру с информацией о версии словарной базы
*
* @return указатель на структуру
************************************************************************/
const TDictionaryVersionInfo* SlovoEdProject::GetDictionaryVersionInfo(void)
{
	return &m_VersionInfo;
}

// processes localization strings (mostly regarding default "0000" localization)
static int postProcessStrings(std::vector<TLocalizedNames> &aStrings, UInt32 aLangCode)
{
	const auto defaultCode = SldLanguage::Default;

	const TLocalizedNames *names = nullptr;
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
		sldILog("Notice. Autogenerating default project localization strings from '%s' locale.\n", langCode);
	}
	else
	{
		sld::printf(eLogStatus_Error, "\nError! No default project localization strings! See log file for details.");

		if (aStrings.empty())
			sldILog("Error! Autogenerating default project localization strings failed because there are no project strings!\n");
		else
			sldILog("Error! Autogenerating default project localization strings failed because no '%s' locale strings exist!\n", langCode);

		return ERROR_EMPTY_STRING;
	}

	aStrings.insert(aStrings.cbegin(), *names)->LanguageCode = defaultCode;
	return ERROR_NO;
}

/***********************************************************************
* Делает постобработку загруженного проекта
*
* @return код ошибки
************************************************************************/
int SlovoEdProject::PostProcessProject(ArticlesLoader &aArticlesLoader)
{
	int error = DoPrepagePages();
	if (error != ERROR_NO)
		return error;

	// Обрабатываем строки локализации
	error = postProcessStrings(m_names, m_header.LanguageCodeFrom);
	if (error != ERROR_NO)
		return error;

	// Обрабатываем метаданные
	error = PostProcessMetadata(aArticlesLoader);
	if (error != ERROR_NO)
		return error;

	// Обрабатываем ресурсы в листах
	error = PostProcessListsResources();
	if (error != ERROR_NO)
		return error;

	// Обрабатываем таблицы сортировки
	error = PostProcessSortTables();
	if (error != ERROR_NO)
		return error;

	// Обрабатываем списки
	// Нужно делать после обработки картинок и озвучек
	error = m_WordLists.PostProcessLists(m_header.LanguageCodeFrom, m_header.LanguageCodeTo,
										 &m_Styles, m_IsMultiLanguageBase);
	if (error != ERROR_NO)
		return error;

	// Устанавливаем индексы стилей у блоков и проверяем корректность стилизации
	error = PostProcessStyles();
	if (error != ERROR_NO)
		return error;

	// Заполняем информацию заголовка словаря
	m_header.HeaderSize = sizeof(m_header);

	m_header.Version = GetDictionaryVersion();
	sldILog("Engine version: %u\n", m_header.Version);

	m_header.NumberOfLists = GetNumberOfLists();
	m_header.articlesDataType = RESOURCE_TYPE_ARTICLE_DATA;
	m_header.articlesTreeType = RESOURCE_TYPE_ARTICLE_TREE;
	m_header.articlesStyleType = RESOURCE_TYPE_ARTICLE_STYLE;
	m_header.articlesQAType = RESOURCE_TYPE_ARTICLE_QA;
	m_header.NumberOfArticles = m_Articles.GetNumberOfArticles();
	m_header.ArticlesCompressionMethod = m_Articles.GetCompressionMethod().type;
	m_header.NumberOfStyles = m_Styles.StylesCount()+1;
	m_header.NumberOfStringsLanguages = GetStringsCount();
	m_header.SoundSourceType = GetSoundSourceType();
	m_header.ImageSourceType = GetImageSourceType();
	m_header.VideoSourceType = GetVideoSourceType();
	m_header.SceneSourceType = GetSceneSourceType();
	m_header.TotalPictureCount = m_ImageManager.GetImageCount();
	m_header.TotalVideoCount = m_VideoManager.GetVideoCount();
	m_header.TotalSoundCount = m_SoundManager.GetSoundCount();
	m_header.TotalSceneCount = m_SceneManager.GetSceneCount();
	m_header.TotalAbstractItemCount = m_AbstractItemManager.GetItemsCount();
	m_header.HasMorphologyData = m_MorphoDataManager.GetMorphoBasesCount() ? 1 : 0;
	m_header.HasSwitchThematics = m_IsHasSwitchThematics;

	// Заполняем структуру дополнительной информации
	if (m_pAdditionalInfo)
		m_pAdditionalInfo->StructSize = sizeof(*m_pAdditionalInfo);

	return ERROR_NO;
}

/***********************************************************************
* Постобработка таблиц сортировки
*
* Для каждой таблицы сортировки для языка LangCode создаем (если возможно) таблицу соответствия
* символов в верхнем и нижнем регистрах для символов, встречающихся в словах списков слов
* с таким же LangCode в качестве основного
*
* @return код ошибки
************************************************************************/
int SlovoEdProject::PostProcessSortTables(void)
{
	// Массив кодов языков и списков слов с этими кодами языков
	map<UInt32, vector<const CWordList*>> mLangCodes;

	for (const CWordList &list : lists())
		mLangCodes[list.GetLanguageCodeFrom()].push_back(&list);

	// Для каждого языка составим таблицу символов из слов, содержащихся в списках
	std::unordered_set<UInt16> mListSymbols;

	for (const auto &it : mLangCodes)
	{
		const UInt32 LangCode = it.first;
		mListSymbols.clear();

		// Просто проверка, что имеется таблица сортировки для указанного языка
		// Она должна быть, потому что имеются списки слов с этим языком в качестве основного
		const TLanguageSortTable* pSortTable = m_SortTablesManager.GetLanguageSortTable(LangCode);
		if (!pSortTable)
		{
			sldILog("Error! SlovoEdProject::PostProcessSortTables : sort table doesn't exist!\n");
			return ERROR_NULL_SORT_TABLE;
		}

		for (const CWordList *list : it.second)
		{
			for (const ListElement &listElement : list->elements())
			{
				for (const std::wstring &wlName : listElement.Name)
				{
					// this is obviously wrong for non bmp utf16 on platforms with u32 wchar_t
					for (wchar_t chr : wlName)
						mListSymbols.insert(chr);
				}
			}
		}

		if (!mListSymbols.empty())
		{
			int error = m_SortTablesManager.AddDictionarySymbolPairsTable(LangCode, mListSymbols);
			if (error != ERROR_NO)
				return error;
		}
	}

	return ERROR_NO;
}

/**
 * Производит постобработку метаданных
 *
 * @param[in] aArticlesLoader - загрузчик статей
 *
 * @return код ошибки
 */
int SlovoEdProject::PostProcessMetadata(ArticlesLoader &aArticlesLoader)
{
	// устанавливаем флаг, что в базе есть тематики
	if (m_Articles.GetNumberOfArticles() && CArticle::m_ThematicToStatesNumberMap.size())
		m_IsHasSwitchThematics = 1;

	const auto imageMap = m_ImageManager.AddImageContainer(aArticlesLoader.GetImageContainer());
	const auto videoMap = m_VideoManager.AddVideoContainer(aArticlesLoader.GetVideoContainer());
	const auto soundMap = m_SoundManager.AddSoundContainer(aArticlesLoader.GetSoundContainer());
	const auto abstractItemsMap =
		m_AbstractItemManager.AddAbstractItemContainer(aArticlesLoader.GetAbstractItemContainer());
	const auto sceneMap = m_SceneManager.AddSceneContainer(aArticlesLoader.GetSceneContainer(),
														   m_MaterialManager, m_MeshManager);

	for (CArticle &article : articles())
	{
		for (BlockType &block : article.blocks())
		{
			if (block.isClosing)
				continue;

			switch (block.metaType)
			{
			case eMetaBackgroundImage:
			{
				auto metaData = m_MetadataManager.findMetadata<eMetaBackgroundImage>(block.metadata());
				if (metaData)
					metaData->PictureIndex = imageMap[metaData->PictureIndex];
				break;
			}
			case eMetaImage:
			{
				auto metaData = m_MetadataManager.findMetadata<eMetaImage>(block.metadata());
				if (metaData) {
					metaData->PictureIndex = imageMap[metaData->PictureIndex];
					metaData->FullPictureIndex = imageMap[metaData->FullPictureIndex];
				}
				break;
			}
			case eMetaMap:
			{
				auto metaData = m_MetadataManager.findMetadata<eMetaMap>(block.metadata());
				if (metaData) {
					metaData->PictureIndex = imageMap[metaData->PictureIndex];
					metaData->TopPictureIndex = imageMap[metaData->TopPictureIndex];
				}
				break;
			}
			case eMetaMediaContainer:
			{
				auto metaData = m_MetadataManager.findMetadata<eMetaMediaContainer>(block.metadata());
				if (metaData) {
					metaData->Background = imageMap[metaData->Background];
					metaData->OverlaySrc = imageMap[metaData->OverlaySrc];
				}
				break;
			}
			case eMetaPopupImage:
			{
				auto metaData = m_MetadataManager.findMetadata<eMetaPopupImage>(block.metadata());
				if (metaData)
					metaData->PictureIndex = imageMap[metaData->PictureIndex];
				break;
			}
			case eMetaImageArea:
			{
				auto metaData = m_MetadataManager.findMetadata<eMetaImageArea>(block.metadata());
				if (metaData)
					metaData->Mask = imageMap[metaData->Mask];
				break;
			}
			case eMetaVideoSource:
			{
				auto metaData = m_MetadataManager.findMetadata<eMetaVideoSource>(block.metadata());
				if (metaData && metaData->Id && metaData->Id != SLD_INDEX_VIDEO_NO)
				{
					metaData->Id = videoMap[metaData->Id];
					metaData->ExtKey = TMetadataExtKey();
				}
				break;
			}
			case eMetaScene:
			{
				auto metaData = m_MetadataManager.findMetadata<eMetaScene>(block.metadata());
				if (metaData && metaData->Id && metaData->Id != SLD_INDEX_SCENE_NO)
					metaData->Id = sceneMap[metaData->Id];
				break;
			}
			case eMetaSound:
			{
				auto metaData = m_MetadataManager.findMetadata<eMetaSound>(block.metadata());
				if (metaData)
					metaData->Index = soundMap[metaData->Index];
				break;
			}
			case eMetaAbstractResource:
			{
				auto metaData = m_MetadataManager.findMetadata<eMetaAbstractResource>(block.metadata());
				if (metaData && metaData->ItemIdx && !checkExtKey(metaData->ExtKey, &m_MetadataManager))
				{
					metaData->ItemIdx = abstractItemsMap[metaData->ItemIdx];
					// just in case
					metaData->ExtKey = TMetadataExtKey();
				}
				break;
			}
			case eMetaFormula:
			{
				auto metaData = m_MetadataManager.findMetadata<eMetaFormula>(block.metadata());
				if (metaData)
					metaData->AbstractItemIdx = abstractItemsMap[metaData->AbstractItemIdx];
				break;
			}
			case eMetaList:
			{
				auto metaData = m_MetadataManager.findMetadata<eMetaList>(block.metadata());
				if (metaData && block.TextLength(BlockType::ListMarkers))
				{
					// XXX: these can be moved to outside of the articles loop as an optimization
					STString<1024 * 2> wbuf;
					std::vector<sld::wstring_ref> markers;

					const CImageContainer &imageContainer = aArticlesLoader.GetImageContainer();
					split(block.GetText(BlockType::ListMarkers), L'|', markers);
					for (int marker_index = 0; marker_index < markers.size(); ++marker_index)
					{
						sld::wstring_ref marker = markers[marker_index];
						if (marker_index != 0)
							wbuf.append(L'|');

						if (marker.starts_with(L"img://"))
						{
							marker.remove_prefix(wcslen(L"img://"));
							const uint32_t index = imageMap[imageContainer.GetImageIndexByName(marker)];
							wbuf.appendf(L"img://%X", index);
						}
						else
							wbuf.append(marker);
					}
					metaData->Marker = m_MetadataManager.addString(wbuf);
				}
				break;
			}
			case eMetaSlideShow:
			{
				auto metaData = m_MetadataManager.findMetadata<eMetaSlideShow>(block.metadata());
				if (metaData && block.TextLength(BlockType::SlideShowList))
				{
					UInt32 listIndex = SLD_DEFAULT_LIST_INDEX;
					UInt32 wordIndex = SLD_DEFAULT_WORD_INDEX;
					const std::wstring& name = block.GetText(BlockType::SlideShowList);
					for (auto list : enumerate(lists()))
					{
						wordIndex = list->GetIndexByName(name);
						if (wordIndex != SLD_DEFAULT_WORD_INDEX)
						{
							listIndex = static_cast<UInt32>(list.index);
							break;
						}
					}

					if (listIndex == SLD_DEFAULT_LIST_INDEX || wordIndex == SLD_DEFAULT_WORD_INDEX)
						return ERROR_WRONG_STRING_ATTRIBUTE;

					metaData->ListIndex = listIndex;
					metaData->WordIndex = wordIndex;
				}
				break;
			}
			default:
				break;
			}
		}
	}

	// Переписываем индексы материалов/текстур
	m_MaterialManager.MapNameToGlobalIndex(m_ImageManager.AddImageContainer(m_SceneManager.textures()));

	// Переписываем индексы в CSS свойствах
	m_CSSDataManager.rewriteImageUrls(imageMap);

	return ERROR_NO;
}

namespace {

struct ResourceCounts
{
	UInt32 images = 0;
	UInt32 scenes = 0;
	UInt32 sounds = 0;
	UInt32 videos = 0;
};

ResourceCounts& operator+=(ResourceCounts &lhs, const ResourceCounts& rhs)
{
	lhs.images += rhs.images;
	lhs.scenes += rhs.scenes;
	lhs.sounds += rhs.sounds;
	lhs.videos += rhs.videos;
	return lhs;
}

static ResourceCounts calculateResourceCounts(const CArticle *aArticle, const MetadataManager &aMetaMgr)
{
	ResourceCounts counts;

	for (const BlockType &block : aArticle->blocks())
	{
		if (block.isClosing)
			continue;

		switch (block.metaType)
		{
		case eMetaBackgroundImage:
		{
			const auto bgImage = aMetaMgr.findMetadata<eMetaBackgroundImage>(block.metadata());
			if (bgImage && bgImage->PictureIndex != SLD_INDEX_PICTURE_NO)
				counts.images++;
			break;
		}
		case eMetaImage:
		{
			const auto image = aMetaMgr.findMetadata<eMetaImage>(block.metadata());
			if (image)
			{
				if (image->PictureIndex != SLD_INDEX_PICTURE_NO)
					counts.images++;
				if (image->FullPictureIndex != SLD_INDEX_PICTURE_NO)
					counts.images++;
			}
			break;
		}
		case eMetaPopupImage:
		{
			const auto popupImage = aMetaMgr.findMetadata<eMetaPopupImage>(block.metadata());
			if (popupImage && popupImage->PictureIndex != SLD_INDEX_PICTURE_NO)
				counts.images++;
			break;
		}
		case eMetaVideoSource:
		{
			auto video = aMetaMgr.findMetadata<eMetaVideoSource>(block.metadata());
			if (video && video->Id != SLD_INDEX_VIDEO_NO)
				counts.videos++;
			break;
		}
		case eMetaScene:
		{
			auto scene = aMetaMgr.findMetadata<eMetaScene>(block.metadata());
			if (scene && scene->Id != SLD_INDEX_SCENE_NO)
				counts.scenes++;
			break;
		}
		case eMetaSound:
		{
			auto sound = aMetaMgr.findMetadata<eMetaSound>(block.metadata());
			if (sound && sound->Index != SLD_INDEX_SOUND_NO)
				counts.sounds++;
			break;
		}
		default:
			break;
		}
	}

	return counts;
}

}

/**
 * Производит постобработку ресурсов в листах
 *
 * @return код ошибки
 */
int SlovoEdProject::PostProcessListsResources()
{
	// article resources counts cache
	std::unordered_map<sld::wstring_ref, ResourceCounts> articlesCounts;

	for (CWordList &list : lists())
	{
		// Не считаем ресурсы в списках слов полнотекстового поиска
		if (list.IsFullTextSearchList() || list.GetLinkedList())
			continue;

		// счетчики ресурсов в листе
		ResourceCounts listCounts;

		for (const ListElement &listElement : list.elements())
		{
			// учитывам ресурсы самих слов в списке
			if (listElement.PictureIndex != SLD_INDEX_PICTURE_NO)
				listCounts.images++;

			if (listElement.VideoIndex != SLD_INDEX_VIDEO_NO)
				listCounts.videos++;

			if (listElement.SceneIndex != SLD_INDEX_SCENE_NO)
				listCounts.scenes++;

			if (listElement.SoundIndex != SLD_INDEX_SOUND_NO)
				listCounts.sounds++;

			// учитывам ресурсы в переводах
			for (const ArticleRefStruct &articleRef : listElement.ArticleRefs)
			{
				const auto it = articlesCounts.find(articleRef.ID);
				if (it != articlesCounts.end())
				{
					listCounts += it->second;
					continue;
				}

				const CArticle *article = m_Articles.GetArticleByID(articleRef.ID);
				if (!article)
				{
					sldILog("Error! SlovoEdProject::PostProcessListsResources : invalid article id: '%s', list: '%s'!\n",
							sld::as_ref(articleRef.ID), sld::as_ref(list.GetListId()));
					return ERROR_NULL_POINTER;
				}

				ResourceCounts counts = calculateResourceCounts(article, m_MetadataManager);
				articlesCounts.emplace(articleRef.ID, counts);
				listCounts += counts;
			}

			list.SetTotalPictureCount(listCounts.images);
			list.SetTotalVideoCount(listCounts.videos);
			list.SetTotalSceneCount(listCounts.scenes);
			list.SetTotalSoundCount(listCounts.sounds);
		}
	}

	// количество бит необходимых для представления индексов ресурсов
	const UInt32 imageIndexSize = GetCodeLen(m_ImageManager.GetImageCount());
	const UInt32 sceneIndexSize = GetCodeLen(m_SceneManager.GetSceneCount());
	const UInt32 soundIndexSize = GetCodeLen(m_SoundManager.GetSoundCount());
	const UInt32 videoIndexSize = GetCodeLen(m_VideoManager.GetVideoCount());

	// устанавливаем для каждого списка размер индекса ресуров
	for (CWordList &list : lists())
	{
		if (imageIndexSize && list.GetPicture())
			list.SetPictureIndexSize(imageIndexSize);

		if (sceneIndexSize && list.GetScene())
			list.SetSceneIndexSize(sceneIndexSize);

		if (soundIndexSize && list.GetSound())
			list.SetSoundIndexSize(soundIndexSize);

		if (videoIndexSize && list.GetVideo())
			list.SetVideoIndexSize(videoIndexSize);
	}

	return ERROR_NO;
}

/**
	Возвращает количество загруженных списков слов

	@return количество загруженных списков слов
*/
UInt32 SlovoEdProject::GetNumberOfLists() const
{
	return m_WordLists.GetNumberOfLists();
}

/**
	Возвращает объект списка слов по его номеру среди прочих загруженных списков слов

	@param index	- номер списка слов, может быть [0..#SlovoEdProject::GetNumberOfLists]

	@return  указатель на найденный список слов или NULL в случае ошибки
*/
CWordList * SlovoEdProject::GetWordList(UInt32 index)
{
	return m_WordLists.GetWordList(index);
}

const CWordList* SlovoEdProject::GetWordList(UInt32 index) const
{
	return m_WordLists.GetWordList(index);
}

/************************************************************************
* Вовращает указатель на объект с статьями
*
* @return указатель на объект с статьями
************************************************************************/
CArticles *SlovoEdProject::GetArticles(void)
{
	return &m_Articles;
}

/************************************************************************
* Возвращает указатель на объект для работы с таблицами сортировки
*
* @return указатель на объект для работы с таблицами сортировки
************************************************************************/
CSortTablesManager* SlovoEdProject::GetSortTablesManager(void)
{
	return &m_SortTablesManager;
}

/** *********************************************************************
* Возвращает указатель на структуру заголовка словаря.
* 
* @return указатель на структуру заголовка словаря
************************************************************************/
const TDictionaryHeader *SlovoEdProject::GetDictionaryHeader()
{
	return &m_header;
}

/** *********************************************************************
* Возвращает указатель на структуру с дополнительной информацией учебника.
* 
* @return указатель на структуру с дополнительной информацией учебника
************************************************************************/
const TAdditionalInfo *SlovoEdProject::GetAdditionalInfo()
{
	return m_pAdditionalInfo;
}

/** *********************************************************************
* Возвращает строку с аннотацией к учебнику
* 
* @return строка с аннотацией к учебнику
************************************************************************/
wstring SlovoEdProject::GetAnnotation() const
{
	return m_Annotation;
}


/// Возвращает количество записей с локализованными строчками
UInt32 SlovoEdProject::GetStringsCount()
{
	return (UInt32)m_names.size();
}

/// Возвращает указатель на локализованные строчки по номеру
const TLocalizedNames * SlovoEdProject::GetStrings(UInt32 aIndex)
{
	if (aIndex >= m_names.size())
		return NULL;

	return &m_names[aIndex];
}
/***********************************************************************
* Возвращает количество наборов локализованных строк (названий)
*
* @return количество наборов локализованных строк
************************************************************************/
UInt32 SlovoEdProject::LocalizedNamesCount(void) const
{
	return ((UInt32)m_names.size());
}

/***********************************************************************
* Возвращает указатель на набор локализованных строк (названий) по номеру
*
* @param aIndex	- индекс запрашиваемого набора
*
* @return	- указатель на набор локализованных строк
*			- NULL, если запрашиваемый набор не найден
************************************************************************/
TLocalizedNames* SlovoEdProject::GetLocalizedNames(UInt32 aIndex)
{
	if (aIndex >= LocalizedNamesCount())
		return NULL;
	
	return &(m_names[aIndex]);
}

/** 
	Создает и возвращает структуру дополнительной информации

	@return	указатель на структуру дополнительной информации
*/
TAdditionalInfo *SlovoEdProject::AdditionalInfo()
{
	if (!m_pAdditionalInfo)
	{
		m_pAdditionalInfo = new TAdditionalInfo;
		memset(m_pAdditionalInfo, 0, sizeof(*m_pAdditionalInfo));
	}

	return m_pAdditionalInfo;
}

/***********************************************************************
* Возвращает менеджер стилей
*
* @return указатель на менеджер стилей
************************************************************************/
TSlovoedStyleManager * SlovoEdProject::GetStyles(void)
{
	return &m_Styles;
}

/***********************************************************************
* Возвращает менеджер картинок
*
* @return указатель на менеджер картинок
************************************************************************/
CImageManager* SlovoEdProject::GetImageManager(void)
{
	return &m_ImageManager;
}

/***********************************************************************
* Возвращает менеджер видео
*
* @return указатель на менеджер картинок
************************************************************************/
CVideoManager* SlovoEdProject::GetVideoManager(void)
{
	return &m_VideoManager;
}

/***********************************************************************
* Возвращает менеджер сцен
*
* @return указатель на менеджер сцен
************************************************************************/
CSceneManager* SlovoEdProject::GetSceneManager(void)
{
	return &m_SceneManager;
}

/***********************************************************************
* Возвращает менеджер озвучек
*
* @return указатель на менеджер озвучек
************************************************************************/
CSoundManager* SlovoEdProject::GetSoundManager(void)
{
	return &m_SoundManager;
}

/***********************************************************************
* Возвращает менеджер моделей
*
* @return указатель на менеджер моделей
************************************************************************/
CMeshManager* SlovoEdProject::GetMeshManager(void)
{
	return &m_MeshManager;
}

/***********************************************************************
* Возвращает менеджер материалов
*
* @return указатель на менеджер материалов
************************************************************************/
CMaterialManager* SlovoEdProject::GetMaterialManager(void)
{
	return &m_MaterialManager;
}

/***********************************************************************
* Возвращает менеджер абстракных объектов
*
* @return указатель на менеджер материалов
************************************************************************/
CAbstractItemManager* SlovoEdProject::GetAbstractItemManager(void)
{
	return &m_AbstractItemManager;
}

/** ********************************************************************
* Устанавливает максимальный размер типа перевода
*
* @param aMaxWordtypeSize - максимальный размер типа перевода
*
* @return код ошибки
************************************************************************/
int SlovoEdProject::SetMaxWordtypeSize(UInt32 aMaxWordtypeSize)
{
	m_header.WordtypeSize = aMaxWordtypeSize;
	return ERROR_NO;
}

/** ********************************************************************
* Устанавливает максимальный размер строчки в переводе
*
* @param aMaxBufferSize - максимальный размер строчки в переводе
*
* @return код ошибки
************************************************************************/
int SlovoEdProject::SetMaxBufferSize(UInt32 aMaxBufferSize)
{
	m_header.ArticlesBufferSize = aMaxBufferSize;
	return ERROR_NO;
}

/***********************************************************************
* Устанавливает тип содержимого проекта
*
* @param aDatabaseType	- тип содержимого проекта
************************************************************************/
void SlovoEdProject::SetDatabaseType(ESlovoEdContainerDatabaseTypeEnum aDatabaseType)
{
	m_DatabaseType = aDatabaseType;
}

/***********************************************************************
* Получает тип содержимого проекта
*
* @return тип содержимого проекта
************************************************************************/
ESlovoEdContainerDatabaseTypeEnum SlovoEdProject::GetDatabaseType(void)
{
	return m_DatabaseType;
}

/***********************************************************************
* Проверяет, относится ли база к ЛЭУ
*
* @return true, если база ЛЭУ, false - Paragon
************************************************************************/
bool SlovoEdProject::IsETBDatabase()
{
	return (m_DatabaseType == eDatabaseType_TextBook || m_DatabaseType == eDatabaseType_Book);
}

/***********************************************************************
* Устанавливает тип источника озвучки
*
* @param aType	- тип источника озвучки
************************************************************************/
void SlovoEdProject::SetSoundSourceType(EMediaSourceTypeEnum aType)
{
	m_SoundSourceType = aType;
}
	
/***********************************************************************
* Получает тип источника озвучки
*
* @return тип источника озвучки
************************************************************************/
EMediaSourceTypeEnum SlovoEdProject::GetSoundSourceType(void)
{
	return m_SoundSourceType;
}

/***********************************************************************
* Устанавливает тип источника картинок
*
* @param aType	- тип источника картинок
************************************************************************/
void SlovoEdProject::SetImageSourceType(EMediaSourceTypeEnum aType)
{
	m_ImageSourceType = aType;
}
	
/***********************************************************************
* Получает тип источника картинок
*
* @return тип источника картинок
************************************************************************/
EMediaSourceTypeEnum SlovoEdProject::GetImageSourceType(void)
{
	return m_ImageSourceType;
}

/***********************************************************************
* Устанавливает тип источника видео
*
* @param aType	- тип источника видео
************************************************************************/
void SlovoEdProject::SetVideoSourceType(EMediaSourceTypeEnum aType)
{
	m_VideoSourceType = aType;
}

/***********************************************************************
* Получает тип источника видео
*
* @return тип источника видео
************************************************************************/
EMediaSourceTypeEnum SlovoEdProject::GetVideoSourceType(void)
{
	return m_VideoSourceType;
}

/***********************************************************************
* Устанавливает тип источника сцен
*
* @param aType	- тип источника сцен
************************************************************************/
void SlovoEdProject::SetSceneSourceType(EMediaSourceTypeEnum aType)
{
	m_SceneSourceType = aType;
}

/***********************************************************************
* Получает тип источника сцен
*
* @return тип источника сцен
************************************************************************/
EMediaSourceTypeEnum SlovoEdProject::GetSceneSourceType(void)
{
	return m_SceneSourceType;
}

/***********************************************************************
* Устанавливает тип источника материалов
*
* @param aType	- тип источника материалов
************************************************************************/
void SlovoEdProject::SetMaterialSourceType(EMediaSourceTypeEnum aType)
{
	m_MaterialSourceType = aType;
}

/***********************************************************************
* Получает тип источника материалов
*
* @return тип источника материалов
************************************************************************/
EMediaSourceTypeEnum SlovoEdProject::GetMaterialSourceType(void)
{
	return m_MaterialSourceType;
}

/***********************************************************************
* Устанавливает тип источника моделей
*
* @param aType	- тип источника моделей
************************************************************************/
void SlovoEdProject::SetMeshSourceType(EMediaSourceTypeEnum aType)
{
	m_MeshSourceType = aType;
}

/***********************************************************************
* Получает тип источника моделей
*
* @return тип источника моделей
************************************************************************/
EMediaSourceTypeEnum SlovoEdProject::GetMeshSourceType(void)
{
	return m_MeshSourceType;
}

/***********************************************************************
* Устанавливает метод сжатия, одинаковый для статей и всех списков слов
*
* @param aMethod	- строка, описывающая метод сжатия
* 
* @return код ошибки
************************************************************************/
int SlovoEdProject::SetCompressionMethod(wstring aMethod)
{
	int error = ParseCompressionConfig(aMethod, m_compressionConfig);
	if (error != ERROR_NO)
		return error;

	// Устанавливаем метод сжатия для статей
	m_Articles.SetCompressionMethod(m_compressionConfig);
	m_header.ArticlesCompressionMethod = m_compressionConfig.type;

	// Устанавливаем метод сжатия для списков слов
	for (CWordList &list : lists())
		list.SetCompressionMethod(m_compressionConfig);

	return ERROR_NO;
}

/***********************************************************************
* Возвращает минимальную версию ядра, которая нужна для поддержки всех фич словаря
*
* @return код ошибки
************************************************************************/
UInt32 SlovoEdProject::GetDictionaryVersion()
{
	UInt32 version(COMPILER_VERSION__BASE);	

	return version;
}

/***********************************************************************
* Добавляем файл с базой морфологии
*
* @param aNode - ветвь XML-дерева с нужным разделом
*
* @return код ошибки
************************************************************************/
int SlovoEdProject::AddMorphoBase(pugi::xml_node aNode)
{
	UInt32 languageCode = 0;
	int err = ParseLanguageCodeAttrib(aNode, &languageCode);
	if (err != ERROR_NO)
		return err;

	if (languageCode != m_header.LanguageCodeFrom && languageCode != m_header.LanguageCodeTo)
	{
		sldILog("Warning! Wrong morphology base language attribute. Dictionary doesn't use this language: Tag=%s, Language=%s. Ignored.\n",
				sld::as_ref(aNode.name()), sld::as_ref(aNode.attribute(L"Language").as_string()));
		return ERROR_NO;
	}

	return m_MorphoDataManager.AddMorphoBase(languageCode, aNode.child_value());
}

/************************************************************************
* Возвращает указатель на объект для работы с базами морфологии
*
* @return указатель на объект для работы с базами морфологии
************************************************************************/
CMorphoDataManager* SlovoEdProject::GetMorphoDataManager(void)
{
	return &m_MorphoDataManager;
}

/************************************************************************
* Возвращает указатель на объект для работы с метаданными
*
* @return указатель на объект для работы с метаданными
************************************************************************/
MetadataManager * SlovoEdProject::GetMetadataManager()
{
	return &m_MetadataManager;
}

/************************************************************************
* Возвращает указатель на объект для работы с css метаданными
*
* @return указатель на объект для работы с css метаданными
************************************************************************/
CSSDataManager * SlovoEdProject::GetCSSDataManager()
{
	return &m_CSSDataManager;
}

/***********************************************************************
* Проверяет полная это база или нет
*
* @return флаг того, полная это база или нет
************************************************************************/
UInt32 SlovoEdProject::GetInApp(void)
{
	return m_IsInApp;
}


/***********************************************************************
* Обработка листов, слинкованных с Atomic-ами
* Перемещает переданый лист в список листов
************************************************************************/
int SlovoEdProject::ProcessAtomicReferenceLists(CWordList &aAtomicList)
{
	if (!aAtomicList.GetNumberOfElements())
		return ERROR_NO;

	aAtomicList.SetLanguageCodeFrom(m_header.LanguageCodeFrom);
	aAtomicList.SetLanguageCodeTo(m_header.LanguageCodeTo);
	aAtomicList.SetType(eWordListType_Atomic);

	TFullTextSearchOptions ftsOptions;
	ftsOptions.IsOptionsPresent = 1;
	ftsOptions.LinkType = eLinkType_ListEntryId;
	aAtomicList.SetFullTextSearchOptions(ftsOptions);

	aAtomicList.SetSorted(true);

	map<wstring, set<wstring>> Refs_Atomic_map;
	for (const ListElement &listElement : aAtomicList.elements())
	{
		for (const ArticleRefStruct &ref : listElement.ArticleRefs)
			Refs_Atomic_map[ref.ID].insert(listElement.Name[0]);
	}

	for (CWordList &list : lists())
	{
		if (!(list.GetType() == eWordListType_KES || list.GetType() == eWordListType_FC))
			continue;

		for (ListElement &listElement : list.elements())
		{
			const auto &fit = Refs_Atomic_map.find(listElement.Id);
			if (fit == Refs_Atomic_map.end())
				continue;

			for (const std::wstring &id : fit->second)
			{
				ArticleRefStruct tmpRef;
				tmpRef.ID = ATOMIC_ID_PREFIX + id;
				listElement.ArticleRefs.push_back(tmpRef);
			}
		}
	}

	return AddWordList(std::move(aAtomicList));
}

// Постобработка и проверка стилей
int SlovoEdProject::PostProcessStyles()
{
	UInt32 NotFoundCount = 0;

	// Стили, которые объявлены в файле проекта sproj, но не используются ни в одной статье
	for (auto&& style : m_Styles.unusedStyles())
	{
		sldILog("Warning! Unused style from project file: <%s>.\n", sld::as_ref(style));
		NotFoundCount++;
	}

	if (NotFoundCount)
		sld::printf(eLogStatus_Warning, "\nWarning! Dictionary has defined styles that are never used! See log file for details.");

	// Стили, которые используются в статьях, но не объявлены в файле проекта sproj
	NotFoundCount = 0;
	for (auto&& style : m_Styles.unknownStyles())
	{
		sldILog("Warning! Not found style in project file for block: <%s>. Used default style.\n", sld::as_ref(style));
		NotFoundCount++;
	}

	if (NotFoundCount)
		sld::printf(eLogStatus_Warning, "\nWarning! Not found defined style(s) for some block(s). See log file for details.");

	return ERROR_NO;
}

// Функции подготовки информации о страницах
namespace
{

// a single page numeration "record"
struct PageNumIndex {
	uint32_t pageNum;
	wstring labelId;
	wstring listEntryId;
};

// a bunch of "books" with numeration
struct MultiBook
{
	typedef std::vector<PageNumIndex> Book;

	std::list<Book> books;
	long lastPage;

	MultiBook() : lastPage(-1) {}

	std::list<Book>::iterator begin() { return books.begin(); }
	std::list<Book>::iterator end() { return books.end(); }
	size_t count() { return books.size(); }

	void pushPages(const std::vector<CArticle::PageIndex> &pages, const std::wstring &articleId)
	{
		if (pages.empty())
			return;

		// -1 will be converted to some "really large number"...
		if (pages.front().pageNum < (uint32_t)lastPage)
		{
			books.push_back(Book());
			lastPage = -1;
		}

		for (auto page = pages.cbegin(); page != pages.cend(); ++page)
		{
			const PageNumIndex index = { page->pageNum, page->labelId, articleId };
			books.back().push_back(index);
			lastPage = std::max(lastPage, (long)page->pageNum);
		}
	}
};

// inserts a single page numeration "book" into the given list
static void pageNumerationToList(const MultiBook::Book &book, CListLoader &aList)
{	
	// Due to the way list loading and whatnot is setup it is easier to
	// create an xml_document in memory and feed it to the list loader... oh well...
	// As a bonus we can dump the generated xml (for debug/other purposes)
	pugi::xml_document listDoc;
	pugi::xml_node listRoot = listDoc.append_child(L"List");
	size_t index = 0;
	for (auto it = book.cbegin(); it != book.cend(); ++it, ++index)
	{
		// the numeration list is "num - label - id"
		pugi::xml_node article = listRoot.append_child(L"article");
		article.append_attribute(L"id") = (aList->GetListId() + L"_" + std::to_wstring((long long)index)).c_str();
		article.append_child(L"wl-name").text() = it->pageNum;
		article.append_child(L"wl-name").text() = it->labelId.c_str();
		article.append_child(L"id").text() = it->listEntryId.c_str();
	}

	aList.LoadListElement(listRoot, 0);
}

// builds page numeration for a given list
static int buildPageNumeration(const CWordList& aList, const CArticles* aArticles, MultiBook &books)
{
	for (const ListElement &listElement: aList.elements())
	{
		// skip list entries without any article refs
		if (listElement.ArticleRefs.empty())
			continue;

		uint32_t articleIndex;
		int error = aArticles->GetPositionByID(listElement.ArticleRefs.front().ID, articleIndex);
		if (error == ERROR_NO_ARTICLE)
			continue;
		if (error != ERROR_NO)
			return error;

		const auto &pages = aArticles->GetArticleByIndex(articleIndex)->GetPages();
		books.pushPages(pages, listElement.Id);
	}
	return ERROR_NO;
}

}

/***********************************************************************
* Подготовка информации о страницах
*
************************************************************************/
int SlovoEdProject::DoPrepagePages()
{
	int error = ERROR_NO;

	std::vector<CListLoader> numerationLists;
	for (const auto list : enumerate(lists()))
	{
		if (list->IsFullTextSearchList() || list->GetType() == eWordListType_SimpleSearch || list->GetLinkedList())
			continue;

		MultiBook books;
		error = buildPageNumeration(*list, GetArticles(), books);
		if (error != ERROR_NO)
			return error;

		TFullTextSearchOptions ftsOptions;
		ftsOptions.IsOptionsPresent = 1;
		ftsOptions.LinkType = eLinkType_ListEntryId;
		ftsOptions.Priority = static_cast<UInt32>(0x800 + (list.index << 6)); // this should be good for 64 "books" in a list
		ftsOptions.mAllowedSourceListId.insert(list->GetListId());

		uint32_t index = 0;
		for (auto book = books.begin(); book != books.end(); ++book, ++index)
		{
			if (book->empty())
				continue;

			sld::printf("\nLoading page numeration index list for parent list: %s [%d/%d]\n",
						sld::as_ref(list->GetListId()), index, books.count());

			numerationLists.emplace_back();
			CListLoader &numerationList = numerationLists.back();

			numerationList->SetType(eWordListType_PageNumerationIndex);
			numerationList->SetListId(list->GetListId() + L"_pnum_" + std::to_wstring((long long)index));
			numerationList->SetLanguageCodeFrom(list->GetLanguageCodeFrom());
			numerationList->SetLanguageCodeTo(list->GetLanguageCodeTo());

			ftsOptions.Priority += index;
			numerationList->SetFullTextSearchOptions(ftsOptions);

			static const EListVariantTypeEnum props[] = { eVariantShow, eVariantLabel };
			for (uint32_t i = 0; i < sld::array_size(props); i++)
				numerationList->AddVariantProperty(i, { props[i], 0 });

			pageNumerationToList(*book, numerationList);
		}
	}

	for (CListLoader &list : numerationLists)
		AddWordList(std::move(list.list()));

	return ERROR_NO;
}
