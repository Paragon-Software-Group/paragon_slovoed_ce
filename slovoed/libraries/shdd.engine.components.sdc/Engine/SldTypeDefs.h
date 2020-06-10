#ifndef _SLD_TYPEDEFS_H_
#define _SLD_TYPEDEFS_H_

/**
 * The file is allowed to contain *only* type definitions.
 * Some rules:
 *   * no methods/functions
 *     the only exception are very small and simple (static) inlines to be used with the
 *     types defined here (say, relational operators)
 *     *NO* external functions, the header must be freely includable on its own in a standalone
 *     engine config
 *   * no includes (the only exception are headers following *the same* rules)
 *   * macro defines are allowed only in the context of the types defined here
 *   * no enum definitions ouside of the cases where an enum is completely "meaningless"
 *     without the accompanying type (in which case it may as well be defined inside the class)
 *
 * The main purpose is "code sharing" between the engine and related utilities (mostly the
 * compiler). This file should be freely includable from anywhere as long as the engine is
 * in the "standalone" mode.
 */

#include "SldDefines.h"
#include "SldPlatform.h"

 /// Структура описывающая ресурс считанный из контейнера
typedef struct TResourceType
{
	/// Указатель на данные
	const UInt8	*Pointer;
	/// Количество байт данных
	UInt32	Size;
}TResourceType;

typedef union UIntUnion
{
	/// 32-разрядное машинное слово
	UInt32	ui_32;
	/// два 16-разрядных машинных слова
	UInt16	ui_16[2];
	/// четыре 8-разрядных машинных слова
	UInt8	ui_8[4];
} UIntUnion;

/// Структура с локализованными названиями словаря
typedef struct TLocalizedNames
{
	/// Размер структуры
	UInt32 structSize;
	/// Язык на котором имеются строчки
	UInt32 LanguageCode;

	/// Полное название продукта
	UInt16 ProductName[128];
	/// Полное название словаря
	UInt16 DictionaryName[128];
	/// Сокращенное название словаря
	UInt16 DictionaryNameShort[128];
	/// Класс словаря: Compact, Classic, Deluxe или Medical
	UInt16 DictionaryClass[128];
	/// Название прямой языковой пары (LanguageFrom-LanguageTo)
	UInt16 DictionaryLanguagePair[128];
	/// Сокращенное название прямой языковой пары
	UInt16 DictionaryLanguagePairShort[128];
	/// Название обратной языковой пары (LanguageTo-LanguageFrom)
	UInt16 DictionaryLanguagePairReverse[128];
	/// Сокращенное название обратной языковой пары
	UInt16 DictionaryLanguagePairShortReverse[128];
	/// Название языка, с которого производится перевод
	UInt16 LanguageNameFrom[128];
	/// Название языка, на который производится перевод
	UInt16 LanguageNameTo[128];
	/// Имя автора
	UInt16 AuthorName[128];
	/// Cайт автора
	UInt16 AuthorWeb[128];
} TLocalizedNames;

/// Структура с локализованными названиями списка слов
typedef struct TListLocalizedNames
{
	/// Размер структуры
	UInt32 structSize;
	/// Язык строчек
	UInt32 LanguageCode;

	/// Полное название списка слов
	UInt16 ListName[128];
	/// Сокращенное название списка слов
	UInt16 ListNameShort[128];
	/// Класс списка слов
	UInt16 ListClass[128];
	/// Название прямой языковой пары (LanguageFrom-LanguageTo)
	UInt16 ListLanguagePair[128];
	/// Сокращенное название прямой языковой пары
	UInt16 ListLanguagePairShort[128];
	/// Название языка, с которого производится перевод
	UInt16 ListLanguageNameFrom[128];
	/// Название языка, на который производится перевод
	UInt16 ListLanguageNameTo[128];
} TListLocalizedNames;

