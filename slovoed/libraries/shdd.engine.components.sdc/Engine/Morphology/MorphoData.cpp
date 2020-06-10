#include "MorphoData.h"


MorphoData::MorphoData()
{
	initFlag = 0;
}

MorphoData::~MorphoData()
{
	Close();
}

bool MorphoData::Init(ISDCFile *aFile, ISldLayerAccess * aLayerAcces)
{
	if (!aFile || !aFile->IsOpened())
		return false;

	if (m_database.Open(aFile) != eOK)
		return false;

	if (m_database.GetDatabaseType() == eDatabaseType_InflectionMorphology)
	{
		imorpho = sld2::make_unique<MorphoData_v3>(aFile, aLayerAcces);

		const bool isInit = imorpho->GetMorphoHeader(NULL);
		if (isInit)
			initFlag = 1;

		return isInit;
	}
	// читаем заголовок
	auto resHeader = m_database.GetResource(MORPHODATA_RES_HEADER, 0);
	if (resHeader != eOK)
		return false;

	const MorphoDataHeader*	header = (MorphoDataHeader*)resHeader.ptr();
	const UInt32 format = MORPHO_SWAP_32(header->format);

	// проверяем версию формата
	if (MorphoData_v1::IsCoreSupportFormat(format))
	{
		imorpho = sld2::make_unique<MorphoData_v1>();
		if (!imorpho)
			return false;
	}
	else if (MorphoData_v2::IsCoreSupportFormat(format))
	{
		imorpho = sld2::make_unique<MorphoData_v2>();
		if (!imorpho)
			return false;
	}
	else
	{
		return false;
	}

	// Инициализируем объект
	if (!imorpho->Init(m_database))
		return false;

	// Все хорошо, инициализация прошла успешно
	initFlag = 1;
	return true;
}

void MorphoData::Close()
{
	imorpho = nullptr;
	m_database.Close();
}

UInt32 MorphoData::GetLanguageCode(void) const
{
	if (imorpho)
		return imorpho->GetLanguageCode();
	
	return 0;
}

void MorphoData::WordFormsIteratorInit(WordFormsIterator &iter, const MCHAR *wordPtr, const MorphoInflectionRulesSetHandle* ruleset, Int32 flags) const
{
	if (imorpho)
		imorpho->WordFormsIteratorInit(iter, wordPtr, ruleset, flags);
}

void MorphoData::WordFormsIteratorInitW(WordFormsIterator &iter, const UInt16 *wordPtr, const MorphoInflectionRulesSetHandle* ruleset, Int32 flags) const
{
	if (imorpho)
		imorpho->WordFormsIteratorInitW(iter, wordPtr, ruleset, flags);
}

bool MorphoData::GetNextWordForm(WordFormsIterator &iter, MCHAR *formBuf, const MCHAR** questionPtr, const MCHAR** formNamePtr, bool *isSubTableName) const
{
	if (imorpho)
		return imorpho->GetNextWordForm(iter, formBuf, questionPtr, formNamePtr, isSubTableName);
	
	return false;
}

bool MorphoData::GetNextWordFormW(WordFormsIterator &iter, UInt16 *formBuf, const MCHAR** questionPtr, const MCHAR** formNamePtr, bool *isSubTableName) const
{
	if (imorpho)
		return imorpho->GetNextWordFormW(iter, formBuf, questionPtr, formNamePtr, isSubTableName);
	
	return false;
}

UInt32 MorphoData::GetMorphoEngineVersion()
{
	if (imorpho)
		return imorpho->GetMorphoEngineVersion();
	
	return 0;
}

bool MorphoData::GetMorphoHeader(const MorphoDataHeader** aHeader) const
{
	if (imorpho)
		return imorpho->GetMorphoHeader(aHeader);
	
	return false;
}

void MorphoData::WritingVersionIteratorInit(WritingVersionIterator &iter, const MCHAR *wordForm, UInt32 &length , bool strict) const
{
	if (imorpho)
		imorpho->WritingVersionIteratorInit(iter, wordForm, length, strict);
}

void MorphoData::WritingVersionIteratorInitW(WritingVersionIterator &iter, const UInt16 *wordForm, UInt32 &length , bool strict) const
{
	if (imorpho)
		imorpho->WritingVersionIteratorInitW(iter, wordForm, length, strict);
}

bool MorphoData::GetNextWritingVersion(WritingVersionIterator &iter, MCHAR* outBuf) const
{
	if (imorpho)
		return imorpho->GetNextWritingVersion(iter, outBuf);
	
	return false;
}

bool MorphoData::GetNextWritingVersionW(WritingVersionIterator &iter, UInt16* outBuf) const
{
	if (imorpho)
		return imorpho->GetNextWritingVersionW(iter, outBuf);
	
	return false;
}

void MorphoData::BaseFormsIteratorInit(BaseFormsIterator &iter, const MCHAR *wordFormPtr, Int32 flags) const
{
	if (imorpho)
		imorpho->BaseFormsIteratorInit(iter, wordFormPtr, flags);
}

void MorphoData::BaseFormsIteratorInitW(BaseFormsIterator &iter, const UInt16 *wordFormPtr, Int32 flags) const
{
	if (imorpho)
		imorpho->BaseFormsIteratorInitW(iter, wordFormPtr, flags);
}

