#include "ArticlesCompress.h"

#include "Articles.h"
#include "Log.h"
#include "StringCompare.h"
#include "Styles.h"
#include "Tools.h"

// The maximum number of quck access entries that possible
enum : size_t {
	MaxQAArrayCount = (SLD_MAX_ARTICLE_QA_TABLE_RESOURCE_SIZE - sizeof(TQAHeader)) / sizeof(TQAArticlesEntry)
};

/// Зашифровывает смещение для таблицы быстрого доступа
#define SLD_QA_CRYPT(aShiftBit, aHASH)	(   (((aShiftBit & 022222222222)>>1) ^ (aHASH & 011111111111)) |	\
											(((aShiftBit & 011111111111)<<1) ^ (aHASH & 022222222222)) |	\
											   (aShiftBit & 04444444444)  )

/**
 * Производит шифрование таблицы бцстрого доступа
 *
 * @param aQA   - таблица быстрого доступа
 * @param aHASH - число использующееся для защиты словаря
 *
 * @return код ошибки
 */
static int CryptQA(std::vector<TQAArticlesEntry> &aQA, UInt32 aHASH)
{
	UInt32 MaxShiftBit = 0xFFFFFFFF;
	UInt32 Size = (UInt32)aQA.size();
	for (UInt32 i = 0; i<Size; i++)
	{
		if (aQA[i].ShiftBit >= MaxShiftBit)
		{
			STString<0x1FF> logbuf(L"Error! ShiftBit overflow by %u for Article QA Table in record #%u of %u", aQA[i].ShiftBit - MaxShiftBit, i, Size);
			sld::printf(eLogStatus_Error, "\n%s", logbuf);
			sldILog("%s\n", logbuf);

			logbuf.clear().appendf(L"Error! ArticleQATable[%u].ShiftBit overflow by %u", Size - 1, aQA[Size - 1].ShiftBit - MaxShiftBit);
			sld::printf(eLogStatus_Error, "\n%s", logbuf);
			sldILog("%s\n", logbuf);

			return ERROR_TOO_LARGE_ARTICLES_DATA;
		}
		aQA[i].ShiftBit = SLD_QA_CRYPT(aQA[i].ShiftBit, aHASH);
	}
	return ERROR_NO;
}

