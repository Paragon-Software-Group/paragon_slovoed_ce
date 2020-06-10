#include "SldDictionaryHelper.h"

#include "SldPhraseUtility.h"

namespace
{
	ESldError GetReferencesForList(
			CSldDictionary& aDictionary, Int32 aListIndex, 
			const SldU16String& aRawWord, CSldVector<Int32>& aTmpIndexes, 
			TWordRefInfo& aWordRef, const CSldVector<MorphoDataType*>& aMorphos, 
			const CSldCompare* aSldCompare, UInt8 aUseBuiltInMorpho = 0)
	{
		ESldError error = aDictionary.GetWordSetByTextExtended(aListIndex, aRawWord.c_str(), aTmpIndexes);
		if (error != eOK)
			return error;

		for (const auto wordIndex : aTmpIndexes)
		{
			aWordRef.Refs.emplace_back(TSldWordIndexes(aListIndex, wordIndex), TWordRefInfo::type::exact);
		}

		const auto language = aDictionary.GetListLanguageFrom(aListIndex);

		CSldVector<TSldMorphologyWordStruct> tmpMorphologyForms;

		auto addForms = [&]() -> void
		{
			for (const auto& form : tmpMorphologyForms)
			{
				if (aSldCompare->StrICmp(aRawWord.c_str(), form.MorphologyForm) == 0)
					continue;

				const auto morphoType = form.MorphologyFormType == eMorphologyWordTypeBase ? TWordRefInfo::type::morphoBase : TWordRefInfo::type::morphoForm;

				aDictionary.GetWordSetByTextExtended(aListIndex, form.MorphologyForm.c_str(), aTmpIndexes);

				for (const auto wordIndex : aTmpIndexes)
					aWordRef.Refs.emplace_back(TSldWordIndexes(aListIndex, wordIndex), morphoType);
			}
		};

		for (auto morpho : aMorphos)
		{
			if (language != morpho->GetLanguageCode())
				continue;

			error = GetAllMorphoForms(aRawWord.c_str(), morpho, tmpMorphologyForms);
			if (error != eOK)
				return error;

			addForms();
		}

		if (aUseBuiltInMorpho)
		{
			error = GetAllMorphoForms(aRawWord.c_str(), &aDictionary, language, tmpMorphologyForms);
			if (error != eOK)
				return error;

			addForms();
		}

		return eOK;
	}
}

ESldError CSldDictionaryHelper::GetExternalBaseName(Int32 aListIndex, SldU16WordsArray& aBaseNames)
{
	Int32 currentListIndex = 0;
	ESldError error = m_Dictionary.GetCurrentWordList(&currentListIndex);
	if (error != eOK)
		return error;

	error = m_Dictionary.SetCurrentWordlist(aListIndex);
	if (error != eOK)
		return error;

	Int32 wordsCount = 0;
	error = m_Dictionary.GetNumberOfWords(&wordsCount);
	if (error != eOK)
		return error;

	for (Int32 wordIdx = 0; wordIdx < wordsCount; wordIdx++)
	{
		error = m_Dictionary.GetWordByIndex(wordIdx);
		if (error != eOK)
			return error;

		UInt16* baseName = NULL;
		error = m_Dictionary.GetCurrentWord(0, &baseName);
		if (error != eOK)
			return error;

		aBaseNames.push_back(baseName);
	}

	return m_Dictionary.SetCurrentWordlist(currentListIndex);
}