/// Заголовок словаря
typedef struct TDictionaryHeader
{
	/// Размер структуры для которой выделена память
	UInt32 HeaderSize;
	/// Версия словаря
	UInt32 Version;

	/// Код языка с которого производится перевод
	UInt32 LanguageCodeFrom;
	/// Код языка на который производится перевод
	UInt32 LanguageCodeTo;
	/// Секретное число необходимое для расшифровки словаря
	UInt32 HASH;
	/// Идентификатор словаря
	UInt32 DictID;

	/// Маркетинговое общее количество слов в словаре
	UInt32 MarketingTotalWordsCount;

	/// Количество списков слов в словаре
	UInt32 NumberOfLists;

	/// Тип ресурса для сжатых данных переводов
	UInt32 articlesDataType;
	/// Тип ресурса для таблицы быстрого доступа переводов
	UInt32 articlesQAType;
	/// Тип ресурса для деревьев(вспомогательных данных декодирования) переводов
	UInt32 articlesTreeType;
	/// Тип ресурса для стилей переводов
	UInt32 articlesStyleType;

	/// Количество переводов
	UInt32 NumberOfArticles;
	/// Способ сжатия информации переводов
	UInt32 ArticlesCompressionMethod;

	/// Максимальный размер типа перевода необходимый для декодирования словаря
	UInt32 WordtypeSize;
	/// Максимальный размер типа буфера необходимый для декодирования словаря
	UInt32 ArticlesBufferSize;

	/// Количество стилей используемых в словаре.
	UInt32 NumberOfStyles;

	/// Количество языков на которые переведены названия словаря
	UInt32 NumberOfStringsLanguages;

	/// Количество таблиц символов (таблицы символов языков + общая для всех языков таблица символов-разделителей)
	UInt32 NumberOfLanguageSymbolsTable;

	/// Битовый набор свойств словаря (см. ESldFeatures)
	UInt32 DictionaryFeatures;

	/// Тип источника картинок (см. #EMediaSourceTypeEnum)
	UInt32 ImageSourceType;

	/// Тип источника озвучки (см. #EMediaSourceTypeEnum)
	UInt32 SoundSourceType;

	/// Общее количество картинок в словаре
	UInt32 TotalPictureCount;

	/// Общее количество озвучек в словаре
	UInt32 TotalSoundCount;

	/// Количество таблиц символов-разделителей для конкретных языков
	UInt32 NumberOfLanguageDelimitersSymbolsTable;

	/// Общее количество видео в словаре
	UInt32 TotalVideoCount;

	/// Тип источника видео (см. #EMediaSourceTypeEnum)
	UInt32 VideoSourceType;

	/// Тип источника сцен (см. #EMediaSourceTypeEnum)
	UInt32 SceneSourceType;

	/// Общее количество сцен в словаре
	UInt32 TotalSceneCount;

	/// Тип источника моделей (см. #EMediaSourceTypeEnum)
	UInt32 MeshSourceType;

	/// Общее количество моделей в словаре
	UInt32 TotalMeshCount;

	/// Тип источника материалов (см. #EMediaSourceTypeEnum)
	UInt32 MaterialSourceType;

	/// Общее количество материалов в словаре
	UInt32 TotalMaterialCount;

	/// Флаг, определяющий есть ли в словаре вложеныые базы морфологии
	UInt8 HasMorphologyData;

	/// Флаг, если не равен 0 значит стили храняться упакованными в ~один ресурс
	UInt8 HasPackedStyles;

	// выравнивание до следующего Uint32
	UInt16 _pad0;

	/// Флаг, определяющий есть ли в словаре блоки switch с тематиками
	UInt32 HasSwitchThematics;

	/// Пара неиспользуемых uint32
	UInt32 Unused[2];

	// Общее количество абстрактных ресурсов
	UInt32 TotalAbstractItemCount;

	/// Зарезервировано
	UInt32 Reserved[26];

} TDictionaryHeader;

/// Cтруктура с дополнительной информацией об учебнике
typedef struct TAdditionalInfo
{
	/// Размер структуры
	UInt32	StructSize;

	// Эти четыре значения опиcываеют диапазон классов, для которых предназначен учебник
	// В виде ClassLevel_FromMajor.ClassLevel_FromMinor - ClassLevel_UpToMajor.ClassLevel_UpToMinor
	// "Мажорное" значение - собственно номер класса. "Минорное" - номер четверти, семестра и т.п.
	UInt8	ClassLevel_FromMajor;
	UInt8	ClassLevel_FromMinor;
	UInt8	ClassLevel_UpToMajor;
	UInt8	ClassLevel_UpToMinor;

	/// Полное название издателя
	UInt16	PublisherName[128];
	/// Год издания
	UInt16	PublishYear;

	/// Номер издания
	UInt8	Revision;
	/// Часть (в случае многотомного издания)
	UInt8	BookPart;

	/// Тип содержимого (битовое поле, подробнее см. ESldContentType)
	UInt32	ContentType;

	/// Картинка предпросмотра обложки учебника
	UInt32	CoverThumbnail;
	/// Полноразмерная картинка обложки учебника
	UInt32	CoverFullImage;

	/// Уровень образования
	UInt32 EducationalLevel;

	/// Международный стандартный книжный номер
	UInt8 ISBN[20];

	// Выравниваем структуру под размер в 512 байт
	/// Зарезервировано
	UInt32 Reserved[52];

} TAdditionalInfo;

/// Информация о версии словарной базы
typedef struct TDictionaryVersionInfo
{
	/// Размер структуры
	UInt32 structSize;
	/// Старшая версия
	UInt32 MajorVersion;
	/// Младшая версия
	UInt32 MinorVersion;
	/// Бренд (см. #EDictionaryBrandName)
	UInt32 Brand;
	/// Зарезервировано
	UInt32 Reserved[12];

} TDictionaryVersionInfo;

