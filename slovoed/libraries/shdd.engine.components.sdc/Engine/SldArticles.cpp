#include "SldArticles.h"

#include "SldTools.h"
#include "SldSerialNumber.h"

// initializes a styles array
static ESldError initStyles(const TDictionaryHeader &aHeader, CSDCReadMy &aReader,
							sld2::DynArray<sld2::UniquePtr<CSldStyleInfo>> &aStyles)
{
	ESldError error;
	if (aHeader.HasPackedStyles)
	{
		const UInt32 styleCount = aHeader.NumberOfStyles;
		for (UInt32 resourceIndex = 0, styleIndex = 0; styleIndex < styleCount; resourceIndex++)
		{
			auto res = aReader.GetResource(aHeader.articlesStyleType, resourceIndex);
			if (res != eOK)
				return res.error();

			for (const UInt8 *ptr = res.ptr(), *end = res.ptr() + res.size();
				 ptr < end && styleIndex < styleCount; styleIndex++)
			{
				error = aStyles[styleIndex]->Init(&ptr, styleIndex);
				if (error != eOK)
					return error;
			}
		}
	}
	else
	{
		for (UInt32 i = 0; i < aHeader.NumberOfStyles; i++)
		{
			error = aStyles[i]->Init(aReader, aHeader.articlesStyleType, i);
			if (error != eOK)
				return error;
		}
	}

	return eOK;
}

/**
 * Конструктор
 *
 * @param[in]  aDict        - ссылка на объект словаря
 * @param[in]  aLayerAccess
 * @param[in]  aData        - ссылка на объект отвечающий за получение данных из контейнера
 * @param[in]  aHeader      - ссылка на заголовок словаря в котором находятся данные
 *                            необходимые для работы данного класса
 * @param[out] aError       - статус инициализации объекта
 *
 * Если aError != eOK объект находится в "неопределенном" состоянии с единственным
 * возможноым (определенным) действием - деструкцией
 */
CSldArticles::CSldArticles(CSldDictionary &aDict, ISldLayerAccess &aLayerAccess, CSDCReadMy &aData,
						   const TDictionaryHeader &aHeader, ESldError &aError) :
	m_LayerAccess(aLayerAccess),
	m_CurrentIndex(-1),
	m_WordtypeIndex(0),
	m_Dictionary(aDict),
	m_TranslationComplete(1),
	m_RegistrationData(NULL)
{
#define error_out(_err) do { aError = _err; return; } while(0)

	ESldError error;

	SldInitRandom(&m_RandomSeed, (UInt32)((UInt4Ptr)&aData));

	m_Input = sld2::CreateInput(aHeader.ArticlesCompressionMethod);
	if (!m_Input)
		error_out(eCommonWrongCompressionType);
	error = m_Input->Init(aData, aHeader.articlesDataType, aHeader.articlesTreeType, aHeader.NumberOfStyles, aHeader.articlesQAType, 0);
	if (error != eOK)
		error_out(error);

	m_Input->SetHASH(aHeader.HASH);

	if (!m_StylesInfo.resize(aHeader.NumberOfStyles))
		error_out(eMemoryNotEnoughMemory);

	// allocate all of the styles in one go upfront
	for (auto&& style : m_StylesInfo)
	{
		style = sld2::make_unique<CSldStyleInfo>();
		if (!style)
			error_out(eMemoryNotEnoughMemory);
	}

	error = initStyles(aHeader, aData, m_StylesInfo);
	if (error != eOK)
		error_out(error);

	if (!m_Wordtype.resize(aHeader.WordtypeSize))
		error_out(eMemoryNotEnoughMemory);

	if (!m_Buffer.resize(aHeader.ArticlesBufferSize))
		error_out(eMemoryNotEnoughMemory);
/*
	ESldError (ISldLayerAccess::*MyBuild[2])(const CSldDictionary *aDictionary, 
		const UInt16 *aText, ESldTranslationModeType aTree);
	MyBuild[0] = &ISldLayerAccess::BuildTranslation;
	MyBuild[1] = &ISldLayerAccess::BuildTranslation;
	FTranslationBuilderMethodPtr tmpPtr = &ISldLayerAccess::BuildTranslation;
	m_TranslationBuilder[1] = &ISldLayerAccess::BuildTranslation;

	(m_LayerAccess->*MyBuild[0])(NULL, NULL, eTranslationStart);
//	m_TranslationBuilder[1] = m_LayerAccess->BuildTranslation;
*/
	m_TranslationBuilder[1] = &ISldLayerAccess::BuildTranslationWrong;
	m_TranslationBuilder[2] = &ISldLayerAccess::BuildTranslationRight;
	
	// how to call: (m_LayerAccess->*m_TranslationBuilder[2])(NULL, NULL, eTranslationStart);
	
	SLD_VALIDATE_SECURITY(m_RegistrationData, &m_RandomSeed, m_TranslationBuilder);

	aError = eOK;
#undef error_out
}