bool MorphoData::IsRuleApplyable(const MCHAR *word, const MorphoInflectionRulesSetHandle* ruleset_ptr, const MCHAR **posBuf, Int32 flags) const
{
	if (imorpho)
		return imorpho->IsRuleApplyable(word, ruleset_ptr, posBuf, flags);
	
	return false;
}

bool MorphoData::GetNextBaseForm(BaseFormsIterator &iter, MCHAR *baseFormBuf,  const MorphoInflectionRulesSetHandle** rulesetPtr) const
{
	if (imorpho)
		return imorpho->GetNextBaseForm(iter, baseFormBuf, rulesetPtr);
	
	return false;
}

bool MorphoData::GetNextBaseFormW(BaseFormsIterator &iter, UInt16 *baseFormBuf, const MorphoInflectionRulesSetHandle** rulesetPtr) const
{
	if (imorpho)
		return imorpho->GetNextBaseFormW(iter, baseFormBuf, rulesetPtr);
	
	return false;
}

UInt32 MorphoData::GetNumberOfBaseForms(void) const
{
	if (imorpho)
		return imorpho->GetNumberOfBaseForms();
	return 0;
}

UInt32 MorphoData::GetNumberOfWordForms(void) const
{
	if (imorpho)
		return imorpho->GetNumberOfWordForms();
	return 0;
}

bool MorphoData::IsMorphologySimple(void) const
{
	if (imorpho)
		return imorpho->IsMorphologySimple();
	return false;
}

bool MorphoData::LanguageSpecificTrials(BaseFormsIterator &iter, MCHAR *baseFormBuf, const MorphoInflectionRulesSetHandle** rulesetPtr, UInt32 language) const
{
	if (imorpho)
		return imorpho->LanguageSpecificTrials(iter, baseFormBuf, rulesetPtr, language);
	
	return false;
}

const MorphoStateDescriptionTableHandle* MorphoData::GetTableByRulesetPtr(const MorphoInflectionRulesSetHandle* ruleset_ptr) const
{
	if (imorpho)
		return imorpho->GetTableByRulesetPtr(ruleset_ptr);
	
	return NULL;
}

const MCHAR* MorphoData::ClassNameByRulesetPtr(const MorphoInflectionRulesSetHandle* ruleset_ptr) const
{
	if (imorpho)
		return imorpho->ClassNameByRulesetPtr(ruleset_ptr);
	
	return NULL;
}

void MorphoData::GetBriefClassNameByRulesetPtr(const MorphoInflectionRulesSetHandle* ruleset, MCHAR* buf) const
{
	if (imorpho)
		imorpho->GetBriefClassNameByRulesetPtr(ruleset, buf);
}

void MorphoData::GetBriefClassNameByRulesetPtrW(const MorphoInflectionRulesSetHandle* ruleset, UInt16* buf) const
{
	if (imorpho)
		imorpho->GetBriefClassNameByRulesetPtrW(ruleset, buf);
}

void MorphoData::GetFullClassNameByRulesetPtr(const MorphoInflectionRulesSetHandle* ruleset, MCHAR* buf) const
{
	if (imorpho)
		imorpho->GetFullClassNameByRulesetPtr(ruleset, buf);
}

void MorphoData::GetFullClassNameByRulesetPtrW(const MorphoInflectionRulesSetHandle* ruleset, UInt16* buf) const
{
	if (imorpho)
		imorpho->GetFullClassNameByRulesetPtrW(ruleset, buf);
}

Int32 MorphoData::StrNCMP(UInt8* szString1, UInt8* szString2, Int32 nSize) const
{
	return sld2::StrNCmp(szString1, szString2, nSize);
}

Int8* MorphoData::StrTok(Int8** szToken, const Int8* szDelimit) const
{
	if (imorpho)
		return imorpho->StrTok(szToken, szDelimit);
	
	return NULL;
}

Int8* MorphoData::StrReverse(Int8* szString) const
{
	if (imorpho)
		return imorpho->StrReverse(szString);
	
	return NULL;
}

UInt32 MorphoData::LetterAttributes(MCHAR ch) const
{
	if (imorpho)
		return imorpho->LetterAttributes(ch);
	
	return 0;
}

MCHAR MorphoData::ToLower(MCHAR ch) const
{
	if (imorpho)
		return imorpho->ToLower(ch);
	
	return 0;
}

MCHAR MorphoData::ToUpper(MCHAR ch) const
{
	if (imorpho)
		return imorpho->ToUpper(ch);
	
	return 0;
}

bool MorphoData::Enum(bool bAllForms, EnumCallBackT* callback, void* param) const
{
	if (imorpho)
		return imorpho->Enum(bAllForms, callback, param);
	
	return false;
}

bool MorphoData::EnumPartial(EnumPartialCallBackT* callback, UInt32 aBaseFormIndexFirst, UInt32 aBaseFormIndexLast, void* aParam) const
{
	if (imorpho)
		return imorpho->EnumPartial(callback, aBaseFormIndexFirst, aBaseFormIndexLast, aParam);
	return false;
}

bool MorphoData::IsInit() const
{
	return initFlag != 0;
}
