#include "ListCompress.h"

#include <sstream>

#include "Log.h"
#include "sld_Platform.h"
#include "StringCompare.h"
#include "Tools.h"

using namespace std;

CListCompress::CListCompress(ICompressBy *aCompressor)
	: m_Compress(aCompressor)
{
	assert(m_Compress);
	m_IndexesQAInterleave = SLD_DEFAULT_QA_INTERLEAVE_SIZE;
	m_MaxWordSize = 0;
	memset(&m_IndexesHeader, 0, sizeof(m_IndexesHeader));
	m_IndexesHeader.structSize = sizeof(m_IndexesHeader);
	m_IndexesHeader.Version = SLD_INDEXES_VERSION;
	m_RealWordIndex = -1;
	m_Compress->Init(m_Out);
}

CListCompress::~CListCompress(void)
{
	if (m_Compress)
		delete m_Compress;
}

// Устанавливает флаги заголовка быстрого доступа по конкретному листу
static void UpdateQAIndexesHeader(TIndexesHeader& aHeader, const CWordList *aList)
{
	if (aList->IsFullTextSearchList() || aList->GetLinkedList())
	{
		aHeader.DataMask |= SLD_INDEXES_IS_ARTICLE_INDEX;
		aHeader.SizeOfIndexes = aList->GetFullTextSearchLinkListEntryIndexSize();
		aHeader.ArticleShiftType = aList->GetFullTextSearchShiftType();

		if (aList->GetIsNumberOfArticles())
		{
			aHeader.DataMask |= SLD_INDEXES_IS_COUNT;
			aHeader.SizeOfCount = aList->GetNumberOfArticlesSize();
		}

		// Номер списка слов
		const UInt32 ListIndexSize = aList->GetFullTextSearchLinkListIndexSize();
		if (ListIndexSize)
		{
			aHeader.DataMask |= SLD_INDEXES_IS_LIST_INDEX;
			aHeader.SizeOfListIndex = ListIndexSize;
		}
		else
		{
			aHeader.DefaultListIndex = aList->GetFullTextSearchDefaultLinkListIndex();
		}

		// Номер перевода
		const UInt32 TranslationIndexSize = aList->GetFullTextSearchLinkTranslationIndexSize();
		if (TranslationIndexSize)
		{
			aHeader.DataMask |= SLD_INDEXES_IS_TRANSLATION_INDEX;
			aHeader.SizeOfTranslationIndex = TranslationIndexSize;
		}
		else
		{
			aHeader.DefaultTranslationIndex = aList->GetFullTextSearchDefaultLinkTranslationIndex();
		}

		// Смещение в статье
		const UInt32 ShiftIndexSize = aList->GetFullTextSearchShiftIndexSize();
		if (ShiftIndexSize)
		{
			aHeader.DataMask |= SLD_INDEXES_IS_SHIFT_INDEX;
			aHeader.SizeOfShiftIndex = ShiftIndexSize;
		}
	}
	// Если список слов напрямую ассоциируется со статьями, тогда добавлять индексы нет нужды.
	else if (!aList->GetDirectList())
	{
		aHeader.DataMask |= SLD_INDEXES_IS_ARTICLE_INDEX;
		aHeader.SizeOfIndexes = aList->GetAticleIndexSize();

		if (aList->GetIsNumberOfArticles() && aList->GetNumberOfArticlesSize())
		{
			aHeader.DataMask |= SLD_INDEXES_IS_COUNT;
			aHeader.SizeOfCount = aList->GetNumberOfArticlesSize();
		}
	}
}

