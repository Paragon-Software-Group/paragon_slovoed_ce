#ifndef _SLD_STYLE_INFO_H_
#define _SLD_STYLE_INFO_H_

#include "SldPlatform.h"
#include "SldTypes.h"
#include "SldError.h"
#include "SldSDCReadMy.h"


/// Класс, хранящий информацию о стиле
class CSldStyleInfo
{
public:

	/// Конструктор
	CSldStyleInfo();

	/// Инициализация
	ESldError Init(CSDCReadMy &aData, UInt32 aStyleDataType, UInt32 aStyleIndex);

	/// Инициализация
	ESldError Init(const UInt8 **aData, UInt32 aStyleIndex);

	/// Проверка на то, был ли класс проинициализирован
	UInt32 IsInit(void) const { return m_isInit; };
	

	/// Возвращает язык данного стиля
	ESldLanguage GetLanguage(void) const;
	
	/// Возвращает количество вариантов данного стиля
	Int32 GetNumberOfVariants(void) const;
	
	/// Возвращает тип конкретного варианта стиля
	ESldError GetVariantType(UInt32* aVariantType, Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;
	
	/// Возвращает номер варианта стиля, который используется по умолчанию
	Int32 GetDefaultVariantIndex(void) const;
	
	/// Устанавливает номер варианта стиля, который будет использоваться по умолчанию
	ESldError SetDefaultVariantIndex(Int32 aVariantIndex);
	
	/// Возвращает количество вариантов использования данного стиля
	Int32 GetNumberOfUsages(void) const;
	
	/// Возвращает код предназначения стиля (например: для заголовков, для перевода, для примеров)
	ESldStyleUsageEnum GetStyleUsage(Int32 aUsageIndex = 0) const;

	/// @deprecated Возвращает жирность шрифта данного стиля
	ESldBoldValue GetBold(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;
	
	/// Возвращает жирность шрифта данного стиля (enum #ESldBoldType)
	UInt32 GetBoldValue(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;

	/// Возвращает наклонность шрифта данного стиля
	UInt32 IsItalic(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;

	/// @deprecated Возвращает подчеркнутость шрифта данного стиля
	UInt32 IsUnderline(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;

	/// Возвращает подчеркнутость шрифта данного стиля (enum #ESldUnderlineType)
	UInt32 GetUnderlineValue(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;

	/// Возвращает флаг того, что используется кастомный цвет подчеркивания (не совпадающий с цветом текста)
	UInt32 UnderlineUseColor(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;

	/// Возвращает код цвета в виде типа HTML, т.е. если его напечатать printf("%06X", style->GetColor());
	/// Он даст правильную строчку для данного цвета.
	UInt32 GetUnderlineColor(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;

	/// Возвращает красную компоненту цвета шрифта.
	UInt32 GetUnderlineColorRed(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;
	/// Возвращает зеленую компоненту цвета шрифта.
	UInt32 GetUnderlineColorGreen(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;
	/// Возвращает синюю компоненту цвета шрифта.
	UInt32 GetUnderlineColorBlue(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;
	/// Возвращает значение альфа-канала цвета шрифта.
	UInt32 GetUnderlineColorAlpha(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;
	
	/// Возвращает перечеркнутость шрифта данного стиля
	UInt32 IsStrikethrough(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;
	
	/// Возвращает надчеркнутость шрифта данного стиля
	UInt32 IsOverline(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;
	
	/// Возвращает видимость данного стиля
	UInt32 IsVisible(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;

	/// Возвращает уровень, на котором написан текст данного стиля
	ESldStyleLevelEnum GetLevel(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;

	/// Возвращает размер шрифта данного стиля
	UInt32 GetTextSize(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;

	/// Возвращает размер шрифта данного стиля в "новом" формате
	TSizeValue GetTextSizeValue(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;

	/// Возвращает высоту строки данного стиля
	UInt32 GetLineHeight(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;

	/// Возвращает высоту строки данного стиля в "новом" формате
	TSizeValue GetLineHeightValue(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;

	/// Возвращает константу, описывающую тип семейства шрифта
	ESldStyleFontFamilyEnum GetStyleFontFamily(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;
	
	/// Возвращает константу, описывающую название шрифта
	ESldStyleFontNameEnum GetStyleFontName(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;

	/// Возвращает константу, описывающую тип данных стиля (таблица, просто текст, картинки, ссылка и т.д.)
	ESldStyleMetaTypeEnum GetStyleMetaType(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;

	/// Возвращает код цвета в виде типа HTML, т.е. если его напечатать printf("%06X", style->GetColor());
	/// Он даст правильную строчку для данного цвета.
	UInt32 GetColor(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;

	/// Возвращает красную компоненту цвета шрифта.
	UInt32 GetColorRed(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;
	/// Возвращает зеленую компоненту цвета шрифта.
	UInt32 GetColorGreen(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;
	/// Возвращает синюю компоненту цвета шрифта.
	UInt32 GetColorBlue(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;
	/// Возвращает значение альфа-канала цвета шрифта.
	UInt32 GetColorAlpha(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;

	/// Возвращает код цвета фона в виде типа HTML, т.е. если его напечатать printf("%06X", style->GetColor());
	/// Он даст правильную строчку для данного цвета.
	UInt32 GetBackgroundColor(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;

	/// Возвращает красную компоненту цвета фона.
	UInt32 GetBackgroundColorRed(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;
	/// Возвращает зеленую компоненту цвета фона.
	UInt32 GetBackgroundColorGreen(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;
	/// Возвращает синюю компоненту цвета фона.
	UInt32 GetBackgroundColorBlue(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;
	/// Возвращает значение альфа-канала цвета фона.
	UInt32 GetBackgroundColorAlpha(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;

	/// Возвращает префикс
	const UInt16* GetPrefix(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;
	SldU16StringRef GetPrefixRef(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;

	/// Возвращает постфикс
	const UInt16* GetPostfix(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;
	SldU16StringRef GetPostfixRef(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;

	/// Возвращает доступность текста для кроссрефа
	UInt32 IsUnclickable(Int32 aVariantIndex = SLD_DEFAULT_STYLE_VARIANT_INDEX) const;

	/// Возвращает имя
	const UInt16* GetTag() const;

	/// Возвращает глобальный индекс стиля
	UInt32 GetGlobalIndex() const { return m_GlobalIdx; }

	/// @private
	void SetGlobalIndex(UInt32 aIndex) { m_GlobalIdx = aIndex; }

private:

	/// Флаг, проинициализирован класс или нет
	UInt32						m_isInit;

	/// Заголовок стиля
	TStyleHeader				m_Header;
	
	/// Заголовки вариантов стиля
	struct VariantHeadersStruct {
		TStyleVariantHeader variant0;
		sld2::DynArray<TStyleVariantHeader> variants;

		const TStyleVariantHeader& operator[](UInt32 n) const {
			return n == 0 ? variant0 : variants[n - 1];
		}
		TStyleVariantHeader& operator[](UInt32 n) {
			return n == 0 ? variant0 : variants[n - 1];
		}

		ESldError init(UInt32 count, const UInt8* ptr);
	} m_VariantHeaders;

	/// Варианты использования стиля
	struct UsagesStruct {
		enum : UInt32 { EmbeddedCount = 4 };
		union {
			UInt32 data[EmbeddedCount];
			UInt32 *ptr;
			char _buf[EmbeddedCount * sizeof(UInt32)]; // to allow aliasing
		} m;

		UInt32 count() const {
			return sld2_container_of(this, const CSldStyleInfo, &CSldStyleInfo::m_Usages)->m_Header.NumberOfUsages;
		}

		// public

		UsagesStruct() { sld2::memzero(m); }

		UsagesStruct(UsagesStruct&& other)
		{
			m = other.m;
			sld2::memzero(other.m);
		}

		UsagesStruct(const UsagesStruct& other)
		{
			init(other.count(), (const UInt8*)other.data());
		}

		UsagesStruct& operator=(UsagesStruct&& other)
		{
			auto old(sld2::move(*this));
			m = other.m;
			sld2::memzero(other.m);
			return *this;
		}

		UsagesStruct& operator=(const UsagesStruct& other)
		{
			auto old(sld2::move(*this));
			init(other.count(), (const UInt8*)other.data());
			return *this;
		}

		~UsagesStruct() { if (count() > EmbeddedCount) sldMemFree(m.ptr); }

		ESldError init(UInt32 count, const UInt8* ptr);
		const UInt32* data() const { return (count() <= EmbeddedCount) ? m.data : m.ptr; }
		ESldStyleUsageEnum operator[](UInt32 n) const { return static_cast<ESldStyleUsageEnum>(data()[n]); }

	} m_Usages;

	/// Вариант отображения стиля, используемый по умолчанию
	Int32						m_DefaultVariantIndex;

	/// Глобальный индекс стиля
	UInt32						m_GlobalIdx;
};

#endif