/// Заголовок описывающий один уровень иерархии.
typedef struct THierarchyLevelHeader
{
	/// Количество слов на данном уровне
	UInt32 NumberOfWords;
	/// Уровень вложенности
	UInt32 CurrentLevel;
	/// Количество элементов описывающих текущий уровень
	UInt32 NumberOfElements;
	/// Глобальный номер для первого элемента.
	/** Позволяет пересчитывать глобальные номера элементов локальные. **/
	UInt32 GlobalShift;
	/// Флаг сортированности уровня
	UInt8 IsSortedLevel;
	/// Флаг, определяющий поведение данного уровня см #EHierarchyLevelType
	UInt8 LevelType;
	/// Зарезервировано
	UInt8 Reserved[14];
} THierarchyLevelHeader;

/// Один элемент иерархии
typedef struct THierarchyElement
{
	/// Смещение от начала структуры иерархии до начала уровня на который ссылается данный элемент.
	/** Можно ссылаться на предыдущие уровни или на параллельные, т.к. смещение идет относительно
		начала.
	*/
	UInt32 ShiftToNextLevel;
	/// Номер слова из списка слов на котором начинается следующий уровень.
	/// MAX_UINT_VALUE - если это последний элемент иерархии и дальше ничего нет.
	UInt32 NextLevelGlobalIndex;
	/// Номер слова с которого начинается блок слов имеющих одинаковые атрибуты
	UInt32 BeginIndex;
	/// Номер слова которым заканчивается блок слов имеющих одинаковые атрибуты
	UInt32 EndIndex;
} THierarchyElement;

/// Заголовок таблицы быстрого доступа
typedef struct TQAHeader
{
	/// размер заголовка
	UInt32 HeaderSize;

	/// Размер элемента таблицы быстрого доступа
	UInt32 QAEntrySize;

	/// Версия таблицы быстрого доступа
	UInt32 Version;

	/// Вариант исполнения таблицы
	UInt32 Type;

	/// Количество элементов в таблице быстрого доступа
	UInt32 Count;
} TQAHeader;

/// Количество символов текста в таблице быстрого доступа
#define QA_TEXT_SIZE	(12)

/// Структура таблицы быстрого доступа для списка слов.
typedef struct TQAEntry
{
	/// Номер слова из списка слов.
	UInt32 Index;

	/// Смещение в битах от начала сжатого списка слов, до начала слова под номером #QAEntry::Index
	UInt32 ShiftBit;

	/// Начало слова
	UInt16 text[QA_TEXT_SIZE];
} TQAEntry;

/// Таблица быстрого доступа для переводов.
typedef struct TQAArticlesEntry
{
	/// Номер слова из списка слов.
	UInt32 Index;

	/// Смещение в битах от начала сжатого списка слов, до начала слова под номером #QAListEntry::Index
	UInt32 ShiftBit;
} TQAArticlesEntry;

/// Элемент массива быстрого доступа для индексов.
typedef struct TQAIndexesEntry
{
	/// Номер слова из списка слов которому соответствует следующая запись.
	UInt32 Index;
	/// Смещение от начала данных индексов(количество переводов).
	UInt32 CountShiftBit;
	/// Смещение от начала данных индексов(данные с номерами статей).
	UInt32 DataShiftBit;
} TQAIndexesEntry;