/** *********************************************************************
* Получает информацию о внешних ресурсах по локальному индексу в текущем списке слов
*
* @param[in]	aLocalIndex		- индекс слова, для которого мы хотим получить информацию
* @param[out]	aRefs			- структура с информацией о внешних ресурсах
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionaryHelper::GetExternReference(Int32 aLocalIndex, TExternReference* aRefs)
{
	if (!aRefs)
		return eMemoryNullPointer;

	Int32 currentListIndex = 0;
	ESldError error = m_Dictionary.GetCurrentWordList(&currentListIndex);
	if (error != eOK)
		return error;

	const CSldListInfo* listInfo = NULL;
	Int32 listCount = 0;
	error = m_Dictionary.GetNumberOfLists(&listCount);
	if (error != eOK)
		return error;

	CSldVector<Int32> lists;
	Int32 priorityListCounter = 0;
	for (Int32 listIdx = 0; listIdx < listCount; listIdx++)
	{
		error = m_Dictionary.GetWordListInfo(listIdx, &listInfo);
		if (error != eOK)
			return error;

		EWordListTypeEnum listType = listInfo->GetUsage();
		if (listType >= eWordListType_ExternResourcePriorityFirst && listType <= eWordListType_ExternResourcePriorityLast)
		{
			lists.push_back(priorityListCounter);
			aRefs->push_back(TExternResourcePriority());
			(*aRefs)[priorityListCounter].Priority = listType - eWordListType_ExternResourcePriorityFirst;
			priorityListCounter++;
		}
		else
		{
			lists.push_back(-1);
		}
	}

	Int32 realIndexesCount = 0;
	error = m_Dictionary.GetRealIndexesCount(aLocalIndex, &realIndexesCount);
	if (error != eOK)
		return error;

	for (Int32 currIdx = 0; currIdx < realIndexesCount; currIdx++)
	{
		Int32 wordIndex = -1;
		Int32 listIndex = -1;

		error = m_Dictionary.GetRealIndexes(aLocalIndex, currIdx, &listIndex, &wordIndex);
		if (error != eOK)
			return error;

		if (lists[listIndex] != -1)
		{
			CSldVector<TExternResource>& extResource = (*aRefs)[lists[listIndex]].ExternResources;
			extResource.push_back(TExternResource());
			extResource[extResource.size() - 1].WordIndex = wordIndex;
		}
	}

	CSldVector<SldU16String> BaseNames;

	for (UInt32 listIdx = 0; listIdx < lists.size(); listIdx++)
	{
		if (lists[listIdx] != -1)
		{
			error = m_Dictionary.SetCurrentWordlist(listIdx);
			if (error != eOK)
				return error;

			CSldVector<TExternResource>& extResource = (*aRefs)[lists[listIdx]].ExternResources;
			for (UInt32 wordIdx = 0; wordIdx < extResource.size(); wordIdx++)
			{
				error = m_Dictionary.GoToByGlobalIndex(extResource[wordIdx].WordIndex);
				if (error != eOK)
					return error;

				UInt16* word = NULL;
				error = m_Dictionary.GetCurrentWord(0, &word);
				if (error != eOK)
					return error;

				extResource[wordIdx].ResourceName = word;

				error = m_Dictionary.GetRealIndexesCount(extResource[wordIdx].WordIndex, &realIndexesCount);
				if (error != eOK)
					return error;

				if (realIndexesCount != 0)
				{
					Int32 baseListIndex = -1;
					Int32 baseWordIndex = -1;
					error = m_Dictionary.GetRealIndexes(extResource[wordIdx].WordIndex, 0, &baseListIndex, &baseWordIndex);
					if (error != eOK)
						return error;

					if (baseListIndex != -1)
					{
						if (BaseNames.empty())
						{
							error = GetExternalBaseName(baseListIndex, BaseNames);
							if (error != eOK)
								return error;
						}

						if (baseWordIndex < (Int32)BaseNames.size())
						{
							extResource[wordIdx].BaseName = BaseNames[baseWordIndex];
						}
					}
				}
			}
		}
	}

	error = m_Dictionary.SetCurrentWordlist(currentListIndex);
	if (error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Производит поиск по вспомогательным поисковым спискам
*
* @param[in]	aWord						- указатель на поисковый запрос
* @param[in]	aLangCode					- код языка запроса
* @param[in]	aLocalizationCode			- код текущей локализации(From)
* @param[in]	aUnlockedCategoriesCount	- количество категорий, в которых будет производиться поиск (для демо баз)
*											  если передать 0, поиск будет искать по всем категориям
* @param[out]	aResultListIndex			- индекс пользовательского списка с результатами
*											  текущий установленный список в ядре не меняется
*											  в случае неудачного поиска возвращает индекс пустого кастомного списка
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionaryHelper::SearchByAuxiliarySearchList(const UInt16* aWord, const UInt32 aLangCode, const UInt32 aLocalizationCode, const UInt32 aUnlockedCategoriesCount, Int32 & aResultListIndex)
{
	if (!aWord)
		return eMemoryNullPointer;

	CSldCustomListControl listControl;
	ESldError error = m_Dictionary.AddCustomWordList(&listControl, &aResultListIndex);
	if (error != eOK)
		return error;

	Int32 listCount = 0;
	error = m_Dictionary.GetNumberOfLists(&listCount);
	if (error != eOK)
		return error;

	Int32 currentListIndex = SLD_DEFAULT_LIST_INDEX;
	error = m_Dictionary.GetCurrentWordList(&currentListIndex);
	if (error != eOK)
		return error;

	Int32 mainListIndex = SLD_DEFAULT_LIST_INDEX;
	Int32 searchListIndex = SLD_DEFAULT_LIST_INDEX;


	for (Int32 listIndex = 0; listIndex < listCount; listIndex++)
	{
		const CSldListInfo* listInfo = NULL;
		error = m_Dictionary.GetWordListInfo(listIndex, &listInfo);
		if (error != eOK)
			return error;

		const UInt32 languageCodeFrom = listInfo->GetLanguageFrom();
		const EWordListTypeEnum listType = listInfo->GetUsage();

		if (languageCodeFrom == aLangCode && listType == eWordListType_AuxiliarySearchList)
		{
			searchListIndex = listIndex;
		}
		else if (languageCodeFrom == aLocalizationCode && (listType == eWordListType_Dictionary || listType == eWordListType_Catalog))
		{
			mainListIndex = listIndex;
		}
	}

	if (mainListIndex == SLD_DEFAULT_LIST_INDEX || searchListIndex == SLD_DEFAULT_LIST_INDEX)
		return m_Dictionary.SetCurrentWordlist(currentListIndex);

	error = m_Dictionary.SetCurrentWordlist(searchListIndex);
	if (error != eOK)
		return error;

	const CSldCompare* cmp;
	error = m_Dictionary.GetCompare(&cmp);
	if (error != eOK)
		return error;

	SldU16String requestString;
	requestString.reserve(CSldCompare::StrLen(aWord) + 2);

	error = cmp->GetStrOfMassWithDelimiters(aWord, requestString, 0, 1);
	if (error != eOK)
		return error;

	requestString.insert(0, CMP_MASS_SPECIAL_SYMBOL_ANY_CHARS);
	requestString.push_back(CMP_MASS_SPECIAL_SYMBOL_ANY_CHARS);

	TExpressionBox eBox;
	if (!eBox.AddExpression(requestString.c_str(), eTokenType_Operation_AND))
		return eMemoryNotEnoughMemory;

	eBox.IsQueryWasPrepared = 1;

	const Int32 maxResultCount = 256;
	error = m_Dictionary.DoWildCardSearch(&eBox, maxResultCount);
	if (error != eOK)
		return error;

	Int32 wordCount = 0;
	error = m_Dictionary.GetNumberOfWords(&wordCount);
	if (error != eOK)
		return error;

	if (wordCount == 0)
		return CloseSearch(currentListIndex);

	const CSldListInfo* resultListInfo = NULL;
	error = m_Dictionary.GetWordListInfo(searchListIndex, &resultListInfo);
	if (error != eOK)
		return error;

	const Int32 showVariantIndex = resultListInfo->GetVariantIndexByType(eVariantShow);
	if (showVariantIndex == SLD_DEFAULT_VARIANT_INDEX)
		return CloseSearch(currentListIndex);

	const Int32 sourceSortKeyVariantIndex = resultListInfo->GetVariantIndexByType(eVariantSourceSortKey);
	if (sourceSortKeyVariantIndex == SLD_DEFAULT_VARIANT_INDEX)
		return CloseSearch(currentListIndex);

	const Int32 radioButtonVariantIndex = resultListInfo->GetVariantIndexByType(EListVariantTypeEnum(eVariantGrammarInfoBegin + 4));
	if (radioButtonVariantIndex == SLD_DEFAULT_VARIANT_INDEX)
		return CloseSearch(currentListIndex);

	SldU16WordsArray sortKeys;
	SldU16WordsArray shows;
	SldU16WordsArray radioButtonIndexes;

	for (Int32 resultIndex = 0; resultIndex < wordCount; resultIndex++)
	{
		m_Dictionary.GetWordByGlobalIndex(resultIndex);
		if (error != eOK)
			return error;

		UInt16* currentWord = NULL;
		m_Dictionary.GetCurrentWord(showVariantIndex, &currentWord);
		if (error != eOK)
			return error;
		shows.push_back(currentWord);

		m_Dictionary.GetCurrentWord(sourceSortKeyVariantIndex, &currentWord);
		if (error != eOK)
			return error;
		sortKeys.push_back(currentWord);

		m_Dictionary.GetCurrentWord(radioButtonVariantIndex, &currentWord);
		if (error != eOK)
			return error;
		radioButtonIndexes.push_back(currentWord);
	}

	error = CloseSearch(currentListIndex);
	if (error != eOK)
		return error;

	TListVariantProperty radioButtonVariant;
	radioButtonVariant.LangCode = aLangCode;
	radioButtonVariant.Number = 1;
	radioButtonVariant.Type = eVariantGrammarInfoBegin + 4;

	error = listControl.AddVariantProperty(radioButtonVariant);
	if (error != eOK)
		return error;

	Int32 minBound = 0;
	Int32 maxBound = 0xFFFFFFFF >> 1;

	if (aUnlockedCategoriesCount != 0)
	{
		error = m_Dictionary.GoToByGlobalIndex(0, mainListIndex);
		if (error != eOK)
			return error;

		Int32 numbersOfCategories = 0;
		error = m_Dictionary.GetNumberOfWords(mainListIndex, &numbersOfCategories);
		if (error != eOK)
			return error;

		if (aUnlockedCategoriesCount > (UInt32)numbersOfCategories)
			return eCommonWrongIndex;

		error = m_Dictionary.GetChildrenWordsGlobalIndexes(mainListIndex, 0, &minBound, &maxBound);
		if (error != eOK)
			return error;

		Int32 localizationShift = minBound;
		if (aUnlockedCategoriesCount != 1)
		{
			error = m_Dictionary.GetChildrenWordsGlobalIndexes(mainListIndex, (const Int32)aUnlockedCategoriesCount - 1, &minBound, &maxBound);
			if (error != eOK)
				return error;
		}

		minBound = numbersOfCategories;
		maxBound -= (localizationShift - minBound);
	}

	UInt32 resultWordsCount = 0;
	for (UInt32 resultIndex = 0; resultIndex < sortKeys.size(); resultIndex++)
	{
		UInt32 resultFlag = 0;
		error = m_Dictionary.GetWordByTextExtended(mainListIndex, sortKeys[resultIndex].c_str(), &resultFlag);
		if (error != eOK)
			return error;

		if (resultFlag != 0)
		{
			Int32 currentIndex = SLD_DEFAULT_WORD_INDEX;
			error = m_Dictionary.GetCurrentGlobalIndex(mainListIndex, &currentIndex);
			if (error != eOK)
				return error;

			if (currentIndex < minBound || currentIndex >= maxBound)
				continue;

			error = listControl.AddWord(mainListIndex, currentIndex);
			if (error != eOK)
				return error;

			error = listControl.SetCurrentWord(resultWordsCount, shows[resultIndex].c_str());
			if (error != eOK)
				return error;

			error = listControl.SetCurrentWord(resultWordsCount, radioButtonIndexes[resultIndex].c_str(), radioButtonVariant.Number);
			if (error != eOK)
				return error;

			resultWordsCount++;
		}
	}

	return eOK;
}

ESldError CSldDictionaryHelper::CloseSearch(Int32 aListIndex)
{
	ESldError error = m_Dictionary.ClearSearch();
	if (error != eOK)
		return error;

	Int32 numberOfLists = 0;
	error = m_Dictionary.GetNumberOfLists(&numberOfLists);
	if (error != eOK)
		return error;

	aListIndex = aListIndex < numberOfLists ? aListIndex : numberOfLists - 1;

	return m_Dictionary.SetCurrentWordlist(aListIndex);
}

ESldError CSldDictionaryHelper::AddCustomList(const Int32 aListIndex, CSldCustomListControl & aListControl)
{
	Int32 customListIndex = SLD_DEFAULT_LIST_INDEX;
	ESldError error = m_Dictionary.AddCustomWordList(&aListControl, &customListIndex, aListIndex);
	if (error != eOK)
		return error;

	return CloseSearch(customListIndex);
}

/** *********************************************************************
* Производит поиск по списку типа eWordListType_DictionaryForSearch
* Всегда переключается на пользовательский список. В случае неудачного поиска список будет пустым
* upd Андройд попросили поисковый список для eDictionaryForSearchResultRequest
*
* @param[in]	aWord				- указатель на поисковый запрос
* @param[in]	aMorpho				- указатель на класс морфологии для получения базовых форм слов
*									  может быть NULL, тогда морфология использоваться не будет
* @param[out]	aListControl		- контрол созданного пользовательского списка
* @param[out]	aResult				- результат выполнения функции
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionaryHelper::SearchByDictionaryForSearchList(const UInt16* aWord, MorphoDataType* aMorpho, CSldCustomListControl & aListControl, EDictionaryForSearchResultType & aResult)
{
	aResult = eDictionaryForSearchResultNone;
	const UInt32 maxResultCount = 1024;

	Int32 listCount = 0;
	ESldError error = m_Dictionary.GetNumberOfLists(&listCount);
	if (error != eOK)
		return error;

	Int32 currentListIndex = SLD_DEFAULT_LIST_INDEX;
	error = m_Dictionary.GetCurrentWordList(&currentListIndex);
	if (error != eOK)
		return error;

	Int32 dictForSearchListIndex = SLD_DEFAULT_LIST_INDEX;
	Int32 ftsListIndex = SLD_DEFAULT_LIST_INDEX;
	Int32 sourceListIndex = SLD_DEFAULT_LIST_INDEX;

	for (Int32 listIndex = 0; listIndex < listCount; listIndex++)
	{
		const CSldListInfo* listInfo = NULL;
		error = m_Dictionary.GetWordListInfo(listIndex, &listInfo);
		if (error != eOK)
			return error;

		const UInt32 languageCodeFrom = listInfo->GetLanguageFrom();
		const EWordListTypeEnum listType = listInfo->GetUsage();

		if (listType == eWordListType_DictionaryForSearch)
			dictForSearchListIndex = listIndex;
		else if (listType >= eWordListType_FullTextSearchBase && listType < eWordListType_FullTextSearchLast)
			ftsListIndex = listIndex;
	}

	if (dictForSearchListIndex == SLD_DEFAULT_LIST_INDEX || ftsListIndex == SLD_DEFAULT_LIST_INDEX)
		return AddCustomList(sourceListIndex, aListControl);

	if (CSldCompare::IsWordHasWildCardSymbols(aWord) != 0)
	{
		TExpressionBox expressionBox;
		error = PrepareQueryForWildCardSearch(aWord, &m_Dictionary, dictForSearchListIndex, &expressionBox);
		if (error != eOK)
			return error;

		error = m_Dictionary.DoWildCardSearch(dictForSearchListIndex, &expressionBox, maxResultCount);
		if (error != eOK)
			return error;

		Int32 numberOfWords = 0;
		error = m_Dictionary.GetNumberOfWords(&numberOfWords);
		if (error != eOK)
			return error;

		if (numberOfWords > 0)
		{
			aResult = eDictionaryForSearchResultRequest;
			error = m_Dictionary.GetCurrentWordList(&sourceListIndex);
			if (error != eOK)
				return error;
		}

		return eOK;
	}

	UInt32 result = 0;
	m_Dictionary.SetCurrentWordlist(ftsListIndex);
	if (error != eOK)
		return error;

	m_Dictionary.GetWordByTextExtended(aWord, &result);
	if (error != eOK)
		return error;

	if (result != 0)
	{
		Int32 customListIndex = SLD_DEFAULT_LIST_INDEX;
		error = m_Dictionary.GetSubwordsList(&aListControl, &customListIndex);
		if (error != eOK)
			return error;

		error = m_Dictionary.SetCurrentWordlist(currentListIndex);
		if (error != eOK)
			return error;

		Int32 numberOfWords = 0;
		error = m_Dictionary.GetNumberOfWords(customListIndex, &numberOfWords);
		if (error != eOK)
			return error;

		if (numberOfWords > 0)
			aResult = eDictionaryForSearchResultNormal;

		return m_Dictionary.SetCurrentWordlist(customListIndex);
	}

	error = m_Dictionary.StartMultiSearch();
	if (error != eOK)
		return error;

	SldU16String resultQuery;
	error = PrepareQueryForFullTextSearch(aWord, &m_Dictionary, ftsListIndex, aMorpho, resultQuery);
	if (error != eOK)
		return error;

	error = m_Dictionary.DoFullTextSearch(ftsListIndex, resultQuery.c_str(), 1024);
	if (error != eOK)
		return error;

	Int32 wordsCount = 0;
	error = m_Dictionary.GetNumberOfWords(&wordsCount);
	if (error != eOK)
		return error;

	if (wordsCount != 0)
	{
		error = m_Dictionary.StopMultiSearch();
		if (error != eOK)
			return error;

		aResult = eDictionaryForSearchResultNormal;
		error = m_Dictionary.GetCurrentWordList(&sourceListIndex);
		if (error != eOK)
			return error;

		return AddCustomList(sourceListIndex, aListControl);
	}

	error = CloseSearch(currentListIndex);
	if (error != eOK)
		return error;

	error = m_Dictionary.DoSpellingSearch(ftsListIndex, aWord, maxResultCount);
	if (error != eOK)
		return error;

	error = m_Dictionary.GetNumberOfWords(&wordsCount);
	if (error != eOK)
		return error;

	if (wordsCount > 0)
	{
		aResult = eDictionaryForSearchResultRequest;
		error = m_Dictionary.GetCurrentWordList(&sourceListIndex);
		if (error != eOK)
			return error;
	}

	return m_Dictionary.StopMultiSearch();
}


ESldError AddVariantProperties(const CSldListInfo* aListInfo, CSldCustomListControl & aListControl)
{
	const UInt32 variantCount = aListInfo->GetNumberOfVariants();
	for (UInt32 currentVariant = 0; currentVariant < variantCount; currentVariant++)
	{
		TListVariantProperty variantProperty;
		variantProperty.Type = aListInfo->GetVariantType(currentVariant);
		variantProperty.LangCode = aListInfo->GetVariantLangCode(currentVariant);
		ESldError error = aListControl.AddVariantProperty(variantProperty, true);
		if (error != eOK)
			return error;
	}
	return eOK;
}

/** *********************************************************************
* Производит расширенный поиск по спискам полнотекстового поиска с заголовками
*
* @param[in]	aRequest			- указатель на поисковый запрос
* @param[in]	aLangCode			- код язака запроса
* @param[in]	aMorpho				- указатель на класс морфологии для получения базовых форм слов
*									  может быть NULL, тогда морфология использоваться не будет
* @param[out]	aResultListIndex	- индекс пользовательского списка с результатами
*									  в случае неудачного поиска возвращает индекс пустого кастомного списка
*									  текущий установленный список в ядре не меняется
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionaryHelper::AdvancedHeadwordSearch(const UInt16* aRequest, const UInt32 aLangCode, MorphoDataType* aMorpho, Int32 & aResultListIndex)
{
	if (!aRequest)
		return eMemoryNullPointer;

	static const UInt32 MaxResultCount = 128;

	CSldCustomListControl listControl;
	ESldError error = m_Dictionary.AddCustomWordList(&listControl, &aResultListIndex);
	if (error != eOK)
		return error;

	Int32 currentListIndex = SLD_DEFAULT_LIST_INDEX;
	error = m_Dictionary.GetCurrentWordList(&currentListIndex);
	if (error != eOK)
		return error;

	Int32 listCount = 0;
	error = m_Dictionary.GetNumberOfLists(&listCount);
	if (error != eOK)
		return error;

	Int32 dictionaryListIndex = SLD_DEFAULT_LIST_INDEX;
	Int32 simpleSortedListIndex = SLD_DEFAULT_LIST_INDEX;
	Int32 minSearchBound = 0;
	Int32 maxSearchBound = 0;
	Int32 showVariantIndex = SLD_DEFAULT_VARIANT_INDEX;
	CSldVector<Int32> fullTextSearchListIndexes;

	for (Int32 listIndex = 0; listIndex < listCount; listIndex++)
	{
		const CSldListInfo* listInfo = NULL;
		error = m_Dictionary.GetWordListInfo(listIndex, &listInfo);
		if (error != eOK)
			return error;

		const UInt32 languageCodeFrom = listInfo->GetLanguageFrom();
		const EWordListTypeEnum listType = listInfo->GetUsage();

		if (languageCodeFrom == aLangCode && (listType == eWordListType_Dictionary || listType == eWordListType_InApp))
		{
			if (dictionaryListIndex == SLD_DEFAULT_LIST_INDEX)
			{
				showVariantIndex = listInfo->GetShowVariantIndex();

				dictionaryListIndex = listIndex;
				if (listInfo->GetSimpleSortedListIndex() != SLD_DEFAULT_LIST_INDEX)
					simpleSortedListIndex = listInfo->GetSimpleSortedListIndex();

				maxSearchBound = listInfo->GetNumberOfGlobalWords();
				if (listInfo->GetSearchRange() == eSearchRangeRoot)
				{
					error = m_Dictionary.GoToByGlobalIndex(0, listIndex);
					if (error != eOK)
						return error;

					error = m_Dictionary.GetNumberOfWords(listIndex, &maxSearchBound);
					if (error != eOK)
						return error;
				}

				error = AddVariantProperties(listInfo, listControl);
				if (error != eOK)
					return error;
			}
		}
		else if (languageCodeFrom == aLangCode && listType >= eWordListType_FullTextSearchHeadword && listType < eWordListType_FullTextSearchContent)
		{
			fullTextSearchListIndexes.push_back(listIndex);
		}
	}

	if (dictionaryListIndex == SLD_DEFAULT_LIST_INDEX)
		return m_Dictionary.SetCurrentWordlist(currentListIndex);

	const Int32 sortedListIndex = simpleSortedListIndex == SLD_DEFAULT_LIST_INDEX ? dictionaryListIndex : simpleSortedListIndex;

	const CSldCompare* cmp;
	error = m_Dictionary.GetCompare(&cmp);
	if (error != eOK)
		return error;

	SldU16String effectiveRequest;
	cmp->GetEffectiveString(aRequest, effectiveRequest);

	if (effectiveRequest.empty())
		return m_Dictionary.SetCurrentWordlist(currentListIndex);

	error = m_Dictionary.GetWordByText(sortedListIndex, aRequest);
	if (error != eOK)
		return error;

	UInt16* currentWord = NULL;
	error = m_Dictionary.GetCurrentWord(sortedListIndex, 0, &currentWord);
	if (error != eOK)
		return error;

	SldU16String tempStr;
	cmp->GetEffectiveString(currentWord, tempStr);

	Int32 wordsCount = 0;
	error = m_Dictionary.GetNumberOfWords(sortedListIndex, &wordsCount);
	if (error != eOK)
		return error;

	Int32 currentIndex = SLD_DEFAULT_WORD_INDEX;
	error = m_Dictionary.GetCurrentGlobalIndex(sortedListIndex, &currentIndex);
	if (error != eOK)
		return error;

	CSldVector<Int32> addedIndexes;
	while (++currentIndex < wordsCount && cmp->StrICmp(effectiveRequest.c_str(), tempStr.c_str()) == 0)
	{
		const Int32 addedIndex = simpleSortedListIndex == SLD_DEFAULT_LIST_INDEX ? currentIndex - 1 : GetIndexFromSimpleSortedList(simpleSortedListIndex, currentIndex - 1);

		if (addedIndex >= minSearchBound && addedIndex < maxSearchBound)
		{
			error = listControl.AddWord(dictionaryListIndex, addedIndex);
			if (error != eOK)
				return error;
			sld2::sorted_insert(addedIndexes, addedIndex);
		}

		error = m_Dictionary.GetWordByIndex(sortedListIndex, currentIndex);
		if (error != eOK)
			return error;

		error = m_Dictionary.GetCurrentWord(sortedListIndex, 0, &currentWord);
		if (error != eOK)
			return error;

		cmp->GetEffectiveString(currentWord, tempStr);
	}

	SldU16String ftsRequest;
	SldU16WordsArray allWordForms;
	CSldVector<CSldVector<TSldPair<SldU16String, Int32>>> ftsResult(4);
	for (auto ftsListIndex : fullTextSearchListIndexes)
	{
		if (ftsRequest.empty())
		{
			error = PrepareQueryForFullTextSearch(aRequest, &m_Dictionary, ftsListIndex, aMorpho, ftsRequest, &allWordForms);
			if (error != eOK)
				return error;
		}

		error = m_Dictionary.DoFullTextSearch(ftsListIndex, ftsRequest.c_str(), 1024);
		if (error != eOK)
			return error;

		Int32 resultCount = 0;
		error = m_Dictionary.GetNumberOfWords(&resultCount);
		if (error != eOK)
			return error;

		Int32 realWordIndex = SLD_DEFAULT_WORD_INDEX;
		Int32 realListIndex = SLD_DEFAULT_LIST_INDEX;

		SldU16String requestMass;
		SldU16String resultMass;
		error = cmp->GetStrOfMassWithDelimiters(aRequest, requestMass);
		if (error != eOK)
			return error;

		requestMass.insert(0, CMP_MASS_DELIMITER);
		requestMass.push_back(CMP_MASS_DELIMITER);

		for (Int32 ftsResultIndex = 0; ftsResultIndex < resultCount; ftsResultIndex++)
		{
			error = m_Dictionary.GetRealListIndex(ftsResultIndex, &realListIndex);
			if (error != eOK)
				return error;

			error = m_Dictionary.GetRealGlobalIndex(ftsResultIndex, &realWordIndex);
			if (error != eOK)
				return error;

			if (dictionaryListIndex == realListIndex)
			{
				if (sld2::sorted_find(addedIndexes, realWordIndex) != addedIndexes.npos)
					continue;
				else
					sld2::sorted_insert(addedIndexes, realWordIndex);

				error = m_Dictionary.GoToByGlobalIndex(realWordIndex, dictionaryListIndex);
				if (error != eOK)
					return error;

				error = m_Dictionary.GetCurrentWord(dictionaryListIndex, showVariantIndex, &currentWord);
				if (error != eOK)
					return error;

				error = cmp->GetStrOfMassWithDelimiters(currentWord, resultMass);
				if (error != eOK)
					return error;

				resultMass.insert(0, CMP_MASS_DELIMITER);
				resultMass.push_back(CMP_MASS_DELIMITER);

				const UInt16* wordPos = cmp->StrStr(resultMass.c_str(), requestMass.c_str());
				TSldPair<SldU16String, Int32> res(sld2::move(resultMass), sld2::move(realWordIndex));
				if (wordPos == res.first.c_str())
				{
					ftsResult[1].push_back(sld2::move(res));
				}
				else if (wordPos != NULL)
				{
					UInt32 charIndex = 0;
					while (charIndex < res.first.size())
					{
						if (res.first[charIndex] == CMP_MASS_DELIMITER)
							res.first.erase(charIndex, 1);
						else
							charIndex++;
					}

					sld2::sorted_insert(ftsResult[2], sld2::move(res));
				}
				else
				{
					bool singleForm = false;
					for (auto & form : allWordForms)
					{
						if (cmp->StrICmp(form.c_str(), currentWord) == 0)
						{
							singleForm = true;
							break;
						}
					}

					if (singleForm)
						ftsResult[0].push_back(sld2::move(res));
					else
					{
						UInt32 charIndex = 0;
						while (charIndex < res.first.size())
						{
							if (res.first[charIndex] == CMP_MASS_DELIMITER)
								res.first.erase(charIndex, 1);
							else
								charIndex++;
						}

						sld2::sorted_insert(ftsResult[3], sld2::move(res));
					}
				}
			}
		}
	}

	for (auto & order : ftsResult)
	{
		for (auto & res : order)
		{
			error = listControl.AddWord(dictionaryListIndex, res.second);
			if (error != eOK)
				return error;
		}
	}

	error = m_Dictionary.ClearSearch();
	if (error != eOK)
		return error;

	if (effectiveRequest.length() < tempStr.length())
		tempStr.resize(effectiveRequest.length());

	if (cmp->StrICmp(effectiveRequest.c_str(), tempStr.c_str()) == 0)
	{
		Int32 beginWordIndex = SLD_DEFAULT_WORD_INDEX;
		Int32 endWordIndex = SLD_DEFAULT_WORD_INDEX;

		error = m_Dictionary.GetCurrentGlobalIndex(sortedListIndex, &beginWordIndex);
		if (error != eOK)
			return error;

		tempStr = aRequest;
		tempStr.back() = cmp->GetNextMassSymbol(tempStr.back());

		error = m_Dictionary.GetWordByText(sortedListIndex, tempStr.c_str());
		if (error != eOK)
			return error;

		error = m_Dictionary.GetCurrentGlobalIndex(sortedListIndex, &endWordIndex);
		if (error != eOK)
			return error;

		for (Int32 wordIndex = beginWordIndex; wordIndex < endWordIndex; wordIndex++)
		{
			if (addedIndexes.size() >= MaxResultCount)
				return m_Dictionary.SetCurrentWordlist(currentListIndex);

			const Int32 addedIndex = simpleSortedListIndex == SLD_DEFAULT_LIST_INDEX ? wordIndex : GetIndexFromSimpleSortedList(simpleSortedListIndex, wordIndex);
			if (sld2::sorted_find(addedIndexes, addedIndex) != addedIndexes.npos)
				continue;

			if (addedIndex >= minSearchBound && addedIndex < maxSearchBound)
			{
				sld2::sorted_insert(addedIndexes, addedIndex);
				error = listControl.AddWord(dictionaryListIndex, addedIndex);
				if (error != eOK)
					return error;
			}
		}
	}

	for (auto ftsListIndex : fullTextSearchListIndexes)
	{
		error = m_Dictionary.GetWordByText(ftsListIndex, aRequest);
		if (error != eOK)
			return error;

		error = m_Dictionary.GetCurrentWord(ftsListIndex, 0, &currentWord);
		if (error != eOK)
			return error;

		cmp->GetEffectiveString(currentWord, tempStr);
		if (effectiveRequest.length() < tempStr.length())
			tempStr.resize(effectiveRequest.length());

		if (cmp->StrICmp(effectiveRequest.c_str(), tempStr.c_str()) == 0)
		{
			Int32 beginWordIndex = SLD_DEFAULT_WORD_INDEX;
			Int32 endWordIndex = SLD_DEFAULT_WORD_INDEX;

			error = m_Dictionary.GetCurrentGlobalIndex(ftsListIndex, &beginWordIndex);
			if (error != eOK)
				return error;

			tempStr = aRequest;
			tempStr.back() = cmp->GetNextMassSymbol(tempStr.back());

			error = m_Dictionary.GetWordByText(ftsListIndex, tempStr.c_str());
			if (error != eOK)
				return error;

			error = m_Dictionary.GetCurrentGlobalIndex(ftsListIndex, &endWordIndex);
			if (error != eOK)
				return error;

			Int32 realWordIndex = SLD_DEFAULT_WORD_INDEX;
			Int32 realListIndex = SLD_DEFAULT_LIST_INDEX;

			for (Int32 wordIndex = beginWordIndex; wordIndex < endWordIndex; wordIndex++)
			{
				if (addedIndexes.size() >= MaxResultCount)
					return m_Dictionary.SetCurrentWordlist(currentListIndex);

				Int32 realIndexesCount = 0;
				error = m_Dictionary.GetRealIndexesCount(ftsListIndex, wordIndex, &realIndexesCount);
				if (error != eOK)
					return error;

				for (Int32 realIndex = 0; realIndex < realIndexesCount; realIndex++)
				{
					error = m_Dictionary.GetRealIndexes(ftsListIndex, wordIndex, realIndex, &realListIndex, &realWordIndex);
					if (error != eOK)
						return error;

					if (addedIndexes.size() && sld2::binary_search(&addedIndexes[0], addedIndexes.size(), realWordIndex))
						continue;

					if (dictionaryListIndex == realListIndex)
					{
						error = listControl.AddWord(dictionaryListIndex, realWordIndex);
						if (error != eOK)
							return error;
						sld2::sorted_insert(addedIndexes, realWordIndex);
					}
				}
			}
		}
	}
	return m_Dictionary.SetCurrentWordlist(currentListIndex);
}

Int32 CSldDictionaryHelper::GetIndexFromSimpleSortedList(const Int32 aListIndex, const Int32 aWordIndex)
{
	Int32 result = SLD_DEFAULT_WORD_INDEX;
	UInt16* sortedIndexStr = NULL;

	const Int32 indexVariant = 1;
	ESldError error = m_Dictionary.GetWordByIndex(aListIndex, aWordIndex);
	if (error != eOK)
		return error;

	error = m_Dictionary.GetCurrentWord(aListIndex, indexVariant, &sortedIndexStr);
	if (error != eOK)
		return error;

	Int8* ptrToStr = (Int8*)sortedIndexStr;
	ptrToStr++;
	sortedIndexStr = (UInt16*)ptrToStr;

	if (*sortedIndexStr)
	{
		sldMemMove(&result, ptrToStr, sizeof(Int32));
	}
	else
	{
		UInt16* tmpStr = NULL;
		ESldError error = m_Dictionary.GetCurrentWord(aListIndex, indexVariant, &tmpStr);
		if (error != eOK)
			return error;

		if (CSldCompare::StrLen(tmpStr) == 1)
		{
			result = 0;
		}
		else
		{
			sldMemMove(&result, ptrToStr, sizeof(Int32));
		}
	}

	return result;
}

/** *********************************************************************
* Производит поиск Collocations в базах Oxford Collocations
*
* @param[in]	aRequest			- указатель на поисковый запрос
* @param[in]	aLangCode			- код язака запроса
* @param[in]	aMorpho				- указатель на класс морфологии для получения базовых форм слов
*									  может быть NULL, тогда морфология использоваться не будет
* @param[out]	aListControl		- контрол созданного пользовательского списка
* @param[out]	aResultListIndex	- индекс пользовательского списка с результатами
*									  в случае неудачного поиска возвращает индекс пустого кастомного списка
*									  текущий установленный список в ядре не меняется
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionaryHelper::CollocationsSearch(const UInt16* aRequest, const UInt32 aLangCode, MorphoDataType* aMorpho, CSldCustomListControl & aListControl, Int32 & aResultListIndex)
{
	if (!aRequest)
		return eMemoryNullPointer;

	ESldError error = m_Dictionary.AddCustomWordList(&aListControl, &aResultListIndex);
	if (error != eOK)
		return error;

	Int32 currentListIndex = SLD_DEFAULT_LIST_INDEX;
	error = m_Dictionary.GetCurrentWordList(&currentListIndex);
	if (error != eOK)
		return error;

	Int32 listCount = 0;
	error = m_Dictionary.GetNumberOfLists(&listCount);
	if (error != eOK)
		return error;

	Int32 headwordsListIndex = SLD_DEFAULT_LIST_INDEX;
	Int32 collocationsListIndex = SLD_DEFAULT_LIST_INDEX;
	const Int32 maxResultCount = 1024;

	for (Int32 listIndex = 0; listIndex < listCount; listIndex++)
	{
		const CSldListInfo* listInfo = NULL;
		error = m_Dictionary.GetWordListInfo(listIndex, &listInfo);
		if (error != eOK)
			return error;

		const UInt32 languageCodeFrom = listInfo->GetLanguageFrom();
		if (aLangCode != languageCodeFrom)
			continue;

		const EWordListTypeEnum listType = listInfo->GetUsage();

		if (listType >= eWordListType_FullTextSearchHeadword && listType < eWordListType_FullTextSearchContent)
			headwordsListIndex = listIndex;

		else if (listType >= eWordListType_FullTextSearchContent && listType < eWordListType_FullTextSearchTranslation)
			collocationsListIndex = listIndex;

		else if (listType == eWordListType_Dictionary || listType == eWordListType_InApp)
		{
			error = AddVariantProperties(listInfo, aListControl);
			if (error != eOK)
				return error;
		}
	}

	SldU16String ftsQuery;
	error = PrepareQueryForFullTextSearch(aRequest, &m_Dictionary, headwordsListIndex, aMorpho, ftsQuery);
	if (error != eOK)
		return error;

	CSldVector<TSldMorphologyWordStruct> morphologyForms;
	error = PrepareQueryForSortingSearchResults(aRequest, &m_Dictionary, aMorpho, morphologyForms);
	if (error != eOK)
		return error;

	if (headwordsListIndex != SLD_DEFAULT_LIST_INDEX)
	{
		error = m_Dictionary.DoFullTextSearch(headwordsListIndex, ftsQuery.c_str(), maxResultCount);
		if (error != eOK)
			return error;

		Int32 resultCount = 0;
		error = m_Dictionary.GetNumberOfWords(&resultCount);
		if (error != eOK)
			return error;

		if (resultCount != 0)
		{
			error = aListControl.AddWord(0);
			if (error != eOK)
				return error;

			const CSldListLocalizedString * strings = NULL;
			error = m_Dictionary.GetLocalizedStringsByListIndex(headwordsListIndex, &strings);
			if (error != eOK)
				return error;

			const UInt16 * listName = NULL;
			error = strings->GetListName(SldLanguage::Default, &listName);
			if (error != eOK)
				return error;

			error = aListControl.SetCurrentWord(0, listName);
			if (error != eOK)
				return error;

			Int32 headwordResultListIndex = SLD_DEFAULT_LIST_INDEX;
			error = m_Dictionary.GetCurrentWordList(&headwordResultListIndex);
			if (error != eOK)
				return error;

			error = m_Dictionary.SortSearchListRelevant(headwordResultListIndex, aRequest, morphologyForms, eFTSSortingType_AlphabeticalSortedGroups);
			if (error != eOK)
				return error;

			for (Int32 resultIndex = 0; resultIndex < resultCount; resultIndex++)
			{
				Int32 realWordIndex = SLD_DEFAULT_WORD_INDEX;
				Int32 realListIndex = SLD_DEFAULT_LIST_INDEX;

				error = m_Dictionary.GetRealIndexes(resultIndex, 0, &realListIndex, &realWordIndex);
				if (error != eOK)
					return error;

				error = aListControl.AddSubWord(0, realWordIndex);
				if (error != eOK)
					return error;
			}

			EUncoverFolderResult result;
			error = aListControl.UncoverHierarchy(0, &result);
			if (error != eOK)
				return error;
		}

		error = CloseSearch(currentListIndex);
		if (error != eOK)
			return error;
	}

	if (collocationsListIndex != SLD_DEFAULT_LIST_INDEX)
	{
		error = m_Dictionary.DoFullTextSearch(collocationsListIndex, ftsQuery.c_str(), maxResultCount);
		if (error != eOK)
			return error;

		Int32 phraseCount = 0;
		error = m_Dictionary.GetNumberOfWords(&phraseCount);
		if (error != eOK)
			return error;

		if (phraseCount == 0)
			return CloseSearch(currentListIndex);

		const CSldListInfo * searchResultListInfo = NULL;
		error = m_Dictionary.GetWordListInfo(&searchResultListInfo);
		if (error != eOK)
			return error;

		error = AddVariantProperties(searchResultListInfo, aListControl);
		if (error != eOK)
			return error;

		Int32 currentIndex = SLD_DEFAULT_WORD_INDEX;
		error = m_Dictionary.GetNumberOfWords(aResultListIndex, &currentIndex);
		if (error != eOK)
			return error;

		error = aListControl.AddWord();
		if (error != eOK)
			return error;

		const CSldListLocalizedString * strings = NULL;
		error = m_Dictionary.GetLocalizedStringsByListIndex(collocationsListIndex, &strings);
		if (error != eOK)
			return error;

		const UInt16 * listName = NULL;
		error = strings->GetListName(SldLanguage::Default, &listName);
		if (error != eOK)
			return error;

		error = aListControl.SetCurrentWord(currentIndex, listName);
		if (error != eOK)
			return error;		

		Int32 collocationResultListIndex = SLD_DEFAULT_LIST_INDEX;
		error = m_Dictionary.GetCurrentWordList(&collocationResultListIndex);
		if (error != eOK)
			return error;

		for (Int32 phraseIndex = 0; phraseIndex < phraseCount; phraseIndex++)
		{
			currentIndex++;
			Int32 phraseListIndex = SLD_DEFAULT_LIST_INDEX;
			Int32 phraseWordIndex = SLD_DEFAULT_WORD_INDEX;

			error = m_Dictionary.GetRealIndexes(phraseIndex, 0, &phraseListIndex, &phraseWordIndex);
			if (error != eOK)
				return error;

			error = aListControl.AddWord(phraseListIndex, phraseWordIndex);
			if (error != eOK)
				return error;

			Int32 resultCount = 0;
			error = m_Dictionary.GetRealIndexesCount(phraseListIndex, phraseWordIndex, &resultCount);
			if (error != eOK)
				return error;

			for (Int32 resultIndex = 0; resultIndex < resultCount; resultIndex++)
			{
				Int32 resultListIndex = SLD_DEFAULT_LIST_INDEX;
				Int32 resultWordIndex = SLD_DEFAULT_LIST_INDEX;

				error = m_Dictionary.GetRealIndexes(phraseListIndex, phraseWordIndex, resultIndex, &resultListIndex, &resultWordIndex);
				if (error != eOK)
					return error;

				error = aListControl.AddSubWord(currentIndex, resultWordIndex, resultListIndex);
				if (error != eOK)
					return error;
			}
		}

		error = CloseSearch(currentListIndex);
		if (error != eOK)
			return error;
	}

	return eOK;
}

/** *********************************************************************
* Получает алфавит для заданного сортированного списка
*
* @param[in]	aListIndex			- номер списка, для которого мы хотим получить алфавит
* @param[out]	aAlphabet			- вектор с алфавитом, содержащий пары вида
*									  символ в верхнем регистре - глобальный индекс слова
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionaryHelper::GetAlphabet(const Int32 aListIndex, CSldVector<TSldPair<UInt16, Int32>> & aAlphabet)
{
	Int32 currentListIndex = SLD_DEFAULT_LIST_INDEX;
	ESldError error = m_Dictionary.GetCurrentWordList(&currentListIndex);
	if (error != eOK)
		return error;

	error = m_Dictionary.SetCurrentWordlist(aListIndex);
	if (error != eOK)
		return error;

	Int32 currentWordIndex = SLD_DEFAULT_WORD_INDEX;
	error = m_Dictionary.GetCurrentGlobalIndex(&currentWordIndex);
	if (error != eOK)
		return error;

	error = m_Dictionary.GoToByGlobalIndex(0);
	if (error != eOK)
		return error;

	UInt32 isSorted = 0;
	error = m_Dictionary.IsCurrentLevelSorted(&isSorted);
	if (error != eOK)
		return error;

	if (isSorted != 0)
	{
		const CSldCompare * cmp = NULL;
		error = m_Dictionary.GetCompare(&cmp);
		if (error != eOK)
			return error;

		Int32 wordsCount = 0;
		error = m_Dictionary.GetNumberOfWords(&wordsCount);
		if (error != eOK)
			return error;

		UInt16 nextCharMass[2] {};
		nextCharMass[0] = cmp->GetNextMassSymbol(' ');

		SldU16String effectiveString;
		TSldPair<UInt16, Int32> charInfo(0, 0);
		while (nextCharMass[0] != MAX_UINT16_VALUE)
		{
			error = m_Dictionary.GetWordByText(nextCharMass);
			if (error != eOK)
				return error;

			error = m_Dictionary.GetCurrentGlobalIndex(&charInfo.second);
			if (error != eOK)
				return error;

			if (charInfo.second + 1 >= wordsCount)
				break;

			UInt16 * word = NULL;
			error = m_Dictionary.GetCurrentWord(0, &word);
			if (error != eOK)
				return error;

			cmp->GetEffectiveString(word, effectiveString);
			if (effectiveString.empty())
				break;

			charInfo.first = cmp->ToUpperChr(*effectiveString.c_str());
			aAlphabet.push_back(charInfo);

			nextCharMass[0] = cmp->GetNextMassSymbol(charInfo.first);
		}
	}

	error = m_Dictionary.GoToByGlobalIndex(currentWordIndex);
	if (error != eOK)
		return error;

	return m_Dictionary.SetCurrentWordlist(aListIndex);
}

ESldError AddWordForMorphoTips(const Int32 aListIndex, const Int32 aGlobalIndex, CSldDictionary& aDictionary, const SldU16String& aOriginalWord, const Int32 aMaxSearchBound, CSldVector<Int32>& aMorphoTipsIndexes)
{
	if (aGlobalIndex >= aMaxSearchBound)
		return eOK;

	const CSldListInfo* listInfo = nullptr;
	ESldError error = aDictionary.GetWordListInfo(aListIndex, &listInfo);
	if (error != eOK)
		return error;

	error = aDictionary.GoToByGlobalIndex(aGlobalIndex, aListIndex);
	if (error != eOK)
		return error;

	UInt16* currentWord = nullptr;
	error = aDictionary.GetCurrentWord(listInfo->GetShowVariantIndex(), &currentWord);
	if (error != eOK)
		return error;

	if (CSldCompare::StrCmp(currentWord, aOriginalWord.c_str()) == 0)
		aMorphoTipsIndexes.push_back(aGlobalIndex);

	return eOK;
}

/// @todo заменить на лямбду, если в ядре появится std::function
ESldError RecursiveCheckPhraseForMorphoTips(CSldDictionary& aDictionary, const Int32 aListIndex, const UInt32 aIndex, const UInt16* aOriginalWord,
	const CSldVector<CSldVector<TSldMorphologyWordStruct>>& aFraseBaseForms, SldU16String& aStringBuffer, CSldVector<Int32>& aWordSetBuffer, const Int32 aMaxSearchBound, CSldVector<Int32>& aMorphoTipsIndexes)
{
	const CSldCompare* cmp = nullptr;
	ESldError error = aDictionary.GetCompare(&cmp);
	if (error != eOK)
		return error;

	for (UInt32 checkedWordIndex = aIndex; checkedWordIndex < aFraseBaseForms.size(); checkedWordIndex++)
	{
		for (const auto & wordVariant : aFraseBaseForms[checkedWordIndex])
		{
			cmp->ReplaceWordInPhraseByIndex(aStringBuffer, wordVariant.MorphologyForm, checkedWordIndex);
			error = aDictionary.GetWordSetByTextExtended(aListIndex, aStringBuffer.c_str(), aWordSetBuffer);
			if (error != eOK)
				return error;

			for (const auto wordIndex : aWordSetBuffer)
			{
				error = AddWordForMorphoTips(aListIndex, wordIndex, aDictionary, aStringBuffer, aMaxSearchBound, aMorphoTipsIndexes);
				if (error != eOK)
					return error;
			}

			RecursiveCheckPhraseForMorphoTips(aDictionary, aListIndex, checkedWordIndex + 1, aOriginalWord, aFraseBaseForms, aStringBuffer, aWordSetBuffer, aMaxSearchBound, aMorphoTipsIndexes);
			aStringBuffer = aOriginalWord;
		}
	}

	return eOK;
};

/** *********************************************************************
* Получает морфо-подсказки для заданного списка
*
* @param[in]	aListIndex			- номер списка, в котором мы хотим найти морфо-подсказки
* @param[in]	aWord				- запрос, для которого мы хотим получить морфо-подсказки
* @param[in]	aMorpho				- указатель на морфологическую базу
* @param[out]	aMorphoTipsIndexes	- вектор с глобальными индексами морфо-подсказок в списке aListIndex
* @param[in]	aOnlyRoot			- флаг, определяющий на каком уровне искать морфо-подсказки
*									  true - только в корне списка, false - по всему списку
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionaryHelper::GetMorphoTips(const Int32 aListIndex, const UInt16* aWord, MorphoDataType* aMorpho, CSldVector<Int32> & aMorphoTipsIndexes, const bool aOnlyRoot)
{
	aMorphoTipsIndexes.clear();
	if (!aWord)
		return eMemoryNullPointer;

	Int32 currentState = SLD_DEFAULT_WORD_INDEX;
	ESldError error = m_Dictionary.GetCurrentGlobalIndex(aListIndex, &currentState);
	if (error != eOK)
		return error;

	Int32 maxSearchBound = 0;
	error = GetMaxSearchBound(aListIndex, aOnlyRoot, maxSearchBound);
	if (error != eOK)
		return error;

	EDictionaryBrandName brand = eBrand_Unknown;
	error = m_Dictionary.GetDictionaryBrand((UInt32*)&brand);
	if (error != eOK)
		return error;

	const CSldCompare* cmp;
	error = m_Dictionary.GetCompare(&cmp);
	if (error != eOK)
		return error;

	CSldVector<SldU16StringRef> reqWords;
	cmp->DivideQueryByParts(aWord, reqWords);

	if (brand == eBrand_Van_Dale)
	{
		SldU16String stringBuffer;
		stringBuffer.reserve(cmp->StrLen(aWord) * 2);
		CSldVector<CSldVector<TSldMorphologyWordStruct>> fraseBaseForms;

		for (const auto & word : reqWords)
		{
			stringBuffer = word;
			fraseBaseForms.emplace_back();
			auto& forms = fraseBaseForms.back();

			error = GetAllMorphoForms(stringBuffer.c_str(), aMorpho, forms);
			if (error != eOK)
				return error;

			auto it = forms.begin();
			while(it != forms.end())
			{
				if (it->MorphologyFormType != eMorphologyWordTypeBase || cmp->StrICmp(word, it->MorphologyForm) == 0)
					it = forms.erase(it);
				else
					it++;
			}
		}

		const auto wordWithoutDelimiter = cmp->TrimDelimiters(aWord);
		stringBuffer = wordWithoutDelimiter;
		CSldVector<Int32> wordSetBuffer;

		error = RecursiveCheckPhraseForMorphoTips(m_Dictionary, aListIndex, 0, wordWithoutDelimiter.c_str(), fraseBaseForms, stringBuffer, wordSetBuffer, maxSearchBound, aMorphoTipsIndexes);
		if (error != eOK)
			return error;
	}
	else if (reqWords.size() == 1)
	{
		CSldVector<TSldMorphologyWordStruct> forms;
		error = GetAllMorphoForms(aWord, aMorpho, forms);
		if (error != eOK)
			return error;

		for (const auto & form : forms)
		{
			if (cmp->StrICmp(aWord, form.MorphologyForm) == 0)
				continue;

			CSldVector<Int32> resultWordSet;
			error = m_Dictionary.GetWordSetByTextExtended(aListIndex, form.MorphologyForm.c_str(), resultWordSet);
			if (error != eOK)
				return error;

			for (const auto wordIndex : resultWordSet)
			{
				error = AddWordForMorphoTips(aListIndex, wordIndex, m_Dictionary, form.MorphologyForm, maxSearchBound, aMorphoTipsIndexes);
				if (error != eOK)
					return error;
			}
		}
	}

	return m_Dictionary.GoToByGlobalIndex(currentState, aListIndex);
}

/** *********************************************************************
* Ищет запись, соответсвующую запросу в словарных/каталожных списках
*
* @param[in]	aLangCode	- код языка запроса
* @param[in]	aWord		- запрос, который мы хотим найти
* @param[in]	aMorpho		- указатель на морфологическую базу
* @param[out]	aRefs		- контейнер с возможными результатами.
*							  первый результат - самый релевантный
* @param[in]	aOnlyRoot	- флаг, определяющий на каком уровне искать записи
*							  true - только в корне списка, false - по всему списку
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionaryHelper::GetCrossReference(const ESldLanguage aLangCode, const SldU16String & aWord, MorphoDataType* aMorpho, SldWordsCollection& aRefs, const bool aOnlyRoot)
{
	aRefs.clear();

	Int32 listCount = 0;
	ESldError error = m_Dictionary.GetNumberOfLists(&listCount);
	if (error != eOK)
		return error;

	Int32 dictionaryListIndex = SLD_DEFAULT_LIST_INDEX;
	Int32 catalogListIndex = SLD_DEFAULT_LIST_INDEX;
	Int32 listIndex = 0;
	for (; listIndex < listCount; listIndex++)
	{
		const CSldListInfo *listInfo = NULL;
		error = m_Dictionary.GetWordListInfo(listIndex, &listInfo);
		if (error != eOK)
			return error;

		if (listInfo->GetLanguageFrom() == aLangCode)
		{
			if (listInfo->GetUsage() == eWordListType_Dictionary)
				dictionaryListIndex = listIndex;

			if (listInfo->GetUsage() == eWordListType_Catalog)
				catalogListIndex = listIndex;
		}
	}
	
	listIndex = dictionaryListIndex == SLD_DEFAULT_LIST_INDEX ? catalogListIndex : dictionaryListIndex;
	if(listIndex == SLD_DEFAULT_LIST_INDEX)
		return eOK;

	CSldVector<TSldMorphologyWordStruct> forms;
	error = GetAllMorphoForms(aWord.c_str(), aMorpho, forms);
	if (error != eOK)
		return error;

	const auto requestPos = sld2::find(forms, aWord);
	if(requestPos == forms.npos)
		forms.insert(0, TSldMorphologyWordStruct(aWord, eMorphologyWordTypeBase));
	else if (requestPos != 0)
	{
		forms.erase(requestPos);
		forms.insert(0, TSldMorphologyWordStruct(aWord, eMorphologyWordTypeBase));
	}

	Int32 currentState = SLD_DEFAULT_WORD_INDEX;
	error = m_Dictionary.GetCurrentGlobalIndex(listIndex, &currentState);
	if (error != eOK)
		return error;


	Int32 maxSearchBound = 0;
	error = GetMaxSearchBound(listIndex, aOnlyRoot, maxSearchBound);
	if (error != eOK)
		return error;

	UInt32 res = 0;
	Int32 foundWordIndex = SLD_DEFAULT_WORD_INDEX;
	for (const auto & form : forms)
	{
		error = m_Dictionary.GetWordByTextExtended(listIndex, form.MorphologyForm.c_str(), &res);
		if (error != eOK)
			return error;

		if (res == 0)
			continue;

		error = m_Dictionary.GetCurrentGlobalIndex(listIndex, &foundWordIndex);
		if (error != eOK)
			return error;

		if (foundWordIndex < maxSearchBound)
		{
			aRefs.emplace_back(listIndex, foundWordIndex);
			break;
		}
	}

	if (aRefs.empty())
		return m_Dictionary.GoToByGlobalIndex(currentState, listIndex);

	CSldVector<Int32> similarWords;
	error = m_Dictionary.GetWordSetByTextExtended(aWord.c_str(), similarWords);
	if (error != eOK)
		return error;

	for (const auto wordIndex : similarWords)
	{
		TSldWordIndexes word(listIndex, wordIndex);
		if (wordIndex < maxSearchBound && sld2::find(aRefs, word) == aRefs.npos)
			aRefs.emplace_back(word);
	}

	return m_Dictionary.GoToByGlobalIndex(currentState, listIndex);
}

ESldError CSldDictionaryHelper::GetMaxSearchBound(const Int32 aListIndex, const bool aOnlyRoot, Int32& aMaxBound)
{
	ESldError error;
	if (aOnlyRoot)
	{
		error = m_Dictionary.GoToByGlobalIndex(0, aListIndex);
		if (error != eOK)
			return error;

		error = m_Dictionary.GetNumberOfWords(aListIndex, &aMaxBound);
		if (error != eOK)
			return error;
	}
	else
	{
		error = m_Dictionary.GetTotalWordCount(aListIndex, &aMaxBound);
		if (error != eOK)
			return error;
	}

	return eOK;
}

/** *********************************************************************
* Выполняет свайп в любом списке, в направлении aDirection/abs(aDirection), в режиме aSwipingMode.
*   В случае, когда список в котором нужно выполнить свайп является кастомным и имеет иерархию, дополнительно раскрывает каталоги
*
* @param[in]	aDirection			- Знаковое число, по которому определяется направление свайпа aDirection/abs(aDirection)
* @param[in]	aSwipingMode		- Режим свайпа
* @param[in]	currentListControl	- Указатель на структуру управления кастомным списком.
*									  nullptr - в случае, когда список в котором нужно выполнить свайп не является кастомным
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionaryHelper::Swipe(Int32 aDirection, ESwipingMode aSwipingMode,
	CSldCustomListControl* currentListControl)
{
	if (!aDirection)
		return eOK;
	aDirection = aDirection / abs(aDirection);

	ESldError error;

	const CSldListInfo* listInfo;
	error = m_Dictionary.GetWordListInfo(&listInfo);
	if (error != eOK)
		return error;

	Int32 customWordListIndex = 0;
	Int32 baseWordListIndex = 0;
	Int32 primaryCustomWordIndex = 0;
	Int32 primaryBaseGlobalWordIndex = 0;
	Int32 resultBaseGlobalWordIndex = 0;
	Int32 resultCustomWordIndex = 0;

	//Случай, когда текущий список является кастомным,
	// и нам надо перейти в основной список для выполнения функции перехода.
	if (listInfo->IsHierarchy() && listInfo->GetUsage() == eWordListType_CustomList)
	{
		if (!currentListControl)
			return eOK;

		//Записываем текущий индекс списка слов на случай, когда он является кастомным,
		// и нам надо перейти в основной список и, затем, вернуться в текущий по этому индексу.
		error = m_Dictionary.GetCurrentWordList(&customWordListIndex);
		if (error != eOK)
			return error;
		error = m_Dictionary.GetCurrentIndex(&primaryCustomWordIndex);
		if (error != eOK)
			return error;
		error = m_Dictionary.GetRealGlobalIndex(customWordListIndex, primaryCustomWordIndex,
			&primaryBaseGlobalWordIndex);
		if (error != eOK)
			return error;
		error = m_Dictionary.GetRealListIndex(primaryCustomWordIndex, &baseWordListIndex);
		if (error != eOK)
			return error;
		error = m_Dictionary.SetCurrentWordlist(baseWordListIndex);
		if (error != eOK)
			return error;
		error = m_Dictionary.GoToByGlobalIndex(primaryBaseGlobalWordIndex);
		if (error != eOK)
			return error;
	}

	//Вызов функциий перехода для некастомного списка.
	error = Swipe(aDirection, aSwipingMode);
	if (error != eOK)
		return error;

	//Случай, когда текущий список является кастомным,
	// и нам надо вернуться из основного списка в текущий после выполнения функции перехода
	// для перехода к полученному слову уже в кастомном списке, в режиме aSwipingMode.
	if (listInfo->IsHierarchy() && listInfo->GetUsage() == eWordListType_CustomList)
	{
		if (!currentListControl)
			return eOK;

		error = m_Dictionary.GetCurrentGlobalIndex(&resultBaseGlobalWordIndex);
		if (error != eOK)
			return error;

		//Получаем путь к слову в основном списке.
		TCatalogPath path;
		error = m_Dictionary.GetCurrentPath(&path);
		if (error != eOK)
			return error;
		UInt32 maxDepth = path.BaseListCount;

		//Создаем вектор пар, хранящий глобальные индексы всех элементов,
		// встречаемых на пути path,
		// и глобальные индексы элементов, локольно следующих сразу за ними.
		CSldVector<TSldPair<Int32, Int32>> realGlobalIndicesVector;
		{
			Int32 tempGlobalIndex = 0, tempNextGlobalIndex = 0;
			error = m_Dictionary.GoToByGlobalIndex(0);
			if (error != eOK)
				return error;
			for (Int32 depth = 0; depth < maxDepth; depth++)
			{
				if (depth)
				{
					Int32 localIndex = 0;
					error = m_Dictionary.GetCurrentIndex(&localIndex);
					if (error != eOK)
						return error;
					error = m_Dictionary.SetBase(localIndex);
					if (error != eOK)
						return error;
				}

				//Получаем глобальный индекс эелемента,
				// следующего за элементом, встречаемым на пути path.
				error = m_Dictionary.GetWordByIndex(path.BaseList[depth] + 1);

				//Если элемента, локольно следующего элементу, встречаемому на пути, нет,
				// то записывется -1.
				if (error != eOK)
				{
					tempNextGlobalIndex = -1;
				}
				else
				{
					error = m_Dictionary.GetCurrentGlobalIndex(&tempNextGlobalIndex);
					if (error != eOK)
						return error;
				}

				//Получаем глобальный индекс втречаемого на пути элемента.
				error = m_Dictionary.GetWordByIndex(path.BaseList[depth]);
				if (error != eOK)
					return error;
				error = m_Dictionary.GetCurrentGlobalIndex(&tempGlobalIndex);
				if (error != eOK)
					return error;

				//Записываем полученные глобальные индексы.
				realGlobalIndicesVector.push_back(TSldPair<Int32, Int32>(tempGlobalIndex,
					tempNextGlobalIndex));
			}
			error = m_Dictionary.GoToByGlobalIndex(resultBaseGlobalWordIndex);
			if (error != eOK)
				return error;
		}

		//Переходим в текущий - кастомный список.
		error = m_Dictionary.SetCurrentWordlist(customWordListIndex);
		if (error != eOK)
			return error;

		//Ищем в кастомном списке запись с глобальным индексом равным результату и переходим на нее.
		// Расскрываем каталоги при необходимости.
		resultCustomWordIndex = primaryCustomWordIndex;
		Int32 nextByDirectionCustomWordIndex = primaryCustomWordIndex;
		Int32 nextByDirectionBaseGlobalWordIndex = primaryBaseGlobalWordIndex;
		Int32 tempBaseGlobalWordIndex = primaryBaseGlobalWordIndex;

		while (tempBaseGlobalWordIndex != resultBaseGlobalWordIndex)
		{
			error = m_Dictionary.GetRealGlobalIndex(customWordListIndex, nextByDirectionCustomWordIndex,
				&nextByDirectionBaseGlobalWordIndex);
			if (error != eOK)
				return error;

			//Расскрываем каталоги при необходимости.
			for (Int32 depthI = 0; depthI < maxDepth - 1; ++depthI)
			{
				if (realGlobalIndicesVector[depthI].first == nextByDirectionBaseGlobalWordIndex)
				{
					Int32 tempNextByNextByDirectionBaseGlobalWordIndex = 0;
					error = m_Dictionary.GetRealGlobalIndex(customWordListIndex,
						nextByDirectionCustomWordIndex + 1, &tempNextByNextByDirectionBaseGlobalWordIndex);
					if (error != eOK)
						tempNextByNextByDirectionBaseGlobalWordIndex = -1;

					for (Int32 depthJ = 0; depthJ < maxDepth; ++depthJ)
					{
						if (realGlobalIndicesVector[depthJ].second == tempNextByNextByDirectionBaseGlobalWordIndex)
						{
							EUncoverFolderResult uncoverResult = eFolderNoActions;
							error = currentListControl->UncoverHierarchy(nextByDirectionCustomWordIndex, &uncoverResult);
							if (error != eOK)
								return error;

							error = m_Dictionary.GetCurrentIndex(&nextByDirectionCustomWordIndex);
							if (error != eOK)
								return error;

							Int32 tempBaseGlobalWordIndex1 = 0;
							error = m_Dictionary.GetRealGlobalIndex(customWordListIndex, nextByDirectionCustomWordIndex,
								&tempBaseGlobalWordIndex1);
							if (error != eOK)
								return error;

							while (tempBaseGlobalWordIndex1 != tempBaseGlobalWordIndex)
							{
								nextByDirectionCustomWordIndex -= aDirection;
								error = m_Dictionary.GetRealGlobalIndex(customWordListIndex, nextByDirectionCustomWordIndex,
									&tempBaseGlobalWordIndex1);
								if (error != eOK)
									return error;
							}

							nextByDirectionCustomWordIndex += aDirection;
							error = m_Dictionary.GetRealGlobalIndex(customWordListIndex, nextByDirectionCustomWordIndex,
								&nextByDirectionBaseGlobalWordIndex);
							if (error != eOK)
								return error;
							break;
						}
					}
				}
			}

			tempBaseGlobalWordIndex = nextByDirectionBaseGlobalWordIndex;
			resultCustomWordIndex = nextByDirectionCustomWordIndex;

			nextByDirectionCustomWordIndex += aDirection;
		}
		error = m_Dictionary.GoToByGlobalIndex(resultCustomWordIndex);
		if (error != eOK)
			return error;
	}

	return eOK;
}

/** *********************************************************************
* Выполняет свайп в некастомном списке, в направлении aDirection/abs(aDirection), в режиме aSwipingMode
*
* @param[in]	aDirection			- Знаковое число, по которому определяется направление свайпа aDirection/abs(aDirection)
* @param[in]	aSwipingMode		- Режим свайпа
*
* @return код ошибки
************************************************************************/
ESldError CSldDictionaryHelper::Swipe(Int32 aDirection, ESwipingMode aSwipingMode)
{
	if (!aDirection)
		return eOK;
	aDirection = aDirection / abs(aDirection);

	ESldError error;
	
	//Инициализация вспомогательных переменных;
	Int32 localIndex = 0;
	error = m_Dictionary.GetCurrentIndex(&localIndex);
	if (error != eOK)
		return error;
	Int32 levelElementsCount = 0;
	error = m_Dictionary.GetNumberOfWords(&levelElementsCount);
	if (error != eOK)
		return error;
	Int32 globalIndex = 0;
	error = m_Dictionary.GetCurrentGlobalIndex(&globalIndex);
	if (error != eOK)
		return error;
	Int32 savedOldGlobalIndex = globalIndex;

	//Условные переменные, по которым определяется тип елемента списка.
	// Типы подразделяются на:
	//	-Просто каталог;
	UInt32 isLevelElement = false;
	error = m_Dictionary.isWordHasHierarchy(localIndex, &isLevelElement, NULL);
	if (error != eOK)
		return error;
	//	-Статья;
	Int32 isArticleElement = false;
	error = m_Dictionary.GetNumberOfTranslations(localIndex, &isArticleElement);
	if (error != eOK)
		return error;
	//	-Каталог со ссылкой на статью = статья
	bool isMixedElement = isLevelElement && isArticleElement;

	//Если тип оказался чистым каталожным и режим aSwipingMode не позволяет проходить через него,
	// то выходим из функции.
	if (aSwipingMode == eInterruptingOnCatalogSwipe && isLevelElement && !isArticleElement)
	{
		return eOK;
	}

	bool isFirstIteration = true;
	while (true)
	{
		if (localIndex >= levelElementsCount || localIndex < 0)
		{
			//Если режим aSwipingMode не позволяет переходить на уровень выше,
			// то выходим из функции.
			if (aSwipingMode != eAcrossingCatalogSwipe)
			{
				error = m_Dictionary.GoToByGlobalIndex(savedOldGlobalIndex);
				if (error != eOK)
					return error;
				return eOK;
			}

			//Переход на уровень выше.
			error = m_Dictionary.GoToLevelUp();
			if (error != eOK)
				return error;

			//Если уровень был наивсшим, то выходим из функции.
			Int32 aTmpGlobalIndex = 0;
			error = m_Dictionary.GetCurrentGlobalIndex(&aTmpGlobalIndex);
			if (error != eOK)
				return error;
			if (globalIndex == aTmpGlobalIndex)
			{
				error = m_Dictionary.GoToByGlobalIndex(savedOldGlobalIndex);
				return error;
			}

			//Обновление вспомогательных переменных после изменения уровня;
			error = m_Dictionary.GetCurrentIndex(&localIndex);
			if (error != eOK)
				return error;
			error = m_Dictionary.GetNumberOfWords(&levelElementsCount);
			if (error != eOK)
				return error;
			error = m_Dictionary.GetCurrentGlobalIndex(&globalIndex);
			if (error != eOK)
				return error;

			if (aDirection > 0)
			{
				localIndex += aDirection;
			}
			else
			{
				//Определяем тип текущего элемента.
				error = m_Dictionary.GetNumberOfTranslations(localIndex, &isArticleElement);
				if (error != eOK)
					return error;
				if (isArticleElement)
				{
					return eOK;
				}
				else
				{
					localIndex += aDirection;
				}
			}

			continue;
		}

		if (!isFirstIteration)
		{
			//Определяем тип текущего элемента.
			error = m_Dictionary.isWordHasHierarchy(localIndex, &isLevelElement, NULL);
			if (error != eOK)
				return error;
			error = m_Dictionary.GetNumberOfTranslations(localIndex, &isArticleElement);
			if (error != eOK)
				return error;
			isMixedElement = isLevelElement && isArticleElement;

			//Если тип оказался чистым каталожным и режим aSwipingMode не позволяет проходить через него,
			// то выходим из функции.
			if (aSwipingMode == eInterruptingOnCatalogSwipe && isLevelElement && !isArticleElement)
			{
				error = m_Dictionary.GoToByGlobalIndex(savedOldGlobalIndex);
				if (error != eOK)
					return error;
				return eOK;
			}

			//Если режим aSwipingMode позволяет проходить через любой тип,
			// то переходим на результат и выходим из функции.
			if (aSwipingMode == ePassingAnySwipe)
			{
				error = m_Dictionary.GetWordByIndex(localIndex);
				if (error != eOK)
					return error;
				return eOK;
			}

			//Если тип оказался статейным при доп. условиях,
			// то переходим на результат и выходим из функции.
			if (isArticleElement &&
				(aSwipingMode == eSkipingCatalogSwipe || !isLevelElement || aDirection > 0))
			{
				error = m_Dictionary.GetWordByIndex(localIndex);
				if (error != eOK)
					return error;
				return eOK;
			}
		}

		if (isLevelElement && aSwipingMode == eAcrossingCatalogSwipe &&
			(aDirection > 0 || !isFirstIteration))
		{
			if (isFirstIteration)
			{
				isFirstIteration = false;
			}

			//Вход в подуовень.
			error = m_Dictionary.SetBase(localIndex);
			if (error != eOK)
				return error;

			//Обновление вспомогательных переменных после изменения уровня;
			error = m_Dictionary.GetNumberOfWords(&levelElementsCount);
			if (error != eOK)
				return error;
			if (aDirection < 0)
			{
				error = m_Dictionary.GetWordByIndex(levelElementsCount - 1);
				if (error != eOK)
					return error;
			}
			error = m_Dictionary.GetCurrentIndex(&localIndex);
			if (error != eOK)
				return error;
			error = m_Dictionary.GetCurrentGlobalIndex(&globalIndex);
			if (error != eOK)
				return error;

			continue;
		}

		if (isFirstIteration)
		{
			isFirstIteration = false;
		}
		localIndex += aDirection;
	}
}

