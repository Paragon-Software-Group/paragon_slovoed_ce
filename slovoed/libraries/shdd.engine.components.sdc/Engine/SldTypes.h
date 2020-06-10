#ifndef _SLD_TYPES_H_
#define _SLD_TYPES_H_

#include "SldPlatform.h"

#include "SldDefines.h"
#include "SldError.h"
#include "SldString.h"
#include "SldTypeDefs.h"
#include "SldVector.h"

/// вектор двухбайтовых строк
typedef CSldVector<SldU16String> SldU16WordsArray;

/// Структура, описывающая свойства одного варианта написания списка слов
typedef struct TListVariantProperty
{
	/// Номер варианта написания
	UInt32 Number;
	/// Тип варианта написания (см. #EListVariantTypeEnum)
	UInt32 Type;
	/// Код языка
	UInt32 LangCode;
	/// Зарезервировано
	UInt32 Reserved[5];

	TListVariantProperty()
	{
		sldMemZero(this, sizeof(TListVariantProperty));
	}

}TListVariantProperty;

/// Индекс отсутствующего перевода (для TCatalogPath::WordIndex)
#define TRANSLATION_NO	(0xFFFFFFFFUL)

/// Структура, описывающая путь в иерархии к определенному месту.
class TCatalogPath
{
private:
	class ListIndexStruct {
		friend class TCatalogPath;
		UInt32 value;

		ListIndexStruct(UInt32 v) : value(v) {}
		ListIndexStruct& operator=(const ListIndexStruct&) = default;

		ListIndexStruct& operator=(UInt32 v) { value = v; return *this; }
	public:
		operator UInt32() const { return value; }
		void Set(UInt32 v) { value = v; }
	};

	class BaseListCountStruct {
		friend class TCatalogPath;
		UInt32 value;

		BaseListCountStruct(UInt32 v) : value(v) {}
		BaseListCountStruct& operator=(const BaseListCountStruct&) = default;

		BaseListCountStruct& operator=(UInt32 v) { value = v; return *this; }

		BaseListCountStruct operator++(int) { return value++; }
		BaseListCountStruct operator--(int) { return value--; }
	public:
		operator UInt32() const { return value; }
	};

	class BaseListStruct
	{
		friend TCatalogPath;

		// 5 is chosen because on x64 the class has 8 byte alignement
		// and we have a "hole" between capacity and the pointer, so
		// we can safely use a non-even count and 3 is too small
		enum : UInt32 { EmbeddedCapacity = 5 };
		union {
			struct {
				UInt32 Capacity;
				Int32 Data[EmbeddedCapacity];
			};
			struct {
				UInt32 _Capacity; // hole after this one on x64
				Int32* Ptr;
			};
			char _buf[sizeof(UInt32) + sizeof(Int32) * EmbeddedCapacity]; // to allow aliasing
		} m;

		BaseListStruct() { sld2::memzero(m); }

		BaseListStruct(BaseListStruct&& other)
		{
			m = other.m;
			sld2::memzero(other.m);
		}

		BaseListStruct& operator=(BaseListStruct&& other)
		{
			auto old(sld2::move(*this));
			m = other.m;
			sld2::memzero(other.m);
			return *this;
		}

		BaseListStruct(const BaseListStruct&) = delete;
		BaseListStruct& operator=(const BaseListStruct&) = delete;

		~BaseListStruct();

		Int32* data() { return m.Capacity == 0 ? m.Data : m.Ptr; }
		const Int32* data() const { return m.Capacity == 0 ? m.Data : m.Ptr; }
		UInt32 size() const { return m.Capacity == 0 ? EmbeddedCapacity : m.Capacity; }

		// valid only when the storage is on the heap
		Int32*& dynList() { return m.Ptr; }

		ESldError reallocate(UInt32 newSize = 0);

	public:
		const Int32& operator[](UInt32 n) const { return data()[n]; }

		UInt32 Count() const {
			return sld2_container_of(this, const TCatalogPath, &TCatalogPath::BaseList)->BaseListCount;
		}
		const Int32& First() const { return data()[0]; }
		const Int32& Last() const { return data()[Count() - 1]; }

		explicit operator bool() const { return Count() != 0; }
	};
public:
	/// Номер списка слов
	ListIndexStruct ListIndex;
	/// Количество элементов в пути
	BaseListCountStruct BaseListCount;
	/// Список элементов в пути каталога
	BaseListStruct BaseList;

	/// Указывает ли путь на самый верхний уровень иерархии
	bool isRoot() const { return BaseListCount < 2; }

	/// Возвращает локальный индекс слова, которое является родительским для данного пути
	/// Если путь указывает на самый верхний уровень иерархии (нет родительского слова), возвращает -1
	Int32 GetParentWordLocalIndex() const { return isRoot() ? -1 : BaseList[BaseListCount - 2]; }

	/// Пустой ли путь
	bool Empty() const { return BaseListCount == 0; }

	/// Стандартный конструктор
	TCatalogPath() : ListIndex(0), BaseListCount(0) {}

	TCatalogPath(const TCatalogPath&) = delete;
	TCatalogPath& operator=(const TCatalogPath&) = delete;

	TCatalogPath(TCatalogPath&& other)
		: ListIndex(other.ListIndex), BaseListCount(other.BaseListCount), BaseList(sld2::move(other.BaseList))
	{
		other.ListIndex = 0;
		other.BaseListCount = 0;
	}

	TCatalogPath& operator=(TCatalogPath&& other)
	{
		ListIndex = other.ListIndex;
		BaseListCount = other.BaseListCount;
		BaseList = sld2::move(other.BaseList);

		other.ListIndex = 0;
		other.BaseListCount = 0;
		return *this;
	}

	/// Деструктор
	~TCatalogPath() = default;