/**
	Производит сжатие списка слов.

	@param aList - список слов подлежащий сжатию.

	@return код ошибки.
*/
int CListCompress::DoCompress(CWordList *aList)
{
	int error;

	std::u16string elementName16;
	auto compress = [&](const ListElement &elem, UInt32 variantIndex) {
		const sld::wstring_ref elementName = elem.GetName(variantIndex);
		if (aList->GetType() == eWordListType_SimpleSearch && variantIndex == 1)
		{
			Int32	flag = 0;
			Int8	int8Flag = 0;
			UInt32	realWordIndex = 0;
			wchar_t	tmpChar = 0;

			UInt16	textBuff[4] = {};
			Int8*	ptr = (Int8*)textBuff;

			wstringstream ss(to_string(elementName));
			ss >> flag >> tmpChar >> realWordIndex;

			int8Flag = flag;
			memcpy(ptr, &int8Flag, sizeof(int8Flag));
			ptr++;
			memcpy(ptr, &realWordIndex, sizeof(realWordIndex));

			/// При чтении минимальный размер строчки - 3 символа(1 байт - флаг, 4 байта - индекс)
			if (m_MaxWordSize < 3)
				m_MaxWordSize = 3;

			return m_Compress->AddText(textBuff, CStringCompare::StrLen(textBuff) + 1, SLD_TREE_TEXT);
		}

		sld::as_utf16(elementName, elementName16);
		const UInt32 nameSize = (UInt32)elementName16.size() + 1;
		if (nameSize > m_MaxWordSize)
			m_MaxWordSize = nameSize;

		return m_Compress->AddText((const UInt16*)elementName16.c_str(), nameSize, SLD_TREE_TEXT);
	};

	error = m_Compress->SetMode(ICompressBy::eInitialization);
	if (error != ERROR_NO)
		return error;

	const UInt32 numberOfVariants = aList->GetNumberOfVariants();
	const UInt32 NumberOfElements = aList->GetNumberOfElements();

	for (const ListElement& elem : aList->elements())
	{
		// Сжимаем слово из списка слов(со всеми вариантами)
		for (UInt32 variantIndex : xrange(numberOfVariants))
		{
			error = compress(elem, variantIndex);
			if (error != ERROR_NO)
				return error;
		}
	}

	// Компрессор проинициализирован, можем начинать реальное сжатие

	error = m_Compress->SetMode(ICompressBy::eCompress);
	if (error != ERROR_NO)
		return error;

	// кэшируем флаг нескольких ссылок
	m_isNumberOfArticlesPerItem = aList->GetIsNumberOfArticles();
	// Обновляем заголовок "быстрого доступа"
	UpdateQAIndexesHeader(m_IndexesHeader, aList);

	UInt32 PrevQAIndexesPosition = m_IndexesQAInterleave; // Нужно для того, чтобы была хотя бы 1 запись.

	vector<TAlternativeHeadwordInfo> alternativeWordsInfo;
	vector<UInt32> altIndexes;
	std::vector<UInt32> index_realIndex(NumberOfElements, 0);

	for (auto&& elem : enumerate(aList->elements()))
	{
		if (elem->IndexesForAlternativeWords.empty())
		{
			index_realIndex[elem.index] = aList->GetRealWordCount();
			aList->SetRealWordCount(aList->GetRealWordCount() + 1);

			UInt32 currpos = m_Compress->GetCurrentPos();
			error = m_OutDirectWordsShifts.AddBytes(&currpos, sizeof(UInt32));
			if (error != ERROR_NO)
				return error;

			m_RealWordIndex++;
		}
		else
		{
			TAlternativeHeadwordInfo altInfo;
			altInfo.WordShift = m_Compress->GetCurrentPos();
			altInfo.RealWordCount = elem->IndexesForAlternativeWords.size();
			altInfo.RealWordIndex = altIndexes.size();
			alternativeWordsInfo.push_back(altInfo);

			for (UInt32 altIndex : elem->IndexesForAlternativeWords)
				altIndexes.push_back(altIndex);
		}

		// Сжимаем слово из списка слов(со всеми вариантами)
		for (UInt32 var : xrange(numberOfVariants))
		{
			error = compress(*elem, var);
			if (error != ERROR_NO)
				return error;

			if (elem->IndexesForAlternativeWords.size())
				break;

			// Если у нас есть кастомизированный вариант отбражения - добавим его
			error = AddStylizedVariant(aList, elem.index, var);
			if (error != ERROR_NO)
				return error;
		}

		if (elem->IndexesForAlternativeWords.size())
			continue;

		// Сохраняем при необходимости следующие данные:
		// - ссылки на переводы и их количество
		// - ссылку на картинку
		// - ссылку на звук

		// Если список слов не соответствует напрямую переводам, значит
		// мы должны хранить индексы(фиксированное количество или динамическое - не важно)
		if (aList->IsFullTextSearchList() || aList->GetLinkedList())
			error = AddFullTextSearchIndexes(aList, *elem, &PrevQAIndexesPosition);
		else
			error = AddIndexes(aList, *elem, &PrevQAIndexesPosition);
		if (error != ERROR_NO)
			return error;

		error = AddMediaIndexes(aList, *elem);
		if (error != ERROR_NO)
			return error;
	}

	if (alternativeWordsInfo.size())
	{
		if (alternativeWordsInfo.back().RealWordIndex + alternativeWordsInfo.back().RealWordCount != altIndexes.size())
			return ERROR_WRONG_ALTERNATIVE_HEADWORDS;

		UInt32 alternativeWordsCount = alternativeWordsInfo.size();
		m_OutALternativeHeadwords.AddBytes(&alternativeWordsCount, sizeof(UInt32));

		for (const TAlternativeHeadwordInfo& altInfo : alternativeWordsInfo)
			m_OutALternativeHeadwords.AddBytes(&altInfo, sizeof(altInfo));

		for (UInt32 altIndex : altIndexes)
			m_OutALternativeHeadwords.AddBytes(&index_realIndex[altIndex], sizeof(UInt32));
	}

	// Если в списке достаточно много элементов, тогда добавляем вспомогательное поисковое дерево
	if (NumberOfElements > SLD_MIN_WORDS_COUNT_FOR_QUICK_SEARCH)
	{
		error = AddSearchLeaf(aList, 0, aList->GetNumberOfElements(), 0);
		if (error != ERROR_NO)
			return error;
	}

	if (m_SearchTreePoints.size())
	{
		UInt32 searchTreePointsSize = m_SearchTreePoints.size();

		error = m_OutSearchTreePoints.AddBytes(&searchTreePointsSize, sizeof(UInt32));
		if (error != ERROR_NO)
			return error;

		std::u16string currentWord16;
		for (const TBinaryTreeElement &point : m_SearchTreePoints)
		{
			error = m_OutSearchTreePoints.AddBytes(&point.WordIndex, sizeof(UInt32));
			if (error != ERROR_NO)
				return error;

			error = m_OutSearchTreePoints.AddBytes(&point.LeftLeafIndex, sizeof(UInt16));
			if (error != ERROR_NO)
				return error;

			error = m_OutSearchTreePoints.AddBytes(&point.RightLeafIndex, sizeof(UInt16));
			if (error != ERROR_NO)
				return error;

			sld::as_utf16(point.CurrentWord, currentWord16);
			error = m_OutSearchTreePoints.AddBytes(currentWord16.c_str(), (currentWord16.size() + 1) * sizeof(UInt16));
			if (error != ERROR_NO)
				return error;
		}
	}

	for (const TQAIndexesEntry &entry : m_IndexesTable)
	{
		error = m_OutIndexesQA.AddBytes(&entry, sizeof(entry));
		if (error != ERROR_NO)
			return error;
	}

	const auto hierarchyData = aList->GetHierarchyData();
	if (hierarchyData.size())
	{
		error = m_OutHierarchy.AddBytes(hierarchyData.data(), hierarchyData.size());
		if (error != ERROR_NO)
			return error;
	}

	return ERROR_NO;
}

