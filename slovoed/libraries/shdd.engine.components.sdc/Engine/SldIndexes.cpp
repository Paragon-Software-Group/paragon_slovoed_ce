#include "SldIndexes.h"

/** *********************************************************************
* Производим инициализацию класса доступа к индексам
*
* @param[in]	aData				- ссылка на объект отвечающий за получение данных из контейнера
* @param[in]	aIndexCountType		- тип ресурса с количеством статей приходящихся на одно слово списка слов
* @param[in]	aIndexDataType		- тип ресурса с номерами статей
* @param[in]	aIndexQAType		- тип ресурса с таблицей быстрого доступа к индексам
* @param[in]	aIndexHeaderType	- тип ресурса с заголовком индексов
*
* @return код ошибки
************************************************************************/
ESldError CSldIndexes::Init(CSDCReadMy &aData, UInt32 aIndexCountType, UInt32 aIndexDataType, UInt32 aIndexQAType, UInt32 aIndexHeaderType)
{
	ESldError error;

	auto res = aData.GetResource(aIndexHeaderType, 0);
	if (res != eOK)
		return res.error();

	sldMemCopy(&m_Header, res.ptr(), ((TIndexesHeader*)res.ptr())->structSize);

	error = m_InputCount.Init(aData, aIndexCountType, SLD_INDEXES_RESOURCE_SIZE);
	if (error != eOK)
		return error;
	error = m_InputData.Init(aData, aIndexDataType, SLD_INDEXES_RESOURCE_SIZE);
	if (error != eOK)
		return error;

	res = aData.GetResource(aIndexQAType, 0);
	if (res != eOK)
		return res.error();
	m_QA = res.resource();

	// Вычислим один раз все параметры
	if (m_Header.DataMask & SLD_INDEXES_IS_COUNT)
	{
		m_IsDataHasCount = 1;
	}
	if (m_Header.DataMask & SLD_INDEXES_IS_ARTICLE_INDEX)
	{
		m_isDataHasArticleIndex = 1;
		m_TotalIndexDataSize += m_Header.SizeOfIndexes;
	}
	if (m_Header.DataMask & SLD_INDEXES_IS_LIST_INDEX)
	{
		m_isDataHasListIndex = 1;
		m_TotalIndexDataSize += m_Header.SizeOfListIndex;
	}
	if (m_Header.DataMask & SLD_INDEXES_IS_TRANSLATION_INDEX)
	{
		m_isDataHasTranslationIndex = 1;
		m_TotalIndexDataSize += m_Header.SizeOfTranslationIndex;
	}
	if (m_Header.DataMask & SLD_INDEXES_IS_SHIFT_INDEX)
	{
		m_isDataHasShiftIndex = 1;
		m_TotalIndexDataSize += m_Header.SizeOfShiftIndex;
	}

	return eOK;
}

/** *********************************************************************
* Возвращает количество статей приходящихся на одно слово из списка слов
*
* @param[in]	aIndex				- номер слова в списке слов
* @param[out]	aTranslationCount	- указатель на буфер, в который будет сохранено 
*									  количество статей, на которые ссылается слово из списка слов
*
* @return код ошибки
************************************************************************/
ESldError CSldIndexes::GetNumberOfArticlesByIndex(Int32 aIndex, Int32 *aTranslationCount)
{
	if (!aTranslationCount)
		return eMemoryNullPointer;
		
	if ((UInt32)aIndex >= m_Header.Count)
		return eCommonWrongIndex;
	
	ESldError error;
	UInt32 TotalCount = 0;
	const TQAIndexesEntry* QA = (const TQAIndexesEntry*)m_QA.ptr();

	// Запрашивается то же слово, что и в предыдущий раз - ничего делать не нужно
	if (aIndex == m_CurrentIndex)
	{
		m_CurrentIndex |= 0;
	}
	// Запрашивается следующее по порядку слово
	else if ((m_CurrentIndex >= 0) && (aIndex > m_CurrentIndex) && ((aIndex - m_CurrentIndex) == 1))
	{
		// Если в индексах нет счетчика количества переводов, тогда переводов 1 или 0
		UInt32 DataCount = 0;
		if (!m_IsDataHasCount)
		{
			if (m_Header.SizeOfIndexes)
				DataCount = 1;
		}
		else
		{
			// Читаем данные требуемого (следующего) элемента
			error = m_InputCount.GetData(&DataCount, m_Header.SizeOfCount);
			if (error != eOK)
				return error;
		}
		
		m_CurrentBaseShiftData += m_CurrentDataCount*m_TotalIndexDataSize;
		m_CurrentDataCount = DataCount;
		m_CurrentIndex++;
	}
	else
	{
		// TODO: Сделать оптимизацию поиска, т.к. возможно недавно мы искали что-то в данной окресности
		UInt32 low = 0;
		UInt32 hi = m_Header.QACount;
		UInt32 med = 0;
		
		while (hi-low > 1)
		{
			med = (hi+low)>>1;
			if (QA[med].Index < (UInt32)aIndex)
			{
				low = med;
			}else
			{
				hi = med;
			}
		}

		m_CurrentIndex = QA[low].Index;

		// Если в индексах нет счетчика количества переводов, тогда переводов 1 или 0
		if (!m_IsDataHasCount)
		{
			if (m_Header.SizeOfIndexes)
			{
				m_CurrentDataCount = 1;
				TotalCount = aIndex - m_CurrentIndex;
			}
			else
			{
				m_CurrentDataCount = 0;
			}
			m_CurrentIndex = aIndex;
		}
		else
		{
			error = m_InputCount.GoTo(QA[low].CountShiftBit);
			if (error != eOK)
				return error;

			// Пропускаем...
			while (m_CurrentIndex < aIndex)
			{
				error = m_InputCount.GetData(&m_CurrentDataCount, m_Header.SizeOfCount);
				if (error != eOK)
					return error;
					
				TotalCount += m_CurrentDataCount;
				m_CurrentIndex++;
			}
			
			// Читаем данные требуемого элемента
			error = m_InputCount.GetData(&m_CurrentDataCount, m_Header.SizeOfCount);
			if (error != eOK)
				return error;
		}

		m_CurrentBaseShiftData = QA[low].DataShiftBit;
		m_CurrentBaseShiftData += TotalCount*m_TotalIndexDataSize;
	}
	
	*aTranslationCount = (Int32)m_CurrentDataCount;

	return eOK;
}

