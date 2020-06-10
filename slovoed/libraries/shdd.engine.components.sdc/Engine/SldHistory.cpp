#include "SldHistory.h"

/// Конструктор
CSldHistoryElement::CSldHistoryElement()
{
	sldMemZero(this, sizeof(CSldHistoryElement));

	m_ListIndex		= -1;
	m_ListType		= eWordListType_Unknown;

	m_WordIndex		= -1;

	m_HistorySignature = HISTORY_ELEMENT_SIGNATURE;
}

/// move конструктор
CSldHistoryElement::CSldHistoryElement(CSldHistoryElement&& aRef)
{
	// this is pretty ugly but oh well
	sldMemCopy(this, &aRef, sizeof(aRef));
	sldMemZero(&aRef, sizeof(aRef));
}

/** *********************************************************************
* Освобождает память. Деструктор CSldHistoryElement-а вызывает данную функцию.
* Однако, если память для CSldHistoryElement выделялась самостоятельно
* Данную функцию нужно вызывать обязательно
************************************************************************/
void CSldHistoryElement::ReleaseElement()
{
	if (m_Words)
	{
		for (UInt32 currentWordIndex = 0; currentWordIndex < m_WordsCount; currentWordIndex++)
		{
			if (m_Words[currentWordIndex])
				sldMemFree(m_Words[currentWordIndex]);
		}
		sldMemFree(m_Words);
	}

	if (m_VariantType)
		sldMemFree(m_VariantType);

	if (m_UserData)
		sldMemFree(m_UserData);
}

/// Оператор присваивания
CSldHistoryElement& CSldHistoryElement::operator=(const CSldHistoryElement & aRef)
{
	ReleaseElement();

	sldMemMove(this, &aRef, sizeof(CSldHistoryElement));

	m_Words = NULL;
	m_VariantType = NULL;
	for (UInt32 wordIndex = 0; wordIndex < m_WordsCount; wordIndex++)
	{
		SetCurrentWord(wordIndex, aRef.m_Words[wordIndex]);
		SetVariantType(wordIndex, aRef.m_VariantType[wordIndex]);
	}

	m_UserData = NULL;
	if (m_UserDataSize != 0)
	{
		m_UserData = (Int8*)sldMemNew(m_UserDataSize);
		sldMemCopy(m_UserData, aRef.m_UserData, m_UserDataSize);
	}

	return *this;
}

/// Оператор присваивания из rvalue
CSldHistoryElement& CSldHistoryElement::operator=(CSldHistoryElement&& aRef)
{
	// this is pretty ugly but oh well
	ReleaseElement();
	sldMemCopy(this, &aRef, sizeof(aRef));
	sldMemZero(&aRef, sizeof(aRef));
	return *this;
}

/** *********************************************************************
* Возвращает бинарное представление текущего элемента.
*
* ВНИМАНИЕ! Память для aData выделяется в этом методе
* и должна быть освобождена в вызывающем методе вызовом функции sldMemFree()
*
* @param[out]	aData		- указатель, по которому будет записан бинарный массив данных
* @param[out]	aDataSize	- указатель, по которому будет записан размер массива aData
*
* @return код ошибки
************************************************************************/
ESldError CSldHistoryElement::GetBinaryData(Int8** aData, UInt32* aDataSize) const
{
	if(!aData || !aDataSize)
		return eMemoryNullPointer;

	*aData = NULL;
	*aDataSize = 0;

	if(m_WordsCount == 0)
		return eOK;

	UInt32 historyElementSize = sizeof(CSldHistoryElement);
	UInt32 dataSize = historyElementSize;
	dataSize += sizeof(EListVariantTypeEnum) * m_WordsCount;

	for(UInt32 currentWordIndex = 0; currentWordIndex < m_WordsCount; currentWordIndex++)
	{
		dataSize += sizeof(UInt16) * (CSldCompare::StrLen(m_Words[currentWordIndex]) + 1);
	}

	dataSize += m_UserDataSize;

	*aData = (Int8*)sldMemNewZero(dataSize);
	*aDataSize = dataSize;

	sldMemMove(*aData, this, historyElementSize);

	/// что бы в бинарные данные не попадал всякий мусор - зануляем указатели
	CSldHistoryElement* currentElement = (CSldHistoryElement*)*aData;
	currentElement->m_Words = 0;
	currentElement->m_VariantType = 0;
	currentElement->m_UserData = 0;
	// всегда пишем сигнатуру
	currentElement->m_HistorySignature = HISTORY_ELEMENT_SIGNATURE;

	Int8* workingData = *aData + historyElementSize;
	UInt32 wordLen = 0;
	for(UInt32 currentWordIndex = 0; currentWordIndex < m_WordsCount; currentWordIndex++)
	{
		sldMemMove(workingData, &m_VariantType[currentWordIndex], sizeof(m_VariantType[currentWordIndex]));

		workingData += sizeof(m_VariantType[currentWordIndex]);

		wordLen = (CSldCompare::StrLen(m_Words[currentWordIndex]) + 1) * sizeof(UInt16);
		sldMemMove(workingData, m_Words[currentWordIndex], wordLen);
		workingData += wordLen;
	}

	if(m_UserDataSize)
	{
		sldMemMove(workingData, m_UserData, m_UserDataSize);
	}

	return eOK;
}