/** *********************************************************************
* Ищет записи, соответсвующие словам во фразе
*
* @param[in]	aPhrase		- фраза, разбиваемая на слова
* @param[in]	aMorphos	- указатели на морфологические базы
* @param[out]	aRefs		- контейнер с найденными словами результатами
* @param[in] aUseBuiltInMorpho - флаг, включающий использование встроенной морфологии
* @return код ошибки
************************************************************************/
ESldError CSldDictionaryHelper::GetWordReference(SldU16StringRef aPhrase, const CSldVector<MorphoDataType*>& aMorphos, CSldVector<TWordRefInfo>& aRefs, UInt8 aUseBuiltInMorpho/* = 0*/)
{
	aRefs.clear();
	ESldError error;

	CSldVector<SldU16StringRef> words;
	GetCompare()->DivideQueryByParts(aPhrase, words);
	if (words.empty())
		return eOK;

	const auto dictionaryLists = GetListIndexes(eWordListType_Dictionary);
	SldU16String rawWord;
	CSldVector<Int32> tmpIndexes;
	CSldVector<TSldMorphologyWordStruct> tmpMorphologyForms;

	for (const auto word : words)
	{
		rawWord = GetCompare()->GetEffectiveString(word);
		aRefs.emplace_back();
		auto& wordRef = aRefs.back();

		wordRef.StartPos = word.data() - aPhrase.data();
		wordRef.EndPos = wordRef.StartPos + word.length() - 1;

		for (const auto listIndex : dictionaryLists)
		{
      error = GetReferencesForList(m_Dictionary, listIndex, rawWord, tmpIndexes, 
                                   wordRef, aMorphos, GetCompare(), aUseBuiltInMorpho);

      if (error != eOK)
        return error;
		}
	}

	return eOK;
}

