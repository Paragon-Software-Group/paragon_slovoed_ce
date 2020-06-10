#include "SldInputBase.h"

#include "SldCompare.h"

/**
 * Инициализация потока данных.
 *
 * @param[in] aData        - ссылка объект доступа к данным словаря
 * @param[in] aDataType    - тип ресурса с основными данными
 * @param[in] aAssistType  - тип ресурса с вспомогательными данными
 * @param[in] aAssistCount - количество ресурсов с вспомогательными данными
 * @param[in] aQAType      - тип ресурса данными для быстрого поиска
 * @param[in] aListVersion - номер версии списка слов
 *
 * @return код ошибки
 */
ESldError CSldInputBase::Init(CSDCReadMy &aData, UInt32 aDataType, UInt32 aAssistType, UInt32 aAssistCount, UInt32 aQAType, UInt32 aListVersion)
{
	ESldError error = m_input.Init(aData, aDataType, MAX_COMPRESSED_DATA_RESOURCE_SIZE);
	if (error != eOK)
		return error;

	// Читаем таблицу быстрого доступа
	if (!(aListVersion == 0 || aListVersion == VERSION_LIST_BASE))
		return eOK;

	UInt32 resIdx = 0;
	auto res = aData.GetResource(aQAType, resIdx);
	if (res != eOK)
		return res.error();

	// Заголовок
	sldMemCopy(&m_QAHeader, res.ptr(), sizeof(m_QAHeader));

	// Выделяем память для таблицы
	UInt32 QATotalTableSize = m_QAHeader.QAEntrySize * m_QAHeader.Count;
	if (!m_QA.resize(sld2::default_init, QATotalTableSize))
		return eMemoryNotEnoughMemory;

	// Копируем данные таблицы из первого ресурса
	UInt32 CopySize = res.size() - m_QAHeader.HeaderSize;
	sldMemCopy(m_QA.data(), res.ptr() + m_QAHeader.HeaderSize, CopySize);

	// TODO: этот код, который читает данные из нескольких ресурсов, нужно будет убрать, когда все сконвертированные словари
	// будут собраны свежей версией компилятора, записывающей все большие куски данных в один ресурс независимо от их размера
	// Еще имеются данные в следующих ресурсах
	if (CopySize < QATotalTableSize)
	{
		QATotalTableSize -= CopySize;
		while (QATotalTableSize)
		{
			resIdx++;
			res = aData.GetResource(aQAType, resIdx);
			if (res != eOK)
				return res.error();

			if (res.size() > QATotalTableSize)
				return eCommonWrongResourceSize;

			// Копируем данные таблицы из очередного ресурса
			sldMemCopy(m_QA.data() + CopySize, res.ptr(), res.size());
			CopySize += res.size();

			QATotalTableSize -= res.size();
		}
	}
	return eOK;
}

/// Устанавливаем HASH для декодирования данного списка слов.
void CSldInputBase::SetHASH(UInt32 aHASH)
{
	m_HASH = aHASH;
}

/// Получаем количество точек по которым может производится поиск
UInt32 CSldInputBase::GetSearchPointCount()
{
	return m_QAHeader.Count;
}

/**
 * Получаем текст для указанной точки
 *
 * @param[in]  aPointIndex - номер точки поиска для которой мы хотим получить текст
 * @param[out] aText       - сюда поместим указатель на строчку с текстом
 *
 * @return код ошибки
 */
ESldError CSldInputBase::GetSearchPointText(UInt32 aPointIndex, const UInt16 **aText)
{
	if (aPointIndex >= m_QAHeader.Count)
		return eCommonWrongIndex;

	if (m_QAHeader.Type != eQATypeList)
		return eCommonWrongQAType;

	*aText = ((const TQAEntry*)m_QA.data())[aPointIndex].text;
	return eOK;
}

/**
 * Получаем номер слова для указанной точки
 *
 * @param[in]  aPointIndex - номер точки поиска для которой мы хотим получить номер слова
 * @param[out] aIndex      - сюда поместим номер слова
 *
 * @return код ошибки
 */
ESldError CSldInputBase::GetSearchPointIndex(UInt32 aPointIndex, Int32 *aIndex)
{
	if (aPointIndex >= m_QAHeader.Count)
		return eCommonWrongIndex;
	if (!aIndex)
		return eMemoryNullPointer;

	if (m_QAHeader.Type == eQATypeList)
		*aIndex = ((const TQAEntry*)m_QA.data())[aPointIndex].Index - 1;
	else if (m_QAHeader.Type == eQATypeArticles)
		*aIndex = ((const TQAArticlesEntry*)m_QA.data())[aPointIndex].Index - 1;
	else
		return eCommonWrongQAType;
	return eOK;
}

/**
 * Получаем начало слова для указанной точки
 *
 * @param[in] aPointIndex - номер точки поиска для которой мы хотим получить номер слова
 * @param[in] aWord       - укзатель по которму будет записано слово
 *
 * @return код ошибки
 */
ESldError CSldInputBase::GetSearchPointText(UInt32 aPointIndex, UInt16 *aWord)
{
	if (aPointIndex >= m_QAHeader.Count)
		return eCommonWrongIndex;

	if (!aWord)
		return eMemoryNullPointer;

	if (m_QAHeader.Type != eQATypeList)
		return eCommonWrongQAType;

	CSldCompare::StrCopy(aWord, ((const TQAEntry*)m_QA.data())[aPointIndex].text);
	return eOK;
}

/// Производим переход внутреннего состояния в позицию точки из таблицы быстрого поиска.
ESldError CSldInputBase::GoTo(UInt32 aPointIndex)
{
	if (aPointIndex >= m_QAHeader.Count)
		return eCommonWrongIndex;

	UInt32 shiftBit;
	if (m_QAHeader.Type == eQATypeList)
		shiftBit = ((const TQAEntry*)m_QA.data())[aPointIndex].ShiftBit;
	else if (m_QAHeader.Type == eQATypeArticles)
		shiftBit = ((const TQAArticlesEntry*)m_QA.data())[aPointIndex].ShiftBit;
	else
		return eCommonWrongQAType;

	// Преобразуем смещение в правильное
	shiftBit = SLD_QA_SHIFT_RESTORE(shiftBit, m_HASH);

	return m_input.GoTo(shiftBit);
}

/// Производит получение данных, до 32 бит.
ESldError CSldInputBase::GetData(UInt32 *aDataBuffer, UInt32 aDataSize)
{
	return m_input.GetData(aDataBuffer, aDataSize);
}

/// Возвращает текущее положение в битах.
UInt32 CSldInputBase::GetCurrentPosition()
{
	return m_input.GetCurrentPosition();
}

/// Перемещаем указатель текущей позиции чтения в указанную позицию
ESldError CSldInputBase::SetCurrentPosition(UInt32 aShift)
{
	return m_input.GoTo(aShift);
}
