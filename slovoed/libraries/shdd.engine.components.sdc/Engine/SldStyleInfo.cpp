#include "SldStyleInfo.h"

#include "SldCompare.h"

enum { VariantPrefixOffset = 1, VariantPostfixOffset = 2 };

CSldStyleInfo::CSldStyleInfo()
	: m_isInit(0), m_DefaultVariantIndex(0), m_GlobalIdx(0)
{
	sld2::memzero(m_Header);
}

/** *********************************************************************
* Инициализация класса, отвечающего за хранение данных о свойствах стиля
*
* @param aData - указатель на объект отвечающий за получение данных из контейнера
* @param aStyleDataType - тип ресурсов с данными стиля
* @param aStyleIndex - номер стиля.
*
* @return код ошибки
************************************************************************/
ESldError CSldStyleInfo::Init(CSDCReadMy &aData, UInt32 aStyleDataType, UInt32 aStyleIndex)
{
	// Читаем ресурс с данными стиля
	auto res = aData.GetResource(aStyleDataType, aStyleIndex);
	if (res != eOK)
		return res.error();

	if (res.size() != ((const TStyleHeader*)res.ptr())->TotalStyleDataSize)
		return eInputWrongStructSize;

	const UInt8* dataPointer = res.ptr();
	return Init(&dataPointer, aStyleIndex);
}

/**
 * Инициализация класса, отвечающего за хранение данных о свойствах стиля
 *
 * @param[inout] aData - указатель на указатель где должны храниться данные стиля
 *                       в случае успеха указатель обновляется и указывает сразу после всех данных
 *                       стиля
 * @param[in]    aStyleIndex - номер стиля
 *
 * @return код ошибки
 */
ESldError CSldStyleInfo::Init(const UInt8 **aData, UInt32 aStyleIndex)
{
	if (aData == NULL || *aData == NULL)
		return eMemoryNullPointer;

	const UInt8 *ptr = *aData;
	const TStyleHeader* pHeader = (const TStyleHeader*)ptr;

	// Общий заголовок стиля
	sldMemCopy(&m_Header, pHeader, pHeader->structSize);

	ptr += m_Header.structSize;

	// Варианты использования стилей
	if (m_Header.NumberOfUsages)
	{
		if (m_Header.SizeOfStyleUsage != sizeof(m_Usages[0]))
			return eInputWrongStructSize;

		ESldError error = m_Usages.init(m_Header.NumberOfUsages, ptr);
		if (error != eOK)
			return error;
		ptr += m_Header.NumberOfUsages * m_Header.SizeOfStyleUsage;
	}

	// Заголовки вариантов стилей
	if (m_Header.NumberOfVariants)
	{
		if (m_Header.SizeOfStyleVariant != sizeof(m_VariantHeaders[0]))
			return eInputWrongStructSize;

		ESldError error = m_VariantHeaders.init(m_Header.NumberOfVariants, ptr);
		if (error != eOK)
			return error;
		ptr += m_Header.NumberOfVariants * m_Header.SizeOfStyleVariant;
	}

	m_isInit = 1;
	m_DefaultVariantIndex = (Int32)m_Header.DefaultVariantIndex;
	m_GlobalIdx = aStyleIndex;

	*aData = ptr;
	return eOK;
}

ESldError CSldStyleInfo::VariantHeadersStruct::init(UInt32 aCount, const UInt8* aPtr)
{
	if (aCount > 1 && !variants.resize(sld2::default_init, aCount - 1))
		return eMemoryNotEnoughMemory;

	auto ptr = (const TStyleVariantHeader*)aPtr;
	sld2::memcopy_n(&variant0, ptr, 1);
	sld2::memcopy_n(variants.data(), ptr + 1, aCount - 1);

	for (UInt32 i = 0; i < aCount; i++)
	{
		auto& variant = (*this)[i];
		const UInt32 count = sld2::array_size(variant.Reserved);
		variant.Reserved[count - VariantPrefixOffset] = sld2::StrLen(variant.Prefix);
		variant.Reserved[count - VariantPostfixOffset] = sld2::StrLen(variant.Postfix);
	}

	return eOK;
}

ESldError CSldStyleInfo::UsagesStruct::init(UInt32 aCount, const UInt8* aPtr)
{
	if (aCount > EmbeddedCount)
	{
		m.ptr = sldMemNew<UInt32>(aCount);
		if (!m.ptr)
			return eMemoryNullPointer;
	}
	sld2::memcopy_n(aCount <= EmbeddedCount ? m.data : m.ptr, (const UInt32*)aPtr, aCount);
	return eOK;
}

/// Возвращает язык данного стиля
ESldLanguage CSldStyleInfo::GetLanguage(void) const
{
	return SldLanguage::fromCode(m_Header.Language);
}

