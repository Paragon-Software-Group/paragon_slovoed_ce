#include "SldTypes.h"

#include "SldCompare.h"
#include "SldIntFormatter.h"
#include "SldMacros.h"

TExternContentInfo::InfoPair& TExternContentInfo::InfoPair::operator=(const InfoPair& aRef)
{
	Count = aRef.Count;
	if (!!DictId)
		sldMemFree(DictId);

	DictId = (UInt16*)sldMemNew((CSldCompare::StrLen(aRef.DictId) + 1) * sizeof(UInt16));
	CSldCompare::StrCopy(DictId, aRef.DictId);

	return *this;
}

TCatalogPath::BaseListStruct::~BaseListStruct()
{
	if (m.Capacity != 0)
		sldMemFree(dynList());
}

ESldError TCatalogPath::BaseListStruct::reallocate(UInt32 newSize)
{
	UInt32 newCapacity;
	Int32* newList;
	if (m.Capacity == 0)
	{
		newCapacity = newSize ? newSize : 8;
		newList = sldMemNewZero<Int32>(newCapacity);
		if (newList)
			sld2::memcopy_n(newList, data(), size());
	}
	else
	{
		newCapacity = newSize > m.Capacity ? newSize : m.Capacity * 3 / 2;
		newList = sldMemReallocT(dynList(), newCapacity);
	}
	if (!newList)
		return eMemoryNotEnoughMemory;

	m.Capacity = newCapacity;
	dynList() = newList;
	return eOK;
}

/** ********************************************************************
 * Копирует путь в переданный
 *
 * @param[in]  aPath - ссылка на путь в который будет произведено копирование
 *
 * @return код ошибки
 ************************************************************************/
ESldError TCatalogPath::CopyTo(TCatalogPath &aPath) const
{
	if (BaseListCount > aPath.BaseList.size())
	{
		ESldError error = aPath.BaseList.reallocate(BaseListCount);
		if (error != eOK)
			return error;
	}
	sld2::memcopy_n(aPath.BaseList.data(), BaseList.data(), BaseListCount);
	aPath.BaseListCount = BaseListCount;
	aPath.ListIndex = ListIndex;
	return eOK;
}

void TCatalogPath::popFront()
{
	if (BaseListCount == 0)
		return;
	BaseListCount--;
	sld2::memmove_n(BaseList.data(), BaseList.data() + 1, BaseListCount);
}

static const UInt16* suffix(EMetadataUnitType aUnits)
{
	static const UInt16 *const suffixes[] = {
		SldU16StringLiteral("px"),
		SldU16StringLiteral("pt"),
		SldU16StringLiteral("em"),
		SldU16StringLiteral("mm"),
		SldU16StringLiteral("%")
	};
	static_assert(sizeof(suffixes) / sizeof(suffixes[0]) == eMetadataUnitType_end,
		"The unit suffix data table must be updated.");
	return aUnits >= eMetadataUnitType_end ? SldU16StringLiteral("") : suffixes[aUnits];
}

/** ********************************************************************
 * Переводит размер в строку
 *
 * @param[in]  aStr - указатель на строку куда будет записано строковое представление размера
 *
 * @return код ошибки
 ************************************************************************/
ESldError TSizeValue::ToString(UInt16 *aStr) const
{
	if (!aStr)
		return eMemoryNullPointer;

	sld2::IntFormatter fmt;
	SldU16StringRef string = fmt.format(*this);
	if (string.empty())
	{
		*aStr = 0;
		return eMetadataErrorFromSizeValueConversion;
	}

	sld2::memcopy_n(aStr, string.data(), string.size());
	aStr[string.size()] = '\0';
	return eOK;
}

/** ********************************************************************
 * Парсит размер из строки
 *
 * @param[in]  aStr          - строка откуда будет распарсен размер
 * @param[in]  aDefaultUnits - единицы измерения размера "по умолчанию"
 *
 * @return распарсеный размер
 *         В случае ошибки возвращает "невалидный" размер
 ************************************************************************/
TSizeValue TSizeValue::FromString(const UInt16 *aStr, EMetadataUnitType aDefaultUnits)
{
	if (!aStr)
		return TSizeValue();

	Float32 fValue;
	const UInt16 *end;
	CSldCompare::StrToFloat32(aStr, &end, &fValue);
	if (aStr == end)
		return TSizeValue();

	fValue *= SLD_SIZE_VALUE_SCALE;
	TSizeValue value((Int32)(fValue >= 0.0f ? (fValue + .5f) : (fValue - .5f)), aDefaultUnits);
	for (int index = 0; index < eMetadataUnitType_end; index++)
	{
		const EMetadataUnitType units = (EMetadataUnitType)index;
		if (CSldCompare::StrCmp(end, suffix(units)) == 0)
		{
			value.Units = units;
			break;
		}
	}
	return value;
}