	/// Очищает путь от всех элементов
	void Clear()
	{
		ListIndex = 0;
		BaseListCount = 0;
	}

	/**
	 * Добавляет новый локальный индекс слова в путь
	 *
	 * @param[in]  aLocalWordIndex - локальный индекс слова
	 *
	 * @return код ошибки
	 */
	ESldError PushList(Int32 aLocalWordIndex)
	{
		if (BaseListCount >= BaseList.size())
		{
			ESldError error = BaseList.reallocate();
			if (error != eOK)
				return error;
		}

		BaseList.data()[BaseListCount++] = aLocalWordIndex;
		return eOK;
	}

	/// Копирует текущий путь в переданный
	ESldError CopyTo(TCatalogPath &aPath) const;

	/// Копирует текущий путь в переданный
	ESldError CopyTo(TCatalogPath *aPath) const {
		return aPath ? CopyTo(*aPath) : eMemoryNullPointer;
	}

	// removes the first element from the list
	void popFront();

	// range-based for loop support
	Int32* begin() { return BaseList.data(); }
	Int32* end() { return BaseList.data() + BaseListCount; }
	const Int32* begin()  const { return BaseList.data(); }
	const Int32* end()    const { return BaseList.data() + BaseListCount; }
	const Int32* cbegin() const { return begin(); }
	const Int32* cend()   const { return end(); }
};

#define SLD_SIZE_VALUE_SCALE (100)

/// Структурка отвечающая за размеры
typedef struct TSizeValue
{
	/// Число
	/// ВАЖНО! Число хранится в умноженном на SLD_SIZE_VALUE_SCALE (100) виде.
	/// Т.е. 1 хранится как 100, 1.01 - 101, .01 - 1 и т.п.
	Int32 Value;
	/// Единица измерения (см. #EMetadataUnitType)
	UInt32 Units;

	/// Создает TSizeValue с дефолтными значениями (невалидными)
	TSizeValue() : Value(-1), Units(eMetadataUnitType_UNKNOWN) {}

	/// Создает TSizeValue c переданными значениями
	/// ВАЖНО! aValue должен быть "скалированным" - т.е. 320 вместо 3.2, 100 вместо 1 и т.п.
	TSizeValue(Int32 aValue, EMetadataUnitType aUnits) : Value(aValue), Units(aUnits) {}

	/// Присваивает заданные значения
	void Set(Int32 aValue, EMetadataUnitType aUnits) { Value = aValue; Units = aUnits; }

	/// Возвращает является ли размер "валидным"
	bool IsValid() const { return Units < eMetadataUnitType_end; }

	/// Возвращает размер в виде "скалированного" Float32
	Float32 AsFloat32() const { return (Float32)Value / (Float32)SLD_SIZE_VALUE_SCALE; }

	/// Возвращает целую часть размера
	Int32 Integer() const { return Value / SLD_SIZE_VALUE_SCALE; }

	/// Возвращает дробную часть размера - от 0 до 100
	UInt32 Fractional() const { return (Value >= 0 ? Value : -Value) % SLD_SIZE_VALUE_SCALE; }

	/// Возвращает является ли число 100%
	bool Is100Percent() const { return Value == 100 * SLD_SIZE_VALUE_SCALE && Units == eMetadataUnitType_percent; }

	/// Переводит размер в строку
	ESldError ToString(UInt16 *aStr) const;

	/// Парсит строку на "наличие" размера
	static TSizeValue FromString(const UInt16 *aStr, EMetadataUnitType aDefaultUnits = eMetadataUnitType_UNKNOWN);

	// Арифметические операторы
	TSizeValue& operator*=(Float32 scale)
	{
		Value = (Int32)(Value * scale);
		return *this;
	}

	TSizeValue& operator/=(Float32 scale)
	{
		Value = (Int32)(Value / scale);
		return *this;
	}

	TSizeValue& operator*=(Int32 scale)
	{
		Value *= scale;
		return *this;
	}

	TSizeValue& operator/=(Int32 scale)
	{
		Value /= scale;
		return *this;
	}

} TSizeValue;

static inline bool operator==(const TSizeValue &lhs, const TSizeValue &rhs)
{
	return lhs.Units == rhs.Units && lhs.Value == rhs.Value;
}
static inline bool operator!=(const TSizeValue &lhs, const TSizeValue &rhs)
{
	return !(lhs == rhs);
}

// Арифметические операторы
static inline TSizeValue operator+(const TSizeValue &lhs, const TSizeValue &rhs)
{
	if (lhs.Units != rhs.Units || lhs.Units == eMetadataUnitType_UNKNOWN)
		return TSizeValue();
	return TSizeValue(lhs.Value + rhs.Value, (EMetadataUnitType)lhs.Units);
}

static inline TSizeValue operator-(const TSizeValue &lhs, const TSizeValue &rhs)
{
	if (lhs.Units != rhs.Units || lhs.Units == eMetadataUnitType_UNKNOWN)
		return TSizeValue();
	return TSizeValue(lhs.Value - rhs.Value, (EMetadataUnitType)lhs.Units);
}

static inline TSizeValue operator*(const TSizeValue &lhs, Float32 rhs)
{
	return TSizeValue((Int32)(lhs.Value * rhs), (EMetadataUnitType)lhs.Units);
}

static inline TSizeValue operator/(const TSizeValue &lhs, Float32 rhs)
{
	return TSizeValue((Int32)(lhs.Value / rhs), (EMetadataUnitType)lhs.Units);
}

static inline TSizeValue operator*(const TSizeValue &lhs, Int32 rhs)
{
	return TSizeValue(lhs.Value * rhs, (EMetadataUnitType)lhs.Units);
}