/** *********************************************************************
* Загружает элемент истории из бинарных данных
*
* @param[in]	aData		- указатель на массив бинарных данных
* @param[in]	aDataSize	- размер массива aData
*
* @return код ошибки
************************************************************************/
ESldError CSldHistoryElement::LoadElement(const Int8* aData, const UInt32 aDataSize)
{
	if(!aData)
		return eMemoryNullPointer;

	ReleaseElement();
	sldMemZero(&m_HistorySignature, sizeof(m_HistorySignature));
	UInt32 historyElementSize = sizeof(CSldHistoryElement);
	sldMemMove(this, aData, sld2::min(historyElementSize, aDataSize));

	if (m_HistorySignature != HISTORY_ELEMENT_SIGNATURE)
	{
		ESldError error = ResolveElement(aData, aDataSize, historyElementSize);
		if (error != eOK)
			return error;
	}

	m_Words = NULL;
	m_VariantType = NULL;
	m_UserData = NULL;

	// На андройде важна кратность указателя и его размера
	// Поэтому для адекватной работы со строчками нужно проверить четность указателя
	// При нечетном указателе релоцируем оставшуюся память в четную область
	
	UInt4Ptr dataPointer = (UInt4Ptr)aData + historyElementSize;
	UInt16* relocatedData = NULL;
	UInt16* workingData = NULL;
	const UInt32 UInt16Size = sizeof(UInt16);

	if (dataPointer % UInt16Size)
	{
		relocatedData = (UInt16*)sldMemNew(aDataSize - historyElementSize);
		if (!relocatedData)
			return eMemoryNotEnoughMemory;

		sldMemMove(relocatedData, aData + historyElementSize, aDataSize - historyElementSize);
		workingData = relocatedData;
	}
	else
	{
		workingData = (UInt16*)dataPointer;
	}

	EListVariantTypeEnum listVariantType = eVariantShow;
	const UInt32 variantTypeSize = sizeof(EListVariantTypeEnum);
	for(UInt32 currentWordIndex = 0; currentWordIndex < m_WordsCount; currentWordIndex++)
	{
		sldMemMove(&listVariantType, workingData, variantTypeSize);
		SetVariantType(currentWordIndex, listVariantType);
		workingData += (variantTypeSize / UInt16Size);

		SetCurrentWord(currentWordIndex, workingData);
		workingData += (CSldCompare::StrLen(m_Words[currentWordIndex]) + 1);
	}
	
	ESldError error = SetUserData((Int8*)workingData, m_UserDataSize);
	if(error != eOK)
		return error;

	if (relocatedData)
	{
		sldMemFree(relocatedData);
	}

	return eOK;
}

/** *********************************************************************
* Устанавливает тип варианта написания
*
* @param[in]	aVariantIndex	- индекс устанавливаемого варианта написания
* @param[in]	aVariantType	- тип устанавливаемого варианта написания
*
* @return код ошибки
************************************************************************/
ESldError CSldHistoryElement::SetVariantType(UInt32 aVariantIndex, EListVariantTypeEnum aVariantType)
{
	if(aVariantIndex >= m_WordsCount)
		return eCommonWrongIndex;

	if(!m_VariantType)
	{
		UInt32 size = m_WordsCount * sizeof(*m_VariantType);
		m_VariantType = (EListVariantTypeEnum*)sldMemNewZero(size);
		if(!m_VariantType)
			return eMemoryNotEnoughMemory;
	}

	m_VariantType[aVariantIndex] = aVariantType;

	return eOK;
}

/** *********************************************************************
* Возвращает тип варианта написания
*
* @param[in]	aVariantIndex	- индекс запрашиваемого варианта написания
* @param[out]	aVariantType	- указатель, по которому будет записан запрашиваемый вариант написания
*
* @return код ошибки
************************************************************************/
ESldError CSldHistoryElement::GetVariantType(UInt32 aVariantIndex, EListVariantTypeEnum* aVariantType) const
{
	if(!aVariantType)
		return eMemoryNullPointer;

	*aVariantType = eVariantUnknown;

	if(aVariantIndex > m_WordsCount)
		return eCommonWrongVariantIndex;

	*aVariantType = m_VariantType[aVariantIndex];

	return eOK;
}