/** ********************************************************************
*  Производим сжатие статей
*
* @param aArticles   - указатель на класс хранящий подготовленные к сжатию статьи
* @param aStyles     - указатель на класс управляющий хранением стилей
* @param aHASH       - число использующееся для защиты словаря
* @param aThreadPool - thread pool используемый для сжатия
*
* @return код ошибки
************************************************************************/
int CArticlesCompress::DoCompress(const CArticles *aArticles, const TSlovoedStyleManager *aStyles, UInt32 aHASH, ThreadPool &aThreadPool)
{
	assert(aArticles);
	assert(aStyles);

	m_Compress = CreateCompressClass(aArticles->GetCompressionMethod(), aThreadPool);
	if (!m_Compress)
	{
		sldILog("Error! CArticlesCompress::DoCompress : Can't create compress class!");
		return ERROR_WRONG_COMPRESSION_METHOD;
	}
	m_Compress->Init(m_Out);

	std::vector<uint16_t> blockType;
	std::u16string text16;
	auto compress = [&](const CArticle &article) {
		// Вначале мы создадим строчку с типом перевода
		// Потом сожмем ее с деревом 0.
		// Далее по очереди сожмем все блоки строк с соответствующими деревьями
		blockType.resize(article.GetBlockCount() + 1);
		for (auto&& block : enumerate(article.blocks()))
			blockType[block.index] = block->styleNumber;
		blockType[article.GetBlockCount()] = '\0';

		int error = m_Compress->AddText(blockType.data(), static_cast<uint32_t>(blockType.size()), SLD_TREE_BLOCKTYPE);
		if (error != ERROR_NO)
			return error;

		size_t maxTextSize = 0;
		for (const BlockType &block : article.blocks())
		{
			sld::as_utf16(block.text, text16);
			maxTextSize = (std::max)(maxTextSize, text16.size());

			error = m_Compress->AddText((const UInt16*)text16.c_str(), (UInt32)text16.length() + 1, block.styleNumber);
			if (error != ERROR_NO)
				return error;
		}
		m_maxTextSize = (std::max)(m_maxTextSize, static_cast<UInt32>(maxTextSize));
		return ERROR_NO;
	};

	//////////////////////////////////////////////////////////////////////////
	// Инициализируем сжатие.
	//////////////////////////////////////////////////////////////////////////

	int error = m_Compress->SetMode(ICompressBy::eInitialization);
	if (error != ERROR_NO)
		return error;

	for (const CArticle &article : aArticles->articles())
	{
		error = compress(article);
		if (error != ERROR_NO)
			return error;
	}

	// Компрессор проинициализирован, далее идет основной алгоритм сжатия статей

	const UInt32 stylesCount = aStyles->StylesCount() + 1;
	if (stylesCount > m_Compress->GetCompressedTreeCount())
	{
		// Добавляем пустые деревья, чтобы их количество совпадало с количеством стилей
		error = m_Compress->AddEmptyTrees(stylesCount);
		if (error != ERROR_NO)
			return error;
	}

	error = m_Compress->SetMode(ICompressBy::eCompress);
	if (error != ERROR_NO)
		return error;

	//////////////////////////////////////////////////////////////////////////
	// Основной цикл сжатия
	//////////////////////////////////////////////////////////////////////////
	const UInt32 QAinterleave = aArticles->GetQAInterleave();
	UInt32 prevCompressedSize = 0;	// Хотя бы одна запись быстрого доступа должна быть!
	vector<TQAArticlesEntry> QAArray;
	vector<TQAArticlesEntry> QAArrayTotal;

	for (auto article : enumerate(aArticles->articles()))
	{
		// Определяем размер буфера для длины типа перевода
		m_maxWordTypeSize = (std::max)(m_maxWordTypeSize, article->GetBlockCount());

		//////////////////////////////////////////////////////////////////////////
		// Добавляем запись в таблицу быстрого доступа
		//////////////////////////////////////////////////////////////////////////
		const UInt32 currCompressedSize = m_Compress->GetCurrentPos();

		TQAArticlesEntry qa_entry;
		memset(&qa_entry, 0, sizeof(qa_entry));
		qa_entry.Index = static_cast<uint32_t>(article.index);
		qa_entry.ShiftBit = currCompressedSize;
		QAArrayTotal.push_back(qa_entry);
		
		if (currCompressedSize - prevCompressedSize >= QAinterleave || currCompressedSize == 0)
		{
			QAArray.push_back(qa_entry);
			prevCompressedSize = currCompressedSize;
		}

		error = compress(*article);
		if (error != ERROR_NO)
			return error;
	}
	// терминирующий 0
	m_maxWordTypeSize++;
	// Длина префикса и постфикса и терминирующий 0
	m_maxTextSize += SLD_MAX_STYLE_PREFIX_SIZE + SLD_MAX_STYLE_POSTFIX_SIZE;
	m_maxTextSize++;

	if (QAArray.size() > MaxQAArrayCount)
	{
		QAArray.clear();

		const UInt32 QAArrayTotalCount = (UInt32)QAArrayTotal.size();
		const UInt32 NewQAinterleave = QAArrayTotal.back().ShiftBit / (MaxQAArrayCount - 1);

		UInt32 PrevBitSize = 0;
		for (UInt32 i = 0; (i < QAArrayTotalCount) && (QAArray.size() < MaxQAArrayCount); i++)
		{
			const UInt32 CurrBitSize = QAArrayTotal[i].ShiftBit;
			if (!CurrBitSize || (CurrBitSize - PrevBitSize >= NewQAinterleave))
			{
				QAArray.push_back(QAArrayTotal[i]);
				PrevBitSize = CurrBitSize;
			}
		}
	}
	
	error = CryptQA(QAArray, aHASH);
	if (error != ERROR_NO)
		return error;

	TQAHeader qaHeader = {};
	qaHeader.HeaderSize = sizeof(qaHeader);
	qaHeader.QAEntrySize = sizeof(TQAArticlesEntry);
	qaHeader.Version = 1;
	qaHeader.Type = eQATypeArticles;
	qaHeader.Count = (UInt32)QAArray.size();

	error = m_OutQA.AddBytes(&qaHeader, qaHeader.HeaderSize);
	if (error != ERROR_NO)
		return error;

	for (const TQAArticlesEntry& entry : QAArray)
	{
		error = m_OutQA.AddBytes(&entry, qaHeader.QAEntrySize);
		if (error != ERROR_NO)
			return error;
	}

	return FinalizeData(aStyles);
}

/** ********************************************************************
* Возвращает количество деревьев(стилей) использованных при сжатии. 
*
* @return количество деревьев/стилей
************************************************************************/
UInt32 CArticlesCompress::GetCompressedTreeCount() const
{
	if (!m_Compress)
		return 0;
	return m_Compress->GetCompressedTreeCount();
}

/** ********************************************************************
* Возвращает данные указанного дерева. 
*
* @param aIndex - номер дерева для которого нужно получить данные
* @param aDataPtr - по этому указателю будет записан указатель на данные дерева
* @param aDataSize - по этому указателю будет записан размер данных дерева
*
* @return код ошибки
************************************************************************/
int CArticlesCompress::GetCompressedTree(UInt32 aIndex, const UInt8 **aDataPtr, UInt32 *aDataSize) const
{
	return m_Compress->GetCompressedTree(aIndex, aDataPtr, aDataSize);
}