static inline TSizeValue operator/(const TSizeValue &lhs, Int32 rhs)
{
	return TSizeValue(lhs.Value / rhs, (EMetadataUnitType)lhs.Units);
}

/// Признак того что в стиль зашиты размеры с единицами измерения
#define SIZE_VALUE_UNITS	(0xFFFFFFFFUL)

/// Заголовок варианта стиля
typedef struct TStyleVariantHeader
{
	/// Размер структуры в байтах
	UInt32 structSize;
	
	/// Тип варианта (см. #EStyleVariantType)
	UInt32 VariantType;
	
	/// Флаг, если установлен, значит текст видимый
	UInt32 Visible;
	
	/// Информация, как должен восприниматься текст (см. #ESldStyleMetaTypeEnum)
	/// Если тип - метаданные, то в тексте содержатся только 
	/// конкретные данные для обработки - например ссылки или еще что-то
	UInt32 TextType;
	
	/// Выравнивание текста по высоте (см. #ESldStyleLevelEnum)
	UInt32 Level;
	
	/// Величина красной компоненты цвета шрифта
	UInt32 ColorRed;
	/// Величина зеленой компоненты цвета шрифта
	UInt32 ColorGreen;
	/// Величина синей компоненты цвета шрифта
	UInt32 ColorBlue;
	/// Значение альфа-канала
	UInt32 ColorAlpha;

	/// Величина красной компоненты цвета фона
	UInt32 BackgroundColorRed;
	/// Величина зеленой компоненты цвета фона
	UInt32 BackgroundColorGreen;
	/// Величина синей компоненты цвета фона
	UInt32 BackgroundColorBlue;
	/// Значение альфа-канала
	UInt32 BackgroundColorAlpha;
	
	/// Стиль насыщенности, значение енума #ESldBoldValue
	UInt32 Bold;

	/// Флаг, если установлен, значит текст написан наклонным
	UInt32 Italic;

	/// Стиль подчеркивания, значение енума, #ESldUnderlineType
	UInt32 Underline;
	
	/// Флаг, если установлен, значит текст перечеркнут
	UInt32 Strikethrough;

	/// Размер текста (см. #ESldStyleSizeEnum)
	/// Число больше 5 - конкретный размер текста
	/// Если == SIZE_VALUE_UNITS => смотри TextSizeValue & TextSizeUnits
	UInt32 TextSize;

	/// Высота строки текста (см. #ESldStyleSizeEnum)
	/// Число больше 5 - абсолютный размер
	/// Если == SIZE_VALUE_UNITS => смотри LineHeightValue & LineHeightUnits
	UInt32 LineHeight;

	/// Семейство шрифта (см. #ESldStyleFontFamilyEnum)
	UInt32 FontFamily;

	/// Название шрифта (см. #ESldStyleFontNameEnum)
	UInt32 FontName;
	
	/// Префикс
	UInt16 Prefix[SLD_MAX_STYLE_PREFIX_SIZE + 1];
	
	/// Постфикс
	UInt16 Postfix[SLD_MAX_STYLE_POSTFIX_SIZE + 1];
	
	/// Флаг, если установлен, значит текст надчеркнут
	UInt32 Overline;

	/// Индекс верхней фоновой картинки
	Int32 BackgroundTopImageIndex;

	/// Индекс нижней фоновой картинки
	Int32 BackgroundBottomImageIndex;

	/// Если установлен данный флаг - текст размеченным данным стилем будет недоступен для кроссрефа
	UInt32 Unclickable;

	/// Флаг того, что для подчеркивания нужно использовать свой, отдельно определенный цвет
	UInt32 UnderlineUseCustomColor;

	/// Красная составляющая цвета для подчеркивания 
	UInt32 UnderlineColorRed;

	/// Зеленая составляющая цвета для подчеркивания 
	UInt32 UnderlineColorGreen;

	/// Синяя составляющая цвета для подчеркивания 
	UInt32 UnderlineColorBlue;

	/// Альфа составляющая цвета для подчеркивания 
	UInt32 UnderlineColorAlpha;

	/// Размер текста с явно заданными единицами измерения
	TSizeValue TextSizeValue;

	/// Высота строки текста с явно заданными единицами измерения
	TSizeValue LineHeightValue;

	/// Зарезервировано
	UInt32 Reserved[14];

} TStyleVariantHeader;

/// Структура хранящая информацию для системы защиты.
typedef struct TRegistrationData
{
	/// HASH для декодирования данного словаря
	UInt32	HASH;
	/// Порядковый номер серийного номера.
	UInt32	Number;
	/// Количество просмотров перевода
	UInt32	Clicks;
	/// Срок работоспособности.
	UInt32	Date;
}TRegistrationData;


/// Структура хранящая информацию генератора случайных чисел.
typedef struct TRandomSeed
{
	/// Параметр генератора случайных чисел инициируемый пользователем
	UInt32	Seed;
	/// Остальные параметры генератора случайных чисел
	UInt32	y;
	/// Остальные параметры генератора случайных чисел
	UInt32	z;
	/// Остальные параметры генератора случайных чисел
	UInt32	c;
}TRandomSeed;


/// Структура таблицы аттрибутов символов
typedef struct TSoundLieralType
{
	UInt8 _type;
	UInt8 _small;
	UInt8 _big;
} TSoundLieralType;

/// Общий заголовок файла озвучки
typedef struct TSoundFileHeader
{
	/// Размер структуры
	UInt32 structSize;
	/// Версия формата озвучки
	UInt32 Version;
	/// Формат звуковых данных (spx, wav, ...)
	UInt32 SoundFormat;
	/// Размер собственно звуковых данных без заголовка TSoundFileHeader и специфического заголовка wav или spx
	UInt32 SoundDataSize;

	/// Частота звукового потока
	UInt32 SampleRate;
	/// Битность (8 или 16)
	UInt32 BitFormat;
	/// Количество каналов
	UInt32 ChannelCount;
	/// Зарезервировано
	UInt32 Reserved[1];

} TSoundFileHeader;

