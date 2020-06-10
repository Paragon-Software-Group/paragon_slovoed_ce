#ifndef _SLD_DEFINES_H_
#define _SLD_DEFINES_H_

/************************************************************************/
/* Описания ресурсов                                                    */
/************************************************************************/

/// Тип ресурса заголовка - HEAD
/** 
	Ресурс заголовка словаря - индекс: 0
	Ресурс заголовка списка слов - индекс: 1
*/
#define SLD_RESOURCE_HEADER			'DAEH'

/// Тип ресурса с информацией о версии словарной базы
#define SLD_RESOURCE_DICTIONARY_VERSION_INFO	'IREV'

/// Тип ресурса строчки с названиями словаря в целом - STRW
#define SLD_RESOURCE_STRINGS		'WRTS'

/// Тип ресурса строчки с названиями для списка слов - STRL
#define SLD_RESOURCE_LIST_STRINGS	'LRTS'

/// Тип ресурса таблицы сравнения - UCMP
#define SLD_RESOURCE_COMPARE_TABLE	'PMCU'

/// Тип ресурса заголовка таблиц сравнения - HCMP
#define SLD_RESOURCE_COMPARE_TABLE_HEADER	'PMCH'

/// Тип ресурса базы морфологии - UMRP
#define RESOURCE_TYPE_MORPHOLOGY_DATA	'PRMU'

/// Тип ресурса заголовка таблиц сравнения - HMRP
#define RESOURCE_TYPE_MORPHOLOGY_DATA_HEADER	'PRMH'

/// Тип ресурса для сжатых данных(базовый).
/** 
	Для первого списка слов будет ADAT
	Для второго списка слов будет BDAT
*/
#define SLD_RESOURCE_COMPRESSED		'TADA'
/// Тип ресурса для иерархии(базовый).
/** 
	Для первого списка слов будет ACAT
	Для второго списка слов будет BCAT
*/
#define SLD_RESOURCE_HIERARCHY		'TACA'
/// Тип ресурса для таблицы быстрого доступа(базовый).
/** 
	Для первого списка слов будет AQAT
	Для второго списка слов будет BQAT
*/
#define SLD_RESOURCE_QA				'TAQA'

/// Тип ресурса для дерева сжатия(базовый).
/** 
	Для первого списка слов будет ATRE
	Для второго списка слов будет BTRE
*/
#define SLD_RESOURCE_TREE			'ERTA'

/// Тип ресурса для бинарных данных картинок IMGA
#define RESOURCE_TYPE_IMAGE			'AGMI'

/// Тип ресурса для бинарных данных видео VIDO
#define RESOURCE_TYPE_VIDEO			'OEDV'

/// Тип ресурса для бинарных данных озвучек SOND
#define RESOURCE_TYPE_SOUND			'DNOS'

/// Тип ресурса для бинарных данных сцен SL3D
#define RESOURCE_TYPE_SCENE			'D3LS'

/// Тип ресурса для бинарных данных моделей MESH
#define RESOURCE_TYPE_MESH			'HSEM'

/// Тип ресурса для бинарных данных Материалов MTRL
#define RESOURCE_TYPE_MATERIAL		'LRTM'

/// Тип ресурса для бинарных данных Материалов ABST
#define RESOURCE_TYPE_ABSTRACT			'TSBA'

/// Тип ресурса для хранения описания к учебнику 'INFO'
#define SLD_RESOURCE_INFO			'OFNI'

/// Тип ресурса для хранения списка элементарных объектов AOAT
#define SLD_AO_ARTICLE_TABLE		'TAOA'

/// Тип ресурса для хранения информации о связи базы с внешними 'EXTI'
#define SLD_EXTERN_CONTENT_INFO			'ITXE'

/// Тип ресурса для таблиц символов (символов языков и общей для всех языков таблицы символов-разделителей) SYMT
#define SLD_RESOURCE_SYMBOLS_TABLE	'TMYS'

/// Тип ресурса для таблиц символов-разделителей для конкретных языков SYDT
#define RESOURCE_TYPE_SYMBOLS_DELIMITERS_TABLE	'TDYS'

/// Тип ресурса для таблицы свойств вариантов написания списка слов VARP (см. #TListVariantProperty)
#define SLD_RESOURCE_VARIANTS_PROPERTY_TABLE	'PRAV'

/// Тип ресурса для тематик блоков switch THEM
#define RESOURCE_TYPE_THEMATIC					'MEHT'

/// Тип ресурса для массива смещений каждого слова в списке
/**
Для первого списка слов будет ASDT
Для второго списка слов будет BSDT
*/
#define RESOURCE_TYPE_DIRECT_SHIFT	'TDSA'

/// Тип ресурса для описания стиля страницы
#define RESOURCE_TYPE_PAGE_STYLE		'SEGP'

/// Тип ресурса для дополнительной информации о ресурсах RINF
#define RESOURCE_TYPE_RESOURCE_INFO	'FNIR'

/// Тип ресурса для имён других ресурсов (названия файлов, etc) NAME
#define RESOURCE_TYPE_RESOURCE_NAMES	'EMAN'

/// Тип ресурса для путей других ресурсов (названия файлов, etc) PATH
#define RESOURCE_TYPE_RESOURCE_PATHS	'HTAP'

/// Тип ресурса для хранения параметров метаданных в бинарном виде - "головной" ресурс MTDH
#define SLD_RESOURCE_STRUCTURED_METADATA_HEADER		'HDTM'
/// Тип ресурса для хранения параметров метаданных в бинарном виде - ресурс с данными MTDD
#define SLD_RESOURCE_STRUCTURED_METADATA_DATA		'DDTM'
/// Тип ресурса для хранения параметров метаданных в бинарном виде - ресурс со строками MTDS
#define SLD_RESOURCE_STRUCTURED_METADATA_STRINGS	'SDTM'

/// Тип ресурса для хранения css "метаданных" - "головной" ресурс CSSH
#define SLD_RESOURCE_CSS_DATA_HEADER		'HSSC'
/// Тип ресурса для хранения css "метаданных" - ресурс с блоками параметров CSSB
#define SLD_RESOURCE_CSS_DATA_BLOCKS		'BSSC'
/// Тип ресурса для хранения css "метаданных" - ресурс с параметрами CSSP
#define SLD_RESOURCE_CSS_DATA_PROPERTIES	'PSSC'
/// Тип ресурса для хранения css "метаданных" - ресурс со строками CSSS
#define SLD_RESOURCE_CSS_DATA_STRINGS		'SSSC'

/// Тип ресурса для хранения строки java script'а для статей 'AJSS'
#define SLD_RESOURCE_ARTICLES_JAVA_SCRIPT	'SSJA'


/// Тип ресурса, содержащего бинарные данные или файлы, которые могут понадобиться оболочке, при работе со словарем
/// Получить информацию об этих ресурсах можно через список типа eWordListType_BinaryResource
#define RESOURCE_TYPE_BINARY			'TNIB'

/************************************************************************/
/* Версии ядра, при которых произошли изменения в ресурсах              */
/************************************************************************/

/// Версия ядра, в которой размер заголовка, описывающего уровень иерархии, был изменен (с 16 до 32 байт)
#define ENGINE_VERSION_HIERARCHY_LEVEL_HEADER		112

/************************************************************************/
/* Константы для класса сравнения строк                                 */
/************************************************************************/

/// Длина цепочки символов
#define CMP_CHAIN_LEN				(4)

/// Диапазон часто используемых символов
#define CMP_MOST_USAGE_SYMBOL_RANGE	(0x512)

/// Маска для флага - определяет что это - вес символа или индекс таблицы сложных символов.
#define CMP_MASK_OF_INDEX_FLAG		(0x8000)
/// Маска для индекса - в случае, если это индекс, убирает флаг, оставляя чистый индекс.
#define CMP_MASK_OF_INDEX			(0x7fff)
/// Идентификатор символа игнорирования.
#define CMP_IGNORE_SYMBOL			(0)
/// Идентификатор символа который не найден.
#define CMP_NOT_FOUND_SYMBOL		(0xffff)

/// Версия таблицы сравнения занимающая небольшое место.
#define CMP_VERSION_1		1
/// Версия таблицы сравнения имеющая прямое отображение символов в их веса.
#define CMP_VERSION_2		2

/// Восстанавливает смещение по смещению из таблицы быстрого
#define  SLD_QA_SHIFT_RESTORE(aShiftBit, aHASH)		((((aShiftBit^aHASH) & 022222222222)>>1) |	\
													 (((aShiftBit^aHASH) & 011111111111)<<1) |	\
													   (aShiftBit & 04444444444))
/// Максимальное значение для UInt32
#define MAX_UINT_VALUE					((UInt32)~0)

/// Максимальное значение для UInt16
#define MAX_UINT16_VALUE				((UInt16)~0)

/************************************************************************/
/* Варианты сжатия данных                                               */
/************************************************************************/
/// Без сжатия
/** ********************************************************************
*	Без сжатия, текст представлен в явном виде, бинарные данные представлены 
*	в бинарном виде, из-за этого может происходить сдвиг текстовых данных.
************************************************************************/
#define SLD_COMPRESSION_METHOD_TEXT			(0)

/************************************************************************/
/* Номера типов данных для управления декодированием.                   */
/************************************************************************/
/// Номер типа данных для получения количества одинаковых символов у текущего и предыдущего слов.
#define SLD_DECORER_TYPE_SHIFT	(0)
/// Номер типа данных для декодирования списка стилей в статьях
#define SLD_DECORER_TYPE_STYLES	(0)
/// Номер типа данных для декодирования в списке слов собственно текста
#define SLD_DECORER_TYPE_TEXT	(1)

/// Номер таблицы сравнения по умолчанию.
#define SLD_DEFAULT_CMP			(0)

/// Размер приращения буфера для хранения пути в каталоге(в элементах).
#define SLD_PATH_INCREASE		(8)


/// Количество слов между точками быстрого доступа по умолчанию
#define SLD_DEFAULT_QA_INTERLEAVE			(128)

/// Количество бит которые закодированы в индексе по умолчанию
#define SLD_DEFAULT_QA_INTERLEAVE_SIZE		(1024)

/// Количество бит которые закодированы в индексе по умолчанию
#define SLD_DEFAULT_DIRECT_ACCESS_POINT_COUNT_IN_RESOURCE		(0x2000)

/// Максимальный размер ресурса с сжатыми данными, если данных больше чем на 1 ресурс, тогда остатки переносятся в следующий ресурс.
#define MAX_COMPRESSED_DATA_RESOURCE_SIZE	(0x8000)

/// Количество слов в пользовательском списке, для которых будет выделена память по умолчанию
#define SLD_DEFAULT_WORDS_COUNT_IN_CUSTOM_LIST					(0x10)


/// Макрос для изменения порядка байт в машинном слове. На x86,ARM,MIPS,SH3 и т.д.(кроме PowerPC/DragonBall/Motorolla68000) ничего делаться не должно.
#define MORPHO_SWAP_16(x)    REVERSE_INT16(x)
/// Макрос для изменения порядка байт в машинном слове. На x86,ARM,MIPS,SH3 и т.д.(кроме PowerPC/DragonBall/Motorolla68000) ничего делаться не должно.
#define MORPHO_SWAP_32(x)    REVERSE_INT32(x)

/// Макрос для поворота байт
#define EnsureNativeByteOrder_32(x)	((((x)&0xFF)<<24)|(((x)&0xFF00)<<8)|(((x)&0xFF0000)>>8)|(((x)&0xFF000000)>>24))
#define EnsureNativeByteOrder_16(x) ((((x)& 0xFF) << 8) | (((x)& 0xFF00) >> 8))