/// Возвращает сжатые данные
MemoryRef CArticlesCompress::GetCompressedData() const
{
	return m_Out.GetData();
}

/// Возвращает данные таблицы быстрого доступа.
MemoryRef CArticlesCompress::GetQA() const
{
	return m_OutQA.GetData();
}

// fills up memory related style fields, allocates the needed memory an copies the data in there
static MemoryBuffer initialize(TStyleHeader &aStyleHeader)
{
	aStyleHeader.structSize = sizeof(aStyleHeader);
	aStyleHeader.SizeOfStyleVariant = sizeof(TStyleVariantHeader);
	aStyleHeader.SizeOfStyleUsage = sizeof(UInt32);

	aStyleHeader.TotalStyleDataSize = aStyleHeader.structSize;
	aStyleHeader.TotalStyleDataSize += aStyleHeader.NumberOfUsages * aStyleHeader.SizeOfStyleUsage;
	aStyleHeader.TotalStyleDataSize += aStyleHeader.NumberOfVariants * aStyleHeader.SizeOfStyleVariant;

	MemoryBuffer buf(aStyleHeader.TotalStyleDataSize);
	memcpy(buf.data(), &aStyleHeader, aStyleHeader.structSize);
	return buf;
}

/** ********************************************************************
*  Производим заключительные действия после сжатия
*
* @return код ошибки
************************************************************************/
int CArticlesCompress::FinalizeData(const TSlovoedStyleManager *aStyles)
{
	const UInt32 stylesCount = aStyles->StylesCount() + 1;
	if (stylesCount < m_Compress->GetCompressedTreeCount())
	{
		sldILog("Error! CArticlesCompress::FinalizeData : Wrong count of styles!\n");
		return ERROR_WRONG_STYLE_COUNT;
	}

	// Собранные воедино бинарные данные стилей
	m_StylesBinaryData.resize(stylesCount);

	// Для каждого дерева сжатия устанавливаем данные стиля
	for (const TSlovoedStyle &style : aStyles->styles())
	{
		TStyleHeader styleHeader = {};

		styleHeader.Language = style.Language();
		styleHeader.DefaultVariantIndex = style.DefaultVariantIndex();

		const size_t maxTagSize = sizeof(styleHeader.Tag) / sizeof(styleHeader.Tag[0]) - 1;
		wstring TagName = style.Tag();
		if (TagName.size() > maxTagSize)
		{
			sldILog("Error! CArticlesCompress::FinalizeData : style name '%s' is very large\n",
					sld::as_ref(TagName));
			TagName.resize(maxTagSize);
		}
		sld::copy(styleHeader.Tag, TagName.c_str());

		styleHeader.NumberOfVariants = style.VariantsCount();
		if (styleHeader.NumberOfVariants == 0)
		{
			sldILog("Error! CArticlesCompress::FinalizeData : VariantsCount == 0\n");
			return ERROR_WRONG_STYLE_VARIANTS_COUNT;
		}

		styleHeader.NumberOfUsages = style.UsageCount();
		if (styleHeader.NumberOfUsages == 0)
		{
			sldILog("Error! CArticlesCompress::FinalizeData : UsageCount == 0\n");
			return ERROR_WRONG_STYLE_USAGES_COUNT;
		}

		m_StylesBinaryData[style.Number()] = initialize(styleHeader);
		UInt8 *ptr = m_StylesBinaryData[style.Number()].data() + styleHeader.structSize;

		// Варианты использования стилей
		if (styleHeader.NumberOfUsages)
		{
			std::vector<UInt32> usages(styleHeader.NumberOfUsages);
			for (UInt32 j : xrange(styleHeader.NumberOfUsages))
				usages[j] = style.GetUsage(j);

			const size_t size = styleHeader.SizeOfStyleUsage * styleHeader.NumberOfUsages;
			memcpy(ptr, usages.data(), size);
			ptr += size;
		}

		// Варианты стилей
		for (UInt32 j : xrange(styleHeader.NumberOfVariants))
		{
			const TSlovoedStyleVariant *pVariant = style.GetStyleVariant(j);
			if (!pVariant)
				return eMemoryNULLPointer;

			TStyleVariantHeader styleVariant = {};

			styleVariant.structSize = sizeof(styleVariant);
			styleVariant.VariantType = pVariant->VariantType();
			styleVariant.Visible = pVariant->Visible();
			styleVariant.TextType = style.GetTextType();
			styleVariant.Level = pVariant->FontLevel();

			styleVariant.ColorRed = pVariant->Color().R();
			styleVariant.ColorGreen = pVariant->Color().G();
			styleVariant.ColorBlue = pVariant->Color().B();
			styleVariant.ColorAlpha = pVariant->Color().Alpha();

			styleVariant.BackgroundColorRed = pVariant->BackgroundColor().R();
			styleVariant.BackgroundColorGreen = pVariant->BackgroundColor().G();
			styleVariant.BackgroundColorBlue = pVariant->BackgroundColor().B();
			styleVariant.BackgroundColorAlpha = pVariant->BackgroundColor().Alpha();

			styleVariant.BackgroundTopImageIndex = pVariant->BackgroundImageTop();
			styleVariant.BackgroundBottomImageIndex = pVariant->BackgroundImageBottom();
			styleVariant.Unclickable = pVariant->Unclickable();

			styleVariant.Bold = pVariant->GetBoldValue();
			styleVariant.Italic = pVariant->Italic();

			styleVariant.Underline = pVariant->Underline();
			styleVariant.UnderlineUseCustomColor = pVariant->UnderlineUseColor();
			styleVariant.UnderlineColorRed = pVariant->GetUnderlineColor().R();
			styleVariant.UnderlineColorGreen = pVariant->GetUnderlineColor().G();
			styleVariant.UnderlineColorBlue = pVariant->GetUnderlineColor().B();
			styleVariant.UnderlineColorAlpha = pVariant->GetUnderlineColor().Alpha();

			styleVariant.Strikethrough = pVariant->Strikethrough();
			styleVariant.Overline = pVariant->Overline();

			styleVariant.TextSize = pVariant->FontSize();
			styleVariant.TextSizeValue = pVariant->FontSizeValue();
			styleVariant.LineHeight = pVariant->LineHeight();
			styleVariant.LineHeightValue = pVariant->LineHeightValue();
			styleVariant.FontFamily = pVariant->FontFamily();
			styleVariant.FontName = pVariant->FontName();

			if (pVariant->Prefix().size() > SLD_MAX_STYLE_PREFIX_SIZE)
			{
				sldILog("Error! CArticlesCompress::FinalizeData : too large style prefix\n");
				return ERROR_TO_LARGE_STYLE_PREFIX;
			}
			sld::copy(styleVariant.Prefix, pVariant->Prefix().c_str());

			if (pVariant->Postfix().size() > SLD_MAX_STYLE_POSTFIX_SIZE)
			{
				sldILog("Error! CArticlesCompress::FinalizeData : too large style postfix\n");
				return ERROR_TO_LARGE_STYLE_POSTFIX;
			}
			sld::copy(styleVariant.Postfix, pVariant->Postfix().c_str());

			memcpy(ptr, &styleVariant, sizeof(styleVariant));
			ptr += sizeof(styleVariant);
		}
	}

	// needed because the Engine uses 0 as the invalid style index but a lot of stuff still
	// expects to be able to get a kinda valid style header at this index
	// this is safe because the style manager starts numbering styles from 1
	// and is completely monotonic
	{
		TStyleHeader styleHeader = {};
		m_StylesBinaryData[0] = initialize(styleHeader);
	}

	return ERROR_NO;
}