/// Специфический заголовок файла озвучки speex
typedef struct TSpeexHeader
{
	/// Размер структуры
	UInt32 structSize;
	/// Версия формата озвучки
	UInt32 Version;
	/// В структуре версии 1 здесь хранилось Quality - Качество сжатого звука
	/// 1 - наихудшее (максимальное сжатие), 10 - наилучшее (минимальное сжатие), в версии 2:
	/// Размер пакета декодированных данных
	UInt32 PacketSize;
	/// Зарезервировано
	UInt32 Reserved[1];

} TSpeexHeader;

/// Структура заголовка звуковой базы.
typedef struct TSoundHeaderType
{
	//	0:
	/// Номер версии формата базы.
	UInt32 Version; 
	//	4:
	/// Информация о HASH(для возможности проигрывать озвучку в незарегистрированном режиме)
	UInt32 DictIDxorHASH;
	//	8:
	/// Количество слов имеющихся в базе.
	UInt16 WordCount;
	//	10:
	/// Частота оцифровки
	UInt16 freq;
	//	12:
	/// Точность - количество бит на один отсчет.
	UInt8 precision;
	//	13:
	/// Количество каналов звука - по идее звук должен быть моно.
	UInt8 channels;
	//	14:
	/// Коэффициент сжатия Speex
	UInt8 SpeexCodecKoef;
	//	15:
	/// Краткий номер базы - не используется.
	UInt8 LangIDShort;
	//	16:
	/// Код качества
	UInt8 quality;
	//	17:
	/// Количество записей с озвучкой на один ресурс.
	UInt8 EntriesPerBlock;
	//	18:
	/// Зарезервировано
	UInt8 temp1;
	//	19:
	/// Зарезервировано
	UInt8 temp2;
	//	20:
	/// Некий текст - не используется.
	UInt8 buf[512]; // text field
	//	532
	/// Код языка.
	UInt32 LanguageCode;
	//	536:
	/// Имя автора звуковой базы
	UInt8 Author[64];
	//	600:
	/// Ссылка на автора базы
	UInt8 WebAuthor[64];
	//	664:
	/// Строчка с текстом "Quality"
	UInt8 QualityText[64];
	//	728:
	/// Полное маркетинговое название звука
	UInt8 SoundMarketingName[64];
	//	792:
	/// Идентификатор озвучки
	UInt32 DictID;
	//	796:
	/// Название размера данной звуковой базы.
	/** 
		Согласно последним веяниям это должно быть: \n
		Compact \n
		Classic \n
		DeLuxe
	*/
	UInt8 SizeName[64];
	//	860:
	/// название качества: Low, Phone, Medium, Hi-Fi, Hi-End
	UInt8 QualityName[64];
	//	924:
	/// Название языка.
	UInt8 LanguageName[64];
	//	988:
	/// Количество озвученых слов.
	UInt32	WordCountLong;
	//	992:
	/// Зарезервировано
	UInt8 reserved[96]; // total struct size must be 1088
} TSoundHeaderType, *TSoundHeaderPtr;

/// Структура хранящая данные результатов последнего поиска слова.
typedef struct
{
	/// Флаг:	0 - поиска не было или он не удачный.
	///			1 - поиск был успешен, можно использовать для перехода к слову.
	Int8 set;
	/// Зарезервировано для лучшего выравнивания.
	UInt8	Reserved[3];
	/// Последнее искомое слово.
	UInt8 word[SLD_SOUND_MAX_WORD_LEN];
	/// Смещение в битах.
	Int32 bit;
	/// Номер записи в таблице быстрого доступа.
	Int16 block;
	/// Смещение слова от первой записи.
	Int16 dataOffset;
	/// Номер ресурса с данными.
	UInt32	data_index;
	/// Номер последнего декодированного слова
	Int32	CurrentWord;
} TSoundLastSearchResult;

/// Количество символов текста в одной записи индексной таблицы
#define IndexArrayElementSize	(4)

/// Элемент быстрого доступа к озвучке.
typedef struct
{
	/// Первые 4 символа слова
	UInt8 w[IndexArrayElementSize];
	/// количество бит смещения от начала пожатых данных.
	UInt32 bit;
}TSoundIndexArrayElement;

/// Количество символов текста в одной записи "большой" индексной таблицы.
#define IndexArrayBigElementSize	(8)

/// Элемент быстрого доступа к озвучке.
typedef struct
{
	/// Первые 8 символов слова
	UInt8 w[IndexArrayBigElementSize];
	/// количество бит смещения от начала пожатых данных.
	UInt32 bit;
}TSoundIndexArrayBigElement;

/// Структура ресурса с юникодными строчками.
typedef struct TUnicodeSoundType
{
	/// Код языка.
	UInt32	LanguageCode;

	/// Строчка с маркетинговым названием данной озвучки.
	UInt16	MarketingName[128];

	/// Название языка.
	UInt16	LanguageName[64];
}TUnicodeSoundType, *TUnicodeSoundPtr;

