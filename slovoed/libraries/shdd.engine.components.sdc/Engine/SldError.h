#ifndef _SLD_ERROR_H_
#define _SLD_ERROR_H_

/// код ошибки
typedef enum
{
	/// Нет ошибок
	eOK = 0,

	/// Ошибки памяти
	eMemoryErrorBase = 0x0100,
	/// Недостаточно памяти
	eMemoryNotEnoughMemory,
	/// Получен нулевой указатель
	eMemoryNullPointer,
	/// Извне получен указатель на входные данные, который сам указывает на внутренние структуры ядра (сейчас ситуация такова, что так делать нельзя)
	eMemoryBadInputPointer,
	/// Запрос на выделение памяти нулевого размера
	eMemoryZeroSizeMemoryRequest,

	/// Ошибки доступа к ресурсам
	eResourceErrorBase = 0x0200,
	/// Ошибка открытия контейнера
	eResourceCantOpenContainer,
	/// Ошибка получения ресурса
	eResourceCantGetResource,
	/// Ошибка закрытия ресурса
	eResourceCantCloseResource,
	/// Ошибка в CRC-файла
	eResourceWrongCRC,
	/// Ошибка при инициализации вложенной базы морфологии
	eResourceCantInitMorphology,

	/// Ошибки входных данных
	eInputErrorBase = 0x0300,
	/// Неверный размер структуры:
	/// размер, записанный внутри, отличается от фактического согласно размеру ресурса
	eInputWrongStructSize,


	/// Общие ошибки ядра
	eCommonErrorBase = 0x0400,
	/// Неправильный индекс передан как параметр
	eCommonWrongIndex,
	/// Ошибка с выбором списка слов(не выбран, выбран не подходящий для данного действия, номер выбранного не подходит)
	eCommonWrongList,
	/// Указан неподдерживаемый способ сжатия
	eCommonWrongCompressionType,
	/// Указан неверный тип ресурса для хранения иерархии
	eCommonWrongCatalogType,
	/// Данный уровень последний, дальше идти некуда
	eCommonLastLevel,
	/// Размер ресурса не соответствует расчетному размеру данных
	eCommonWrongResourceSize,
	/// Неверный тип данных в таблице быстрого доступа
	eCommonWrongQAType,
	/// Ошибка поиска в каталоге
	eCommonCatalogSearchError,
	/// Количество таблиц сравнения оказалось неправильным
	eCommonWrongCMPTableCount,
	/// Указан неправильный язык
	eCommonWrongLanguage,
	/// Если после окончания декодирования нас просят получить остатки перевода
	eCommonTranslationCompleted,
	/// Слишком большой текст передан во ввод
	eCommonTooLargeText,
	/// Не получается восстановить номер слова
	eCommonCantFindIndex,
	/// Неправильное количество языков(скорее всего, ноль)
	eCommonWrongNumberOfLanguages,
	/// Неизвестный тип текста в блоке данных
	eCommonUnknownSldStyleType,
	/// Список слов не инициализирован
	eCommonListNotInitialized,
	/// Неправильное смещение начала слова при декодировании
	eCommonWrongShiftSize,
	/// Неправильный номер символа при декодировании по методу подсчета символов
	eCommonWrongCharIndex,
	/// Неправильный номер символа при декодировании по методу подсчета символов
	eCommonWrongInitSearchList,
	/// Неправильное количество списков
	eCommonWrongListCount,
	/// Неправильное количество данных
	eCommonWrongSizeOfData,
	/// Неподдерживаемый тип источника медиаданных (картинок, озвучки и т.д.)
	eCommonWrongMediaSourceType,
	/// Ошибка полнотекстового поиска
	eCommonFullTextSearchError,
	/// Не поддерживаемый полнотекстовым поиском набор свойств вариантов написания в списках слов
	eCommonFullTextSearchWrongVariants,
	/// Версия словарной базы не поддерживается ядром (слишком старая версия словарной базы)
	eCommonTooOldDictionaryVersion,
	/// Версия словарной базы не поддерживается ядром (слишком старая версия ядра для открытия словарной базы)
	eCommonTooHighDictionaryVersion,
	/// Если мы пытаемся что-то переводить в словаре, в котором вообще нет переводов
	eCommonDictionaryHasNoTranslations,
	/// Неизвестный тип поиска похожих слов
	eCommonUnknownFuzzySearchMode,
	/// Ошибка вычисления логического выражения в функции поиска по шаблону
	eCommonWildCardSearchLogicalExpressionError,
	/// Ошибка вычисления логического выражения в функции полнотекстового поиска
	eCommonFullTextSearchLogicalExpressionError,
	/// Передан неверный индекс варианта написания
	eCommonWrongVariantIndex,
	/// Состояние каталога не синхронизировано, поэтому выполнение метода запрещено
	eCommonCatalogIsNotSynchronizedError,
	/// Версия таблицы сортировки слишком стара для требуемого функционала
	eCommonTooOldCompareTable,
	/// Ошибочное количество слов в списке
	eCommonWrongNumberOfWords,
	/// Ошибочное количество переводов у записи
	eCommonWrongNumberOfTranslation,

	/// Это не ошибки - это особые случаи поведения ядра
	eExceptionErrorBase = 0x0500,
	/// Говорит ядру, что декодирование перевода нужно приостановить с 
	/// возможностью в дальнейшем декодировать остатки
	eExceptionTranslationPause,
	/// Говорит ядру, что поиск слов можно прекращать
	eExceptionSearchStop,
	/// Последний код возврата для случая особого поведения ядра
	eExceptionLastException,


	/// Ошибки системы защиты(регистрация, серийный номер и т.д.)
	eSNErrorBase = 0x0600,
	/// Неправильный серийный номер
	eSNWrongSerialNumber,
	/// При попытке считывания сохраненных данных, данные для указанного DictID не найдены
	eSNDictIDNotFound,
	/// Считанные данные или разрушены, или не для того словаря
	eSNWrongLoadedData,
	/// У считанных регистрационных данных неправильный CRC
	eSNWrongDataCRC,


	/// Ошибки системы работы со звуком
	eSoundErrorBase = 0x0700,
	/// Неподдерживаемая версия звуковой базы
	eSoundWrongCodeVersion,
	/// Ядро озвучки не было проинициализированно должным образом
	eSoundNotInitiolized,
	/// Искомое слово не найдено
	eSoundWordNotFound,
	/// Неправильный индекс
	eSoundWrongIndex,
	/// Нет данных для декодирования
	eSoundNoDataForDecoding,
	/// Неправильный формат звуковых данных
	eSoundWrongDataFormat,
	/// Неправильный размер пакета в данных Speex
	eSoundWrongSpeexPacketSize,
	/// Ошибка инициализации движка Speex
	eSoundErrorSpeexInit,
	/// Данные оказались поврежденными
	eSoundSpeexDataCorrupted,
	/// Возвращается если звук не в spx
	eSoundNotSpx,
	/// Декодер не инициализирован
	eSoundDecoderUnInitialize,
	/// Данные у декодера кончились
	eSoundDecoderEnd,

	eCompareErrorBase = 0x0800,
	/// Неизвестный код языка
	eCompareUnknownLanguageCode,

	eMergeErrorBase = 0x0900,
	/// В ядре не оказалось ни одного словаря
	eMergeNoDictionariesLoaded,
	/// Порядок словарей неправильный
	eMergeWrongDictionaryOrder,
	/// Выбран не подходящий для данного действия индекс словаря
	eMergeWrongDictionaryIndex,
	/// Выбран не подходящий для данного действия индекс списка
	eMergeWrongListIndex,
	/// Выбран не подходящий для данного действия индекс слова
	eMergeWrongWordIndex,
	/// Выбран не подходящий для данного действия ID словаря
	eMergeWrongDictionaryID,
	/// Выбран не подходящий для данного действия тип списка
	eMergeWrongListType,
	/// Выбран не подходящий для данного действия индекс перевода
	eMergeWrongTranslationIndex,

	/// Ошибки в метаданных
	eMetadataErrorBase = 0x0A00,
	/// Ошибка конвертации параметра метаданных из строки в число
	eMetadataErrorToUIntConvertion,
	/// Ошибка конвертации параметра метаданных из строки в число
	eMetadataErrorToIntConvertion,
	/// Ошибка разбора параметров для картинки
	eMetadataErrorImageParsing,
	/// Ошибка разбора параметров для вариант написания StylePreferences
	eMetadataErrorStylePreferencesParsing,
	/// Ошибка конвертации размера с единицами измерения в строку
	eMetadataErrorFromSizeValueConversion,

	/// Общая ошибка невалидного контента в текстовых метаданных
	eMetadataErrorParsingTextMetadata,
	/// В базе отсутствуют структрированные метаданные (в общем случае говорит об ошибке при сборке базы)
	eMetadataErrorNoStructuredMetadata,
	/// В базе отсутствуют метаданные с заданным индексом и типом (в общем случае говорит об ошибке при сборке базы)
	eMetadataErrorInvalidStructureRef,
	/// В базе отсутcтвуют css параметры (в общем случае говорит об ошибке при сборке базы)
	eMetadataErrorNoCSSData,
	/// В базе отсутcтвует css параметр/блок параметров под заданным индексом (в общем случае говорит об ошибке при сборке базы)
	eMetadataErrorInvalidCSSIndex,

	/// Неподдерживаемая версия модели
	e3dWrongMeshVersion = 0x0B00,
	/// Неподдерживаемая версия сцены
	e3dWrongSceneVersion,

	/// Ошибка отсутствия слайдов
	eSlideShowErrorHierarchy = 0x0C00,
	
} ESldError;

#endif