/// Макрос приведения 4-х символьной строки к 4-х байтовому коду языка
#define SLD_LANGUAGE_CODE(ch) (EnsureNativeByteOrder_32(ch))

/// Макрос приведения 4-х символьной строки к 4-х байтовому ID словаря
#define SLD_DICT_ID(ch) (EnsureNativeByteOrder_32(ch))

//////////////////////////////////////////////////////////////////////////
// Флаги свойств индексов
//////////////////////////////////////////////////////////////////////////

/// Флаг того, что индексы имеют в своем составе количество статей (иначе количество = 1)
#define SLD_INDEXES_IS_COUNT	(0x00000001UL)

/// Флаг того, что индексы имеют в своем составе номера статей
#define SLD_INDEXES_IS_ARTICLE_INDEX	(0x00000002UL)

/// Флаг того, что индексы имеют в своем составе номера списков слов (иначе номер списка по умолчанию нужно брать из заголовка индексов)
#define SLD_INDEXES_IS_LIST_INDEX	(0x00000004UL)

/// Флаг того, что индексы имеют в своем составе номера переводов (иначе номер перевода по умолчанию нужно брать из заголовка индексов)
#define SLD_INDEXES_IS_TRANSLATION_INDEX	(0x00000008UL)

/// Флаг того, что индексы имеют в своем составе смещения в статье (иначе смещение == 0)
#define SLD_INDEXES_IS_SHIFT_INDEX	(0x00000010UL)

/// Перечисление разновидностей полноты перевода
enum ESldTranslationFullnesType
{
	/// Полный перевод
	eTranslationFull = 0,
	/// Компактный перевод(без примеров и пояснений - только переводы слов)
	eTranslationCompact,
	/// Декодировать статью, но перевод не производить.
	eTranslationWithoutTranslation
};

/// Режимы работы движка перевода - начало/окончание
enum ESldTranslationModeType
{
	/// Начинаем сборку перевода статьи
	eTranslationStart = 0,
	/// Начинаем декодировать остатки статьи после предыдущего останова
	eTranslationContinue = 0xFFFE,
	/// Заканчиваем сборку перевода статьи
	eTranslationStop = 0xFFFF
};

/// Типы данных таблиц быстрого поиска
enum ESldQAType
{
	/// Таблица быстрого поиска для списка слов
	eQATypeList = 0,
	/// Таблица быстрого поиска для переводов
	eQATypeArticles

};

/// Уровни на которых пишется текст
enum ESldStyleLevelEnum
{
	/// Стандартное расположение текста
	eLevelNormal = 0,
	/// Нижнее расположение текста
	eLevelSub,
	/// Верхнее распложение текста
	eLevelSup
};

/// Размер шрифта текста
enum ESldStyleSizeEnum
{
	/// Обычный текст, размер заглавных букв примерно совпадает со строчными буквами eSizeLarge
	eSizeNormal = 0,
	/// размер, промежуточный между eSizeExtraSmall и eSizeSmallCaps, заглавные буквы должны примерно совпадать со строчными буквами eSizeSmallCaps шрифта
	eSizeSmall,
	/// Крупный шрифт
	eSizeLarge,
	/// размер, промежуточный между Normal и Small
	/// заглавные буквы должны примерно совпадать с размером строчных букв Normal шрифта
	eSizeSmallCaps,
	/// Мелкий шрифт, размер заглавных букв примерно совпадает со строчными буквами eSizeSmall
	eSizeExtraSmall
};


/// Разновидности использования текста. Нужно для автоматического установления стилистики
enum ESldStyleUsageEnum
{
	/*
		Общий формат имени варианта использования:
		{DIRECTION}{TYPE}[EXPLANATION]
		
		{DIRECTION}		- языковое направление (This | Pair | Both)
		{TYPE}			- название варианта
		[EXPLANATION]	- уточняющая информация о варианте (необязательно)

		TODO: не хватает content и phrase
	*/
	
	/// Предназначение не известно
	eStyleUsage_Unknown = 0,
	/// Заголовок
	eStyleUsage_ThisHeadword,
	eStyleUsage_PairHeadword,
	/// Альтернативный заголовок
	eStyleUsage_ThisHeadwordAlternative,
	eStyleUsage_PairHeadwordAlternative,
	/// Перевод
	eStyleUsage_ThisTranslation,
	eStyleUsage_PairTranslation,
	/// Фонетика
	eStyleUsage_ThisPhonetic,
	eStyleUsage_PairPhonetic,
	/// Грамматическая информация (часть речи, число и т.д.)
	eStyleUsage_ThisGrammatic,
	eStyleUsage_PairGrammatic,
	/// Пример
	eStyleUsage_ThisExample,
	eStyleUsage_PairExample,
	/// Определение
	eStyleUsage_ThisDefinition,
	eStyleUsage_PairDefinition,
	/// Комментарий
	eStyleUsage_ThisComment,
	eStyleUsage_PairComment,
	/// Пояснение к слову/фразе (обычно проясняющее, какое из значений слова/фразы имеется ввиду)
	eStyleUsage_ThisClarification,
	eStyleUsage_PairClarification,
	/// Нумерация
	eStyleUsage_ThisNumeration,
	eStyleUsage_PairNumeration,
	/// Нумерация заголовочных слов (омонимы)
	eStyleUsage_ThisNumerationHeadword,
	eStyleUsage_PairNumerationHeadword,
	/// Нумерация значений
	eStyleUsage_ThisNumerationMeaning,
	eStyleUsage_PairNumerationMeaning,
	/// Ударение
	eStyleUsage_ThisStress,
	eStyleUsage_PairStress,
	/// Синоним
	eStyleUsage_ThisSynonym,
	eStyleUsage_PairSynonym,
	/// Аббревиатура
	eStyleUsage_ThisAbbreviation,
	eStyleUsage_PairAbbreviation,
	/// Этимология
	eStyleUsage_ThisEtymology,
	eStyleUsage_PairEtymology,
	/// Фразеологизм (идиома)
	eStyleUsage_ThisIdiom,
	eStyleUsage_PairIdiom,
	/// Разделитель (пробелы, скобки, пунктуация, переводы строки и т.д.)
	eStyleUsage_ThisSeparator,
	eStyleUsage_PairSeparator,
	/// Стилистическая помета (помета, означающая неправильный глагол, фразеологизм, приближенный перевод и т.д.)
	eStyleUsage_ThisStylisticLitter,
	eStyleUsage_PairStylisticLitter,
	/// Ссылка на слово, статью
	eStyleUsage_ThisReference,
	eStyleUsage_PairReference,
	/// Метаданные
	eStyleUsage_BothMetadata,
	/// Фразы
	eStyleUsage_ThisPhrase,
	eStyleUsage_PairPhrase,
};

/**
 * Макрос "перечислающий" все типы метаданных
 *
 * eMetaImage               - картинка
 * eMetaSound               - озвучка
 * eMetaTable               - таблица
 * eMetaTableRow            - строка таблицы
 * eMetaTableCol            - столбец таблицы
 * eMetaParagraph           - параграф
 * eMetaLabel               - метка
 * eMetaLink                - ссылка
 * eMetaHide                - блок, который можно скрыть/показать
 * eMetaHideControl         - блок, при нажатии на который происходит скрытие/отображение блока eMetaHide
 * eMetaTest                - грамматический тест
 * eMetaTestInput           - информация, описывающее вопрос теста, на который пользователь должен ответить
 * eMetaTestToken           - один из блоков текста в тесте, который надо расположить в правильном порядке наряду с остальными такими же блоками
 * eMetaPopupImage          - всплывающая картинка
 * eMetaUrl                 - внешняя ссылка
 * eMetaUiElement           - элемент интерфейса
 * eMetaPopupArticle        - всплывающая статья
 * eMetaNoBrText            - блок текста без переносов
 * eMetaInfoBlock           - блок info; на место этих метаданных нужно подставить информацию (иконку, текст и т.п.) о том, что используется InApp (сокращенная) база
 * eMetaBackgroundImage     - картинка, подставляемая в качестве фона текста
 * eMetaFlashCardsLink      - ссылка для флеш-карточек
 * eMetaVideo               - видео
 * eMetaScene               - 3d сцены
 * eMetaImageArea           - выделенная область на изображении
 * eMetaSlideShow           - слайдшоу
 * eMetaVideoSource         - ресурс видео
 * eMetaMediaContainer      - контейнер медиаданных
 * eMetaTestSpear           - объект, подлежащий сопоставлению в тесте на сопоставление
 * eMetaTestTarget          - область, с которой будет установлено сопоставление
 * eMetaTestControl         - объект, содержащий ответ теста на сопоставление
 * eMetaSwitch              - блок, который можно скрыть/показать
 * eMetaSwitchControl       - блок, при нажатии на который происходит скрытие/отображение блока eMetaSwitch
 * eMetaSwitchState         - объект, определяющий состояния блоков eMetaSwitch и eMetaSwitchControl
 * eMetaManagedSwitch       - блок eMetaSwitch, управляемый текущим блоком eMetaSwitchControl
 * eMetaDiv                 - блок div (html `div`)
 * eMetaMap                 - географические карты
 * eMetaMapElement          - элемент географических карт
 * eMetaCaption             - подпись к картинками
 * eMetaTestResult          - блок результатов тестов (блок с "шариками")
 * eMetaTestResultElement   - результат теста ("шарик")
 * eMetaTextControl         - текст для тестов с выделением
 * eMetaTaskBlockEntry      - внутренний элемент для блока "дано/найти/решение" (может быть дано, найти, доказать, анализ, доказательство и др.)
 * eMetaSidenote            - памятка на полях
 * eMetaConstructionSet     - конструктор
 * eMetaDrawingBlock        - блок рисования
 * eMetaArticleEventHandler - обработчик событий на статье (представляется атрибутами на узле article)
 * eMeta_Unused1            - unused
 * eMeta_UnusedBroken       - unused and can *NEVER* be recycled
 * eMetaTestContainer       - контейнер тестов разного типа
 * eMetaLegendItem          - легенда
 * eMetaAtomicObject        - элементарный объект
 * eMetaCrossword           - элемент кроссворд
 * eMetaExternArticle       - внешняя статья
 * eMetaList                - список
 * eMetaLi                  - элемент списка
 * eMetaInteractiveObject   - список?
 * eMeta_Unused0            - unused
 * eMetaTimeLine            - лента времени
 * eMetaTimeLineItem        - элемент ленты времени, отображаемые в разные моменты
 * eMetaAbstractResource    - контейнер абстрактных ресурсов
 * eMetaFormula             - формула
 * eMetaCrosswordHint       - подсказка для кроссворда
 * eMetaFootnoteBrief       - краткая сноска
 * eMetaFootnoteTotal       - полная сноска
 */