/** *********************************************************************
* Устанавливает слово для заданного варианта написания
*
* @param[in]	aVariantIndex	- индекс варианта написания устанавливаемого слова
* @param[in]	aWord			- устанавливаемое слово
*
* @return код ошибки
************************************************************************/
ESldError CSldHistoryElement::SetCurrentWord(UInt32 aVariantIndex, UInt16* aWord)
{
	if(!aWord)
		return eMemoryNullPointer;

	if(aVariantIndex >= m_WordsCount)
		return eCommonWrongIndex;

	if(!m_Words)
	{
		UInt32 size = m_WordsCount * sizeof(*m_Words);
		m_Words = (UInt16**)sldMemNewZero(size);
		if(!m_Words)
			return eMemoryNotEnoughMemory;
	}

	if(m_Words[aVariantIndex])
	{
		sldMemFree(m_Words[aVariantIndex]);
	}

	UInt32 wordLen = CSldCompare::StrLen(aWord);
	m_Words[aVariantIndex] = (UInt16*)sldMemNew((wordLen + 1) * sizeof(UInt16));

	CSldCompare::StrCopy(m_Words[aVariantIndex], aWord);
	m_Words[aVariantIndex][wordLen] = 0;

	return eOK;
}

/** *********************************************************************
* Возвращает слово для заданного варианта написания
*
* @param[in]	aVariantIndex	- индекс варианта написания запрашиваемого слова
* @param[out]	aWord			- указатель, по которому будет записано запрашиваемое слово
*
* @return код ошибки
************************************************************************/
ESldError CSldHistoryElement::GetCurrentWord(UInt32 aVariantIndex, UInt16** aWord) const
{
	if(!aWord)
		return eMemoryNullPointer;

	*aWord = NULL;

	if(aVariantIndex >= m_WordsCount)
		return eCommonWrongIndex;

	*aWord = m_Words[aVariantIndex];
	return eOK;
}

/** *********************************************************************
* Возвращает Show-вариант для данного элемента
*
* @param[out]	aWord	- указатель, по которому будет записан show-вариант
*
* @return код ошибки
************************************************************************/
ESldError CSldHistoryElement::GetShowVariant(UInt16** aWord) const
{
	if(!aWord)
		return eMemoryNullPointer;

	*aWord = NULL;

	UInt32 currentWordIndex = 0;
	for (; currentWordIndex < m_WordsCount; currentWordIndex++)
	{
		if(m_VariantType[currentWordIndex] == eVariantShow)
		{
			break;
		}
	}

	if(currentWordIndex != m_WordsCount)
	{
		*aWord = m_Words[currentWordIndex];
	}

	return eOK;
}

/** *********************************************************************
* Устанавливает дату
*
* @param[in]	aDay	- текущий день
* @param[in]	aMonth	- текущий месяц
* @param[in]	aYear	- текущий год
*
************************************************************************/
void CSldHistoryElement::SetDate(UInt32 aDay, UInt32 aMonth, UInt32 aYear)
{
	m_Date = 0;

	m_Date += aDay;
	m_Date += aMonth*100;
	m_Date += aYear*10000;
}

/** *********************************************************************
* Устанавливает время
*
* @param[in]	aHour	- час
* @param[in]	aMinute	- минута
* @param[in]	aSecond	- секунда
*
************************************************************************/
void CSldHistoryElement::SetTime(UInt32 aHour, UInt32 aMinute, UInt32 aSecond)
{
	m_Time = 0;

	m_Time += aSecond;
	m_Time += aMinute*100;
	m_Time += aHour*10000;
}

/** *********************************************************************
* Возвращает дату
*
* @param[out]	aDay	- указатель, по которому будет записан день
* @param[out]	aMonth	- указатель, по которому будет записан месяц
* @param[out]	aYear	- указатель, по которому будет записан год
*
* @return код ошибки
************************************************************************/
ESldError	CSldHistoryElement::GetDate(UInt32* aDay, UInt32* aMonth, UInt32* aYear) const
{
	if(!aDay || !aMonth || !aYear)
		return eMemoryNullPointer;

	*aYear = m_Date / 10000;
	*aMonth = (m_Date - (*aYear * 10000)) / 100;
	*aDay = m_Date - (*aYear * 10000) - (*aMonth * 100);

	return eOK;
}

/** *********************************************************************
* Возвращает время
*
* @param[out]	aHour	- указатель, по которому будет записан час
* @param[out]	aMinute	- указатель, по которому будет записан минута
* @param[out]	aSecond	- указатель, по которому будет записан секунда
*
* @return код ошибки
************************************************************************/
ESldError	CSldHistoryElement::GetTime(UInt32* aHour, UInt32* aMinute, UInt32* aSecond) const
{
	if(!aHour || !aMinute || !aSecond)
		return eMemoryNullPointer;

	*aHour = m_Time / 10000;
	*aMinute = (m_Time - (*aHour * 10000)) / 100;
	*aSecond = m_Time - (*aHour * 10000) - (*aMinute * 100);

	return eOK;
}