/// Структура для хранения слова, полученного из морфологии
typedef struct TSldMorphologyWordStruct
{
	/// Слово
	SldU16String				MorphologyForm;

	/// Тип слова
	ESldMorphologyWordTypeEnum	MorphologyFormType;

	/// Конструктор
	TSldMorphologyWordStruct(void) :
		MorphologyFormType(eMorphologyWordTypeBase) {};

	TSldMorphologyWordStruct(const SldU16String & aForm, const ESldMorphologyWordTypeEnum aType) :
		MorphologyForm(aForm), MorphologyFormType(aType) {};

	TSldMorphologyWordStruct(SldU16String && aForm, const ESldMorphologyWordTypeEnum aType) :
		MorphologyForm(sld2::move(aForm)), MorphologyFormType(aType) {};

	bool operator==(const TSldMorphologyWordStruct & aRef)	const { return MorphologyForm == aRef.MorphologyForm; }
	bool operator>(const TSldMorphologyWordStruct & aRef)	const { return MorphologyForm > aRef.MorphologyForm; }
	bool operator<(const TSldMorphologyWordStruct & aRef)	const { return MorphologyForm < aRef.MorphologyForm; }

	bool operator==(const SldU16String & aRefString)		const { return MorphologyForm == aRefString; }
	bool operator>(const SldU16String & aRefString)			const { return MorphologyForm > aRefString; }
	bool operator<(const SldU16String & aRefString)			const { return MorphologyForm < aRefString; }

} TSldMorphologyWordStruct;

/// Структура для хранения инофрмации о слайдшоу(без информации о слайдах)
typedef struct TSldSlideShowStruct
{
	/// Индекс глобальной озвучки
	UInt32 SlideShowSound;
	/// Время показа одного кадра
	UInt32 SlideShowTime;
	/// Cпособ обтекания слайдшоу текстом (см. #ESldFlow)
	UInt32 SlideShowFlow;
	/// Cпособ расположения слайдшоу на странице (см. #ESldPlace)
	UInt32 SlideShowPlace;
	/// Эффект переключения между кадрами (см. #ESldMoveEffect)
	UInt32 SlideShowMoveEffect;
	/// Эффект отображения слайдшоу на странице (см. #ESldShowEffect)
	UInt32 SlideShowShowEffect;
	/// Индекс списка слов
	UInt32 SlideShowListIndex;
	/// Индекс слова в списке слов, которое отвечает за слайдшоу
	UInt32 SlideShowWordIndex;
	/// ширина контейнера
	TSizeValue SlideShowWidth;
	/// высота контейнера
	TSizeValue SlideShowHeight;

	TSldSlideShowStruct(void)
	{
		Clear();
	};

	void Clear(void)
	{
		SlideShowSound		=	0;
		SlideShowTime		=	0;
		SlideShowFlow		=	0;
		SlideShowPlace		=	0;
		SlideShowMoveEffect	=	0;
		SlideShowShowEffect	=	0;
		SlideShowListIndex	=	0;
		SlideShowWordIndex	=	0;
	};

} TSldSlideShowStruct;

/// Структура для заголовка файла 3d сцены
typedef struct TSceneHeader
{
	/// Размер структуры
	UInt32 sizeHeader;
	/// Версия заголовка
	UInt32 version;
	/// Размер бинаризованной сцены
	UInt32 sizeScene;

	UInt32 hasAnimation;

	/// Зарезервировано
	UInt32 Reserved[4];
} TSceneHeader;

/// Структура для описание ориентации (3d)
/// ориентация задается кватернионом вида W + Xi + Yj + Zk
typedef struct TQuaternion
{
	/// одноименные коэффициенты из формулы кватерниона
	/// X
	Float32 X;
	/// Y
	Float32 Y;
	/// Z
	Float32 Z;
	/// W
	Float32 W;

	TQuaternion() 
	{
		X = 0.f;
		Y = 0.f;
		Z = 0.f;
		W = 0.f;
	}
	TQuaternion(Float32 x, Float32 y, Float32 z, Float32 w) 
	{
		X = x;
		Y = y;
		Z = z;
		W = w;
	}

	TQuaternion operator*(TQuaternion& q2) const
	{
		return TQuaternion( W * q2.W - X * q2.X - Y * q2.Y - Z * q2.Z,
							W * q2.X + X * q2.W + Y * q2.Y - Z * q2.Y,
							W * q2.Y - X * q2.Z + Y * q2.W + Z * q2.X,
							W * q2.Z + X * q2.Y - Y * q2.X + Z * q2.W);
	}
} TQuaternion;

/// Структура для описания положения в пространстве (3d)
typedef struct TPosition
{
	/// координата по оси X
	Float32 X;
	/// координата по оси Y
	Float32 Y;
	/// координата по оси Z
	Float32 Z;

	TPosition()
	{
		X = 0;
		Y = 0;
		Z = 0;
	}
	TPosition(Float32 x, Float32 y, Float32 z)
	{
		X = x;
		Y = y;
		Z = z;
	}

	TPosition operator+(TPosition& p2) const
	{
		return TPosition(X + p2.X, Y + p2.Y, Z + p2.Z);
	}
} TPosition;

/// Структура для описания масштаба (3d)
typedef struct TScale
{
	/// масштаб по оси X
	Float32 X;
	/// масштаб по оси Y
	Float32 Y;
	/// масштаб по оси Z
	Float32 Z;

	TScale()
	{
		X = 1;
		Y = 1;
		Z = 1;
	}
	TScale(Float32 x, Float32 y, Float32 z)
	{
		X = x;
		Y = y;
		Z = z;
	}

	TScale operator*(TScale& p2) const
	{
		return TScale(X * p2.X, Y * p2.Y, Z * p2.Z);
	}
} TScale;

/// Структура для описания одной подсущности (3d)
typedef struct TSubentity
{
	/// Индекс подсущности
	UInt32 Index;
	/// Глобальный индекс материала
	UInt32 Material;
} TSubentity;

typedef struct TKeyframe
{
	TKeyframe() : time(0), pos(), rotation(), size() {}
	/// Время кадра
	double time;
	/// Параметры кадра
	TPosition pos;
	TQuaternion rotation;
	TScale size;
} TKeyframe;