/** ********************************************************************
*  Возвращает максимальный размер типа перевода в символах
*
* @return максимальный размер типа перевода в символах
************************************************************************/
UInt32 CArticlesCompress::GetMaxWordtypeSize() const
{
	return m_maxWordTypeSize;
}

/** ********************************************************************
*  Возвращает максимальный размер буфера перевода в символах
*
* @return максимальный размер буфера перевода в символах
************************************************************************/
UInt32 CArticlesCompress::GetMaxBufferSize() const
{
	return m_maxTextSize;
}

/** ********************************************************************
* Возвращает данные с описанием стиля
*
* @param aIndex		- номер дерева для которого нужно получить данные
* @param aDataPtr	- по этому указателю будет записан указатель на данные дерева
* @param aDataSize	- по этому указателю будет записан размер данных дерева
*
* @return код ошибки
************************************************************************/
int CArticlesCompress::GetStyleData(UInt32 aIndex, const UInt8 **aDataPtr, UInt32 *aDataSize) const
{
	if (aIndex >= m_StylesBinaryData.size())
	{
		sldILog("Error! CArticlesCompress::GetStyleData : wrong style index!\n");
		return ERROR_WRONG_INDEX;
	}

	*aDataPtr = m_StylesBinaryData[aIndex].data();
	*aDataSize = static_cast<UInt32>(m_StylesBinaryData[aIndex].size());
	return ERROR_NO;
}

/** ********************************************************************
* Возвращает количество стилей
*
* @return количество стилей
************************************************************************/
UInt16 CArticlesCompress::GetStylesCount() const
{
	return static_cast<UInt32>(m_StylesBinaryData.size());
}