/// Возвращает количество вариантов данного стиля
Int32 CSldStyleInfo::GetNumberOfVariants(void) const
{
	return (Int32)m_Header.NumberOfVariants;
}

/// Возвращает количество вариантов использования данного стиля
Int32 CSldStyleInfo::GetNumberOfUsages(void) const
{
	return (Int32)m_Header.NumberOfUsages;
}

// simple macro which sanitizes @_index if it's SLD_DEFAULT_STYLE_VARIANT_INDEX
// and returns @_ret if it's out of range
// may be called only inside CSldStyleInfo methods as it accesses its fields
#define CHECK_VARIANT_INDEX(_index, _ret) do { \
	if (_index == SLD_DEFAULT_STYLE_VARIANT_INDEX) \
		_index = m_DefaultVariantIndex; \
	if ((UInt32)_index >= m_Header.NumberOfVariants) \
		return _ret; \
} while(0)

/** *********************************************************************
* Возвращает тип конкретного варианта стиля
*
* @param aVariantType[out]	- указатель на переменную, куда будет сохранен тип варианта
* @param aVariantIndex[in]	- номер запрашиваемого варианта
*
* @return код ошибки
************************************************************************/
ESldError CSldStyleInfo::GetVariantType(UInt32* aVariantType, Int32 aVariantIndex) const
{
	if (!aVariantType)
		return eMemoryNullPointer;

	CHECK_VARIANT_INDEX(aVariantIndex, eCommonWrongIndex);

	*aVariantType = m_VariantHeaders[aVariantIndex].VariantType;
	return eOK;
}

/// Возвращает номер варианта стиля, который используется по умолчанию
Int32 CSldStyleInfo::GetDefaultVariantIndex(void) const
{
	return m_DefaultVariantIndex;
}

/// Устанавливает номер варианта стиля, который будет использоваться по умолчанию
ESldError CSldStyleInfo::SetDefaultVariantIndex(Int32 aVariantIndex)
{
	if ((UInt32)aVariantIndex >= m_Header.NumberOfVariants)
		return eCommonWrongIndex;

	m_DefaultVariantIndex = aVariantIndex;
	return eOK;
}

/// Возвращает жирность шрифта данного стиля
ESldBoldValue CSldStyleInfo::GetBold(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, eBoldValueNormal);
	return (ESldBoldValue)m_VariantHeaders[aVariantIndex].Bold;
}

/// Возвращает жирность шрифта данного стиля (enum #ESldBoldType)
UInt32 CSldStyleInfo::GetBoldValue(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, eBoldValueNormal);
	return m_VariantHeaders[aVariantIndex].Bold;
}

/// Возвращает наклонность шрифта данного стиля
UInt32 CSldStyleInfo::IsItalic(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	return m_VariantHeaders[aVariantIndex].Italic;
}

/// Возвращает подчеркнутость шрифта данного стиля
UInt32 CSldStyleInfo::IsUnderline(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	return m_VariantHeaders[aVariantIndex].Underline;
}

/// Возвращает подчеркнутость шрифта данного стиля (enum #ESldUnderlineType)
UInt32 CSldStyleInfo::GetUnderlineValue(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, eUnderlineNone);
	return m_VariantHeaders[aVariantIndex].Underline;
}

/// Возвращает флаг того, что используется кастомный цвет подчеркивания (не совпадающий с цветом текста)
UInt32 CSldStyleInfo::UnderlineUseColor(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, false);
	return m_VariantHeaders[aVariantIndex].UnderlineUseCustomColor;
}

/// Возвращает код цвета в виде типа HTML, т.е. если его напечатать printf("%06X", style->GetColor());
/// Он даст правильную строчку для данного цвета.
UInt32 CSldStyleInfo::GetUnderlineColor(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	const auto &v = m_VariantHeaders[aVariantIndex];
	return (v.UnderlineColorRed << 16) | (v.UnderlineColorGreen << 8) | (v.UnderlineColorBlue);
}

/// Возвращает красную компоненту для цвета подчеркивания шрифта.
UInt32 CSldStyleInfo::GetUnderlineColorRed(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	return m_VariantHeaders[aVariantIndex].UnderlineColorRed;
}

/// Возвращает зеленую компоненту для цвета подчеркивания шрифта.
UInt32 CSldStyleInfo::GetUnderlineColorGreen(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	return m_VariantHeaders[aVariantIndex].UnderlineColorGreen;
}

/// Возвращает синюю компоненту для цвета подчеркивания шрифта.
UInt32 CSldStyleInfo::GetUnderlineColorBlue(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	return m_VariantHeaders[aVariantIndex].UnderlineColorBlue;
}

