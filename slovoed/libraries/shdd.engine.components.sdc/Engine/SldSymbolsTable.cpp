#include "SldSymbolsTable.h"


/// Конструктор
CSldSymbolsTable::CSldSymbolsTable(void)
{
	sldMemZero(&m_Header, sizeof(m_Header));
}

/** *********************************************************************
* Инициализация
*
* @param aData			- ссылка на объект, отвечающий за получение данных из контейнера
* @param aResourceType	- тип ресурса с таблицей
* @param aResourceIndex	- индекс ресурса с таблицей
*
* @return код ошибки
************************************************************************/
ESldError CSldSymbolsTable::Init(CSDCReadMy &aData, UInt32 aResourceType, UInt32 aResourceIndex)
{
	auto res = aData.GetResource(aResourceType, aResourceIndex);
	if (res != eOK)
		return res.error();

	auto header = (const TLanguageSymbolsTableHeader*)res.ptr();

	// Проверка на корректность загруженных данных
	if (res.size() != (header->structSize + header->TableSize))
		return eInputWrongStructSize;

	// Создаем таблицу
	if (!m_Table.resize(sld2::default_init, header->TableSize))
		return eMemoryNotEnoughMemory;

	// Копируем данные
	sldMemCopy(&m_Header, res.ptr(), sizeof(m_Header));
	sldMemCopy(m_Table.data(), res.ptr() + m_Header.structSize, m_Header.TableSize);

	return eOK;
}

/** *********************************************************************
* Получает код языка таблицы
* Если код языка таблицы равен SldLanguage::Delimiters, то 
* это таблица символов-разделителей
*
* @return код языка таблицы
************************************************************************/
ESldLanguage CSldSymbolsTable::GetLanguageCode() const
{
	if (m_Header.LanguageCode == 0)
		return SldLanguage::Delimiters;

	return SldLanguage::fromCode(m_Header.LanguageCode);
}

/** *********************************************************************
* Проверяет, принадлежит ли символ языку
*
* @param aCode	- код проверяемого символа
* @param aFlag	- указатель на переменную, куда сохраняется результат:
*				  0 - не принадлежит
*				  1 - принадлежит
*
* @return принадлежит ли символ языку
************************************************************************/
bool CSldSymbolsTable::IsSymbolBelongToLanguage(UInt16 aCode) const
{
	UInt32 byteShift = aCode >> 3;
	UInt32 bitShift = (byteShift) ? aCode % (byteShift << 3) : aCode;
	UInt8 mask = 1 << bitShift;

	UInt8 byte = m_Table[byteShift];

	return (byte & mask) != 0;
}

/** *********************************************************************
* Объединяет таблицы символов
*
* @param aTable	- указатель на таблицу элементы которой нужно добавить
*
* @return код ошибки
************************************************************************/
ESldError CSldSymbolsTable::MergeTables(const CSldSymbolsTable* aTable)
{
	if (!aTable)
		return eMemoryNullPointer;

	for (UInt32 currentByte = 0; currentByte < m_Header.TableSize; currentByte++)
	{
		m_Table[currentByte] |= aTable->m_Table[currentByte];
	}

	return eOK;
}