/// Заголовок списка слов.
typedef struct TListHeader
{
	/// Размер структуры заголовка списка слов
	UInt32 HeaderSize;
	/// Версия списка слов
	UInt32 Version;
	/// Способ сжатия
	UInt32 CompressionMethod;
	/// Количество элементов в списке слов.
	UInt32 NumberOfWords;
	/// Количество вариантов написания в одном элементе
	/**
	*	Для случая когда вариантов больше чем 1 нужно где-то хранить табличку
	*	с информацией о том в каком столбце какой язык содержится.
	*
	*	Для каждого нового варианта целесообразно использовать свое дерево(при сжатии Хаффманом)
	*/
	UInt32 NumberOfVariants;
	/// Максимальное количество символов в слове которое может присутствовать в данном списке слов.
	UInt32 MaximumWordSize;
	/// Количество бит необходимых для хранения количества индексов переводов на которые ссылается
	/// данное слово.
	UInt32 SizeOfNumberOfArticles;
	/// Количество бит необходимых для хранения индекса перевода на который ссылается
	/// данное слово.
	UInt32 SizeOfArticleIndex;
	/// Флаг: если установлен, значит нужно получать из битового потока количество переводов
	/**
	*	Если сброшен, тогда считается, что количество переводов = 1.
	*/
	UInt32 IsNumberOfArticles;
	/// Флаг: Если установлен, значит список слов впрямую ассоциирован с переводами
	/**
	*	Номер элемента списка слов имеет в качестве перевода статью с тем-же номером, в количестве 1.
	*/
	UInt32 IsDirectList;
	/// Флаг: является ли данный список слов иерархическим(каталог)
	UInt32 IsHierarchy;
	/// Флаг: Является ли данный список слов отсортированным(т.е. доступен ли в нем поиск по тексту)
	UInt32 IsSortedList;
	/// Код языка с которого производится перевод
	UInt32 LanguageCodeFrom;
	/// Код языка на который производится перевод
	UInt32 LanguageCodeTo;
	/// Флаг того, как планируется использовать данный список слов: #EWordListTypeEnum
	UInt32 WordListUsage;

	/// Флаг того, что некоторым словам из списка слов сопоставлены картинки
	UInt32 IsPicture;
	/// Флаг того, что некоторым словам из списка слов сопоставлена озвучка
	UInt32 IsSound;
	/// Количество бит необходимых для представления индекса картинки
	UInt32 PictureIndexSize;
	/// Количество бит необходимых для представления индекса озвучки
	UInt32 SoundIndexSize;

	/// Индекс ресурса с локализованными строками для списка слов
	UInt32 LocalizedStringsResourceIndex;
	/// Количество записей в ресурсе с локализованными строками (количество локализованных строк у списка слов)
	UInt32 LocalizedStringsRecordsCount;

	/// Тип ресурса в котором хранятся сжатые данные
	UInt32 Type_CompressedData;
	/// Тип ресурса в котором хранятся прямые смещения слов в списке
	/// Для листов ниже версии в нем хранилась таблица быстрого доступа (Type_QATable)
	UInt32 Type_DirectWordsShifts;
	/// Тип ресурса в котором хранится дерево Хаффмана
	UInt32 Type_Tree;
	/// Тип ресурса в котором хранится иерархия.
	UInt32 Type_Hierarchy;
	/// Тип ресурса в котором хранятся количества переводов статей на которые ссылаются слова из списка слов
	UInt32 Type_IndexesCount;
	/// Тип ресурса в котором хранятся номера статей на которые ссылается слово из списка слов.
	UInt32 Type_IndexesData;
	/// Тип ресурса в котором хранятся таблица быстрого доступа для #TListHeader::Type_Indexes.
	UInt32 Type_IndexesQA;
	/// Тип ресурса в котором хранятся заголовок индексов.
	UInt32 Type_IndexesHeader;

	/// Является ли данный список слов полнотекстового поиска суффиксным
	UInt32 IsSuffixFullTextSearchList;

	/// Индекс ресурса с таблицей свойств вариантов написания (количество записей в ресурсе - NumberOfVariants)
	UInt32 VariantsPropertyResourceIndex;

	/// Общее количество картинок в списке слов
	/// Учитываются картинки самих слов списка (иконки) и картинки в переводах, на которые ссылаются слова списка
	UInt32 TotalPictureCount;

	/// Общее количество озвучек в списке слов
	/// Учитываются озвучки самих слов списка и озвучки в переводах, на которые ссылаются слова списка
	UInt32 TotalSoundCount;

	/// Является ли данный список списком слов полнотекстового поиска
	UInt32 IsFullTextSearchList;

	/// Содержит или нет данный список для каждого слова все словоформы
	UInt32 IsFullMorphologyWordFormsList;

	/// Содержит тип алфавита основного языка списка
	UInt32 AlphabetType;

	/// Содержит или нет данный список локализацию слов
	UInt32 IsLocalizedList;

	/// Флаг того, что некоторым словам из списка слов сопоставлено видео
	UInt32 IsVideo;

	/// Общее количество видео в списке слов
	/// Не учитываются видео самих слов списка (иконки). Учитывается только видео в переводах, на которые ссылаются слова списка
	UInt32 TotalVideoCount;

	/// Флаг того, что некоторым словам из списка слов сопоставлена 3d сцена
	UInt32 IsScene;

	/// Общее количество сцен в списке слов
	/// Не учитываются сцен самих слов списка. Учитывается только сцены в переводах, на которые ссылаются слова списка
	UInt32 TotalSceneCount;

	/// Количество бит необходимых для представления индекса видео
	UInt32 VideoIndexSize;

	/// Индекс списка, для которого данный список является вспомогательным
	Int32 SimpleSortedListIndex;

	/// Тип ресурса в котором хранятся элементы дерева бинарного поиска
	UInt32 Type_SearchTreePoints;

	/// Тип ресурса в котором хранится информация об альтернативных заголовках
	UInt32 Type_AlternativeHeadwordsInfo;

	/// Список содержит стилизованные варианты написания
	UInt32 IsStylizedVariant;

	/// Количество бит необходимых для представления индекса сцен
	UInt32 SceneIndexSize;

	/// Определяет дефолтные уровни каталога, на которых будет производиться поиск см #ESldSearchRange
	UInt32 SearchRange;

	/// ID морфологической базы, с учетом которой собирался данный список
	/// В списке должны быть только базовые формы
	UInt32 MorphologyId;

	/// Зарезервировано
	UInt32 Reserved[15];

} TListHeader;

