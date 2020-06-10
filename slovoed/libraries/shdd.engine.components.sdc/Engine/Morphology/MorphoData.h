#ifndef __MORPHO_DATA_INCLUDE___
#define __MORPHO_DATA_INCLUDE___


#include "MorphoData_v1.h"
#include "MorphoData_v2.h"
#include "MorphoData_v3.h"

struct MorphoData
{
private:
	sld2::UniquePtr<IMorphoData> imorpho;
	/// Структура источника с данными (с ее помощью производится чтение из SDC-файла)
	CSDCReadMy	m_database;
	UInt32 initFlag;

public:

	MorphoData();
	~MorphoData();

	bool Init(ISDCFile *aFile, ISldLayerAccess * aLayerAcces);
	void Close();
	bool IsInit() const;

	
	UInt32 GetLanguageCode(void) const;
	UInt32 GetMorphoEngineVersion();
	bool GetMorphoHeader(const MorphoDataHeader** aHeader) const;
	
	const MCHAR* ClassNameByRulesetPtr(const MorphoInflectionRulesSetHandle* ruleset_ptr) const;
	void GetBriefClassNameByRulesetPtr(const MorphoInflectionRulesSetHandle* ruleset, MCHAR *buf) const;
	void GetBriefClassNameByRulesetPtrW(const MorphoInflectionRulesSetHandle* ruleset, UInt16* buf) const;
	void GetFullClassNameByRulesetPtr(const MorphoInflectionRulesSetHandle* ruleset, MCHAR* buf) const;
	void GetFullClassNameByRulesetPtrW(const MorphoInflectionRulesSetHandle* ruleset, UInt16* buf) const;
	const MorphoStateDescriptionTableHandle* GetTableByRulesetPtr(const MorphoInflectionRulesSetHandle* ruleset_ptr) const;
	
	
	Int32 StrNCMP(UInt8* szString1, UInt8* szString2, Int32 nSize) const;
	Int8* StrTok(Int8** szToken, const Int8* szDelimit) const;
	Int8* StrReverse(Int8* szString) const;
	
	
	UInt32 LetterAttributes(MCHAR ch) const;
	MCHAR ToLower(MCHAR ch) const;
	MCHAR ToUpper(MCHAR ch) const;
	
	
	bool IsRuleApplyable(const MCHAR *word, const MorphoInflectionRulesSetHandle* ruleset_ptr, const MCHAR **posBuf, Int32 flags) const;
	
	
	void WordFormsIteratorInit(WordFormsIterator &iter, const MCHAR *wordPtr, const MorphoInflectionRulesSetHandle* ruleset, Int32 flags) const;
	void WordFormsIteratorInitW(WordFormsIterator &iter, const UInt16 *wordPtr, const MorphoInflectionRulesSetHandle* ruleset, Int32 flags) const;
	bool GetNextWordForm(WordFormsIterator &iter, MCHAR *formBuf, const MCHAR** questionPtr, const MCHAR** formNamePtr, bool *isSubTableName = 0) const;
	bool GetNextWordFormW(WordFormsIterator &iter, UInt16 *formBuf, const MCHAR** questionPtr, const MCHAR** formNamePtr, bool *isSubTableName = 0) const;
	
	void WritingVersionIteratorInit(WritingVersionIterator &iter, const MCHAR *wordForm, UInt32 &length , bool strict = false) const;
	void WritingVersionIteratorInitW(WritingVersionIterator &iter, const UInt16 *wordForm, UInt32 &length , bool strict = false) const;
	bool GetNextWritingVersion(WritingVersionIterator &iter, MCHAR* outBuf) const;
	bool GetNextWritingVersionW(WritingVersionIterator &iter, UInt16* outBuf) const;
	
	
	void BaseFormsIteratorInit(BaseFormsIterator &iter, const MCHAR *wordFormPtr, Int32 flags) const;
	void BaseFormsIteratorInitW(BaseFormsIterator &iter, const UInt16 *wordFormPtr, Int32 flags) const;
	bool GetNextBaseForm(BaseFormsIterator &iter, MCHAR *baseFormBuf,  const MorphoInflectionRulesSetHandle** rulesetPtr) const;
	bool GetNextBaseFormW(BaseFormsIterator &iter, UInt16 *baseFormBuf, const MorphoInflectionRulesSetHandle** rulesetPtr) const;
	
	
	bool LanguageSpecificTrials(BaseFormsIterator &iter, MCHAR *baseFormBuf, const MorphoInflectionRulesSetHandle** rulesetPtr, UInt32 language) const;
	
	UInt32 GetNumberOfBaseForms(void) const;
	UInt32 GetNumberOfWordForms(void) const;
	
	bool IsMorphologySimple(void) const;
	
	bool Enum(bool bAllForms, EnumCallBackT* callback, void* param) const;
	
	bool EnumPartial(EnumPartialCallBackT* callback, UInt32 aBaseFormIndexFirst, UInt32 aBaseFormIndexLast, void* aParam) const;
	
};

#endif