/** ********************************************************************
* Производит перевод указанной статьи(из глобального списка переводов) 
*
* @param[in]	aIndex		- номер перевода среди всех переводов
* @param[in]	aFullness	- степень полноты полученного перевода
*
* @return код ошибки
************************************************************************/
ESldError CSldArticles::Translate(Int32 aIndex, ESldTranslationFullnesType aFullness, UInt32 aStartBlock, UInt32 aEndBlock)
{
	ESldError error;
	// TODO: Сделать оптимизацию поиска, т.к. возможно недавно мы искали что-то в данной окресности
	UInt32 low = 0;
	UInt32 hi = m_Input->GetSearchPointCount();
	UInt32 med = 0;
	Int32 wordIndex;

	while (hi-low > 1)
	{
		med = (hi+low)>>1;
		error = m_Input->GetSearchPointIndex(med, &wordIndex);
		if (error != eOK)
			return error;

		if (wordIndex < aIndex)
		{
			low = med;
		}else
		{
			hi = med;
		}
	}

	error = m_Input->GetSearchPointIndex(low, &m_CurrentIndex);
	if (error != eOK)
		return error;
	error = m_Input->GoTo(low);
	if (error != eOK)
		return error;

	while (m_CurrentIndex < aIndex)
	{
		error = GetNextArticle(m_CurrentIndex+1 < aIndex? eTranslationWithoutTranslation : aFullness, aStartBlock, aEndBlock );
		if (error != eOK)
			return error;
	}

	return eOK;
}

/** ********************************************************************
* Производит получение остатков перевода 
*
* @param[in]	aFullness	- степень полноты полученного перевода
*
* @return код ошибки
************************************************************************/
ESldError CSldArticles::TranslateContinue(ESldTranslationFullnesType aFullness)
{
	if (m_TranslationComplete)
		return eCommonTranslationCompleted;

	ESldError error;
	error = GetNextArticle( aFullness );
	if (error != eOK)
		return error;

	return eOK;
}

/** *********************************************************************
* Производит очистку состояния перевода
*
* @return код ошибки
************************************************************************/
ESldError CSldArticles::TranslateClear()
{
	if (m_TranslationComplete)
		return eCommonTranslationCompleted;

	sldMemZero(m_Wordtype.data(), sizeof(m_Wordtype[0]) * m_Wordtype.size());
	sldMemZero(m_Buffer.data(), sizeof(m_Buffer[0]) * m_Buffer.size());

	m_WordtypeIndex = 0;
	m_TranslationComplete = 1;

	return eOK;
}