/// Заголовок хранилища индексов.
typedef struct TIndexesHeader
{
	/// Размер структуры в байтах
	UInt32 structSize;

	/// Версия хранилища индексов
	UInt32 Version;

	/// Количество записей в хранилище
	UInt32 Count;

	/// Количество записей в таблице быстрого доступа
	UInt32 QACount;

	/// Маска для данных которые имеются в данном хранилище индексов
	UInt32 DataMask;

	/// Размер в битах числа для представления количества ссылок на статьи для одного элемента списка слов
	UInt32 SizeOfCount;

	/// Размер в битах числа для представления ссылки на статью (или на номер слова в списке в случае поискового списка)
	UInt32 SizeOfIndexes;

	/// Количество бит, необходимых для представления номера списка, на который ссылается запись в поисковом списке
	/// Если != 0, то в данных сохраняются номера списков, иначе номером списка для всех записей является значение поля DefaultListIndex
	UInt32 SizeOfListIndex;

	/// Номер списка, на который ссылаются все записи в поисковом списке в случае, если эта информация не сохраняется непосредственно в данных
	UInt32 DefaultListIndex;

	/// Количество бит, необходимых для представления номера перевода в записи списка слов, на который ссылается запись в поисковом списке
	/// Если != 0, то в данных сохраняются номера переводов, иначе номером перевода для всех записей является значение поля DefaultTranslationIndex
	UInt32 SizeOfTranslationIndex;

	/// Номер перевода, на который ссылаются все записи в поисковом списке в случае, если эта информация не сохраняется непосредственно в данных
	UInt32 DefaultTranslationIndex;

	/// Количество бит, необходимых для представления величины смещения в статье, на которую ссылается запись в поисковом списке
	/// Если != 0, то в данных сохраняются смещения, иначе смещение всегда == 0
	UInt32 SizeOfShiftIndex;

	/// Тип смещений, которые используются в списке слов полнотекстового поиска (#EFullTextSearchShiftType)
	UInt32 ArticleShiftType;

	/// Зарезервировано
	UInt32 Reserved[17];

} TIndexesHeader;

/// Заголовок стиля (дерева)
typedef struct TStyleHeader
{
	/// Размер структуры в байтах
	UInt32 structSize;

	/// Полный размер в байтах данных стиля со всеми заголовками, вариантами использования, вариантами стиля
	UInt32 TotalStyleDataSize;

	/// Язык стиля (код конкретного языка)
	UInt32 Language;

	/// Количество вариантов стиля
	UInt32 NumberOfVariants;

	/// Размер в байтах блока данных с одним вариантом стиля
	UInt32 SizeOfStyleVariant;

	/// Номер варианта стиля для отображения по умолчанию
	UInt32 DefaultVariantIndex;

	/// Количество вариантов использования (см. #ESldStyleUsageEnum)
	UInt32 NumberOfUsages;

	/// Размер в байтах блока данных с одним вариантом использования
	UInt32 SizeOfStyleUsage;

	/// Название тега со стилем
	UInt16 Tag[32];

	/// Зарезервировано
	UInt32 Reserved[8];

} TStyleHeader;

/// Заголовок в котором описано как именно нужно декодировать данные
typedef struct TCountCharMethodHeader
{
	/// Размер структуры
	UInt32 structSize;

	/// Количество бит необходимых для представления одного символа.
	UInt32 CodeSize;

	/// Количество символов описанных здесь
	UInt32 NumberOfChars;
} TCountCharMethodHeader;

/// Заголовок в котором описано как именно нужно декодировать данные
typedef struct TCharChainMethodHeader
{
	/// Размер структуры
	UInt32 structSize;

	/// Количество бит необходимых для представления одного символа.
	UInt32 CodeSize;

	/// Количество символов описанных здесь
	UInt32 NumberOfChars;

	/// Зарезервировано
	UInt32 Reserved;
} TCharChainMethodHeader;

/// Структура описывающая место хранения для одного куска текста.
typedef struct TCharChainMethodData
{
	/// Смещение на начало куска текста
	UInt16 Shift;
	/// длина куска текста в символах(1 символ = 2 байта)
	UInt16 Len;
} TCharChainMethodData;

/// Заголовок ресурса с таблицей символов языка
/// или общей для всех языков таблицей символов-разделителей (LanguageCode == SldLanguage::Delimiters)
typedef struct TLanguageSymbolsTableHeader
{
	/// Размер структуры
	UInt32 structSize;

	/// Версия таблицы
	UInt32 Version;

	/// Код языка таблицы символов
	UInt32 LanguageCode;

	/// Размер таблицы
	UInt32 TableSize;

} TLanguageSymbolsTableHeader;

/// Описание базы морфологии
typedef struct TMorphoBaseElement
{
	/// Код языка для которого предназначена база морфологии
	UInt32 LanguageCode;
	/// id базы морфологии
	UInt32 DictId;
	/// Резерв
	UInt32 Reserved[2];
} TMorphoBaseElement;

/// Структура с информацией об альтернативном заголовке
typedef struct TAlternativeHeadwordInfo
{
	/// Смещение данного слова в списке
	UInt32 WordShift;

	/// Количество ссылок на реальные слова
	UInt32 RealWordCount;

	/// Индекс первого элемента
	UInt32 RealWordIndex;
} TAlternativeHeadwordInfo;

