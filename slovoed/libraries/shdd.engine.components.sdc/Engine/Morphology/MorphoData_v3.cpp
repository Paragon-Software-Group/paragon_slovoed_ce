#include "MorphoData_v3.h"
#include "../SldDictionary.h"


MorphoData_v3::MorphoData_v3(ISDCFile * aFile, ISldLayerAccess * aLayerAcces) :
	m_BaseFormListIndex(SLD_DEFAULT_LIST_INDEX),
	m_InflectionListIndex(SLD_DEFAULT_LIST_INDEX),
	m_IsInit(false),
	m_CharBuff(0)
{
	sldMemZero(&m_Header, sizeof(m_Header));
	m_Header.HeaderSize = sizeof(m_Header);

	m_Dictionary = sld2::make_unique<CSldDictionary>();
	if (!m_Dictionary)
		return;

	if (m_Dictionary->Open(aFile, aLayerAcces) != eOK)
		return;

	if (m_Dictionary->GetLanguageFrom() == SldLanguage::Unknown)
		return;
	m_Header.language = m_Dictionary->GetLanguageFrom();

	if (m_Dictionary->GetDictionaryID(&m_Header.DictID) != eOK)
		return;

	Int32 listCount = 0;
	if (m_Dictionary->GetNumberOfLists(&listCount) != eOK)
		return;

	for (UInt32 listIndex = 0; listIndex < listCount; listIndex++)
	{
		UInt32 listUsage = 0;
		if (m_Dictionary->GetListUsage(listIndex, 0, &listUsage) != eOK)
			return;

		if (listUsage == eWordListType_MorphologyBaseForm)
			m_BaseFormListIndex = listIndex;

		if (listUsage == eWordListType_MorphologyInflectionForm)
			m_InflectionListIndex = listIndex;
	}

	if (m_BaseFormListIndex == SLD_DEFAULT_LIST_INDEX || m_InflectionListIndex == SLD_DEFAULT_LIST_INDEX)
		return;

	m_IsInit = true;
}

MorphoData_v3::~MorphoData_v3() {}

bool MorphoData_v3::GetMorphoHeader(const MorphoDataHeader** aHeader) const
{
	if (aHeader)
		*aHeader = &m_Header;
	return m_IsInit;
}

void MorphoData_v3::BaseFormsIteratorInitW(BaseFormsIterator &iter, const UInt16 *wordFormPtr, Int32 flags) const
{
	iter.baseForms.clear();

	if (m_Dictionary->SetCurrentWordlist(m_InflectionListIndex) != eOK)
		return;

	UInt32 result = 0;
	if (m_Dictionary->GetWordByTextExtended(wordFormPtr, &result) != eOK)
		return;

	if (result == 0)
		return;

	Int32 currentIndex = SLD_DEFAULT_WORD_INDEX;
	if (m_Dictionary->GetCurrentGlobalIndex(&currentIndex) != eOK)
		return;

	Int32 baseWordsCount = 0;
	if (m_Dictionary->GetRealIndexesCount(currentIndex, &baseWordsCount) != eOK)
		return;

	if (baseWordsCount == 0)
		return;

	for (Int32 i = 0; i < baseWordsCount; i++)
	{
		Int32 realListIndex = SLD_DEFAULT_LIST_INDEX;
		Int32 realWordIndex = SLD_DEFAULT_WORD_INDEX;

		if (m_Dictionary->GetRealIndexes(currentIndex, i, &realListIndex, &realWordIndex) != eOK)
			return;

		UInt16* currentWord = NULL;
		if (m_Dictionary->GetWordByGlobalIndex(realListIndex, realWordIndex, 0, &currentWord) != eOK)
			return;

		iter.baseForms.push_back(currentWord);
	}
}
bool MorphoData_v3::GetNextBaseFormW(BaseFormsIterator &iter, UInt16 *baseFormBuf, const MorphoInflectionRulesSetHandle** rulesetPtr) const
{
	if (iter.baseForms.empty())
		return false;

	CSldCompare::StrCopy(baseFormBuf, iter.baseForms.back().c_str());
	iter.baseForms.erase(iter.baseForms.size() - 1, 1);
	return true;
}

void MorphoData_v3::WritingVersionIteratorInitW(WritingVersionIterator &iter, const UInt16 *wordForm, UInt32 &length, bool strict) const
{
	iter.string = wordForm;
}

bool MorphoData_v3::GetNextWritingVersionW(WritingVersionIterator &iter, UInt16* outBuf) const
{
	if (iter.string.size())
	{
		CSldCompare::StrCopy(outBuf, iter.string.c_str());
		iter.string.clear();
		return true;
	}
	return false;
}