/** *********************************************************************
* Возвращает данные индекса по номеру слова и номеру перевода в слове
*
* @param[in]	aWordIndex				- глобальный номер слова в списке слов
* @param[in]	aWordTranslationIndex	- номер перевода слова (от 0 до количества переводов, приходящихся на данное слово)
* @param[out]	aListIndex				- указатель на переменную, куда будет сохранен номер списка слов, на который ссылается слово из списка слов
*										  (в случае списка слов полнотекстового поиска)
* @param[out]	aArticleIndex			- указатель на переменную, куда будет сохранен номер статьи, на которую ссылается слово из списка слов,
*										  или номер слова в списке слов (в случае списка слов полнотекстового поиска)
* @param[out]	aTranslationIndex		- указатель на переменную, куда будет сохранен номер перевода (в случае списка слов полнотекстового поиска)
* @param[out]	aShiftIndex				- указатель на переменную, куда будет сохранено смещение (в случае списка слов полнотекстового поиска)
*
* @return код ошибки
************************************************************************/
ESldError CSldIndexes::GetIndexData(Int32 aWordIndex, Int32 aWordTranslationIndex, Int32* aListIndex, Int32* aArticleIndex, Int32* aTranslationIndex, Int32* aShiftIndex)
{
	if (!aListIndex || !aArticleIndex || !aTranslationIndex || !aShiftIndex)
		return eMemoryNullPointer;
		
	Int32 TranslationCount = 0;
	ESldError error = GetNumberOfArticlesByIndex(aWordIndex, &TranslationCount);
	if (error != eOK)
		return error;
	
	if (!m_CurrentDataCount || ((UInt32)aWordTranslationIndex >= m_CurrentDataCount))
		return eCommonWrongIndex;
	
	UInt32 shiftData = m_CurrentBaseShiftData + aWordTranslationIndex*m_TotalIndexDataSize;
	
	error = m_InputData.GoTo(shiftData);
	if (error != eOK)
		return error;
	
	UInt32 dListIndex = m_Header.DefaultListIndex;
	UInt32 dArticleIndex = 0;
	UInt32 dTranslationIndex = m_Header.DefaultTranslationIndex;
	UInt32 dShiftIndex = 0;
	
	if (m_isDataHasListIndex)
	{
		error = m_InputData.GetData(&dListIndex, m_Header.SizeOfListIndex);
		if (error != eOK)
			return error;
	}
	if (m_isDataHasArticleIndex)
	{
		error = m_InputData.GetData(&dArticleIndex, m_Header.SizeOfIndexes);
		if (error != eOK)
			return error;
	}
	if (m_isDataHasTranslationIndex)
	{
		error = m_InputData.GetData(&dTranslationIndex, m_Header.SizeOfTranslationIndex);
		if (error != eOK)
			return error;
	}
	if (m_isDataHasShiftIndex)
	{
		error = m_InputData.GetData(&dShiftIndex, m_Header.SizeOfShiftIndex);
		if (error != eOK)
			return error;
	}
	
	*aListIndex = (Int32)dListIndex;
	*aArticleIndex = (Int32)dArticleIndex;
	*aTranslationIndex = (Int32)dTranslationIndex;
	*aShiftIndex = (Int32)dShiftIndex;
	
	return eOK;
}