/// Элементарный объект
typedef struct TAtomicObjectInfo
{
	/// Индекс перевода
	UInt32 TranslateIndex;
	/// Индекс первого блока в статье
	UInt32 FirstBlockIndex;
	/// Количество блоков элементарного объекта
	UInt32 BlockCount;
} TAtomicObjectInfo;

/************************************************************************/
/* Структуры для сравнения строчек                                      */
/************************************************************************/

/// Заголовок таблицы сравнения
typedef struct CMPHeaderType
{
	/// Размер заголовка
	UInt32 HeaderSize;
	/// Версия таблицы сравнения
	UInt32 Version;
	/// Дата создания таблицы сравнения
	UInt32 Date;
	/// Количество записей в таблице простых символов
	UInt16 SimpleCount;
	/// Количество записей в таблице сложных символов
	UInt16 ComplexCount;
	/// Код вторичного стоп-символа, для большинства словарей это 0),
	/// однако в словарях с объединенными списками это может быть символ
	/// разделитель - например \t (код символа 9).
	UInt16 EOL;
	/// Битовый флаг, хранит информацию о дополнительном контенте таблицы сравнения
	UInt16 FeatureFlag;
	/// Количество записей в таблице разделителей
	UInt16 DelimiterCount;
	/// Количество родных символов языка
	UInt16 NativeCount;
	/// Количество частичных разделителей
	UInt16 HalfDelimiterCount;
	/// Два лишних байта резерва
	UInt16 UInt16Reserved;
	/// Резерв
	UInt32 Reserved[9];
} CMPHeaderType;

/// Тип для разделителей
typedef UInt16 CMPDelimiterType;

/// Тип для родных символов языка
typedef UInt16 CMPNativeType;

/// Тип для частичных разделителей
typedef UInt16 CMPHalfDelimiterType;

/// Заголовок таблицы соответствия символов в верхнем и нижнем регистрах
typedef struct
{
	/// Размер структуры
	UInt32 structSize;
	/// Версия таблицы
	UInt32 Version;
	/// Количество элементов в таблице пар для данного языка
	UInt32 NativePairCount;
	/// Количество элементов в общей таблице пар: большая таблица соответствия для всех символов, которые мы знаем
	UInt32 CommonPairCount;
	/// Количество элементов в таблице пар символов словаря: таблица пар соответствий для символов, встречающихся в списках слов конкретного словаря
	UInt32 DictionaryPairCount;
	/// Зарезервировано
	UInt32 Reserved[11];
} CMPSymbolPairTableHeader;

/// Тип описания одного простого символа.
/// Т.е. такого которому сопоставлен только 1 вес и сам символ состоит из 1 элемента.
typedef struct
{
	/// код символа
	UInt16 ch;
	/// вес символа
	UInt16 mass;

	bool operator==(const UInt16 & aRef)				const { return ch == aRef; }
	bool operator>(const UInt16 & aRef)					const { return ch > aRef; }
	bool operator>=(const UInt16 & aRef)				const { return ch >= aRef; }
	bool operator<(const UInt16 & aRef)					const { return ch < aRef; }
	bool operator<=(const UInt16 & aRef)				const { return ch <= aRef; }

} CMPSimpleType;

/**
 * Тип описания одного сложного символа.
 * Т.е. такого которому сопоставлено либо несколько весов на 1 символ,
 * либо длинна символа более 1 элемента.
 * Используется для хранения информации о сравнении таких символов как:
 *  - лигатуры "ae", "oe", и т.д.
 *  - эсцет (что-то вроде бетты) в немецком - один символ соответсвует двум
 *    "эсцет" = "ss"
 *  - ch - в чешском расположено после `H` т.е. порядок слов должен быть такой:
 *    Hz
 *    Cha
 *    Ia
 */
typedef struct
{
	/// цепочка символов которая участвует в сравнении.
	UInt16 chain[CMP_CHAIN_LEN];
	/// веса символов в ответе цепочки.
	UInt16 mass[CMP_CHAIN_LEN];
} CMPComplexType;

/// Структура описывает соответствие символов в верхнем и нижнем регистрах
typedef struct
{
	/// код символа в верхнем регистре
	UInt16 up;
	/// код символа в нижнем регистре
	UInt16 low;
} CMPSymbolPair;

/// Элемент в котором описано, что происходит с таблицей сравнения - где ее брать, какой язык и т.д.
typedef struct TCMPTableElement
{
	/// Тип ресурса в котором находится таблица сравнения
	UInt32 ResourceType;
	/// Номер ресурса в котором находится таблица сравнения
	UInt32 ResourceIndex;
	/// Код языка для которого предназначена данная таблица сравнения
	UInt32 LanguageCode;
	/// Приоритет данной таблицы сравнения(может быть базовая или например уточняющая)
	UInt32 Priority;
} TCMPTableElement;