void MorphoData_v3::WordFormsIteratorInitW(WordFormsIterator &iter, const UInt16 *wordPtr, const MorphoInflectionRulesSetHandle* ruleset, Int32 flags) const
{
	iter.wordForms.clear();

	if (m_Dictionary->SetCurrentWordlist(m_BaseFormListIndex) != eOK)
		return;

	UInt32 result = 0;
	if (m_Dictionary->GetWordByTextExtended(wordPtr, &result) != eOK)
		return;

	if (result == 0)
		return;

	Int32 currentIndex = SLD_DEFAULT_WORD_INDEX;
	if (m_Dictionary->GetCurrentGlobalIndex(&currentIndex) != eOK)
		return;

	Int32 wordFormsCount = 0;
	if (m_Dictionary->GetRealIndexesCount(currentIndex, &wordFormsCount) != eOK)
		return;

	if (wordFormsCount == 0)
		return;

	for (Int32 i = 0; i < wordFormsCount; i++)
	{
		Int32 realListIndex = SLD_DEFAULT_LIST_INDEX;
		Int32 realWordIndex = SLD_DEFAULT_WORD_INDEX;

		if (m_Dictionary->GetRealIndexes(currentIndex, i, &realListIndex, &realWordIndex) != eOK)
			return;

		UInt16* currentWord = NULL;
		if (m_Dictionary->GetWordByGlobalIndex(realListIndex, realWordIndex, 0, &currentWord) != eOK)
			return;

		iter.wordForms.push_back(currentWord);
	}

	return;
}

bool MorphoData_v3::GetNextWordFormW(WordFormsIterator &iter, UInt16 *formBuf, const MCHAR** questionPtr, const MCHAR** formNamePtr, bool *isSubTableName) const
{
	if (iter.wordForms.empty())
		return false;

	CSldCompare::StrCopy(formBuf, iter.wordForms.back().c_str());
	iter.wordForms.erase(iter.wordForms.size() - 1, 1);
	return true;
}

UInt32 MorphoData_v3::GetNumberOfBaseForms(void) const
{
	Int32 result = 0;

	if (m_Dictionary->GetNumberOfWords(m_BaseFormListIndex, &result) != eOK)
		return 0;

	return result;
}

UInt32 MorphoData_v3::GetNumberOfWordForms(void) const
{
	Int32 result = 0;

	if (m_Dictionary->GetNumberOfWords(m_InflectionListIndex, &result) != eOK)
		return 0;

	return result;
}

bool MorphoData_v3::EnumPartial(EnumPartialCallBackT* callback, UInt32 aBaseFormIndexFirst, UInt32 aBaseFormIndexLast, void* aParam) const
{
	UInt16* baseWord = nullptr;
	UInt8 baseWordU8[1024];
	UInt16* wordForm = nullptr;
	UInt8 wordFormU8[1024];
	Int32 realListIndex = SLD_DEFAULT_LIST_INDEX;
	Int32 realWordIndex = SLD_DEFAULT_WORD_INDEX;
	Int32 wordFormCount = 0;

	const CSldCompare* cmp = nullptr;
	if (m_Dictionary->GetCompare(&cmp) != eOK)
		return false;

	for (UInt32 baseFormIndex = 0; baseFormIndex < aBaseFormIndexLast; baseFormIndex++)
	{
		if (m_Dictionary->GetWordByGlobalIndex(m_BaseFormListIndex, baseFormIndex, 0, &baseWord) != eOK)
			return false;

		if (cmp->Unicode2ASCIIByLanguage(baseWord, baseWordU8, GetLanguageCode()) != eOK)
			return false;

		if (m_Dictionary->GetRealIndexesCount(m_BaseFormListIndex, baseFormIndex, &wordFormCount) != eOK)
			return false;

		for (UInt32 wordFormIndex = 0; wordFormIndex < wordFormCount; wordFormIndex++)
		{
			if (m_Dictionary->GetRealIndexes(m_BaseFormListIndex, baseFormIndex, wordFormIndex, &realListIndex, &realWordIndex) != eOK)
				return false;

			if (m_Dictionary->GetWordByGlobalIndex(realListIndex, realWordIndex, 0, &wordForm) != eOK)
				return false;

			if (cmp->Unicode2ASCIIByLanguage(wordForm, wordFormU8, GetLanguageCode()) != eOK)
				return false;

			callback((MCHAR*)wordFormU8, (MCHAR*)baseWordU8, aParam);
		}
	}
	return false;
}