typedef struct TAnimation
{
	/// Время анимации
	double length;
	/// Название анимации
	UInt16 name[128];

	/// Число кадров
	UInt32 keyframesCount;

	/// Кадры
	TKeyframe* keyframes;

	TAnimation() : length(0), keyframesCount(0) 
	{
		sldMemZero(name, 128*sizeof(UInt16));
	}

	TAnimation &operator=(const TAnimation& aRef)
	{
		length			= aRef.length;
		keyframesCount  = aRef.keyframesCount;

		sldMemZero(name, 128*sizeof(UInt16));
		sldMemMove(name, aRef.name, 128 * sizeof(UInt16));

		return *this;
	};

	TAnimation(const TAnimation& aRef)
	{
		*this = aRef;
	};

	~TAnimation()
	{

	};

} TAnimation;

/// Структура для описания нода 
typedef struct TSldSceneNode
{
	/// координаты точки положения в пространстве нода
	TPosition	Position;
	/// кватернион вращения нода
	TQuaternion	Rotation;
	/// масштаб нода
	TScale		Scale;
	/// индекс меша в базе
	UInt32		MeshIndex;
	/// количество подсущностей в ноде
	UInt32		CountSubEntity;
	/// массив подсущностей
	TSubentity*	SubEntities;
	/// Количество анимаций в ноде
	UInt32		CountAnimations;
	/// Анимации
	TAnimation* Animations;

	TSldSceneNode() : MeshIndex(0), CountSubEntity(0), SubEntities(NULL), CountAnimations(0), Animations(NULL) {};
	TSldSceneNode(TPosition p, TQuaternion q, TScale s) : Position(p), Rotation(q), Scale(s), MeshIndex(0), CountSubEntity(0), SubEntities(NULL), CountAnimations(0), Animations(NULL) {};

	TSldSceneNode &operator=(const TSldSceneNode& aRef)
	{
		Position		=	aRef.Position;
		Rotation		=	aRef.Rotation;
		Scale			=	aRef.Scale;
		MeshIndex		=	aRef.MeshIndex;

		if (!!SubEntities)
			sldMemFree(SubEntities);

		CountSubEntity	=	aRef.CountSubEntity;
		SubEntities		=	(TSubentity*)sldMemNew(CountSubEntity * sizeof(TSubentity));
		sldMemMove(SubEntities, aRef.SubEntities, CountSubEntity * sizeof(TSubentity));

		if (!!Animations)
			sldMemFree(Animations);

		CountAnimations	=	aRef.CountAnimations;
		Animations		=	(TAnimation*)sldMemNew(CountAnimations * sizeof(TAnimation));
		sldMemMove(Animations, aRef.Animations, CountAnimations * sizeof(TAnimation));

		return *this;
	};

	TSldSceneNode(const TSldSceneNode& aRef)
	{
		*this = aRef;
	};

	~TSldSceneNode()
	{
		sldMemFree(SubEntities);
		sldMemFree(Animations);
	};
} TSldSceneNode;

/// Блок перевода статей
typedef struct TArticleBlock
{
	TArticleBlock(const Int32 aStyleIndex, SldU16StringRef aText)
	: StyleIndex(aStyleIndex), Text(aText) {}

	/// Индекс стиля, которым размечен данный блок
	Int32 StyleIndex;
	/// Текст блока
	SldU16String Text;

}TArticleBlock;

/// Статья, разбитая на блоки
using SplittedArticle = CSldVector<TArticleBlock>;

/// Один элемент, представляющий озвучку
typedef struct TSoundElement
{
	/// Индекс озвучки
	UInt32	SoundIndex;
	/// Заголовок файла с озвучкой
	TSoundFileHeader Header;
	/// Указатель на загруженные данные
	UInt8*	Data;
	/// Размер загруженных данных
	UInt32	Size;

	TSoundElement(void)
	{
		Clear();
	}

	~TSoundElement(void)
	{
		Close();
	}

	/// Инициализация членов по умолчанию
	void Clear(void)
	{
		SoundIndex = 0;
		Data = NULL;
		Size = 0;
		sldMemZero(&Header, sizeof(TSoundFileHeader));
	}

	/// Очистка памяти
	void Close(void)
	{
		if(Data)
			sldMemFree(Data);
		Clear();
	}
	/// Инициализация
	ESldError Init(const UInt8* aData, UInt32 aSize)
	{
		Close();
		Size = aSize;
		Data = (UInt8*)sldMemNew(Size);
		if(!Data)
			return eMemoryNotEnoughMemory;

		sldMemMove(Data, aData, aSize);
		sldMemMove(&Header, Data, sizeof(TSoundFileHeader));
		return eOK;
	}

	TSoundElement(const TSoundElement& aSoundElement)
	{
		Data = NULL;
		Size = 0;
		SoundIndex = 0;
		*this = aSoundElement;
	}

	TSoundElement& operator=(const TSoundElement& aSoundElement)
	{
		Size = aSoundElement.Size;
		SoundIndex = aSoundElement.SoundIndex;
		if (Data)
		{
			sldMemFree(Data);
			Data = NULL;
		}
		if(Size)
		{
			Data = (UInt8*)sldMemNew(Size);
			sldMemMove(Data, aSoundElement.Data, Size);
		}
		return *this;
	}

}TSoundElement;