#define SLD_FOREACH_METADATA_TYPE(DEF) \
	DEF(eMetaImage)               \
	DEF(eMetaSound)               \
	DEF(eMetaTable)               \
	DEF(eMetaTableRow)            \
	DEF(eMetaTableCol)            \
	DEF(eMetaParagraph)           \
	DEF(eMetaLabel)               \
	DEF(eMetaLink)                \
	DEF(eMetaHide)                \
	DEF(eMetaHideControl)         \
	DEF(eMetaTest)                \
	DEF(eMetaTestInput)           \
	DEF(eMetaTestToken)           \
	DEF(eMetaPopupImage)          \
	DEF(eMetaUrl)                 \
	DEF(eMetaUiElement)           \
	DEF(eMetaPopupArticle)        \
	DEF(eMetaNoBrText)            \
	DEF(eMetaInfoBlock)           \
	DEF(eMetaBackgroundImage)     \
	DEF(eMetaFlashCardsLink)      \
	DEF(eMetaVideo)               \
	DEF(eMetaScene)               \
	DEF(eMetaImageArea)           \
	DEF(eMetaSlideShow)           \
	DEF(eMetaVideoSource)         \
	DEF(eMetaMediaContainer)      \
	DEF(eMetaTestSpear)           \
	DEF(eMetaTestTarget)          \
	DEF(eMetaTestControl)         \
	DEF(eMetaSwitch)              \
	DEF(eMetaSwitchControl)       \
	DEF(eMetaSwitchState)         \
	DEF(eMetaManagedSwitch)       \
	DEF(eMetaDiv)                 \
	DEF(eMetaMap)                 \
	DEF(eMetaMapElement)          \
	DEF(eMetaCaption)             \
	DEF(eMetaTestResult)          \
	DEF(eMetaTestResultElement)   \
	DEF(eMetaTextControl)         \
	DEF(eMetaTaskBlockEntry)      \
	DEF(eMetaSidenote)            \
	DEF(eMetaConstructionSet)     \
	DEF(eMetaDrawingBlock)        \
	DEF(eMetaArticleEventHandler) \
	DEF(eMetaDemoLink)            \
	DEF(eMeta_UnusedBroken)       \
	DEF(eMetaTestContainer)       \
	DEF(eMetaLegendItem)          \
	DEF(eMetaAtomicObject)        \
	DEF(eMetaCrossword)           \
	DEF(eMetaExternArticle)       \
	DEF(eMetaList)                \
	DEF(eMetaLi)                  \
	DEF(eMetaInteractiveObject)   \
	DEF(eMeta_Unused0)            \
	DEF(eMetaTimeLine)            \
	DEF(eMetaTimeLineItem)        \
	DEF(eMetaAbstractResource)    \
	DEF(eMetaFormula)             \
	DEF(eMetaCrosswordHint)       \
	DEF(eMetaFootnoteBrief)       \
	DEF(eMetaFootnoteTotal)

/// Разновидность текста
enum ESldStyleMetaTypeEnum
{
	/// Текст
	eMetaText = 0,
	/// Транскрипция
	eMetaPhonetics,

#define ENUM(enum_) enum_,
	SLD_FOREACH_METADATA_TYPE(ENUM)
#undef ENUM

	/// Общее количество определенных enum'ов
	eMeta_Last,

	// NOTE: deprecated alias
	eMetaItemTimeLine = eMetaTimeLineItem,

	/// Неизвестный тип
	eMetaUnknown = 0xFFFF
};

/// типы управления блоком switch
enum ESldMetaSwitchManageTypeEnum
{
	/// блок switch управляется блоком switch-control 
	eSwitchManagedBySwitchControl,
	/// блок switch управляется настройками приложения
	eSwitchManagedByApplicationSettings
};

/// типы тематик блоков switch
enum ESldMetaSwitchThematicTypeEnum
{
	eSwitchThematicDefault = 0,
	
	eSwitchThematicExamples,
	eSwitchThematicEtymology,

	eSwitchThematicPhrase,
	eSwitchThematicIdioms,

	eSwitchThematicPhonetics,
	eSwitchThematicMorph,

	eSwitchThematicGram,
	eSwitchThematicMenu,

	eSwitchThematicImages,
	eSwitchThematicLinks,
	eSwitchThematicSynonyms,
	eSwitchThematicAntonyms,
	eSwitchThematicHelpNotes,
	eSwitchThematicUsageNotes,

	eSwitchThematicRadioButton,

	eSwitchThematicAbbreviations,

	eSwitchThematicTestHint,

	/// Общее количество тематик, последний элемент enum-а
	eSwitchThematicsNumber
};

/// Типы семейства шрифтов
enum ESldStyleFontFamilyEnum
{
	/// Рубленные шрифты (шрифты без засечек или гротески)
	eFontFamily_SansSerif = 0,
	/// Шрифты с засечками
	eFontFamily_Serif,
	/// Декоративные шрифты
	eFontFamily_Fantasy,
	/// Моноширинные шрифты
	eFontFamily_Monospace,
	/// Неизвестный тип
	eFontFamily_Unknown = 0xFFFF
};

/// Возможные названия шрифтов
enum ESldStyleFontNameEnum
{
	/// DejaVu Sans
	eFontName_DejaVu_Sans = 0,
	/// Lucida Sans
	eFontName_Lucida_Sans,
	/// Verdana
	eFontName_Verdana,
	/// Georgia
	eFontName_Georgia,
	/// HelveticaNeueLT Std
	eFontName_HelveticaNeueLT_Std,
	/// DejaVu Serif
	eFontName_DejaVu_Serif,
	/// Helvetica
	eFontName_Helvetica,
	/// Source Sans Pro
	eFontName_Source_Sans_Pro,
	/// Gentium
	eFontName_Gentium,
	/// Merriweather
	eFontName_Merriweather,
	/// Merriweather Sans
	eFontName_Merriweather_Sans,
	/// Noto Sans
	eFontName_Noto_Sans,
	/// Noto Serif
	eFontName_Noto_Serif,
	/// Trajectum
	eFontName_Trajectum,
	/// Combi Numerals
	eFontName_Combi_Numerals,
	/// Charis SIL
	eFontName_Charis_SIL,
	/// Helvetica Neue
	eFontName_HelveticaNeue,
	/// Times New Roman
	eFontName_TimesNewRoman,
	/// Lyon Text
	eFontName_Lyon_Text,
	/// Atlas Grotesk
	eFontName_Atlas_Grotesk,
	/// 1234 Sans
	eFontName_1234_Sans,
	/// Наш собственный шрифт, при необходимости может расширяться
	eFontName_Augean,
	/// Courier New
	eFontName_Courier_New,
	/// Wittenberger
	eFontName_Wittenberger,
	/// Kruti Dev
	eFontName_Kruti_Dev,
	/// Win Innwa
	eFontName_Win_Innwa,
 	/// Myriad Pro Cond
  	eFontName_Myriad_Pro_Cond,
    /// Phonetic TM
  	eFontName_Phonetic_TM,	
    /// Symbol
	eFontName_Symbol,
	/// Неизвестный шрифт
	eFontName_Unknown = 0xFFFF
};

/// Способы перехода к определенному слову в словаре
enum ESldNavigationTypeEnum
{
	/// Переход к самому слову
	/// Нужно перейти на это слово и отобразить его перевод, если он есть (неважно, статья это или раздел каталога)
	eGoToWord = 0,
	/// Переход внутрь раздела каталога
	/// Нужно перейти внутрь раздела для отображения его элементов (если слово является разделом каталога);
	/// Если слово не является разделом каталога - тогда работает как eGoToWord
	eGoToSubSection
};

/// Типы поиска слова (подмотки к слову) по переданным вариантам написания
enum ESldWordSearchTypeEnum
{
	/// бинарное совпадение всех переданных вариантов
	eFullBinaryMatch = 0,
	/// бинарное совпадение Show-варианта
	eShowVariantBinaryMatch,
	/// наиболее подходящий по расстоянию редактирования Show-вариант
	eShowVariantFuzzyMatch,
	/// совпадение всех переданных вариантов по массе
	eFullMassMatch,
	/// ближайшее вхождение Show-варианта на текущем уровне
	eClosestShowOnCurrentLevel
};

/// Перечисление вариантов предназначения списков слов.
enum EWordListTypeEnum
{
	/// неизвестный - ошибка
	eWordListType_Unknown = 0,
	/// Словарь
	eWordListType_Dictionary,
	/// Каталог (разговорник); содержит иерархический список статей
	eWordListType_Catalog,
	/// Дополнительные информационные статьи - информация о провайдере, статьи об обозначениях в словаре, различные таблицы
	eWordListType_AdditionalInfo,
	/// Cписок с результатами поиска
	eWordListType_RegularSearch,
	/// Список слов с озвучкой
	eWordListType_Sound,
	/// Начало диапазона полнотекстовых поисковых списков
	eWordListType_FullTextSearchBase = 0x100,
	/// Список полнотекстового поиска по заголовка
	eWordListType_FullTextSearchHeadword = 0x110,
	/// Список полнотекстового поиска по всему контенту статьи
	eWordListType_FullTextSearchContent = 0x120,
	/// Список полнотекстового поиска по переводам
	eWordListType_FullTextSearchTranslation = 0x130,
	/// Список полнотекстового поиска по примерам
	eWordListType_FullTextSearchExample = 0x140,
	/// Список полнотекстового поиска по определениям
	eWordListType_FullTextSearchDefinition = 0x150,
	/// Список полнотекстового поиска по фразам
	eWordListType_FullTextSearchPhrase = 0x160,
	/// Список полнотекстового поиска по идиомам
	eWordListType_FullTextSearchIdiom = 0x170,
	/// Конец диапазона полнотекстовых поисковых списков
	eWordListType_FullTextSearchLast = 0x1FF,
	/// Скрытый список слов (не отображаемый)
	/// Обычно содержит статьи с дополнительной информацией, перейти на которые можно по ссылкам из других списков;
	/// Используется для:
	/// 1) хранения статьей с отображаемой информацией - биографиями людей, со списком примеров к конкретному слову в словаре;
	/// ссылки на эти статьи могут встречаться в самом словаре (ссылка на статью в этом списке ничем не отличается от ссылок на статьи в словарных списках)
	/// 2) как вспомогательный список для полнотекстового поиска (в базах старой генерации - v.104 build 53 и ранее);
	/// в этом случае он содержит несколько вариантов написания (см. EListVariantTypeEnum) -
	/// с их помощью кроме самой найденной статьи можно, например, узнать:
	/// - фразу, в которой встретилось слово
	/// - метку в статье, на которую нужно перейти
	/// - часть речи
	eWordListType_Hidden,
	/// Список пресетов полнотекстового поиска(см. CSldDictionaryHelper::SearchForDictionaryForSearchList())
	eWordListType_DictionaryForSearch,
	/// Морфологический список, который содержит базовые формы слов, ссылающиеся на их возможные словоформы
	eWordListType_MorphologyBaseForm,
	/// Морфологический список, который содержит все возможные словоформы (то есть и базовые формы, и производные), которые ссылаются на свои базовые формы
	eWordListType_MorphologyInflectionForm,
	/// Грамматические тесты
	eWordListType_GrammaticTest,
	/// Начало диапазона специальных списков слов с доп. статьями
	eWordListType_SpecialAdditionalInfoBase = 0x300,
	/// Конец диапазона специальных списков слов с доп. статьями
	eWordListType_SpecialAdditionalInfoLast = 0x3FF,
	/// Объединенный словарный список слов (не каталог), в котором есть слова из обоих направлений (прямого и обратного).
	/// Коды языков направлений хранятся в соответствующих полях заголовка списка слов: LanguageCodeFrom и LanguageCodeTo.
	eWordListType_MergedDictionary,
	/// Начало диапазона специальных списков слов с интерактивными доп. статьями (которые можно просматривать из различных GUI-элементов)
	eWordListType_SpecialAdditionalInteractiveInfoBase = 0x500,
	/// Конец диапазона специальных списков слов с интерактивными доп. статьями
	eWordListType_SpecialAdditionalInteractiveInfoLast = 0x5FF,
	/// Морфологический список, содержащий статьи с морфологической информацией, например, таблицы словоформ (аналог eWordListType_Dictionary)
	eWordListType_MorphologyArticles,
	/// Список, содержащий статьи, на примере которых настраивается отображение скрываемых блоков в статьях
	eWordListType_ArticlesHideInfo,
	/// Список, содержащий каталог со статьями лингвистических игр
	eWordListType_GameArticles,
	/// Список, содержащий лицевые стороны встроенных в базу флеш-карточек
	eWordListType_FlashCardsFront,
	/// Список, содержащий обратные стороны встроенных в базу флеш-карточек
	eWordListType_FlashCardsBack,
	/// Список, содержащий избранные статьи InApp базы
	eWordListType_InApp,
	/// Вспомогательный список для полнотекстового поиска
	/// т.е. список с результатами полнотекстового поиска может ссылаться на этот список
	eWordListType_FullTextAuxiliary,
	/// Список, содержащий основной учебный материал в учебнике
	eWordListType_TextBook,
	/// Список, содержащий тесты к учебному материалу
	eWordListType_Tests,
	/// Список, содержащий предметный указатель учебного материала
	eWordListType_SubjectIndex,
	/// Список, содержащий всплывающие статьи
	eWordListType_PopupArticles,
	/// Сортированный по Show варианту вспомогательный список для несортированных списков
	eWordListType_SimpleSearch,
	/// Список, содержащий информацию о пользовании словарем(help)
	eWordListType_DictionaryUsageInfo,
	/// Пользовательский список
	eWordListType_CustomList,
	/// Список, содержащий ссылки на слайдшоу
	eWordListType_SlideShow,
	/// Список, содержащий карты
	eWordListType_Map,
	/// Список, содержащий КЭСы
	eWordListType_KES,
	/// Список, содержащий ФП
	eWordListType_FC,
	/// Список, содержащий AtomicObject-ы
	eWordListType_Atomic,
	/// Список, содержащий индекс нумерации страниц
	eWordListType_PageNumerationIndex,
	/// Список бинарных ресурсов, содержащихся в базе
	eWordListType_BinaryResource,
	/// Список, с проиндексированными внешними ресурсами, которые должны загружаться в первую очередь
	/// Далее идет диапазон списков, соответсвующий приоритету загрузки ресурсов из внешних баз 
	eWordListType_ExternResourcePriorityFirst,
	/// Список, с проиндексированными внешними ресурсами, которые должны загружаться в последнюю очередь
	eWordListType_ExternResourcePriorityLast = eWordListType_ExternResourcePriorityFirst + 0xF,
	/// Список имен баз, в которых содержаться внешние ресурсы
	eWordListType_ExternBaseName,
	/// Список строковых значений в структурированных метаданных
	eWordListType_StructuredMetadataStrings,
	/// Список строковых значений в css метаданных
	eWordListType_CSSDataStrings,
	/// Список шаблонов для статей
	eWordListType_ArticleTemplates,
	/// Вспомогательный поисковый список
	eWordListType_AuxiliarySearchList,
	/// Список, содержащий справочную информацию
	eWordListType_Enchiridion,
	/// Список для информации страницы Word of the Day (OALD10)
	eWordlistType_WordOfTheDay,
	/// Список предустановленного избранного
	eWordlistType_PreloadedFavourites
};