/** *********************************************************************
* Ищет записи, соответсвующие словам во фразе в указанном списке
*
* @param[in]	aPhrase		- фраза, разбиваемая на слова
* @param[in]	aMorphos	- указатели на морфологические базы
* @param[out]	aRefs		- контейнер с найденными словами результатами
* @param[in]	aListIndex	- индекс списка, в котором ищутся записи, по умолчанию в текущем
* @param[in] aUseBuiltInMorpho - флаг, включающий использование встроенной морфологии
* @return код ошибки
************************************************************************/
ESldError CSldDictionaryHelper::GetWordReferenceInList(SldU16StringRef aPhrase, const CSldVector<MorphoDataType*>& aMorphos, CSldVector<TWordRefInfo>& aRefs, Int32 aListIndex /*= SLD_DEFAULT_LIST_INDEX*/, UInt8 aUseBuiltInMorpho/* = 0*/)
{
	Int32 listIndex = aListIndex;

	if (listIndex == SLD_DEFAULT_LIST_INDEX)
	{
		ESldError error = m_Dictionary.GetCurrentWordList(&listIndex);
		if (error != eOK)
			return error;

		if (listIndex == SLD_DEFAULT_LIST_INDEX)
			return eCommonWrongList;
	}

	aRefs.clear();
	ESldError error = ESldError::eOK;

	CSldVector<SldU16StringRef> words;
	GetCompare()->DivideQueryByParts(aPhrase, words);
	if (words.empty())
		return eOK;

	SldU16String rawWord;
	CSldVector<Int32> tmpIndexes;
	CSldVector<TSldMorphologyWordStruct> tmpMorphologyForms;

	for (const auto word : words)
	{
		rawWord = GetCompare()->GetEffectiveString(word);
		aRefs.emplace_back();
		auto& wordRef = aRefs.back();

		wordRef.StartPos = word.data() - aPhrase.data();
		wordRef.EndPos = wordRef.StartPos + word.length() - 1;

		error = GetReferencesForList(m_Dictionary, listIndex, rawWord, tmpIndexes,
			wordRef, aMorphos, GetCompare(), aUseBuiltInMorpho);

		if (error != eOK)
			return error;
	}

	return eOK;
}