/// Структура с информацией о связях с внешними базами
typedef struct TExternContentInfo
{
	// Пары id словаря - число использований
	struct InfoPair
	{
		UInt16* DictId;
		UInt32  Count;

		InfoPair()
		{
			Count = 0;
			DictId = NULL;
		}

		~InfoPair()
		{
			if (!!DictId)
				sldMemFree(DictId);
			DictId = NULL;
		}

		InfoPair &operator=(const InfoPair& aRef);

		InfoPair(const InfoPair& aRef)
		{
			*this = aRef;
		}

	};

	// Массив пар
	struct InfoPair* Info;
	// Число пар
	UInt32 Size;

	TExternContentInfo()
	{
		Size = 0;
		Info = NULL;
	}

	~TExternContentInfo()
	{
		if(!!Info)
		{
			for (UInt32 i = 0; i < Size; i++)
			{
				Info[i].~InfoPair();
			}
			sldMemFree(Info);
		}
	}

	TExternContentInfo &operator=(const TExternContentInfo& aRef)
	{
		if (!!aRef.Info)
		{
			if(!!Info)
			{
				for (UInt32 i = 0; i < Size; i++)
				{
					Info[i].~InfoPair();
				}
				sldMemFree(Info);
			}

			Size = aRef.Size;
			Info = (InfoPair*)sldMemNew(Size*sizeof(InfoPair));
			for (UInt32 i = 0; i < Size; i++)
			{
				Info[i] = aRef.Info[i];
			}
		}

		return *this;
	}

	TExternContentInfo(const TExternContentInfo& aRef)
	{
		*this = aRef;
	}
} TExternContentInfo;

/// Структура для хранения незагруженной дополнительной информации по ресурсу
typedef struct TResourceInfoIndexes
{
	UInt32 NameIndex;
	UInt16 Reserved[63];
} TResourceInfoIndexes;


/// Структура для хранения дополнительной информации по ресурсу 
typedef struct TResourceInfo
{
	/// Имя ресурса
	UInt16* ResName;
	/// Путь ресурса
	UInt16* ResPath;
	/// Длина имени (потенциально не нужна, строка 0-терминирована)
	UInt32 ResNameLength;
	/// Длина пути (потенциально не нужна, строка 0-терминирована)
	UInt32 ResPathLength;

	TResourceInfo()
	{
		ResName = NULL;
		ResPath = NULL;
		ResNameLength = 0;
		ResPathLength = 0;
	}

	~TResourceInfo()
	{
		if(ResName)
		{
			sldMemFree(ResName);
			ResName = NULL;
		}
		if (ResPath)
		{
			sldMemFree(ResPath);
			ResPath = NULL;
		}
	}

	TResourceInfo &operator=(const TResourceInfo& aRef)
	{
		ResNameLength = aRef.ResNameLength;
		if(ResName)
		{
			sldMemFree(ResName);
			ResName = NULL;
		}
		if (ResPath)
		{
			sldMemFree(ResPath);
			ResPath = NULL;
		}
		if (ResNameLength)
		{
			ResName = (UInt16*)sldMemNew(sizeof(UInt16) * ResNameLength);
			sldMemMove(ResName, aRef.ResName, sizeof(UInt16) * ResNameLength);
		}
		if (ResPathLength)
		{
			ResPath = (UInt16*)sldMemNew(sizeof(UInt16) * ResPathLength);
			sldMemMove(ResPath, aRef.ResPath, sizeof(UInt16) * ResPathLength);
		}
		return *this;
	}

	TResourceInfo(const TResourceInfo& aRef)
	{
		ResPath = NULL;
		ResName = NULL;
		ResNameLength = 0;
		ResPathLength = 0;
		*this = aRef;
	}
} TResourceInfo;

/// Структура, содержащая ключ-значение, позволяющая производить сравнение по ключу
template<typename Key, typename Value>
struct TSldPair
{
	Key first;
	Value second;

	TSldPair(const Key & aFirst, const Value & aSecond) : first(aFirst), second(aSecond) {}
	TSldPair(Key && aFirst, Value && aSecond) : first(sld2::move(aFirst)), second(sld2::move(aSecond)) {}

	TSldPair(const TSldPair&) = default;
	TSldPair& operator=(const TSldPair&) = default;

	TSldPair(TSldPair && aRef) : first(sld2::move(aRef.first)), second(sld2::move(aRef.second)) {}
	TSldPair & operator=(TSldPair && aRef)
	{
		first = sld2::move(aRef.first);
		second = sld2::move(aRef.second);
		return *this;
	}

	bool operator==(const TSldPair & aRef)	const { return first == aRef.first; }
	bool operator>(const TSldPair & aRef)	const { return first > aRef.first; }
	bool operator>=(const TSldPair & aRef)	const { return first >= aRef.first; }
	bool operator<(const TSldPair & aRef)	const { return first < aRef.first; }
	bool operator<=(const TSldPair & aRef)	const { return first <= aRef.first; }

	bool operator==(const Key & aRef)		const { return first == aRef; }
	bool operator>(const Key & aRef)		const { return first > aRef; }
	bool operator>=(const Key & aRef)		const { return first >= aRef; }
	bool operator<(const Key & aRef)		const { return first < aRef; }
	bool operator<=(const Key & aRef)		const { return first <= aRef; }
};

/// Структура, содержащая информацию о статьях в слитых словарях и соответсвующих метаданных
struct TMergedMetaInfo
{
	enum IndexType { Article, Style, Sound, Picture };

	/// Вектор количества статей
	CSldVector<Int32>							ArticlesCount;

	/// Вектор количества стилей
	CSldVector<Int32>							StylesCount;

	/// Вектор количества звуков
	CSldVector<Int32>							SoundsCount;

	/// Вектор количества изображений
	CSldVector<Int32>							PicturiesCount;

	/// Вектор индексов словарей, учавствующих в слиянии каждого из списков
	CSldVector<CSldVector<Int32>>				DictIndexes;

	/// Вектор индексов списков слияния для одиночных индексов списков каждого словаря
	CSldVector<CSldVector<Int32>>				ListIndexes;

	///Вектор индексов слов в смерженых списках для индексов в одиночных словарях
	CSldVector<CSldVector<CSldVector<Int32>>>	WordIndexes;