/// Возвращает значение альфа-канала для цвета подчеркивания шрифта.
UInt32 CSldStyleInfo::GetUnderlineColorAlpha(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	return m_VariantHeaders[aVariantIndex].UnderlineColorAlpha;
}

/// Возвращает перечеркнутость шрифта данного стиля
UInt32 CSldStyleInfo::IsStrikethrough(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	return m_VariantHeaders[aVariantIndex].Strikethrough;
}

/// Возвращает надчеркнутость шрифта данного стиля
UInt32 CSldStyleInfo::IsOverline(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	return m_VariantHeaders[aVariantIndex].Overline;
}

/// Возвращает видимость данного стиля
UInt32 CSldStyleInfo::IsVisible(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	return m_VariantHeaders[aVariantIndex].Visible;
}

/// Возвращает уровень, на котором написан текст данного стиля
ESldStyleLevelEnum CSldStyleInfo::GetLevel(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, eLevelNormal);
	return (ESldStyleLevelEnum)m_VariantHeaders[aVariantIndex].Level;
}

/**
 * Возвращает размер шрифта данного стиля
 *
 * Возвращает либо #ESldStyleSizeEnum для "именованных" размеров либо SIZE_VALUE_UNITS
 * для "численных" размеров с единицами измерения.
 * При SIZE_VALUE_UNITS для получения размера необходимо вызвать ::GetTextSizeValue()
 */
UInt32 CSldStyleInfo::GetTextSize(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	return m_VariantHeaders[aVariantIndex].TextSize;
}

/**
 * Возвращает размер шрифта данного стиля в "новом" формате
 *
 * Примеры использования данного API:
 *
 *  1. "Полностью новый" API
 *    TSizeValue size = style->GetTextSizeValue();
 *    if (!size.Valid())
 *    {
 *      ESldStyleSizeEnum eSize = style->GetTextSize();
 *      // Используем "именованные" размеры
 *    }
 *    else
 *    {
 *      // Используем size напрямую
 *    }
 *
 *  2. Предполагаемый "порт" со старого API
 *    UInt32 size = style->GetTextSize();
 *    if (size == SIZE_VALUE_UNITS)
 *    {
 *      TSizeValue sizeValue = style->GetTextSizeValue();
 *      // Используем новую логику для размеров
 *    }
 *    else
 *    {
 *      // Используем старую логику
 *    }
 */
TSizeValue CSldStyleInfo::GetTextSizeValue(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, TSizeValue());

	const TStyleVariantHeader *variant = &m_VariantHeaders[aVariantIndex];
	const UInt32 textSize = variant->TextSize;
	if (textSize == SIZE_VALUE_UNITS)
		return variant->TextSizeValue;

	// Ideally we'd have a eSize_MAX enum value or smth to check for
	if (textSize > eSizeExtraSmall)
		return TSizeValue(textSize * SLD_SIZE_VALUE_SCALE, eMetadataUnitType_pt);

	return TSizeValue();
}

/**
 * Возвращает размер межстрочного интервала данного стиля
 *
 * Возвращает либо #ESldStyleSizeEnum для "именованных" размеров либо SIZE_VALUE_UNITS
 * для "численных" размеров с единицами измерения.
 * При SIZE_VALUE_UNITS для получения "реального" размера необходимо вызвать ::GetLineHeightValue()
 */
UInt32 CSldStyleInfo::GetLineHeight(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	return m_VariantHeaders[aVariantIndex].LineHeight;
}

/// Возвращает размер межстрочного интервала данного стиля в новом формате
// Ведет себя так же как ::GetTextSizeValue()
TSizeValue CSldStyleInfo::GetLineHeightValue(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, TSizeValue());

	const TStyleVariantHeader *variant = &m_VariantHeaders[aVariantIndex];
	const UInt32 lineHeight = variant->LineHeight;
	if (lineHeight == SIZE_VALUE_UNITS)
		return variant->LineHeightValue;

	// Ideally we'd have a eSize_MAX enum value or smth to check for
	if (lineHeight > eSizeExtraSmall)
		return TSizeValue(lineHeight * SLD_SIZE_VALUE_SCALE, eMetadataUnitType_pt);

	return TSizeValue();
}

/// Возвращает код предназначения стиля (например: для заголовков, для перевода, для примеров)
ESldStyleUsageEnum CSldStyleInfo::GetStyleUsage(Int32 aUsageIndex) const
{
	if (!m_Header.NumberOfUsages)
		return eStyleUsage_Unknown;
	
	if ((UInt32)aUsageIndex >= m_Header.NumberOfUsages)
		return eStyleUsage_Unknown;
	
	return m_Usages[aUsageIndex];
}