/** *********************************************************************
* Ищет слова из запроса во фразе
*
* @param[in]	aPhrase		- фраза, разбиваемая на слова
* @param[in]	aMorphos	- указатели на морфологические базы
* @param[out]	aRefs		- контейнер с найденными индексами слов
* @param[in]	aQuery	- запрос, слова которого ищутся во фразу
* @param[in] aUseBuiltInMorpho - флаг, включающий использование встроенной морфологии
* @return код ошибки
************************************************************************/
ESldError CSldDictionaryHelper::GetWordReferenceInQuery(SldU16StringRef aPhrase, const CSldVector<MorphoDataType*>& aMorphos, CSldVector<UInt32>& aRefs, SldU16StringRef aQuery, UInt8 aUseBuiltInMorpho/* = 0*/) const
{
	aRefs.clear();

	CSldVector<SldU16StringRef> phrase_words;
	GetCompare()->DivideQueryByParts(aPhrase, phrase_words);
	if (phrase_words.empty())
		return eOK;

	CSldVector<SldU16StringRef> query_words;
	GetCompare()->DivideQueryByParts(aQuery, query_words);
	if (query_words.empty())
		return eOK;

	CSldVector<SldU16String> effective_query_words(query_words.size());
	for (UInt32 i = 0; i < query_words.size(); ++i)
		effective_query_words[i] = GetCompare()->GetEffectiveString(query_words[i]);

	CSldVector<SldU16String> query_bases;
	query_bases.reserve(query_words.size()*(1 + aMorphos.size()));

	const Int32 kBaseFormsIteratorFlags = 
		M_BASEFORMS_SHOULD_MATCH_PRECONDITION |
		M_BASEFORMS_USE_DICTIONARY |
		M_BASEFORMS_ONLY_DICTIONARY_WORDS;

	ESldError error = ESldError::eOK;

	for (UInt32 i = 0; i < effective_query_words.size(); ++i)
	{
		query_bases.push_back(effective_query_words[i]);
		for (const MorphoDataType* morpho : aMorphos)
		{
			if (!morpho)
				continue;

			BaseFormsIterator it;
			morpho->BaseFormsIteratorInitW(it, effective_query_words[i].c_str(),
				kBaseFormsIteratorFlags);

			UInt16 base_buf[MAX_WORD_LEN] = { 0 };
			const MorphoInflectionRulesSetHandle* ruleset = nullptr;
			while (morpho->GetNextBaseFormW(it, base_buf, &ruleset))
				query_bases.emplace_back(base_buf);
		}

		if (aUseBuiltInMorpho)
		{
			const ESldLanguage language = m_Dictionary.GetLanguageFrom();
			CSldVector<TSldMorphologyWordStruct> morphologyForms;
			error = GetMorphoForms(effective_query_words[i].c_str(), &m_Dictionary, language, morphologyForms, 0, EMorphoFormsType::eMorphoBase);
			if (error != eOK)
				return error;

			for (const TSldMorphologyWordStruct& morphoWord : morphologyForms)
				query_bases.push_back(morphoWord.MorphologyForm);
		}
	}

	auto isInQuery = [&](SldU16StringRef aWord) -> bool
	{
		const CSldCompare* kCompare = GetCompare();

		for (const SldU16String& base : query_bases)
		{
			if (kCompare && kCompare->StrICmp(aWord, base) == 0)
				return true;
			else if (!kCompare && base == aWord)
				return true;
		}

		return false;
	};

	for (UInt32 i = 0; i < phrase_words.size(); ++i)
	{
		SldU16String rawWord = GetCompare()->GetEffectiveString(phrase_words[i]);

		if (!isInQuery(rawWord))
		{
			bool found = false;
			for (const MorphoDataType* morpho : aMorphos)
			{
				if (!morpho)
					continue;

				BaseFormsIterator it;
				morpho->BaseFormsIteratorInitW(it, rawWord.c_str(),
					kBaseFormsIteratorFlags);

				UInt16 base_buf[MAX_WORD_LEN] = { 0 };
				const MorphoInflectionRulesSetHandle* ruleset = nullptr;
				while (morpho->GetNextBaseFormW(it, base_buf, &ruleset))
				{
					if (isInQuery(base_buf))
					{
						found = true;
						break;
					}
				}

				if (found)
					break;
			}

			if (aUseBuiltInMorpho)
			{
				const ESldLanguage language = m_Dictionary.GetLanguageFrom();
				CSldVector<TSldMorphologyWordStruct> morphologyForms;
				error = GetMorphoForms(rawWord.c_str(), &m_Dictionary, language, morphologyForms, 0, EMorphoFormsType::eMorphoBase);
				if (error != eOK)
					return error;

				for (const TSldMorphologyWordStruct& morphoWord : morphologyForms)
				{
					if (isInQuery(morphoWord.MorphologyForm))
					{
						found = true;
						break;
					}
				}
			}

			if (!found)
				continue;
		}

		aRefs.emplace_back(i);
	}

	return eOK;
}