/// Заголовок для системы сравнения строк.
typedef struct TCMPTablesHeader
{
	/// Размер структуры
	UInt32 structSize;

	/// Версия заголовка
	UInt32 HeaderVersion;

	/// Количество таблиц сравнения
	UInt32 NumberOfCMPtables;

	/// Далее идет массив элементов
	/// TCMPTableElement CMPInfo[0];
} TCMPTablesHeader;


// the main header of all binary metadata
struct TStructuredMetadataHeader
{
	// the number of bits reserved to the resource slot (with the corresponding counts)
	// this is good enough for 4 slots and 1bil metadata structs
	enum { SlotBits = 2, MaxSlotCount = 1 << SlotBits, MaxMetadataCount = 1 << (32 - SlotBits) };

	// the size of the header
	UInt16 _size;

	// the total number of metadata resources
	UInt16 resourceCount;
	// the size (in bytes) of TStructuredMetadataResourceInfoStruct stored
	UInt16 resourceInfoSize;
	// the size (in bytes) of TStructuredMetadataResourceHeader stored
	UInt16 resourceHeaderSize : 16 - SlotBits;
	// maximum number of metadata resource "slots" used by articles
	UInt16 maxSlot : SlotBits;

	// the size (in bytes) of TStructuredMetadataMetaEnumInfoStruct stored
	UInt16 metadataInfoSize;
	// the count of TStructuredMetadataMetaEnumInfoStruct structs
	UInt16 metadataInfoCount;

	// resource type of the string store used for metadata strings
	UInt32 stringsResourceType;
};

// the "descriptors" for resources
struct TStructuredMetadataResourceInfoStruct
{
	// the *next after last* index of metadata struct contained inside this resource
	// treated basically like a `startIndex + count`
	UInt32 endIndex : 32 - TStructuredMetadataHeader::SlotBits;
	// the "slot" for the metadata contained inside
	UInt32 slot : TStructuredMetadataHeader::SlotBits;
};

// the info for metadata enums
struct TStructuredMetadataMetaEnumInfoStruct
{
	// the size of the metadata structure
	UInt32 metadataStructSize : 12;
	// * explictly pad to the next uint32 */
	UInt32 _pad0 : 20;
};

struct TStructuredMetadataResourceHeader
{
	// the count of bits used by the metadata type
	UInt32 typeBits : 5;
	// the count of bits used by the metadata offset
	UInt32 offsetBits : 5;
	// the offset of the offsets table (must start inside the first 16k)
	UInt32 offsetsTableOffset : 14;
	// padding
	UInt32 _pad0 : 8;
};

// the main css resource header
struct TSldCSSDataHeader {
	// the size of the header
	UInt16 _size;

	// the size of a TSldCSSResourceDescriptor stored
	UInt16 resourceDescriptorSize;
	// the size of a TSldCSSResourceHeader stored
	UInt16 resourceHeaderSize;

	// the count of blocks resources
	UInt16 blocksResourceCount;
	// the count of properties resources
	UInt16 propsResourceCount;
	// the count of bits used for a property index
	UInt16 propsIndexBits : 5;
	// padding
	UInt16 _pad0 : 11;
	// resource type used by the string store
	UInt32 stringsResourceType;
};

// the "descriptor" for css resources (shared between blocks and properties)
struct TSldCSSResourceDescriptor
{
	// the *next after last* index of css property/block contained inside this resource
	// treated basically like a `startIndex + count`
	UInt32 endIndex;
};

struct TSldCSSResourceHeader
{
	// the count of bits used by the css struct offset
	UInt32 offsetBits : 5;
	// padding
	UInt32 _pad0 : 27;
};

// the "main" string store header
struct TSldStringStoreHeader
{
	// the size of the header
	UInt16 _size;

	// if the string store can resolve "indexed" strings
	UInt16 indexable : 1;
	UInt16 _flags0 : 15;

	// the total count of string resources
	UInt16 resourceCount;
	// the size of the descriptor for resources
	// (may be 0, in this case there is no descriptor array following the header)
	// if they ever need to be bigger than 256 bytes we'd be doing something terribly wrong
	UInt16 resourceDescriptorSize : 8;
	UInt16 resourceHeaderSize : 8;

	// the method that was used to compress the string data
	UInt16 compressionMethod;
	// the maximum string length
	UInt16 maxStringLength;

	// the maximum resource size
	UInt32 maxResourceSize;
};

// the "descriptor" for string stores that have indexes
struct TSldStringStoreResourceDescriptor
{
	// the *next after last* index of the string inside this resource
	// treated basically like a `startIndex + count`
	UInt32 endIndex;
};

// the "header" living at the start of every resource
struct TSldStringStoreResourceHeader
{
	// the count of bits used by the string offset (valid really only for indexable stores)
	UInt32 offsetBits : 5;
	// the maximum string offset in the resource
	UInt32 maxOffset : 27;
};

// the header of a string store containing a single string
struct TSldSingleStringStoreHeader
{
	// the size of the header
	UInt16 _size;

	// the method that was used to compress the string data
	UInt16 compressionMethod;