/**
 * Создает еще один элемент таблицы быстрого доступа для индексов если нужно
 *
 * @param[in|out] aPrevQAPosition - предыдущая позиция в таблице быстрого доступа для индексов
 */
void CListCompress::AddQAIndexesEntry(UInt32 *aPrevQAPosition)
{
	const UInt32 compressedPosition = m_OutIndexesCount.GetCurrentPos();
	if (compressedPosition - *aPrevQAPosition < m_IndexesQAInterleave)
		return;

	TQAIndexesEntry entry = {};
	entry.Index = m_RealWordIndex; // Номер сохраняется без "-1"
	entry.CountShiftBit = compressedPosition;
	entry.DataShiftBit = m_OutIndexesData.GetCurrentPos();

	m_IndexesTable.push_back(entry);
	*aPrevQAPosition = compressedPosition;
	m_IndexesHeader.QACount++;
}

/**
	Добавляем индексы(фиксированное количество или динамическое не важно)
	переводов.

	@param[in] aList - указатель на список слов.
	@param[in] aListElem - слово из списка слов
	@param[in|out] aPrevQAPosition - предыдущая позиция в таблице быстрого доступа для индексов.

	@return код ошибки.
*/
int CListCompress::AddIndexes(const CWordList *aList, const ListElement &aListElem, UInt32 *aPrevQAPosition)
{
	int error;

	// Если список слов напрямую ассоциируется со статьями, тогда добавлять индексы нет нужды.
	if (aList->GetDirectList())
		return ERROR_NO;

	// Создаем еще один элемент таблицы быстрого доступа для индексов если нужно
	AddQAIndexesEntry(aPrevQAPosition);

	// Если на слово из списка слов может приходится несколько статей, 
	// тогда сохраняем это количество.
	if (m_isNumberOfArticlesPerItem)
	{
		error = m_OutIndexesCount.AddBits(aListElem.ArticleRefs.size(), m_IndexesHeader.SizeOfCount);
		if (error != ERROR_NO)
			return error;
	}

	for (auto&& ref : aListElem.ArticleRefs)
	{
		error = m_OutIndexesData.AddBits(ref.index, m_IndexesHeader.SizeOfIndexes);
		if (error != ERROR_NO)
			return error;
	}

	m_IndexesHeader.Count++;

	return ERROR_NO;
}