/** *********************************************************************
* Возвращает диапазоны во фразе, соответсвующие указанным словам с обрамляющими пробелами
*
* @param[in]	aPhrase		- фраза
* @param[in]	aWords		- индексы слов во фразе
* @param[out]	aRefs	- диапазоны слов во фразе
* @return код ошибки
************************************************************************/
ESldError CSldDictionaryHelper::GetWordsPositionsInPhraseIncludingWhitespaces(SldU16StringRef aPhrase, const CSldVector<UInt32>& aWords, CSldVector<TWordRefInfo>& aRefs)
{
	CSldVector<SldU16StringRef> words;
	GetCompare()->DivideQueryByParts(aPhrase, words);

	struct ExtendedWord
	{
		UInt32 m_Begin;
		UInt32 m_Size;

		Int8 operator<(const ExtendedWord& rhs) const
		{
			return m_Begin < rhs.m_Begin ||
				(m_Begin == rhs.m_Begin && m_Size < rhs.m_Size);
		}

		Int8 operator==(const ExtendedWord& rhs) const
		{
			return m_Begin == rhs.m_Begin && m_Size == rhs.m_Size;
		}
	};

	CSldVector<ExtendedWord> sortedExtendedWords;
	sortedExtendedWords.reserve(aWords.size());

	for (UInt32 idx : aWords)
	{
		if (idx >= words.size())
			return ESldError::eInputErrorBase;

		ExtendedWord word{ static_cast<UInt32>(words[idx].data() - aPhrase.data()), words[idx].size() };

		if (GetCompare()->IsZeroSymbol(aPhrase[word.m_Begin]) && GetCompare()->IsZeroSymbol(aPhrase[word.m_Begin + word.m_Size - 1]))
		{
			word.m_Begin++;
			word.m_Size -= 2;
			UInt32 symbol = word.m_Begin;
			while (GetCompare()->IsZeroSymbol(aPhrase[symbol]) && symbol < word.m_Begin + word.m_Size)
			{
				word.m_Begin++;
				word.m_Size--;
				++symbol;
			}
			symbol = word.m_Begin + word.m_Size - 1;
			while (GetCompare()->IsZeroSymbol(aPhrase[symbol]) && symbol > word.m_Begin)
			{
				word.m_Size--;
				--symbol;
			}
		}

		if (word.m_Begin > 0 && GetCompare()->IsWhitespace(aPhrase[word.m_Begin - 1]))
		{
			word.m_Begin--;
			word.m_Size++;
		}

		const UInt32 kNextCharPos = word.m_Begin + word.m_Size;
		if (kNextCharPos < aPhrase.size() && GetCompare()->IsWhitespace(aPhrase[kNextCharPos]))
			word.m_Size++;

		sld2::sorted_insert(sortedExtendedWords, sld2::move(word));
	}

	aRefs.clear();
	aRefs.reserve(sortedExtendedWords.size());

	for (const ExtendedWord& word : sortedExtendedWords)
	{
		if (aRefs.empty() || aRefs.back().EndPos < word.m_Begin)
			aRefs.push_back({word.m_Begin, word.m_Begin + word.m_Size});
		else
			aRefs.back().EndPos = word.m_Begin + word.m_Size;
	}

	return ESldError::eOK;
}

