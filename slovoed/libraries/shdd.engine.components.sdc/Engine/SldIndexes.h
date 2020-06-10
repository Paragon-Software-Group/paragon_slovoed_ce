#ifndef _SLD_INDEXES_H_
#define _SLD_INDEXES_H_

#include "SldError.h"
#include "SldBitInput.h"

/// Максимальный размер ресурса. На самом деле все ресурсы кроме последнего должны быть этого размера.
#define SLD_INDEXES_RESOURCE_SIZE	(32*1024)

/// Получить только количество ссылок на статьи
#define SLD_INDEX_GET_COUNT_ONLY		(0)
/// Получить номер статьи на которую ссылается слово
#define SLD_INDEX_GET_DATA				(1)


/// Класс отвечающий за хранение индексов статей.
class CSldIndexes
{
public:

	/// Конструктор
	CSldIndexes(void):
		m_CurrentIndex(-1),
		m_CurrentBaseShiftData(0),
		m_CurrentDataCount(0),
		m_IsDataHasCount(0),
		m_isDataHasArticleIndex(0),
		m_isDataHasListIndex(0),
		m_isDataHasTranslationIndex(0),
		m_isDataHasShiftIndex(0),
		m_TotalIndexDataSize(0)
	{
		sldMemZero(&m_Header, sizeof(m_Header));
	};

	/// Производим инициализацию класса доступа к индексам
	ESldError Init(CSDCReadMy &aData, UInt32 aIndexCountType, UInt32 aIndexDataType, UInt32 aIndexQAType, UInt32 aIndexHeaderType);

	/// Возвращает количество статей приходящихся на одно слово из списка слов
	ESldError GetNumberOfArticlesByIndex(Int32 aIndex, Int32 *aTranslationCount);

	/// Возвращает данные индекса по номеру слова и номеру перевода в слове
	ESldError GetIndexData(Int32 aWordIndex, Int32 aWordTranslationIndex, Int32* aListIndex, Int32* aArticleIndex, Int32* aTranslationIndex, Int32* aShiftIndex);

private:

	/// Заголовок
	TIndexesHeader m_Header;

	/// Номер текущего индекса
	Int32 m_CurrentIndex;
	
	/// Базовое смещение для данных текущего индекса
	UInt32 m_CurrentBaseShiftData;
	
	/// Количество записей с номерами статей для текущего индекса (количество переводов)
	UInt32 m_CurrentDataCount;

	/// ресурс с данными быстрого доступа
	CSDCReadMy::Resource	m_QA;

	/// Входной поток данных для количества статей
	CSldBitInput	m_InputCount;
	/// Входной поток данных для номеров статей.
	CSldBitInput	m_InputData;
	
	
	/// Индексы имеют в своем составе количество статей (иначе количество == 1 или 0)
	UInt8 m_IsDataHasCount;
	
	/// Индексы имеют в своем составе номера статей
	UInt8 m_isDataHasArticleIndex;
	
	/// Индексы имеют в своем составе номера списков слов (иначе номер списка по умолчанию нужно брать из заголовка индексов)
	UInt8 m_isDataHasListIndex;
	
	/// Индексы имеют в своем составе номера переводов (иначе номер перевода по умолчанию нужно брать из заголовка индексов)
	UInt8 m_isDataHasTranslationIndex;
	
	/// Индексы имеют в своем составе смещения в статье (иначе смещение == 0)
	UInt8 m_isDataHasShiftIndex;
	
	/// Общий размер одного индекса в битах
	UInt32 m_TotalIndexDataSize;
	
};
#endif
