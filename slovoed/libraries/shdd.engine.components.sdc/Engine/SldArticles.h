#ifndef _C_SLD_ARTICLES_H_
#define _C_SLD_ARTICLES_H_
#include "SldError.h"
#include "ISldInput.h"
#include "SldSDCReadMy.h"
#include "ISldLayerAccess.h"
#include "SldStyleInfo.h"
#include "SldCompare.h"
#include "SldDefines.h"



/// Класс занимающийся работой с переводами статей.
class CSldArticles
{
public:
	/// Конструктор
	CSldArticles(CSldDictionary &aDict, ISldLayerAccess &aLayerAccess, CSDCReadMy &aData,
				 const TDictionaryHeader &aHeader, ESldError &aError);

	/// Устанавливает указатель на данные, необходимые для системы защиты
	ESldError SetRegisterData(TRegistrationData *aRegData);

	/// Производит перевод указанной статьи(из текущего списка слов) 
	ESldError Translate(Int32 aIndex, ESldTranslationFullnesType aFullness,  UInt32 aStartBlock = 0, UInt32 aEndBlock = -1);

	/// Производит получение остатков перевода 
	ESldError TranslateContinue(ESldTranslationFullnesType aFullness);

	/// Производит очистку состояния перевода
	ESldError TranslateClear();

	/// Возвращает класс хранящий информацию о указанном стиле.
	ESldError GetStyleInfo(UInt32 aIndex, const CSldStyleInfo **aStyleInfo) const;

	/// Устанавливает для стиля вариант отображения по умолчанию
	ESldError SetDefaultStyleVariant(Int32 aStyleIndex, Int32 aVariantIndex);

	///  Возвращает количество стилей
	UInt32 GetNumberOfStyles() const;

	/// Возвращает содержимое статьи
	ESldError GetArticle(const Int32 aIndex, SplittedArticle & aArticle, const ESldStyleMetaTypeEnum aStopType = eMetaUnknown);

private:
	/// Декодирует следующую статью
	ESldError GetNextArticle(ESldTranslationFullnesType aFullness, UInt32 aStartBlock = 0, UInt32 aEndBlock = -1);
	
private:
	/// Класс отвечающий за получение данных.
	sld2::UniquePtr<ISldInput> m_Input;

	/// Класс отвечающий за взаимодействие ядра и программы.
	ISldLayerAccess &m_LayerAccess;

	/// Номер текущего индекса.
	Int32 m_CurrentIndex;

	/// Тип перевода
	sld2::DynArray<UInt16> m_Wordtype;

	/// Номер типа перевода с которого нужно продолжить декодирование перевода.
	UInt32 m_WordtypeIndex;

	/// Буфер для получения декодированных данных.
	sld2::DynArray<UInt16> m_Buffer;

	/// Указатель на класс словаря в котором находятся данные переводы.
	const CSldDictionary &m_Dictionary;

	/// Информация о стилях
	sld2::DynArray<sld2::UniquePtr<CSldStyleInfo>>	m_StylesInfo;

	/// Флаг того, что декодирование статьи завершено и мы не остановились где-то посредине статьи.
	UInt32 m_TranslationComplete;

	/// Массив указателей на функцию сборки перевода.
	FTranslationBuilderMethodPtr	m_TranslationBuilder[4];

	/// Указатель на данные системы защиты.
	TRegistrationData	*m_RegistrationData;

	/// Структура необходимая для работы генератора случайных чисел.
	TRandomSeed			m_RandomSeed;

};
#endif