	// offset of the encoded data
	UInt32 stringOffset;
	// encoded string length
	UInt32 stringLength;
};

//////////////////////////////////////////////////////////////////////////
// Коды языков
//////////////////////////////////////////////////////////////////////////
namespace SldLanguage
{
	enum ESldLanguage : UInt32
	{
		Afrikaans	= SLD_LANGUAGE_CODE('afri'),
		Albanian	= SLD_LANGUAGE_CODE('alba'),
		AmericanEnglish = SLD_LANGUAGE_CODE('amen'),
		Arabic		= SLD_LANGUAGE_CODE('arab'),
		Argentinian = SLD_LANGUAGE_CODE('arge'),
		Basque		= SLD_LANGUAGE_CODE('basq'),
		Brazilian	= SLD_LANGUAGE_CODE('braz'),  /// Пока у нас нет диалектов - для бразильских словарей нужен отдельный код
		BritishEnglish = SLD_LANGUAGE_CODE('bren'),
		Bulgarian	= SLD_LANGUAGE_CODE('bulg'),
		Burmese		= SLD_LANGUAGE_CODE('burm'),
		Cantonese	= SLD_LANGUAGE_CODE('cant'),
		Catalan		= SLD_LANGUAGE_CODE('ctln'),
		Catalonian	= SLD_LANGUAGE_CODE('ctnn'),
		Chinese		= SLD_LANGUAGE_CODE('chin'),
		Croatian	= SLD_LANGUAGE_CODE('croa'),
		Czech		= SLD_LANGUAGE_CODE('czec'),
		Danish		= SLD_LANGUAGE_CODE('dani'),
		Dutch		= SLD_LANGUAGE_CODE('dutc'),
		English		= SLD_LANGUAGE_CODE('engl'),
		Estonian	= SLD_LANGUAGE_CODE('esto'),
		Farsi		= SLD_LANGUAGE_CODE('fars'),
		Filipino	= SLD_LANGUAGE_CODE('fili'),
		Finnish		= SLD_LANGUAGE_CODE('finn'),
		French		= SLD_LANGUAGE_CODE('fren'),
		German		= SLD_LANGUAGE_CODE('germ'),
		Greek		= SLD_LANGUAGE_CODE('gree'),
		Hebrew		= SLD_LANGUAGE_CODE('hebr'),
		Hindi		= SLD_LANGUAGE_CODE('hind'),
		Hungarian	= SLD_LANGUAGE_CODE('hung'),
		Icelandic	= SLD_LANGUAGE_CODE('icel'),
		Indonesian	= SLD_LANGUAGE_CODE('indo'),
		Irish		= SLD_LANGUAGE_CODE('iris'),
		Italian		= SLD_LANGUAGE_CODE('ital'),
		Japanese	= SLD_LANGUAGE_CODE('japa'),
		Korean		= SLD_LANGUAGE_CODE('kore'),
		Latin		= SLD_LANGUAGE_CODE('lati'),
		Latvian		= SLD_LANGUAGE_CODE('latv'),
		Lithuanian	= SLD_LANGUAGE_CODE('lith'),
		Malay		= SLD_LANGUAGE_CODE('mala'),
		Norwegian	= SLD_LANGUAGE_CODE('norw'),
		Polish		= SLD_LANGUAGE_CODE('poli'),
		Portuguese	= SLD_LANGUAGE_CODE('port'),
		Romanian	= SLD_LANGUAGE_CODE('roma'),
		Russian		= SLD_LANGUAGE_CODE('russ'),
		Serbian		= SLD_LANGUAGE_CODE('serb'),
		Shona		= SLD_LANGUAGE_CODE('shon'),
		Slovak		= SLD_LANGUAGE_CODE('slvk'),
		Slovenian	= SLD_LANGUAGE_CODE('slvn'),
		Spanish		= SLD_LANGUAGE_CODE('span'),
		Swahili		= SLD_LANGUAGE_CODE('swah'),
		Swedish		= SLD_LANGUAGE_CODE('swed'),
		Thai		= SLD_LANGUAGE_CODE('thai'),
		Tswana		= SLD_LANGUAGE_CODE('tswa'),
		Turkish		= SLD_LANGUAGE_CODE('turk'),
		Ukrainian	= SLD_LANGUAGE_CODE('ukra'),
		Urdu		= SLD_LANGUAGE_CODE('urdu'),
		Uzbek		= SLD_LANGUAGE_CODE('uzbe'),
		Vietnamese	= SLD_LANGUAGE_CODE('viet'),
		WorldEnglish = SLD_LANGUAGE_CODE('woen'),

		Default		= SLD_LANGUAGE_CODE('0000'),
		Delimiters	= SLD_LANGUAGE_CODE('deli'),
		Unknown		= 0
	};

	inline ESldLanguage fromCode(UInt32 code) { return static_cast<ESldLanguage>(code); }
}
using ESldLanguage = SldLanguage::ESldLanguage;

#endif