/**
	Добавляем индексы (фиксированное количество или динамическое не важно) переводов,
	если список является списком слов полнотекстового поиска

	@param[in] aList				- указатель на список слов.
	@param[in] aListElem			- слово из списка слов
	@param[in|out] aPrevQAPosition	- предыдущая позиция в таблице быстрого доступа для индексов.

	@return код ошибки.
*/
int CListCompress::AddFullTextSearchIndexes(const CWordList *aList, const ListElement &aListElem, UInt32 *aPrevQAPosition)
{
	int error;

	UInt32 TranslationCount = static_cast<UInt32>(aListElem.ListEntryRefs.size());

	// Создаем еще один элемент таблицы быстрого доступа для индексов если нужно
	AddQAIndexesEntry(aPrevQAPosition);

	// Если на слово из списка слов может приходится несколько статей, 
	// тогда сохраняем это количество.
	if (m_isNumberOfArticlesPerItem)
	{
		error = m_OutIndexesCount.AddBits(TranslationCount, m_IndexesHeader.SizeOfCount);
		if (error != ERROR_NO)
			return error;
	}
	else if (TranslationCount > 1)
	{
		sldILog("Error! AddFullTextSearchIndexes::TranslationCount=%u; should be <= 1\n", TranslationCount);
		return ERROR_WRONG_LIST_TYPE;
	}

	const UInt32 ListIndexSize = aList->GetFullTextSearchLinkListIndexSize();
	const UInt32 ListEntryIndexSize = aList->GetFullTextSearchLinkListEntryIndexSize();
	const UInt32 TranslationIndexSize = aList->GetFullTextSearchLinkTranslationIndexSize();
	const UInt32 ShiftIndexSize = aList->GetFullTextSearchShiftIndexSize();

	for (auto&& ref : aListElem.ListEntryRefs)
	{
		// Номер списка слов
		if (ListIndexSize)
		{
			error = m_OutIndexesData.AddBits(ref.list_idx, ListIndexSize);
			if (error != ERROR_NO)
				return error;
		}

		// Номер слова
		error = m_OutIndexesData.AddBits(ref.entry_idx, ListEntryIndexSize);
		if (error != ERROR_NO)
			return error;

		// Номер перевода
		if (TranslationIndexSize)
		{
			error = m_OutIndexesData.AddBits(ref.translation_idx, TranslationIndexSize);
			if (error != ERROR_NO)
				return error;
		}

		// Смещение в статье
		if (ShiftIndexSize)
		{
			error = m_OutIndexesData.AddBits(ref.shift, ShiftIndexSize);
			if (error != ERROR_NO)
				return error;
		}
	}

	m_IndexesHeader.Count++;

	return ERROR_NO;
}