	// возвращает локальный индекс и индекс словаря по смерженому индексу
	template <IndexType I> Int32 ToLocalIndex(Int32 aIndex, UInt32 *aDictIdx = nullptr) const
	{
		const auto table = getTable(sld2::meta::tag_t<I>{});
		const UInt32 dictIdx = getDictIdByMergedIndex(aIndex, table);
		if (dictIdx < table.size())
			aIndex -= table[dictIdx];
		if (aDictIdx)
			*aDictIdx = dictIdx;
		return aIndex;
	}

	// преобразуют локальные индексы в словаре в глобальный индекс
	Int32 ToMergedStyleIndex(UInt32 aDictIdx, Int32 aStyleIdx) const
	{
		if (aDictIdx != 0)
			aStyleIdx += StylesCount[aDictIdx];
		return aStyleIdx;
	}

	Int32 ToMergedSoundIndex(UInt32 aDictIdx, Int32 aSoundIdx) const
	{
		if (aSoundIdx != SLD_INDEX_SOUND_NO && aDictIdx != 0)
			aSoundIdx += SoundsCount[aDictIdx];
		return aSoundIdx;
	}

	Int32 ToMergedPictureIndex(UInt32 aDictIdx, Int32 aPictureIdx) const
	{
		if (aPictureIdx != SLD_INDEX_PICTURE_NO && aDictIdx != 0)
			aPictureIdx += PicturiesCount[aDictIdx];
		return aPictureIdx;
	}

	ESldError ToMergedLinkIndexes(UInt32 aDictIdx, UInt32 &aListIndex, UInt32 &aWordIndex) const
	{
		if (aListIndex == static_cast<UInt32>(SLD_DEFAULT_LIST_INDEX))
			return eOK;

		if (aListIndex >= ListIndexes[aDictIdx].size())
			return eCommonWrongList;

		Int32 listIndex = ListIndexes[aDictIdx][aListIndex];
		if (listIndex < 0 || static_cast<UInt32>(listIndex) >= WordIndexes.size())
			return eCommonWrongList;

		aListIndex = listIndex;
		const auto & wordIndexes = WordIndexes[aListIndex];
		if (aWordIndex != static_cast<UInt32>(SLD_DEFAULT_LIST_INDEX) && aDictIdx < wordIndexes.size() && aWordIndex < wordIndexes[aDictIdx].size())
		{
			aWordIndex = wordIndexes[aDictIdx][aWordIndex];
		}

		return eOK;
	}

private:
	sld2::Span<const Int32> getTable(sld2::meta::tag_t<Article>) const { return ArticlesCount; }
	sld2::Span<const Int32> getTable(sld2::meta::tag_t<Style>) const { return StylesCount; }
	sld2::Span<const Int32> getTable(sld2::meta::tag_t<Sound>) const { return SoundsCount; }
	sld2::Span<const Int32> getTable(sld2::meta::tag_t<Picture>) const { return PicturiesCount; }

	// template to catch invalid enums
	template <IndexType I> sld2::Span<const Int32> getTable(sld2::meta::tag_t<I>) const {
		static_assert(I < 0, "Unsupported index type!"); return nullptr;
	}

	UInt32 getDictIdByMergedIndex(Int32 aIndex, sld2::Span<const Int32> aCounts) const
	{
		for (UInt32 dictIdx = 1; dictIdx < aCounts.size(); dictIdx++)
		{
			if (aIndex < aCounts[dictIdx])
				return dictIdx - 1;
		}
		return static_cast<UInt32>(SLD_DEFAULT_DICTIONARY_INDEX);
	}
};

struct TMergedDictInfo
{
	TMergedDictInfo() : MetaInfo(nullptr), DictIndex(0) {}

	const TMergedMetaInfo*		MetaInfo;
	Int32						DictIndex;
};

/// Структура, описывающая индексы слова в словаре
struct TSldWordIndexes
{
	TSldWordIndexes() : ListIndex(SLD_DEFAULT_LIST_INDEX), WordIndex(SLD_DEFAULT_WORD_INDEX) {}
	TSldWordIndexes(const Int32 aListIndex, const Int32 aWordIndex) : ListIndex(aListIndex), WordIndex(aWordIndex) {}

	bool IsValid() const { return ListIndex != SLD_DEFAULT_LIST_INDEX && WordIndex != SLD_DEFAULT_WORD_INDEX; }
	bool operator==(const TSldWordIndexes aRef) const { return ListIndex == aRef.ListIndex && WordIndex == aRef.WordIndex; }

	/// Номер списка слов
	Int32 ListIndex;
	/// Номер слова
	Int32 WordIndex;
};

using SldWordsCollection = CSldVector<TSldWordIndexes>;

/// Структура, хранящая данный о ссылке
struct TLinkInfo
{
	TLinkInfo(const Int32 aListIndex, const Int32 aWordIndex)
		: Indexes(aListIndex, aWordIndex) { }

	TLinkInfo(const Int32 aListIndex, const Int32 aWordIndex, const SldU16StringRef aDictId, const SldU16StringRef aKey)
		: Indexes(aListIndex, aWordIndex), DictId(aDictId), Key(aKey) { }

	/// Индекс списка / статьи
	TSldWordIndexes			Indexes;

	/// ID внешней базы, если пустой - ссылка ведет на туже базу
	SldU16String			DictId;

	/// Ключ поиска во внешней базе
	SldU16String			Key;
};

using SldLinksCollection = CSldVector<TLinkInfo>;

struct TWordRefInfo
{
	enum class type { exact, morphoBase, morphoForm };

	UInt32 StartPos;
	UInt32 EndPos;

	CSldVector<TSldPair<TSldWordIndexes, type>> Refs;
};

#endif