// block text fixup function
namespace {

UInt32 fixupBlockText(UInt16 *aBuffer, UInt32 aLength, const CSldStyleInfo *aStyle)
{
	// replace '\%0[aA]' sequence with newlines, can be only at the end of the string
	if (aLength >= 4 && sld2::StrNCmp(&aBuffer[aLength - 4], SldU16StringLiteral("\\%0"), 3) == 0 &&
		(aBuffer[aLength - 1] == 'a' || aBuffer[aLength - 1] == 'A'))
	{
		aBuffer[aLength - 4] = '\n';
		aBuffer[aLength - 3] = '\0';
		aLength -= 3;
	}

	// add prefix & postfix
	const SldU16StringRef prefix = aStyle->GetPrefixRef();
	const SldU16StringRef postfix = aStyle->GetPostfixRef();
	if (prefix.size() && postfix.size())
	{
		if (prefix.size())
		{
			sld2::memmove_n(aBuffer + prefix.size(), aBuffer, aLength + 1);
			sld2::memcopy_n(aBuffer, prefix.data(), prefix.size());
			aLength += prefix.size();
		}

		if (postfix.size())
		{
			sld2::memcopy_n(aBuffer + aLength, postfix.data(), postfix.size() + 1);
			aLength += postfix.size();
		}
	}

	return aLength;
}

}

/** ********************************************************************
* Декодирует следующую статью 
*
* @param[in]	aFullness	- полнота перевода
*
* @return код ошибки
************************************************************************/
ESldError CSldArticles::GetNextArticle(ESldTranslationFullnesType aFullness, UInt32 aStartBlock, UInt32 aEndBlock)
{
	SLD_VALIDATE_SECURITY(m_RegistrationData, &m_RandomSeed, m_TranslationBuilder);
	ESldError error;
	// TODO: если у нас не был закончен перевод, то мы не получаем заново типы блоков текста, однако там содержится мусор,
	// хотя должны быть данные, оставшиеся от предыдущего вызова
	// (можно проверить, два раза подряд запросив перевод одного и того же слова, во время первого вызова приостановив получение перевода)
	if (m_TranslationComplete)
	{
		m_CurrentIndex++;
		error = m_Input->GetText(SLD_DECORER_TYPE_STYLES, m_Wordtype);
		if (error != eOK)
			return error;
	}

	const UInt16 *wordType = m_Wordtype.data() + m_WordtypeIndex;

	if (aFullness != eTranslationWithoutTranslation)
	{
		error = (m_LayerAccess.*m_TranslationBuilder[0])(&m_Dictionary, NULL, m_TranslationComplete?eTranslationStart:eTranslationContinue);
		if (error != eOK)
			return error;

		m_TranslationComplete = 0;
		for (UInt32 currentBlock = 0; *wordType; wordType++, currentBlock++)
		{
			UInt32 bufLen;
			error = m_Input->GetText(*wordType, m_Buffer, &bufLen);
			if (error != eOK)
				return error;

			if (currentBlock >= aStartBlock && currentBlock <= aEndBlock)
			{
				const auto& style = m_StylesInfo[*wordType];
				if (style->GetStyleMetaType() == eMetaText || style->GetStyleMetaType() == eMetaPhonetics)
					fixupBlockText(m_Buffer.data(), bufLen, style.get());

				error = (m_LayerAccess.*m_TranslationBuilder[0])(&m_Dictionary, m_Buffer.data(), (ESldTranslationModeType)*wordType);
				if (error == eExceptionTranslationPause)
				{
					//////////////////////////////////////////////////////////////////////////
					// Нас просят приостановить получение перевода !
					//////////////////////////////////////////////////////////////////////////
					m_WordtypeIndex = (wordType - m_Wordtype.data()) + 1;
					return eOK;
				}
				if (error != eOK)
					return error;
			}
		}

		error = (m_LayerAccess.*m_TranslationBuilder[0])(&m_Dictionary, NULL, eTranslationStop);
		if (error != eOK)
			return error;

		m_RegistrationData->Clicks++;
	}
	else
	{
		m_TranslationComplete = 0;
		for (; *wordType; wordType++)
		{
			error = m_Input->GetText(*wordType, m_Buffer);
			if (error != eOK)
				return error;
		}
	}

	m_TranslationComplete = 1;
	m_WordtypeIndex = 0;

	//SLD_VALIDATE_SECURITY(m_RegistrationData, &m_RandomSeed, m_TranslationBuilder);

	return eOK;
}