// generic resource addition function
static int addResource(ICompressBy *aCompressor, UInt32 aResourceIndex,
					   UInt32 aResourceIndexSize, UInt32 aInvalidResourceIndex)
{
	// Сохраняем признак того, есть ресурс или нет
	const UInt32 oneBit = aResourceIndex == aInvalidResourceIndex ? 0 : 1;
	int error = aCompressor->AddData(oneBit, 1);
	if (error != ERROR_NO)
		return error;

	// Если ресурс есть, тогда сохраняем его индекс
	if (aResourceIndex == aInvalidResourceIndex)
		return ERROR_NO;

	return aCompressor->AddData(aResourceIndex, aResourceIndexSize);
}

/**
	Добавляем медиа индексы (картинка, звук, видео, сцена) соответствующие данному элементу

	@param[in] aList - указатель на список слов.
	@param[in] aListElem - слово из списка слов

	@return код ошибки.
*/
int CListCompress::AddMediaIndexes(const CWordList* aList, const ListElement &aListElem)
{
	int error;

	if (aList->GetPicture())
	{
		error = addResource(m_Compress, aListElem.PictureIndex, aList->GetPictureIndexSize(), SLD_INDEX_PICTURE_NO);
		if (error != ERROR_NO)
			return error;
	}

	if (aList->GetSound())
	{
		error = addResource(m_Compress, aListElem.SoundIndex, aList->GetSoundIndexSize(), SLD_INDEX_SOUND_NO);
		if (error != ERROR_NO)
			return error;
	}

	if (aList->GetVideo())
	{
		error = addResource(m_Compress, aListElem.VideoIndex, aList->GetVideoIndexSize(), SLD_INDEX_VIDEO_NO);
		if (error != ERROR_NO)
			return error;
	}

	if (aList->GetScene())
	{
		error = addResource(m_Compress, aListElem.SceneIndex, aList->GetSceneIndexSize(), SLD_INDEX_SCENE_NO);
		if (error != ERROR_NO)
			return error;
	}

	return ERROR_NO;
}

/// Возвращаем сжатые данные.
MemoryRef CListCompress::GetCompressedData() const
{
	return m_Out.GetData();
}

/// Возвращаем иерархию.
MemoryRef CListCompress::GetCompressedHierarchy() const
{
	return m_OutHierarchy.GetData();
}

/// Возвращаем ссылки на статьи
MemoryRef CListCompress::GetIndexesData() const
{
	return m_OutIndexesData.GetData();
}

/// Возвращаем ссылки на статьи
MemoryRef CListCompress::GetIndexesCount() const
{
	return m_OutIndexesCount.GetData();
}

/// Возвращаем таблицу быстрого доступа ссылок на статьи
MemoryRef CListCompress::GetIndexesQA() const
{
	return m_OutIndexesQA.GetData();
}

/// Возвращаем таблицу быстрого доступа ссылок на статьи
MemoryRef CListCompress::GetDirectWordsShifts() const
{
	return m_OutDirectWordsShifts.GetData();
}

/// Возвращаем массив точек для бинарного поиска
MemoryRef CListCompress::GetSearchTreePoints() const
{
	return m_OutSearchTreePoints.GetData();
}

/// Возвращаем информацию об альтернативных заголовках
MemoryRef CListCompress::GetAlternativeHeadwordInfo() const
{
	return m_OutALternativeHeadwords.GetData();
}

/// Возвращаем количество деревьев.
UInt32 CListCompress::GetCompressedTreeCount() const
{
	return m_Compress->GetCompressedTreeCount();
}