/// Перечисление типов вариантов написания в списке слов
enum EListVariantTypeEnum
{
	/// Основной вариант написания
	eVariantShow = 0,
	/// Второй вариант написания (для китайского, японского и т.д.)
	eVariantShowSecondary,
	/// Вариант написания для сортировки списка (по умолчанию не отображается)
	eVariantSortKey,
	/// Вариант написания, который используется для отображения части речи в списке слов
	eVariantPartOfSpeech,
	/// Вариант написания - метка, на которую нужно перейти в статье при переводе слова (по умолчанию не отображается)
	eVariantLabel,
	/// Вариант написания - содержимое ключа сортировки для слова в исходном списке слов
	eVariantSourceSortKey,
	eSourceSortKey = eVariantSourceSortKey,
	/// Вариант написания - код языка варианта написания eVariantShow
	/// Используется для списка слов, в котором разные элементы списка слов могут быть на разных языках
	eVariantLanguageCode,
	/// Вариант написания - флаг, явно указывающий доступен ли элемент списка для просмотра или нет
	eVariantLockFlag,
	/// Вариант написания - отступ для слова в списке, соответствующий положению элемента в иерархии
	eVariantDepth,
	/// Вариант написания - фраза, в которой встретилось слово (используется при полнотекстовом поиске)
	eVariantPhrase,
	/// Вариант написания - настройки стилизации других вариантов написания текущей записи списка слов
	eVariantStylePreferences,
	/// Вариант написания - нумерация в списке статей; это отображаемый вариант
	eVariantNumeration,
	/// Вариант написания - настройки локализации (по умолчанию не отображается)
	eVariantLocalizationPreferences,
	/// Вариант написания - тип элемента структуры учебника
	eVariantTextBookLevelType,
	/// Вариант написания - номер элемента в структуре учебника
	eVariantTextBookLevelNumber,
	/// Вариант написания - описание элемента структуры учебника
	eVariantTextBookLevelDescription,
	/// Вариант написания - индекс слова в несортированном списке (см eWordListType_SimpleSearch)
	eVariantNotSortedRealGlobalIndex,
	/// Начало диапазона дополнительной информации о слове
	eVariantGrammarInfoBegin,
	/// Конец диапазона дополнительной информации о слове
	eVariantGrammarInfoEnd = eVariantGrammarInfoBegin + 0x8,
	/// Вариант написания - тип контента 
	/// Необходимо для отображения иконок напротив элементов списка по типу контента на конечных платформах
	eVariantTypeOfContent,
	/// Ссылка на внешнюю статью
	eVariantExternArticleLink,
	/// Вариант написания, содержащий в себе текст, по которому мы можем поискать контент во внешней базе
	eVariantExternalKey,
	/// Вариант написания, содержащий в себе текст, по которому внешняя база  может найти данную запись
	eVariantInternalKey,
	/// Вариант написания, содержащий в себе имя внешнего html-файла, который должен быть отображен вместо перевода статьи
	eVariantHtmlSourceName,
	/// Вариант написания, содержащий в себе автора, для хрестоматий учебника
	eVariantAuthorName,
	/// Вариант написания, содержащий уникальный ID Kes-а
	eVariantKesID,
	/// Последний значащий вариант, нужен для определения размера енама
	eVariantEnd,
	/// Неизвестный тип - ошибка
	eVariantUnknown = 0xFFFF
};

/// Разновидность типа содержимого контейнера
enum ESlovoEdContainerDatabaseTypeEnum
{
	/// Разговорник
	eDatabaseType_Phrasebook = 0,
	/// Словарная база
	eDatabaseType_Dictionary,
	/// Звуковая база
	eDatabaseType_Sound,
	/// Морфологическая база
	eDatabaseType_Morphology,
	/// Бандл
	eDatabaseType_Bundle,
	/// База лингвистических игр
	eDatabaseType_Games,
	/// База, загружаемая через механизм встроенных покупок
	eDatabaseType_InApp,
	/// База, содержащая контент учебника
	eDatabaseType_TextBook,
	/// База с изображениями
	eDatabaseType_Images,
	/// Книга
	eDatabaseType_Book,
	/// Визуальный словарь
	eDatabaseType_PictureDictionary,
	/// Отображаемые морфо таблицы
	eDatabaseType_DisplayMorphology,
	/// Морфология, содержащая только базовые и производные формы
	eDatabaseType_InflectionMorphology,
	/// Неизвестный тип - ошибка
	eDatabaseType_Unknown = 0xFFFF
};

/// Разновидность типа источника медиаданных: озвучки, картинок и т.д.
enum EMediaSourceTypeEnum
{
	/// Медиаданные находятся непосредственно в базе SDC
	eMediaSourceType_Database = 0,
	/// Медиаданные находятся на сервере в интернете
	eMediaSourceType_InternetServer,
	/// Неизвестный тип источника - ошибка
	eMediaSourceType_Unknown = 0xFFFF
};

/// Перечисление, описывающее возможные типы вариантов стиля
enum EStyleVariantType
{
	eStyleVariant_Default = 0
};

/// Перечисление, описывающее возможные типы алфавитов
enum EAlphabetType
{
	/// Для языков, представленных одним алфавитом
	EAlphabet_Standard = 0,
	/// Китайский - иероглифы
	EAlphabet_Chin_Hierogliph,
	/// Китайский - pinjin
	EAlphabet_Chin_Pinyin,
	/// Японский - kana
	EAlphabet_Japa_Kana,
	/// Японский - kanji
	EAlphabet_Japa_Kanji,
	/// Японский - romanji
	EAlphabet_Japa_Romanji,
	/// Корейский - иероглифы
	EAlphabet_Kore_Hangul,
	/// Корейский - pinjin
	EAlphabet_Kore_Pinyin,
	/// Ничего нельзя сказать об алфавите
	EAlphabet_Unknown
};

/// Максимальная длина текста при поиске похожих слов
#define ARRAY_DIM	(128)

/// Максимальная длина текста при поиске похожих слов
#define MAX_FUZZY_WORD	ARRAY_DIM


/// Количество итераций поиска, через которое происходит взаимодействие с оболочкой
#define SLD_SEARCH_CALLBACK_INTERLEAVE	(1000)

/// Количество деревьев необходимых для декодирования списка слов.
#define SLD_NUMBER_OF_TREES_FOR_LIST	(2)



/// Количество бит отведенных для хранения контрольной суммы серийного номера.
#define SLD_SN_CRC_SIZE		(9)
/// Маска для контрольной суммы серийного номера.
#define SLD_SN_CRC_MASK		((1<<(SLD_SN_CRC_SIZE)) -1)

/// Количество бит отведенных для флага того, что это серийный номер бандла
#define SLD_SN_BUNDLE_FLAG_SIZE	(1)
/// Маска для флага того, что этот серийный номер для бандла.
#define SLD_SN_BUNDLE_FLAG_MASK	(1)

/// Количество бит для хранения срока действия серийного номера.
#define SLD_SN_DATE_SIZE		(3)

/// Количество бит для хранения HASH
#define SLD_SN_HASH_SIZE		(16)

/// Количество бит для хранения порядкового номера серийного номера.
#define SLD_SN_NUMBER_SIZE		(20)

/// Версия сохраненных данных.
#define SLD_SN_SAVE_VERSION_1	(1)

/// Количество сохраняемых данных для версии 1
#define SLD_SN_ELEMENT_COUNT_VERSION_1	(8)


/// Расположение версии в сохраняемых данных.
#define SLD_SN_DATA_VERSION		(0)
/// Случайное число необходимое для восстановления остальных данных
#define SLD_SN_DATA_V1_RANDOM	(1)
/// HASH 
#define SLD_SN_DATA_V1_HASH		(2)
/// Срок действия серийного номера
#define SLD_SN_DATA_V1_DATE		(3)
/// Порядковый номер серийного номера
#define SLD_SN_DATA_V1_NUMBER	(4)
/// Количество просмотренных статей
#define SLD_SN_DATA_V1_CLICKS	(5)
/// Идентификатор словаря для которого сохранен данный серийный номер.
#define SLD_SN_DATA_V1_DICTID	(6)
/// Контрольная сумма сохраняемого серийного номера.
#define SLD_SN_DATA_V1_CRC		(7)