/** ********************************************************************
* Возвращает класс хранящий информацию о указанном стиле
*
* @param[in]	aIndex		- номер стиля
* @param[out]	aStyleInfo	- по данному указателю будет записан адрес класса 
*							  который хранит информацию о стиле
*
* @return код ошибки
************************************************************************/
ESldError CSldArticles::GetStyleInfo(UInt32 aIndex, const CSldStyleInfo **aStyleInfo) const
{
	if (aIndex >= m_StylesInfo.size())
		return eCommonWrongIndex;
	if (!aStyleInfo)
		return eMemoryNullPointer;

	*aStyleInfo = m_StylesInfo[aIndex].get();

	return eOK;
}

/** *********************************************************************
* Устанавливает для стиля вариант отображения по умолчанию
*
* @param[in]	aStyleIndex		- номер стиля
* @param[in]	aVariantIndex	- номер варианта отображения
*
* @return код ошибки
************************************************************************/
ESldError CSldArticles::SetDefaultStyleVariant(Int32 aStyleIndex, Int32 aVariantIndex)
{
	if (aStyleIndex >= m_StylesInfo.size() || aStyleIndex < 0)
		return eCommonWrongIndex;
	
	if (!m_StylesInfo[aStyleIndex])
		return eMemoryNullPointer;
	
	ESldError error = m_StylesInfo[aStyleIndex]->SetDefaultVariantIndex(aVariantIndex);
	if (error != eOK)
		return error;
	
	return eOK;
}

/** *********************************************************************
* Возвращает количество стилей
*
* @return количество стилей
************************************************************************/
UInt32 CSldArticles::GetNumberOfStyles() const
{
	return m_StylesInfo.size();
}

/** *********************************************************************
* Устанавливает указатель на данные, необходимые для системы защиты
*
* @param[in]	aRegData	- указатель на структуру с регистрационной информацией
*
* @return код ошибки
************************************************************************/
ESldError CSldArticles::SetRegisterData(TRegistrationData *aRegData)
{
	m_RegistrationData = aRegData;
	m_Input->SetHASH(m_RegistrationData->HASH);

	SLD_VALIDATE_SECURITY(m_RegistrationData, &m_RandomSeed, m_TranslationBuilder);
	
	return eOK;
}

/** *********************************************************************
* Возвращает содержимое статьи
*
* @param[in]	aIndex		- индекс статьи, перевод которой мы хотим получить
* @param[out]	aArticle	- разобранная на блоки статья
* @param[in]	aStopType	- тип метаданных, встретив который, мы закончим получение статьи
*							  в результат этот блок не попадет
*
* @return код ошибки
************************************************************************/
ESldError CSldArticles::GetArticle(const Int32 aIndex, SplittedArticle & aArticle, const ESldStyleMetaTypeEnum aStopType)
{
	ESldError error = Translate(aIndex - 1, eTranslationWithoutTranslation);
	if (error != eOK)
		return error;

	UInt32 wordTypeLen;
	error = m_Input->GetText(SLD_DECORER_TYPE_STYLES, m_Wordtype, &wordTypeLen);
	if (error != eOK)
		return error;

	const UInt16 *wordType = m_Wordtype.data() + m_WordtypeIndex;
	const UInt32 blocksCount = wordTypeLen - m_WordtypeIndex;
	for (UInt32 blockIndex = 0; blockIndex < blocksCount; blockIndex++)
	{
		const Int32 styleIndex = wordType[blockIndex];

		UInt32 bufLen;
		error = m_Input->GetText(styleIndex, m_Buffer, &bufLen);
		if (error != eOK)
			return error;

		const auto& style = m_StylesInfo[styleIndex];
		const auto styleType = style->GetStyleMetaType();

		if (styleType == aStopType)
			break;

		if (styleType == eMetaText || styleType == eMetaPhonetics)
			bufLen = fixupBlockText(m_Buffer.data(), bufLen, style.get());

		aArticle.emplace_back(styleIndex, SldU16StringRef(m_Buffer.data(), bufLen));
	}

	return eOK;
}