CSldVector<Int32> CSldDictionaryHelper::GetListIndexes(const EWordListTypeEnum aType, const ESldLanguage aLang)
{
	CSldVector<Int32> result;

	Int32 listCount = 0;
	ESldError error = m_Dictionary.GetNumberOfLists(&listCount);
	if (error != eOK)
		return result;

	for (Int32 listIndex = 0; listIndex < listCount; listIndex++)
	{
		const CSldListInfo *listInfo = NULL;
		error = m_Dictionary.GetWordListInfo(listIndex, &listInfo);
		if (error != eOK)
			return result;

		if (listInfo->GetUsage() == aType && (aLang == SldLanguage::Unknown || aLang == listInfo->GetLanguageFrom()))
			result.push_back(listIndex);
	}

	return result;
}

/** *********************************************************************
* Ищет диапазоны фразы и слов запроса в тексте
*
* @param[in]	aText		- текст 
* @param[in]	aPhrase		- фраза, искомая в тексте
* @param[in]	aQuery	- запрос, по которму нашлась фраза
* @param[in]	aHeadword	- заголовок, для замены тильды
* @param[in]	aMorphos	- указатели на морфологические базы
* @param[out]	aHighlightData	- диапазоны фразы и слов из запроса
* @param[in] aUseBuiltInMorpho - флаг, включающий использование встроенной морфологии
* @return код ошибки
************************************************************************/
ESldError CSldDictionaryHelper::GetQueryHighlightData(SldU16StringRef aText, SldU16StringRef aPhrase, SldU16StringRef aQuery, SldU16StringRef aHeadword, const CSldVector<MorphoDataType*>& aMorphos, CSldVector<UInt32>& aHighlightData, UInt8 aUseBuiltInMorpho/* = 0*/) const
{
	CSldVector<UInt32> wordsInPhrase;
	ESldError error = GetWordReferenceInQuery(aPhrase, aMorphos, wordsInPhrase, aQuery, aUseBuiltInMorpho);
	if (error != ESldError::eOK)
		return error;

	class StandardSubphraseSearch : public CSldSubphraseSearchInstance
	{
	public:
		explicit StandardSubphraseSearch(const CSldCompare* aSldCompare) : m_SldCompare(aSldCompare) {}

		Int8 TryForBestMatch(
			const SldU16String& aPhrase,
			const SldU16String& aSubphrase,
			TPhraseSubrange& aRange) override
		{
			if (!m_SldCompare)
				return 0;

			CSldVector<SldU16StringRef> phraseWords, subphraseWords;
			m_SldCompare->DivideQueryByParts(aPhrase, phraseWords);
			m_SldCompare->DivideQueryByParts(aSubphrase, subphraseWords);

			if (subphraseWords.empty())
			{
				aRange = TPhraseSubrange();
				return 1;
			}

			if (subphraseWords.size() > phraseWords.size())
				return 0;

			if (subphraseWords.size() == 1)
			{
				for (UInt32 i = 0; i < phraseWords.size(); ++i)
				{
					if (m_SldCompare->StrICmp(subphraseWords[0], phraseWords[i]) == 0)
					{
						aRange.m_Begin = phraseWords[i].data() - aPhrase.data();
						aRange.m_Size = phraseWords[i].size();

						m_SubphraseWords = CSldVector<TPhraseSubrange>({ aRange });

						return 1;
					}
				}

				return 0;
			}

			CSldVector<UInt32> wordIndices;
			wordIndices.reserve(subphraseWords.size());

			CSldVector<UInt32> bestWordIndices;
			UInt32 bestStart = -1, bestEnd = -1;
			for (UInt32 i = 0; i < phraseWords.size() - subphraseWords.size() + 1; ++i)
			{
				wordIndices.clear();
				if (m_SldCompare->StrICmp(subphraseWords[0], phraseWords[i]) != 0)
					continue;

				wordIndices.push_back(i);

				UInt32 lastWord = GetBestLastWord(phraseWords, subphraseWords, wordIndices, i + 1, 1);
				if (lastWord == -1)
					continue;

				if (bestStart == -1 || lastWord - i + 1 < m_LeastWords)
				{
					bestStart = i;
					bestEnd = lastWord;

					m_LeastWords = bestEnd - bestStart + 1;
					bestWordIndices = wordIndices;
				}
			}

			if (bestStart == -1)
				return 0;

			aRange.m_Begin = phraseWords[bestStart].data() - aPhrase.data();
			aRange.m_Size = phraseWords[bestEnd].data() - phraseWords[bestStart].data() + phraseWords[bestEnd].size();

			m_SubphraseWords.resize(subphraseWords.size());
			for (UInt32 i = 0; i < subphraseWords.size(); ++i)
			{
				m_SubphraseWords[i].m_Begin = phraseWords[bestWordIndices[i]].data() - aPhrase.data();
				m_SubphraseWords[i].m_Size = phraseWords[bestWordIndices[i]].size();
			}

			return 1;
		}

		CSldVector<TPhraseSubrange> GetSubphraseWords() const
		{
			return m_SubphraseWords;
		}

	private:
		UInt32 GetBestLastWord(
			const CSldVector<SldU16StringRef>& aPhraseWords,
			const CSldVector<SldU16StringRef>& aSubphraseWords,
			CSldVector<UInt32>& aWordIndices,
			UInt32 aPhraseOffset = 0, UInt32 aSubphraseOffset = 0)
		{
			UInt32 remainingWords = aSubphraseWords.size() - aSubphraseOffset;

			CSldVector<UInt32> bestWordIndices;
			UInt32 bestLastWord = -1;
			for (UInt32 i = aPhraseOffset; i < aPhraseWords.size() - remainingWords + 1; ++i)
			{
				aWordIndices.resize(aSubphraseOffset);
				if (m_SldCompare->StrICmp(aSubphraseWords[aSubphraseOffset], aPhraseWords[i]) == 0)
				{
					aWordIndices.push_back(i);
					if (remainingWords == 1)
						return i;

					UInt32 candidate = GetBestLastWord(aPhraseWords, aSubphraseWords, aWordIndices, i + 1, aSubphraseOffset + 1);
					if (candidate < bestLastWord)
					{
						bestWordIndices = aWordIndices;
						bestLastWord = candidate;
					}
				}
			}

			aWordIndices = bestWordIndices;
			return bestLastWord;
		}

		const CSldCompare* m_SldCompare = nullptr;

		CSldVector<TPhraseSubrange> m_SubphraseWords;

		UInt32 m_LeastWords = -1;
	};

	class TildeReplacer : public CSldControlledTransform
	{
	public:
		explicit TildeReplacer(SldU16String aHeadword) :
			m_Headword(sld2::move(aHeadword)) {}

		Result Act(const SldU16String& aFrom) const override
		{
			Result result;

			UInt32 resultLength = 0;
			for (UInt32 i = 0; i < aFrom.size(); ++i)
				resultLength += (aFrom[i] == SldU16Char('~') ? m_Headword.size() : 1);

			result.m_String.reserve(resultLength);

			CSldVector<UInt32> originalPositions;
			originalPositions.reserve(resultLength);

			for (UInt32 i = 0; i < aFrom.size(); ++i)
			{
				if (aFrom[i] == SldU16Char('~'))
				{
					result.m_String += m_Headword;
					originalPositions.insert(originalPositions.size(), m_Headword.size(), i);
				}
				else
				{
					result.m_String += aFrom[i];
					originalPositions.push_back(i);
				}
			}

			class MyPullback : public CSldSubrangePullback
			{
			public:
				explicit MyPullback(CSldVector<UInt32> aOriginalPositions) :
					m_OriginalPositions(sld2::move(aOriginalPositions)) {}

				TPhraseSubrange Act(const TPhraseSubrange& aFrom) const override
				{
					if (m_OriginalPositions.empty() || aFrom.m_Size == 0 ||
						aFrom.m_Begin >= m_OriginalPositions.size())
						return TPhraseSubrange();

					UInt32 resultSize = 0;
					if (aFrom.m_Begin + aFrom.m_Size >= m_OriginalPositions.size())
						resultSize = m_OriginalPositions.back() + 1 - m_OriginalPositions[aFrom.m_Begin];
					else
						resultSize = m_OriginalPositions[aFrom.m_Begin + aFrom.m_Size] - m_OriginalPositions[aFrom.m_Begin];

					TPhraseSubrange result;
					result.m_Begin = m_OriginalPositions[aFrom.m_Begin];
					result.m_Size = resultSize;

					return result;
				}

			private:
				CSldVector<UInt32> m_OriginalPositions;
			};

			result.m_RangePullback.reset(new MyPullback(sld2::move(originalPositions)));

			return result;
		}

	private:
		SldU16String m_Headword;
	};

	class StandardPhraseExpander : public CSldControlledTransformGenerator
	{
	public:
		explicit StandardPhraseExpander(SldU16String aHeadword) :
			m_Headword(sld2::move(aHeadword)) {}

		Int8 Init(const SldU16String& aPhrase) override
		{
			SetUpTildeReplacement(aPhrase);

			return 1;
		}

		Result Transform() override
		{
			m_End = 1;
			return { sld2::move(m_NoTildePhrase), sld2::move(m_TildePullback) };
		}

		Int8 End() const override
		{
			return m_End;
		}

	private:
		void SetUpTildeReplacement(const SldU16String& aPhrase)
		{
			TildeReplacer replacer(m_Headword);
			Result replacerResult = replacer.Act(aPhrase);
			m_NoTildePhrase = replacerResult.m_String;
			m_TildePullback = sld2::move(replacerResult.m_RangePullback);
		}

		SldU16String m_Headword;
		SldU16String m_NoTildePhrase;
		sld2::UniquePtr<CSldSubrangePullback> m_TildePullback;

		Int8 m_End = 0;
	};

	StandardSubphraseSearch search(GetCompare());
	StandardPhraseExpander exp(sld2::to_string(aHeadword));

	TFindBestSubphraseResult result = FindBestSubphrase(
		sld2::to_string(aText), sld2::to_string(aPhrase), exp, search);

	if (result.m_BestRange == TPhraseSubrange())
		return ESldError::eOK;

	aHighlightData.clear();
	aHighlightData.reserve(2 * (1 + wordsInPhrase.size()));

	aHighlightData.push_back(result.m_BestRange.m_Begin);
	aHighlightData.push_back(result.m_BestRange.m_Begin + result.m_BestRange.m_Size);

	CSldVector<TPhraseSubrange> words = search.GetSubphraseWords();
	for (UInt32 wordIdx : wordsInPhrase)
	{
		TPhraseSubrange realSubrange = 
			result.m_TransformResult.m_RangePullback->Act(words[wordIdx]);

		aHighlightData.push_back(realSubrange.m_Begin);
		aHighlightData.push_back(realSubrange.m_Begin + realSubrange.m_Size);
	}

	return ESldError::eOK;
}