/// Максимальное количество показов(2^5=32) статьи пока не начнет показывать только сообщение о незарегистрированной версии
#define SLD_SN_CLICKS_MAX		(5)
/// Количество показов которые гарантированно не будут иметь сообщений о незарегистрированной версии.(2^2 = 4)
#define SLD_SN_CLICKS_MIN		(2)

/// Смещение до значащих битов в Number которые означают, что словарь не зарегестрирован(если эти биты не равны 0) или
/// что зарегестрирован если биты = 0
#define SLD_SN_REGISTRATION_FLAG_SHIFT	(24)


/// максимальная длина слова которое может быть в списке слов озвучки.
#define SLD_SOUND_MAX_WORD_LEN				(64)

/// Флаг означает, что начато декодирование, никаких реальных данных не передается.
#define SLD_SOUND_FLAG_START				(0)
/// Флаг означает, что продолжается декодирование.
#define SLD_SOUND_FLAG_CONTINUE				(1)
/// Флаг означает, что декодирование закончено, никаких реальных данных не передается.
#define SLD_SOUND_FLAG_FINISH				(2)

/// Звуковые данные в формате spx
#define SLD_SOUND_FORMAT_SPX				(1)
/// Звуковые данные в формате wav
#define SLD_SOUND_FORMAT_WAV				(2)
/// Звуковые данные в формате amr
#define SLD_SOUND_FORMAT_AMR				(3)
/// Звуковые данные в формате mp3
#define SLD_SOUND_FORMAT_MP3				(4)
/// Звуковые данные в формате ogg
#define SLD_SOUND_FORMAT_OGG				(5)

/// Макрос для изменения порядка байт в машинном слове.
//#define REVERSE_INT16(x)				((((x)&0xFF)<<8)|(((x)&0xFF00)>>8))
#define REVERSE_INT16(x)				(x)

/// Макрос для изменения порядка байт в двойном машинном слове.
#define REVERSE_INT32(x)	(x)
//#define REVERSE_INT32(x)				((((x)&0xFF)<<24)|(((x)&0xFF00)<<8)|(((x)&0xFF0000)>>8)|(((x)&0xFF000000)>>24))

/// Максимальный размер блока со сжатыми данными(списка слов)
#define SLD_SOUND_DECODER_MAX_BLOCK_SIZE		(0x00008000)
/// Битовая маска для номера бита в пределах блока данных
#define SLD_SOUND_DECODER_BLOCK_BIT_MASK		(0x0003ffff)
/// Смещение которое делает из номера бита номер ресурса.
#define SLD_SOUND_DECODER_BLOCK_SHIFT			(15+3)


/// Индекс картинки, обозначающий, что картинки нет
#define	SLD_INDEX_PICTURE_NO			(-1)


/// Индекс видео, обозначающий, что видео нет
#define	SLD_INDEX_VIDEO_NO				(-1)

/// Индекс 3d сцены, обозначающий, что 3d сцены нет
#define	SLD_INDEX_SCENE_NO				(-1)


/// Индекс озвучки, обозначающий, что озвучки нет
#define	SLD_INDEX_SOUND_NO				(-1)

/// Индекс стилизованного варианта написания, обозначающий, что стилизованного варианта написания нет
#define	SLD_INDEX_STYLIZED_VARIANT_NO	(-1)


/// Максимальная глубина поиска при разборе китайского слова.
#define MAX_SEARCH_DEPTH		(32)
/// Максимальное количество символов в слоге в китайском слове.
#define MAX_SEARCH_CHAR		(6)

/// Проверяет является ли символ согласным(т.е. в китайском языке слог с него начинаться не может).
#define isConsonant(ch)	(ch == 'q' || ch == 'w' || ch == 'r' || ch == 't' || ch == 'p' || ch == 's' || ch == 'd' || ch == 'f' || ch == 'g' || ch == 'h' || ch == 'k' || ch == 'l' || ch == 'z' || ch == 'x' || ch == 'c' || ch == 'v' || ch == 'b' || ch == 'n' || ch == 'm')
/// Является ли символ в слове подходящим для поиска китайской озвучки.
#define isChinSoundCh(ch)	(ch < 0x2000) 

/// Макрос проверки выравнивания указателя на данные морфологии, считанные из ресурсов
//#define MorphoTestPtr( p ) CheckPtrAlignment( p )
#define MorphoTestPtr( p )

/// Константа определяющая расстояние между точками входа в таблице быстрого доступа.
/// Нужна для оценки расстояния между началом декодирования если поиск начнется с начала
#define SLD_MERGE_QA_INTERLEAVE	(50)

/// Количество бит в одной единице битового поля
#define SLD_SEARCH_RESULT_BUFFER_SIZE		(32)

/// Количество бит необходимых для кодирования смещения бита в одной единице битового поля
#define SLD_SEARCH_RESULT_BUFFER_SHIFT		(5)

/// Максимальный размер префикса для стиля в символах (UInt16)
#define SLD_MAX_STYLE_PREFIX_SIZE			(16)
/// Максимальный размер постфикса для стиля в символах (UInt16)
#define SLD_MAX_STYLE_POSTFIX_SIZE			(16)

/// Индекс варианта стиля для запроса варианта по умолчанию
#define SLD_DEFAULT_STYLE_VARIANT_INDEX		(-1)

/// Неопределенный текущий индекс списка слов (до начала декодирования)
#define SLD_DEFAULT_LIST_INDEX		(-1)

/// Неопределенный текущий индекс слова в списке
#define	SLD_DEFAULT_WORD_INDEX		(-1)

/// Неопределенный индекс варианта написания
#define	SLD_DEFAULT_VARIANT_INDEX	(-1)

/// Неопределенный индекс словаря
#define	SLD_DEFAULT_DICTIONARY_INDEX (-1)

/// Маска, которая отвечает за признак наличия в таблице сравнения таблицы пар соответствий символов верхнего и нижнего регистров
#define SLD_CMP_TABLE_FEATURE_SYMBOL_PAIR_TABLE	(0x1)

/// Максимальная длина строки со значением параметра
#define SLD_META_PARAM_MAX_VAL_SIZE (255)

/// Минимальное количество слов, необходимое для добавления дерева быстрого поиска
#define SLD_MIN_WORDS_COUNT_FOR_QUICK_SEARCH	(512)

/// Типы таблиц пар соответствий символов верхнего и нижнего регистров в таблице сравнения
enum ESymbolPairTableTypeEnum
{
	/// Таблица пар соответствий символов верхнего и нижнего регистров алфавита конкретного языка
	eSymbolPairTableType_Native = 0,
	/// Таблица всех возможных пар соответствий символов верхнего и нижнего регистров
	eSymbolPairTableType_Common,
	/// Таблица пар соответствий символов верхнего и нижнего регистров для символов, встречающихся в списках слов словаря
	eSymbolPairTableType_Dictionary
};

/// Перечисление, описывающее возможные виды выравнивания текста по горизонтали
enum ESldTextAlignEnum
{
	/// Выравнивание по левому краю (по умолчанию)
	eTextAlign_Left = 0,
	/// Выравнивание по центру
	eTextAlign_Center,
	/// Выравнивание по правому краю
	eTextAlign_Right,
	/// Выравнивание по ширине
	eTextAlign_Justify,
	/// Неизвестный вид выравнивания
	eTextAlign_Unknown = 0xFFFF
};

/// Перечисление, описывающее возможные виды выравнивания текста по вертикали
enum ESldVerticalTextAlignEnum
{
	/// Выравнивание по центру (по умолчанию)
	eVerticalTextAlign_Center = 0,
	/// Выравнивание по верхней границе
	eVerticalTextAlign_Top,
	/// Выравнивание по нижней границе
	eVerticalTextAlign_Bottom,
	/// Неизвестный вид выравнивания
	eVerticalTextAlign_Unknown = 0xFFFF
};

/// Перечисление, описывающее возможные направления написания текста по горизонтали
enum ESldTextDirectionEnum
{
	/// Направление написания текста слева направо (по умолчанию)
	eTextDirection_LTR = 0,
	/// Направление написания текста справа налево (арабский язык и иврит)
	eTextDirection_RTL,
	/// Неизвестный вид направления написания текста
	eTextDirection_Unknown = 0xFFFF
};

/// Перечисление, описывающее возможные типы поиска похожих слов
enum EFuzzySearchMode
{
	/// Обычный поиск без всяких оптимизаций:
	/// не проводится предварительная проверка наличия одних и тех же символов в сравниваемых словах, используется точное сравнение символов
	eFuzzy_WithoutOptimization = 0,
	/// Поиск с оптимизацией, используется точное сравнение символов
	eFuzzy_CompareDirect,
	/// Поиск с оптимизацией, используется сравнение символов на основе таблицы сортировки
	eFuzzy_CompareSortTable
};

/// Перечисление, описывающее возможные типы грамматических тестов
enum ESldTestTypeEnum
{
	/// Тест, в котором нужно напечатать ответ (в свободной форме)
	eTestType_FreeAnswer = 0,
	/// Тест, в котором нужно выбрать один или несколько вариантов ответов
	eTestType_MultipleChoice,
	/// Тест, в котором нужно заполнить пропущенные слова из предложенных вариантов или самостоятельно вписать ответ
	eTestType_CloseDeletion,
	/// Тест, в котором нужно расположить слова в правильном порядке
	eTestType_Reordering,
	/// Тест, в котором нужно сопоставить элементы в правильном порядке
	eTestType_Mapping,
	/// Тест, в котором необходимо выделить часть текста
	eTestType_Highlighting,
	/// Тест, в котором ведется распознавание рукописного ввода
	eTestType_Handwriting,
	/// Кроссворд
	eTestType_Crossword,
	/// тест с выбором области на изображении в качестве ответа
	eTestType_Area,
	/// тест с выбором нескольких областей на изображении в качестве ответа
	eTestType_MultipleArea,
	/// тест на рисование
	eTestType_Drawing,
	/// Неизвестный тип теста
	eTestType_Unknown = 0xFFFF
};

/// Перечисление, описывающее возможные типы ответа пользователя в грамматических тестах
enum ESldTestInputTypeEnum
{
	/// Написать свой вариант ответа
	eTestInputType_Text = 0,
	/// Выбрать один или несколько правильных ответов из предложенных
	eTestInputType_Checkbox,
	/// Выбрать только один правильный ответ из предложенных
	eTestInputType_Radio,
	/// Указать файл для загрузки
	eTestInputType_File,
	/// Указать ссылку
	eTestInputType_Link,
	/// Указать число
	eTestInputType_Numeric,
	/// Написать одно слово
	eTestInputType_OneWord,
	/// Выбрать ответ слайдером
	eTestInputType_Slider
};

/// Перечисление форматов картинок
enum EPictureFormatType
{
	/// PNG
	ePictureFormatType_PNG = 0,
	/// JPG
	ePictureFormatType_JPG,
	/// SVG
	ePictureFormatType_SVG,
	/// GIF
	ePictureFormatType_GIF,
	/// Unknown
	ePictureFormatType_UNKNOWN = 0xFFFF
};

/// Перечисление единиц измерения размеров
enum EMetadataUnitType
{
	/// Pixels
	eMetadataUnitType_px = 0,
	/// Pt
	eMetadataUnitType_pt,
	/// Em
	eMetadataUnitType_em,
	/// Mm
	eMetadataUnitType_mm,
	/// Проценты
	eMetadataUnitType_percent,
	/// Последнее значащее значение, нужно для определения размера енама 
	eMetadataUnitType_end,