/** *********************************************************************
* Возвращает дату
*
* @param[in]	aDate	- 4-байтовое представление даты вида год/месяц/день
*						  Например: 20140312 - 2014/03/12
*
* @return код ошибки
************************************************************************/
ESldError	CSldHistoryElement::GetDate(UInt32* aDate) const
{
	if(!aDate)
		return eMemoryNullPointer;

	*aDate = m_Date;
	return eOK;
}

/** *********************************************************************
* Возвращает время
*
* @param[in]	aTime	- 4-байтовое представление времени вида час/минута/секунда
*						  Например: 145540 - 14:55:40
*
* @return код ошибки
************************************************************************/
ESldError	CSldHistoryElement::GetTime(UInt32* aTime) const
{
	if(!aTime)
		return eMemoryNullPointer;

	*aTime = m_Time;
	return eOK;
}

/** *********************************************************************
* Устанавливает пользовательские данные
*
* @param[in]	aUserData	- указатель, на бинарный массив пользовательских данных
* @param[in]	aDataSize	- размер массива пользовательских данных
*
* @return код ошибки
************************************************************************/
ESldError	CSldHistoryElement::SetUserData(const Int8* aUserData, UInt32 aDataSize)
{
	if(!aUserData)
		return eMemoryNullPointer;

	if(!aDataSize)
		return eOK;

	if(m_UserData)
	{
		sldMemFree(m_UserData);
		m_UserData = NULL;
	}

	m_UserData = (Int8*)sldMemNew(aDataSize);
	if(!m_UserData)
		return eMemoryNotEnoughMemory;

	sldMemMove(m_UserData, aUserData, aDataSize);

	m_UserDataSize = aDataSize;

	return eOK;
}

/** *********************************************************************
* Возвращает пользовательские данные
*
* @param[out]	aUserData	- указатель, по которому будет записан массив пользовательских данных
*							  Память для массива выделена внутри CSldHistoryElement и очищается 
*							  с помощью функции ReleaseElement()
*
* @return размер массива с пользовательскими данными
************************************************************************/
UInt32	CSldHistoryElement::GetUserData(Int8** aUserData) const
{
	*aUserData = m_UserData;

	return m_UserDataSize;
}

/** *********************************************************************
* Восстанавливает целостность загруженного из бинарных данных элемента
*
* @param[in]	aData		- указатель на массив бинарных данных
* @param[in]	aDataSize	- размер массива aData
*
* @return код ошибки
************************************************************************/
ESldError CSldHistoryElement::ResolveElement(const Int8* aData, const UInt32 aDataSize, UInt32& aOldHistoryElementSize)
{
	Int8 hasHistorySignature = 0;
	UInt32 historySignature = 0;
	UInt32 historySignatureSize = sizeof(m_HistorySignature);
	const Int8* ptr = aData;

	for (UInt32 i = 0; i < aDataSize - historySignatureSize; i++)
	{
		sldMemMove(&historySignature, ptr, historySignatureSize);
		if (historySignature == HISTORY_ELEMENT_SIGNATURE)
		{
			hasHistorySignature = 1;
			break;
		}

		ptr++;
	}

	if (hasHistorySignature)
	{
		UInt32 oldStructSize = ptr - aData + historySignatureSize;
		const CSldHistoryElement* currentElement = (const CSldHistoryElement*)aData;
		sldMemZero(&m_Words, aOldHistoryElementSize - ((const Int8*)&currentElement->m_Words - aData));
		const UInt32* userDataSizePtr = &currentElement->m_UserDataSize;

		if (oldStructSize < aOldHistoryElementSize)
		{
			// Размер указателя увеличился с 4 до 8 байт
			userDataSizePtr -= 2;
			sldMemMove(&m_UserDataSize, userDataSizePtr, sizeof(UInt32));
		}
		// на всякий случай проверяем выход за пределы
		else if (oldStructSize > aOldHistoryElementSize && oldStructSize < 1000)
		{
			// Размер указателя уменьшился с 8 до 4 байт
			userDataSizePtr += 2;
			sldMemMove(&m_UserDataSize, userDataSizePtr, sizeof(UInt32));
		}

		aOldHistoryElementSize = oldStructSize;
	}

	return eOK;
}

/** *********************************************************************
* Возвращает хэш элемента (без учета даты, юзер даты и пр.)
*
* @return хэш элемента
************************************************************************/
UInt32 CSldHistoryElement::GetHash() const
{
	const UInt32 significationSize = 6 * sizeof(UInt32);

	return sld2::MurmurHash3(&m_DictID, significationSize, 0x736c6432);
}