/// Возвращаем дерево.
int CListCompress::GetCompressedTree(UInt32 aTreeIndex, const UInt8 **aPtr, UInt32 *aSize) const
{
	return m_Compress->GetCompressedTree(aTreeIndex, aPtr, aSize);
}

/// Возвращает максимальный размер слова в символах.
UInt32 CListCompress::GetMaxWordSize() const
{
	return m_MaxWordSize;
}

/** ********************************************************************
* Возвращает указатель на заголовок хранилища индексов 
*
* @return указатель на заголовок хранилища индексов
************************************************************************/
const TIndexesHeader* CListCompress::GetIndexesHeader() const
{
	return &m_IndexesHeader;
}

/** ********************************************************************
* Добавляет элемент дерева быстрого поиска по тексту
*
* @param	aList		- указатель на список слов
* @param	aMinIndex	- нижняя граница поиска
* @param	aMaxIndex	- верхняя граница поиска
* @param	aDepth		- текущая глубина дерева
*
* @return код ошибки
************************************************************************/
int CListCompress::AddSearchLeaf(const CWordList *aList, Int32 aMinIndex, Int32 aMaxIndex, UInt32 aDepth)
{
	int error;

	Int32 wordIndex = (aMinIndex + aMaxIndex) >> 1;
	const ListElement *elem = aList->GetListElement(wordIndex);
	const sld::wstring_ref elementName = elem->GetName(0);

	TBinaryTreeElement elementLeaf;
	size_t elementIndex = m_SearchTreePoints.size();

	m_SearchTreePoints.push_back(elementLeaf);

	aDepth++;

	m_SearchTreePoints[elementIndex].WordIndex = wordIndex;
	m_SearchTreePoints[elementIndex].CurrentWord = to_string(elementName);

	if (aDepth < SLD_MAX_SEARCH_TREE_DEPTH)
	{
		m_SearchTreePoints[elementIndex].LeftLeafIndex = m_SearchTreePoints.size();
		error = AddSearchLeaf(aList, aMinIndex, wordIndex, aDepth);
		if (error != eOK)
			return error;

		m_SearchTreePoints[elementIndex].RightLeafIndex = m_SearchTreePoints.size();
		error = AddSearchLeaf(aList, wordIndex, aMaxIndex, aDepth);
		if (error != eOK)
			return error;
	}

	return ERROR_NO;
}

/** ********************************************************************
* Добавляет кастомизированный вариант отбражения
*
* @param[in] aList				- указатель на список слов.
* @param[in] elementIndex		- номер слова из списка слов
* @param[in] aVariantIndex		- номер варианта
*
* @return код ошибки
************************************************************************/
int CListCompress::AddStylizedVariant(const CWordList *aList, UInt32 aElementIndex, UInt32 aVariantIndex)
{
	if (!aList->HasStylizedVariants())
		return ERROR_NO;

	UInt32 articleIndex = aList->GetStylizedVariantArticleIndex(aElementIndex, aVariantIndex);
	return addResource(m_Compress, articleIndex, aList->GetAticleIndexSize(), STYLIZED_VARIANT_NO);
}

/**
 * Производит сжатие списка слов.
 *
 * @param aList       - список слов подлежащий сжатию
 * @param aThreadPool - thread pool используемый для сжатия
 *
 * @return код ошибки
 */
std::pair<int, CListCompress*> CListCompress::Compress(CWordList &aList, ThreadPool &aThreadPool)
{
	ICompressBy *compressor = CreateCompressClass(aList.GetCompressionMethod(), aThreadPool);
	if (!compressor)
	{
		sldILog("Error! CListCompress::SetCompressionMethod : Can't create compress class!\n");
		return{ ERROR_WRONG_COMPRESSION_METHOD, nullptr };
	}

	CListCompress *list = new CListCompress(compressor);
	int error = list->DoCompress(&aList);
	if (error != ERROR_NO)
	{
		delete list;
		list = nullptr;
	}
	return{ error, list };
}