	/// Default
	eMetadataUnitType_UNKNOWN = 0xFFFF
};

/////////////////////////////////////////////////////////////////////////////////////
// Константы сортировки результатов полнотекстового поиска
/////////////////////////////////////////////////////////////////////////////////////

/// Количество совпадающих слов равных искомым
#define SLD_FTS_GRADE_EQUAL		(50*1000000)
/// Минимальное расстояние между словами
#define SLD_FTS_GRADE_DISTANCE	(1000000)
/// Количество базовых форм равных искомым
#define SLD_FTS_GRADE_EQUAL_MORPHO_BASE		(1000000)
/// Количество словоформ равных искомым
#define SLD_FTS_GRADE_EQUAL_MORPHO_FORM		(1200000)
/// Расстояние до ближайшего к началу слову
#define SLD_FTS_GRADE_START		(10000)
/// Насколько большое различие по количеству слов
#define SLD_FTS_GRADE_DIFFERENCE_WORDS	(100)
/// Насколько большое различие по длине
#define SLD_FTS_GRADE_DIFFERENCE_LENGTH	(1)

///
#define  SLD_FTS_NUMBER_OF_WORDS_FOR_FULL_MODE	(256)

/// Максимальное число слов в запросе в TExpressionBox
#define		SLD_DEFAULT_NUMBER_OF_REQUEST_WORD	(512)

/// У слова в словарном списке есть точное соответсвие
#define		SLD_SIMPLE_SORTED_NORMAL_WORD		(1)		
/// У слова в словарном списке нет точного соответсвия
#define		SLD_SIMPLE_SORTED_VIRTUAL_WORD		(-1)		

/// Перечисление, описывающее возможные типы сортировки результатов полнотекстового поиска.
enum ESldFTSSortingTypeEnum
{
	/// Упрощенный метод сортировки - работает быстро
	eFTSSortingType_Simple = 0,
	/// Разбивает на группы по числу параметров большему, чем в Simple, и меньшему, чем в Full, а также сортирует внутри группы по алфавиту
	eFTSSortingType_AlphabeticalSortedGroups,
	/// Полный метод сортировки - работает медленнее
	eFTSSortingType_Full,
	/// Умный - сам выбирает какой метод использовать в зависимости от количества найденых результатов (eFTSSortingType_Simple или eFTSSortingType_Full)
	eFTSSortingType_Smart,
	/// сортировка по алфавиту
	eFTSSortingType_Alphabetical
};

/// Перечисление, описывающее типы слов, получаемых из морфологии
enum ESldMorphologyWordTypeEnum
{
	/// Базовая форма слова
	eMorphologyWordTypeBase = 0,
	/// Одна из словоформ
	eMorphologyWordTypeWordform,
};

/// Перечисление, описывающее типы видеоконтента
enum ESldVideoType
{
	/// Видео, закодированное кодеком h.264
	eVideoType_H264 = 0,
	/// Видео, закодированное кодеком VP8
	eVideoType_WEBM,
	/// Неизвестный формат видео
	eVideoType_Unknown = 0xFFFF,
};

/// Перечисление, описывающее тип действия, с которым ассоциирована выделенная на изображении область
enum ESldImageAreaType
{
	/// Ссылка на другой материал
	eImageAreaType_Link = 0,
	/// Вызывается при перетаскивания другого объекта на эту область
	eImageAreaType_Drop,
	/// Используется для переключения в рамках слайдшоу
	eImageAreaType_Slide,
	/// Используется для ввода
	eImageAreaType_Text,
	/// Используется для ответа на тест
	eImageAreaType_Test,
	/// Внешняя ссылка
	eImageAreaType_Url,
	/// Popup контент
	eImageAreaType_Popup,
	/// Неизвестный тип действия
	eImageAreaType_Unknown = 0xFFFF,
};

/// Перечисление, описывающее возможные формы области
typedef enum ESldAreaShape
{
	/// Прямоугольник
	eAreaShapeRect = 0,
	/// Многоугольник
	eAreaShapePoly,

	eShapeTypeUnknown = 0xFFFF
} ESldAreaShape;

/// Cпособ обтекания текстом
enum ESldFlow
{
	/// В тексте
	eFlow_InText = 0,
	/// Вокруг рамки
	eFlow_Square,
	/// По контуру
	eFlow_Tight,
	/// Сквозное
	eFlow_Through,
	/// Сверху и снизу
	eFlow_TopBottom,
	/// Перед текстом
	eFlow_FrontText,
	/// За текстом
	eFlow_BehindText,
	/// Справа
	eFlow_Right,
	/// Слева
	eFlow_Left,
	/// Неизвестный тип
	eFlow_Unknown = 0xFFFF
};

/// Способ отображения объекта
enum ESldDisplay
{
	/// Объект отображается как встроенный (без переноса текста)
	eDisplay_Inline = 0,
	/// Объект показывается как блочный (происходит перенос строк в начале и в конце содержимого)
	eDisplay_Block,
	/// Объект отображается поверх остального контента без вмешательства в его верстку
	eDisplay_Front,
	/// Объект отображается позади остального контента без вмешательства в его верстку
	eDisplay_Behind,
	/// Неизвестный тип
	eDisplay_Unknown = 0xFFFF,
};

/// Способ выравнивания объекта
enum ESldFloat
{
	/// Выравнивания нет
	eFloat_None = 0,
	/// Выравнивание по левому краю
	eFloat_Left,
	/// Выравнивание по правому краю
	eFloat_Right,
	/// Неизвестный тип выравнивания
	eFloat_Unknown = 0xFFFF,
};

/// Cпособ отмены выравнивания объекта
enum ESldClearType
{
	/// Отменяет действие свойства clear, при этом обтекание элемента происходит, как задано с помощью свойства float или других настроек.
	eClearType_None = 0,
	/// Отменяет обтекание с левой стороны элемента. 
	eClearType_Left,
	/// Отменяет обтекание с правой стороны элемента. 
	eClearType_Right,
	/// Отменяет обтекание элемента одновременно с правого и левого края.
	/// Это значение рекомендуется устанавливать, когда требуется снять обтекание элемента, но неизвестно точно с какой стороны.
	eClearType_Both,
	/// Неизвестный тип
	eClearType_Unknown = 0xFFFF,
};

/// Cпособ расположения на странице
enum ESldPlace
{
	/// В тексте
	ePlace_InText = 0,
	/// Левый верхний угол
	ePlace_UpLeft,
	/// Вверху по центру
	ePlace_UpMiddle,
	/// Правый верхний угол
	ePlace_UpRight,
	/// Посередине слева
	ePlace_MiddleLeft,
	/// По центру
	ePlace_Centre,
	/// Посередине справа
	ePlace_MiddleRight,
	/// Левый нижний угол
	ePlace_DownLeft,
	/// Снизу по центру
	ePlace_DownMiddle,
	/// Правый нижний угол
	ePlace_DownRight,
	/// Неизвестный тип
	ePlace_Unknown = 0xFFFF
};

/// Эффект отображения на странице
enum ESldShowEffect
{
	eShowEffect_Base = 0,
	/// Неизвестный тип
	eShowEffect_Unknown = 0xFFFF
};

/// Эффект переключения между кадрами
enum ESldMoveEffect
{
	eMoveEffect_Base = 0,
	/// Неизвестный тип
	eMoveEffect_Unknown = 0xFFFF
};

/// Битовый набор свойств словаря
enum ESldFeatures
{
	eFeature_SkeletalMode = 0,
	eFeature_Taboo,
	eFeature_Stress,
    eFeature_DarkTheme,
};

/// Типы содержимого учебника
enum ESldContentType
{
	/// теория
	eDataBaseContentType_Theory = 1 << 0,
	/// практика
	eDataBaseContentType_Practice = 1 << 1,
	/// методическое пособие
	eDataBaseContentType_MethodBook = 1 << 2,
	/// задачник
	eDataBaseContentType_ProblemBook = 1 << 3,

	eDataBaseContentType_Corps = 1 << 4,
	eDataBaseContentType_Video = 1 << 5,
	eDataBaseContentType_Animation = 1 << 6,
	eDataBaseContentType_Image = 1 << 7,
	eDataBaseContentType_Sound = 1 << 8,
	eDataBaseContentType_3DScene = 1 << 9,
	eDataBaseContentType_TestSimulator = 1 << 10,
	eDataBaseContentType_TestExaminer = 1 << 11,
	eDataBaseContentType_TestPractical = 1 << 12,
	eDataBaseContentType_Slideshow = 1 << 13,
	eDataBaseContentType_Scenario = 1 << 14,
	eDataBaseContentType_Fiction = 1 << 15,

	/// База, содержащая бинарные ресурсы для баз
	eDataBaseContentType_ExternalResource = 1 << 16,

	eDataBaseContentType_Other = 1 << 20, // мелкие типы база, пока здесь только определитель
	/// неизвестный тип
	eDataBaseContentType_Wrong = 0xFFFF - 0xFFFF/2
};

/// Тим медиа данных в контейнере
enum ESldMediaType
{
	/// текст
	eMedia_Text = 0,
	/// изображение
	eMedia_Image,
	/// озвучка
	eMedia_Audio,
	/// таблица
	eMedia_Table,
	/// видео
	eMedia_Video,
	/// трехмерная сцена
	eMedia_3d,
	/// слайдшоу
	eMedia_SlideShow,
	/// горизонтальная линия
	eMedia_Line,
	/// сноски на полях
	eMedia_Marginal,
	/// карта
	eMedia_Map,
	/// дано/найти/решение
	eMedia_TaskBlock,
	/// конструктор
	eMedia_Creator,
	/// зона для рисования
	eMedia_Paint,
	/// область с интерактивными объектами
	eMedia_InteractiveArea,
	/// сноски
	eMedia_Footnote,
	/// поля учебника
	eMedia_Overlay,
	/// для блочной верстки контента (подразумевается различный тип содержимого: картинки, видео итд итп)
	eMedia_Block,
	/// Последнее значащее значение, нужно для определения размера енама 
	eMedia_Last
};

/// Тип тайлинга в медиаконтейнере
enum ESldTiling
{
	eTilingNone = 0,
	eTilingHorizontal,
	eTilingVertical,
	eTilingBoth,
	eTilingSpread,
};

/// Тип вертикального выравнивания
enum ESldAlignVertical
{
	eAlignVNone = 0,
	eAlignVTop,
	eAlignVCentre,
	eAlignVBottom,
};

/// Тип горизонтального выравнивания
enum ESldAlignHorizont
{
	eAlignHNone = 0,
	eAlignHLeft,
	eAlignHCentre,
	eAlignHRight,
};

enum ESldGradient
{
	eGradientNone = 0,
	eGradientTop,
	eGradientLeft,
	eGradientBottom,
	eGradientRight,
	eGradientTopLeft,
	eGradientTopRight,
	eGradientBottomLeft,
	eGradientBottomRight,
};

enum ESldSymbolType
{
	eCmpDelimiterCharType = 1,
	eCmpNativeCharType
};

/// Типы токенов логического выражения
/// Также описывают приоритет операций
enum ELogicalExpressionTokenTypes
{
	eTokenType_Operand = 0,
	eTokenType_Operation_AND,
	eTokenType_Operation_OR,
	eTokenType_Operation_NOT,
	eTokenType_Operation_OPEN_BR,
	eTokenType_Operation_CLOSE_BR,
	eTokenType_Unknown = 0xFF,
};