/// Возвращает константу, описывающую тип семейства шрифта
ESldStyleFontFamilyEnum CSldStyleInfo::GetStyleFontFamily(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, eFontFamily_SansSerif);
	return (ESldStyleFontFamilyEnum)m_VariantHeaders[aVariantIndex].FontFamily;
}

/// Возвращает константу, описывающую название шрифта
ESldStyleFontNameEnum CSldStyleInfo::GetStyleFontName(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, eFontName_DejaVu_Sans);
	return (ESldStyleFontNameEnum)m_VariantHeaders[aVariantIndex].FontName;
}

/// Возвращает константу, описывающую тип данных стиля (таблица, просто текст, картинки, ссылка и т.д.)
ESldStyleMetaTypeEnum CSldStyleInfo::GetStyleMetaType(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, eMetaUnknown);
	return (ESldStyleMetaTypeEnum)m_VariantHeaders[aVariantIndex].TextType;
}

/// Возвращает код цвета в виде типа HTML, т.е. если его напечатать printf("%06X", style->GetColor());
/// Он даст правильную строчку для данного цвета.
UInt32 CSldStyleInfo::GetColor(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	const auto &v = m_VariantHeaders[aVariantIndex];
	return (v.ColorRed << 16) | (v.ColorGreen << 8) | (v.ColorBlue);
}

/// Возвращает красную компоненту цвета шрифта.
UInt32 CSldStyleInfo::GetColorRed(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	return m_VariantHeaders[aVariantIndex].ColorRed;
}
/// Возвращает зеленую компоненту цвета шрифта.
UInt32 CSldStyleInfo::GetColorGreen(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	return m_VariantHeaders[aVariantIndex].ColorGreen;
}
/// Возвращает синюю компоненту цвета шрифта.
UInt32 CSldStyleInfo::GetColorBlue(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	return m_VariantHeaders[aVariantIndex].ColorBlue;
}

/// Возвращает значение альфа-канала цвета шрифта.
UInt32 CSldStyleInfo::GetColorAlpha(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	return m_VariantHeaders[aVariantIndex].ColorAlpha;
}

/// Возвращает код цвета фона в виде типа HTML, т.е. если его напечатать printf("%06X", style->GetColor());
/// Он даст правильную строчку для данного цвета.
UInt32 CSldStyleInfo::GetBackgroundColor(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	const auto &v = m_VariantHeaders[aVariantIndex];
	return (v.BackgroundColorRed << 16) | (v.BackgroundColorGreen << 8) | (v.BackgroundColorBlue);
}

/// Возвращает красную компоненту цвета фона.
UInt32 CSldStyleInfo::GetBackgroundColorRed(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	return m_VariantHeaders[aVariantIndex].BackgroundColorRed;
}
/// Возвращает зеленую компоненту цвета фона.
UInt32 CSldStyleInfo::GetBackgroundColorGreen(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	return m_VariantHeaders[aVariantIndex].BackgroundColorGreen;
}
/// Возвращает синюю компоненту цвета фона.
UInt32 CSldStyleInfo::GetBackgroundColorBlue(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	return m_VariantHeaders[aVariantIndex].BackgroundColorBlue;
}

/// Возвращает значение альфа-канала цвета фона.
UInt32 CSldStyleInfo::GetBackgroundColorAlpha(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	return m_VariantHeaders[aVariantIndex].BackgroundColorAlpha;
}

/// Возвращает префикс
const UInt16* CSldStyleInfo::GetPrefix(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, nullptr);
	return m_VariantHeaders[aVariantIndex].Prefix;
}

/// Возвращает префикс
SldU16StringRef CSldStyleInfo::GetPrefixRef(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, nullptr);
	const auto &v = m_VariantHeaders[aVariantIndex];
	return SldU16StringRef(v.Prefix, v.Reserved[sld2::array_size(v.Reserved) - VariantPrefixOffset]);
}

/// Возвращает постфикс
const UInt16* CSldStyleInfo::GetPostfix(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, nullptr);
	return m_VariantHeaders[aVariantIndex].Postfix;
}

/// Возвращает префикс
SldU16StringRef CSldStyleInfo::GetPostfixRef(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, nullptr);
	const auto &v = m_VariantHeaders[aVariantIndex];
	return SldU16StringRef(v.Postfix, v.Reserved[sld2::array_size(v.Reserved) - VariantPostfixOffset]);
}

/// Возвращает доступность текста для кроссрефа
UInt32 CSldStyleInfo::IsUnclickable(Int32 aVariantIndex) const
{
	CHECK_VARIANT_INDEX(aVariantIndex, 0);
	return m_VariantHeaders[aVariantIndex].Unclickable;
}

/// Возвращает название тега
const UInt16* CSldStyleInfo::GetTag() const
{
	return m_Header.Tag;
}
