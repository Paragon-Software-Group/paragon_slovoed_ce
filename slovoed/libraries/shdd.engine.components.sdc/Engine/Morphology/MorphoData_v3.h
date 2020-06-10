#ifndef __MORPHO_DATA_INCLUDE_V3__
#define __MORPHO_DATA_INCLUDE_V3__

#include "IMorphoData.h"
#include "../ISldLayerAccess.h"

class CSldDictionary;

struct MorphoData_v3 : public IMorphoData
{
	MorphoData_v3(ISDCFile * aFile, ISldLayerAccess * aLayerAcces);

	~MorphoData_v3();

public:

	bool Init(CSDCReadMy&) { return 0; }
	void Close() {}

	ESldLanguage GetLanguageCode(void) const { return SldLanguage::fromCode(m_Header.language); }
	UInt32 GetMorphoEngineVersion() { return 0; }
	bool GetMorphoHeader(const MorphoDataHeader** aHeader) const;

	const MCHAR* ClassNameByRulesetPtr(const MorphoInflectionRulesSetHandle*) const{ return &m_CharBuff; }
	void GetBriefClassNameByRulesetPtr(const MorphoInflectionRulesSetHandle*, MCHAR*) const {}
	void GetBriefClassNameByRulesetPtrW(const MorphoInflectionRulesSetHandle*, UInt16*) const {}
	void GetFullClassNameByRulesetPtr(const MorphoInflectionRulesSetHandle*, MCHAR*) const {}
	void GetFullClassNameByRulesetPtrW(const MorphoInflectionRulesSetHandle*, UInt16*) const {}
	const MorphoStateDescriptionTableHandle* GetTableByRulesetPtr(const MorphoInflectionRulesSetHandle*) const { return 0; }

	UInt32 LetterAttributes(MCHAR) const{ return 0; }
	MCHAR ToLower(MCHAR) const{ return 0; }
	MCHAR ToUpper(MCHAR) const{ return 0; }

	bool IsRuleApplyable(const MCHAR*, const MorphoInflectionRulesSetHandle*, const MCHAR**, Int32) const { return 0; }

	void WordFormsIteratorInit(WordFormsIterator&, const MCHAR*, const MorphoInflectionRulesSetHandle*, Int32) const {}
	void WordFormsIteratorInitW(WordFormsIterator &iter, const UInt16 *wordPtr, const MorphoInflectionRulesSetHandle* ruleset, Int32 flags) const;
	bool GetNextWordForm(WordFormsIterator&, MCHAR*, const MCHAR**, const MCHAR**, bool * = 0) const { return 0; }
	bool GetNextWordFormW(WordFormsIterator &iter, UInt16 *formBuf, const MCHAR** questionPtr, const MCHAR** formNamePtr, bool *isSubTableName = 0) const;

	void WritingVersionIteratorInit(WritingVersionIterator&, const MCHAR*, UInt32&, bool = false) const {}
	void WritingVersionIteratorInitW(WritingVersionIterator &iter, const UInt16 *wordForm, UInt32 &length, bool strict = false) const;
	bool GetNextWritingVersion(WritingVersionIterator&, MCHAR*) const { return 0; }
	bool GetNextWritingVersionW(WritingVersionIterator &iter, UInt16* outBuf) const;

	void BaseFormsIteratorInit(BaseFormsIterator&, const MCHAR*, Int32) const {}
	void BaseFormsIteratorInitW(BaseFormsIterator &iter, const UInt16 *wordFormPtr, Int32 flags) const;
	bool GetNextBaseForm(BaseFormsIterator&, MCHAR*, const MorphoInflectionRulesSetHandle**) const { return 0; }
	bool GetNextBaseFormW(BaseFormsIterator &iter, UInt16 *baseFormBuf, const MorphoInflectionRulesSetHandle** rulesetPtr) const;

	bool LanguageSpecificTrials(BaseFormsIterator&, MCHAR*, const MorphoInflectionRulesSetHandle**, UInt32) const{ return 0; }

	UInt32 GetNumberOfBaseForms(void) const;
	UInt32 GetNumberOfWordForms(void) const;

	bool IsMorphologySimple(void) const { return 0; }

	bool Enum(bool, EnumCallBackT*, void*) const { return 0; }

	bool EnumPartial(EnumPartialCallBackT* callback, UInt32 aBaseFormIndexFirst, UInt32 aBaseFormIndexLast, void* aParam) const;

private:

	sld2::UniquePtr<CSldDictionary>	m_Dictionary;

	MorphoDataHeader		m_Header;

	Int32					m_BaseFormListIndex;

	Int32					m_InflectionListIndex;

	bool					m_IsInit;

	MCHAR					m_CharBuff;
};

#endif