enum EExpressionBoxPreparedType
{
	eExpressionBoxNotPrepared = 0,
	eExpressionBoxPrepared,
	eExpressionBoxOnlyZeroSymbol
};

/// Базовая версия листа с точками быстрого доступа
#define VERSION_LIST_BASE					(1)

/// Версия листа с прямым доступом к словам
#define VERSION_LIST_DIRECT_ACCESS			(2)

/// Определяет, на каких уровнях каталога будет производиться поиск
enum ESldSearchRange
{
	// Уровни определяются автоматически(на случай, если выведем данную функцию в интерфейс)
	eSearchRangeDefault = 0,
	// Поиск по всему списку
	eSearchRangeFullRecursive,
	// Поиск на корневом уровне
	eSearchRangeRoot,
	// Поиск на текущем уровне
	eSearchRangeCurrentLevel,
	// Поиск на текущем уровне и всех его подуровнях
	eSearchRangeCurrentLevelRecursive,
};

/// Насыщенность
enum ESldBoldValue
{
	/// соответствует html-евскому font-weight: normal
	eBoldValueNormal = 0,
	/// html bold
	eBoldValueBold,
	/// html bolder
	eBoldValueBolder,
	/// html lighter
	eBoldValueLighter
};

// Тип подчеркивания
enum ESldUnderlineType
{
	eUnderlineMin = 0,
	eUnderlineNone = eUnderlineMin,
	eUnderlineLine = 1, 
	eUnderlineDoubleLine = 2,
	eUnderlineBoldLine,
	eUnderlineDots,
	eUnderlineBoldDots,
	eUnderlineHachureLine,
	eUnderlineBoldHachureLine,
	eUnderlineLongHachureLine,
	eUnderlineLongBoldHachureLine,
	eUnderlineHachureDotLine,
	eUnderlineBoldHachureDotLine,
	eUnderlineHachureDoubleDotLine,
	eUnderlineBoldHachureDoubleDotLine,
	eUnderlineWavyLine,
	eUnderlineBoldWavyLine,
	eUnderlineDoubleWavyLine,

	// надо соблюдать, чтобы всегда eUnderlineMax = максимальному значению
	eUnderlineMax = eUnderlineDoubleWavyLine
};

enum ESldInfoResourceIndex
{
	eInfoIndex_CommonInfo = 0,
	eInfoIndex_Annotation,
};

/// Тип карты
enum ESldMapType
{
	/// слайдшоу
	eMap_Slide = 0,
	/// слой
	eMap_Layer,
};


/// Тип контента элементарного объекта
enum ESldAtomicObjectContentType
{
	/// Текст
	eContentType_Text = 0,
	/// Изображение
	eContentType_Image,
	/// Видео
	eContentType_Video,
	/// Звук
	eContentType_Sound,
	/// Текст
	eContentType_3D,
	/// Слайдшоу
	eContentType_Slideshow,
	/// Таблица
	eContentType_Table,
	/// Тест
	eContentType_Test,
	/// Объект для рисования
	eContentType_DrawingObject,
	/// Заметка на полях
	eContentType_Sidenote,
	/// Диаграма
	eContentType_Diagram,

	/// Неизвестный тип
	eContentType_Unknown = 0xFFFF
};

/// Логический тип элементарного объекта
enum ESldLogicalType
{
	/// Задача
	eLogicalType_Problem = 0,
	/// Задание
	eLogicalType_Task,
	/// Пример
	eLogicalType_Example,
	/// Объяснение
	eLogicalType_Explanation,
	/// Фактологический материал
	eLogicalType_Fact,
	/// Дополнительный материал
	eLogicalType_Additional,

	/// Неизвестный тип
	eLogicalType_Unknown = 0xFFFF
};


/// Тип деятельности для элементарного объекта
enum ESldActivityType
{
	/// Знать
	eActivityType_Know = 0,
	/// Называть
	eActivityType_Name,
	/// Составлять
	eActivityType_DrawUp,
	/// Объяснять
	eActivityType_Explain,
	/// Характеризовать
	eActivityType_Characterise,
	/// Определять
	eActivityType_Determine,
	/// Классифицировать
	eActivityType_Classify,
	/// Отображать
	eActivityType_Display,
	/// Проводить эксперимент
	eActivityType_ConductExperiment,
	/// Вычислять
	eActivityType_Calculate,
	/// Понимать
	eActivityType_Understand,
	/// Моделировать
	eActivityType_Simulate,
	/// Анализировать
	eActivityType_Analyze,
	/// Сравнивать и различать
	eActivityType_CompareDiffer,
	/// Описывать
	eActivityType_Describe,
	/// Планировать
	eActivityType_Plan,
	/// Сопоставлять
	eActivityType_Collate,
	/// Доказывать
	eActivityType_Prove,
	/// Искать информацию
	eActivityType_SearchInformation,
	/// Делать устные и письменные сообщения
	eActivityType_Report,
	/// Обобщать
	eActivityType_Generalize,
	/// Переводить
	eActivityType_Translate,
	/// Применять знания
	eActivityType_ApplyKnowledge,
	/// Решать задачи
	eActivityType_Solve,
	/// Создавать тексты
	eActivityType_Compose,
	/// Вести диалог
	eActivityType_Dialog,
	/// Читать
	eActivityType_Read,
	/// Писать
	eActivityType_Write,
	/// Говорение
	eActivityType_Talk,
	/// Аудировать
	eActivityType_Listen,
	/// Интерпретировать
	eActivityType_Interpret,
	/// Оценивать
	eActivityType_Evaluate,
	/// Определять тип
	eActivityType_DefineType,
	/// Формулировать
	eActivityType_Formulate,
	/// Аргументировать и обосновывать
	eActivityType_ArgueJustify,
	/// Исследовать
	eActivityType_Explore,
	/// Измерять
	eActivityType_Measure,
	/// Выполнять учебные и творческие задачи
	eActivityType_PerfromTasks,
	/// Проектировать
	eActivityType_Design,
	/// Показывать на карте
	eActivityType_ShowOnMap,

	/// неизвестный тип
	eActivityType_Unknown = 0xFFFF
};

enum ESldShadowType
{
	eShadowNone = 0,
	eShadowTopBottom,
	eShadowLast
};

enum ESldContainerExtansion
{
	eExtansionNone = 0,
	eExtansionFull,
	eExtansionLeft,
	eExtansionRight,
};

/// Перечисление, описывающее возможные типы грамматических тестов
enum ESldTestModeEnum
{
	/// Тест, экзаменационный
	eTestModeExam = 0,
	/// Тест, проверочная
	eTestModeTesting,
	/// Лабораторная работа
	eTestModePractical,
};

/// Перечисление, описывающее возможные типы вывода вариантов ответов
enum ESldTestOrderEnum
{
	/// Значение по умолчанию
	eTestOrderDefault = 0,
	/// Фиксированный порядок
	eTestOrderFixed = eTestOrderDefault,
	/// Случайный порядок
	eTestOrderRandom,
};

/// Перечисление, описывающее варианты отображения в демо-версии
enum ESldTestDemoEnum
{
	/// Значение по умолчанию
	eTestDemoDefault = 0,
	/// Показывать в демо-версии
	eTestDemoShow,
	/// Не показывать в демо-версии
	eTestDemoHide,
};

/// Перечисление, описывающее возможную сложность теста
enum ESldTestDifficultyEnum
{
	/// Значение по умолчанию
	ESldTestDifficultyDefault = 0,

	/// Базовый тест
	ESldTestDifficulty1,
	/// Профильный тест
	ESldTestDifficulty2,
	/// Тест повышенной сложности
	ESldTestDifficulty3,
};

/// тесты с выделением текста
enum ESldTextControl
{
	/// свободное выделение
	eTextControlFree = 0,
	/// выделение слова
	eTextControlWord,
	/// выделение предложения
	eTextControlSentence,
	/// последний
	eTextControlLast = eTextControlSentence,
};

/// варианты в блоке дано/найти/решение
enum ESldTaskBlockType
{
	/// Дано
	eTaskBlockGiven = 0,
	/// Найти
	eTaskBlockFind,
	/// Решение
	eTaskBlockSolvation,
	/// Доказать
	eTaskBlockProve,
	/// Доказательство
	eTaskBlockProof,
	/// Анализ
	eTaskBlockAnalysis,
	/// Дополнительно
	eTaskBlockAdditional
};

// вариаты типов контента
enum ESldContentTypeVariant
{
	eContentTypeVariant3d,
	eContentTypeVariantAnimation,
	eContentTypeVariantBiography,
	eContentTypeVariantVideo,
	eContentTypeVariantDeterminator,
	eContentTypeVariantDiagram,
	eContentTypeVariantSlideshow,
	eContentTypeVariantTable,
	eContentTypeVariantMapLayer,
	eContentTypeVariantMapSlide,
	eContentTypeVariantTest,
	eContentTypeVariantDictionary,
	eContentTypeVariantLinkBlock,
	eContentTypeVariantInteresting,
	eContentTypeVariantChrestomathy,
	eContentTypeVariantPainting,
	eContentTypeVariantPhoto
};

/// элементы легенды
enum ESldLegendItemType
{
	eLegendItemTypeDefault = 0,
	/// Легенда для заливки
	eLegendItemTypeFill,
	/// Легенда для линий (стрелок)
	eLegendItemTypeLine,
	/// Легенда иконок
	eLegendItemTypeIcon,
};

/// Тип подписи (под изображением)
enum ESldCaptionType
{
	/// Собственно подпись
	eCaptionTypeCaption = 0,
	/// Индекс изображения (например, 13.1)
	eCaptionTypeIndex,
	/// Пояснитeльный текст
	eCaptionTypeExplanation,

	eCaptionTypeUnknown = 0xFFFF
};

/// Тип контента, который может содержаться во внешних базах
enum ESldExternContentType
{
	/// Изображения
	eExternContentTypeImage = 0,
	/// Видео
	eExternContentTypeVideo,
	/// Звук
	eExternContentTypeSound,
	/// 3D
	eExternContentTypeScene,
	/// Контент, на который ведет ссылка
	eExternContentTypeLink,
	/// Контент, который вставляется через extern_article
	eExternContentTypeExternArticle,
	
	// Добавляем выше
	eExternContentTypeMax
};

/// Форма интерактивного объекта
enum ESldInteractiveObjectShapeEnum
{
	/// Прямоугольник
	eInteractiveObjectShape_Rect = 0,
	/// Круг
	eInteractiveObjectShape_Circle,
};

/// Расположение ленты в ленте времени
enum ESldTimeLinePosition
{
	/// Над виджетом
	eTimeLineAbove = 0,
	/// Под
	eTimeLineBelow,
	/// Слева
	eTimeLineLeft,
	/// Справа
	eTimeLineRight
};

/// Тип метки (label)
enum ESldLabelType
{
	/// Обычная метка
	eLabelNormal = 0,
	/// Метка страницы
	eLabelPage,
	/// Метка ПП
	eLabelFullTextSearch,
};

/// Форматы формул
enum ESldFormulaType
{
	eFormulaMathMl = 0,
	eFormulaAsciiMathMl
};

/// Типы Callback-ов, передаваемых в функцию WordFound()
enum ESldWordFoundCallbackType
{
	/// Начало поиска
	eWordFoundCallbackStartSearch = 0,
	/// Callback, возвращаемый через каждые SLD_SEARCH_CALLBACK_INTERLEAVE слов
	eWordFoundCallbackInterleave,
	/// Слово найдено
	eWordFoundCallbackFound,
	/// Мы превысили требуемое количество результатов поиска
	eWordFoundCallbackOverflow,
	/// Конец поиска
	eWordFoundCallbackStopSearch,
	/// Начало мультипоиска поиска (в процессе может вызываться несколько поисковых функций)
	eWordFoundCallbackStartMultiSearch,
	/// Конец мультипоиска поиска
	eWordFoundCallbackStopMultiSearch
};

/// Значение по умолчанию, для максимального раскрытия всех подуровней иерархии
#define SLD_UNCOVER_HIERARHCHY_MAXIMUM_DEPTH (MAX_UINT_VALUE)

/// Состояние записи в пользовательском списке слов
enum ESubwordsState
{
	/// Запись не имеет связанных с ней слов
	eSubwordsStateHasNotSubwords = 0,
	/// Связанные слова не отображаются в списке
	eSubwordsStateCollapsed,
	/// Связанные слова отображаются в списке
	eSubwordsStateUncovered
};

/// Тип уровня иерархии
enum EHierarchyLevelType
{
	/// Обычная папка, при инициализации списка закрыта, при переходе отображает отдельный список с ее содержимым
	eLevelTypeNormal = 0,
	/// Закрытая при инициализации папка, при переходе содержимое папки добавляется в основной список
	eLevelTypeCovered,
	/// Открытая при инициализации папка, при переходе содержимое папки удаляется из основного списка
	eLevelTypeUncovered,
	/// Открытая при инициализации папка, никаких действий с папкой не предусмотрено
	eLevelTypeStaticUncovered
};

/// Стиль границы вокруг элемента
enum EBorderStyle
{
	/// None
	eBorderStyleNone = 0,
	/// Hidden
	eBorderStyleHidden,
	/// Dotted
	eBorderStyleDotted,
	/// Dashed
	eBorderStyleDashed,
	/// Solid
	eBorderStyleSolid,
	/// Double
	eBorderStyleDouble,
	/// Groove
	eBorderStyleGroove,
	/// Ridge
	eBorderStyleRidge,
	/// Inset
	eBorderStyleInset,
	/// Outset
	eBorderStyleOutset,
	/// Unknown
	eBorderStyleUnknown = 0xFFFF
};

/// Уровни образования
enum EEducationalLevel
{
	/// дошкольное образование
	eEducationalLevelPreschoolEducation = (1 << 0),
	/// начальное общее образование
	eEducationalLevelPrimaryEducation = (1 << 1),
	/// основное общее
	eEducationalLevelBasicEducation = (1 << 2),
	/// среднее (полное) общее
	eEducationalLevelSecondaryEducation = (1 << 3),
	/// начальное профессиональное
	eEducationalLevelTechnicalSchoolFirstCycle = (1 << 4),
	/// среднее профессиональное
	eEducationalLevelTechnicalSchoolSecondCycle = (1 << 5),
	/// высшее профессиональное
	eEducationalLevelHigherEducation = (1 << 6),
	/// послевузовское профессиональное
	eEducationalLevelUniversityPostgraduate = (1 << 7),
	/// профессиональная подготовка
	eEducationalLevelVocationalTraining = (1 << 8),
	/// дополнительное образование
	eEducationalLevelAdditionalEducation = (1 << 9),

	/// Unknown
	eEducationalLevelUnknown = 0
};

/// Перечисление, описывающее возможные бренды словарной базы
enum EDictionaryBrandName
{
	/// Slovoed
	eBrand_SlovoEd = 0,
	/// Merriam-Webster
	eBrand_Merriam_Webster,
	/// Oxford
	eBrand_Oxford,
	/// Duden
	eBrand_Duden,
	/// PONS
	eBrand_PONS,
	/// VOX
	eBrand_VOX,
	/// Van Dale
	eBrand_Van_Dale,
	/// AL-MAWRID
	eBrand_AL_MAWRID,
	/// Harrap
	eBrand_Harrap,
	/// AKADEMIAI KIADO
	eBrand_AKADEMIAI_KIADO,
	/// MultiLex
	eBrand_MultiLex,
	/// Berlitz
	eBrand_Berlitz,
	/// Langenscheidt
	eBrand_Langenscheidt,
	/// Britannica
	eBrand_Britannica,
	/// Mondadori
	eBrand_Mondadori,
	/// Slovari XXI veka
	eBrand_Slovari_XXI_veka,
	/// Enciclopedia Catalana
	eBrand_Enciclopedia_Catalana,
	/// Collins
	eBrand_Collins,
	/// WAHRIG
	eBrand_WAHRIG,
	/// Wat&Hoe
	eBrand_Wat_N_Hoe,
	/// Le Robert
	eBrand_Le_Robert,
	/// AUP PONS
	eBrand_AUP_PONS,
	/// Independent Publishing
	eBrand_Independent_Publishing,
	/// Chambers (Hodder)
	eBrand_Chambers,
	/// Barron's
	eBrand_Barrons,
	/// Cambridge
	eBrand_Cambridge,
	/// Librairie Orientale
	eBrand_Librairie_Orientale,
	/// Hoepli
	eBrand_Hoepli,
	/// Drofa
	eBrand_Drofa,
	/// RedHouse
	eBrand_Red_House,
	/// Living Language
	eBrand_Living_Language,
	/// PASSWORD
	eBrand_PASSWORD,
	/// Richmond
	eBrand_Richmond,
	/// Magnus
	eBrand_Magnus,
	/// Ecovit Kiado
	eBrand_Ecovit_Kiado,
	/// Priroda
	eBrand_Priroda,
	/// Operator's Dictionary
	eBrand_Operators_Dictionary,
	/// Lexikon 2K (Peter Bondesson)
	eBrand_Lexikon_2K,
	/// Lexicology Centre (G. BABINIOTIS)
	eBrand_Lexicology_Centre,
	/// Туровер
	eBrand_Turover,
	/// Международные отношения
	eBrand_International_Relations,
	/// MYJMK
	eBrand_MYJMK,
	/// TransLegal
	eBrand_TransLegal,
	/// Focalbeo
	eBrand_Focalbeo,
	/// Insight Guides
	eBrand_Insight_Guides,
	/// Editura Litera
	eBrand_Editura_Litera,
	/// ЛЭУ
	eBrand_EtbLab,
	/// ЭЛОКОНТ
	eBrand_ELOKONT,
	/// Просвещение
	eBrand_Prosveschenie,
	/// Городской Методический Центр
	eBrand_Gorodskoy_Metodologicheskiy_Centr,
	/// Вентана - Граф
	eBrand_Ventana_Graf,
	/// Астрель
	eBrand_Astrel,
	/// Кузнецов(словенский)
	eBrand_Kuznetsov,
	/// The Kosciuszko Foundation
	eBrand_The_Kosciuszko_Foundation,
	/// Druid Multimedia
	eBrand_Druid,
	/// Academia International
	eBrand_AcademiaInternational,
	/// Zanichelli Editore
	eBrand_Zanichelli_Editore,

	// общее количество брендов и последний "значащий"
	eBrandName_Count,
	eBrandName_Last = eBrandName_Count - 1,

	/// Неизвестный тип
	eBrand_Unknown = 0xFFFF
};

/// Перечисление, описывающее, каким образом вычислялось смещение до начала слова внутри статьи в списке слов полнотекстового поиска
enum EFullTextSearchShiftType
{
	/// Смещение не вычислялось
	eShiftType_None = 0,
	/// Смещение является количеством символов от начала статьи до начала слова
	eShiftType_SymbolsFromArticleBegin,
	/// Неизвестный тип
	eShiftType_Unknown = 0xFFFF
};

enum ESldAbstractResourceType
{
	/// Лаборатория алгебры
	eAbstractResourceAlgebraLab = 0,
	/// Лаборатория геометрии
	eAbstractResourceGeometryLab,
	/// Карты
	eAbstractResourceMapLab,

	/// ???
	eAbstractResourceHtmlSite,
	/// ???
	eAbstractResourceFlash,

	/// Лаборатория стереометрии
	eAbstractResourceStereometryLab,
	/// Лаборатория физики
	eAbstractResourcePhysicsLab
};

/// Направления кроссворда
enum ESldCrosswordItemDirection
{
	/// По горизонтали
	eCrosswordItemDirectionHorizontal = 0,
	/// По вертикали
	eCrosswordItemDirectionVertical,

	eCrosswordItemDirectionUnknown = -1
};


/// Тип списка
enum ESldListType
{
	/// Неупорядоченный
	eListTypeUnordered = 0,
	/// Упорядоченный
	eListTypeOrdered,
	/// Упорядоченный в обратном порядке
	eListTypeOrderedReversed
};

/// Перечисление, описывающее возможные подтипы тестов на сопоставление
enum ESldMappingTestTypeEnum
{
	// Тип по умолчанию (деревья)
	eMappingTestTypeDefault = 0,
	// Простой тип (прямое соответстие)
	eMappingTestTypeSimple,
	// Связывание
	eMappingTestTypeConnection,

	/// Неизвестный тип теста
	eTestMappingTypeUnknown = 0xFFFF,
};

/// Тип получаемых морфоформ
enum EMorphoFormsType
{
	/// Все морфоформы
	eMorphoAll = 0,
	/// Только базовые формы
	eMorphoBase,
	/// Только производные формы
	eMorphoDerivative
};

/// Версия speex-контейнера, в заголовок которого зашивается условное качество звука
#define SPEEX_FORMAT_VERSION_QUALITY					(1)
/// Версия speex-контейнера, в заголовок которого зашивается размер пакета декодированных данных
#define SPEEX_FORMAT_VERSION_PACKET_SIZE				(2)
/// В период с 10.11.10 по 20.04.11 вместо версии зашивалась версия ядра, по функционалу не отличается от SPEEX_FORMAT_VERSION_QUALITY
#define SPEEX_FORMAT_VERSION_OLD						(100)

/// Сигнатура элемента истории (для точного определения границ элемента, вне зависимости от разрядности системы)
#define HISTORY_ELEMENT_SIGNATURE						('TSIH')

/// Селектор для Emoji-символов по умолчанию (для отображения стандартного юникодного символа)
#define SLD_DEFAULT_EMOJI_SELECTOR						(0xFE0E)

/// Тип Emoji-символов
enum EEmojiTypes
{
	/// Символы, которые мы считаем Emoji
	eSlovoedEmoji = 0,
	/// Стандартные Emoji-символы
	eFullEmoji,
};

enum EHereditaryListType
{
	eHereditaryListTypeNormal = 0,
	eHereditaryListTypeSearch,
	eHereditaryListTypeCustom,
	eHereditaryListTypeMerged
};

enum EDictionaryForSearchResultType
{
	/// Список с результатами поиска
	eDictionaryForSearchResultNormal = 0,
	/// Список исправленных (опечатки, поиск по шаблону) запросов
	eDictionaryForSearchResultRequest,
	/// Пустой список
	eDictionaryForSearchResultNone
};

/// Режимы свайпа
enum ESwipingMode
{
	/// Свайп на любой элемент в пределах данного уровня вложенности
	ePassingAnySwipe = 0,
	/// Свайп на элемент статейного типа (пропуская элементы нестатейного типа) в пределах данного уровня вложенности
	eSkipingCatalogSwipe,
	/// Сквозной свайп на элемент статейного типа (пропуская элементы нестатейного типа)
	eAcrossingCatalogSwipe,
	/// Свайп на элемент статейного типа (блокируясь на элементах нестатейного типа) в пределах данного уровня вложенности
	eInterruptingOnCatalogSwipe,
};

#endif
