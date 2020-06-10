#ifndef _SLD_CUSTOM_LIST_CONTROL_H_
#define _SLD_CUSTOM_LIST_CONTROL_H_

#include "ISldList.h"

// forward decl
class CSldCustomList;

/// Описывает возможные результаты выполнения функции UncoverHierarchy()
enum EUncoverFolderResult
{
	/// Со списком ничего не произошло
	eFolderNoActions = 0,
	/// Размер списка увеличился, за счет "раскрытия папки"
	eFolderUncover,
	/// Размер списка уменьшился, за счет "закрытия папки"
	eFolderCollapse,
	/// Размер списка увеличился, за счет добавления связанных со словом записей
	eFolderUncoverSubwords,
	/// Размер списка уменьшился, за счет удаления связанных со словом записей
	eFolderCollapseeSubwords
};

/// Класс для управления пользовательскими списками
class CSldCustomListControl
{
public:
	/// Конструктор
	CSldCustomListControl() : m_CustomListIndex(-1) {}

	/// Очистка
	void Clear() { m_WordLists.resize(0); m_CustomListIndex = -1; }

	/// Освобождение памяти
	void Close();

	/// Инициализация
	ESldError Init(ISldList** aWordLists, const Int32 aListIndex, const UInt32 aNumberOfLists);

	/// Проверяет, проинициализирован ли контрол
	UInt32 IsInit() { return !m_WordLists.empty(); }

	/// Добавляет слово в список
	ESldError AddWord(const Int32 aRealListIndex, const Int32 aRealWordIndex, const UInt32 aUncoverHierarchy = 0);
	ESldError AddWord(const Int32 aRealListIndex = SLD_DEFAULT_LIST_INDEX);

	/// Привязывает слово к записи из списка
	ESldError AddSubWord(const Int32 aWordIndex, const Int32 aSubWordIndex, const Int32 aSubWordListIndex = SLD_DEFAULT_LIST_INDEX);

	/// Удаляет записи из списка
	ESldError RemoveWords(const Int32 aBeginWordIndex, const Int32 aEndWordIndex = SLD_DEFAULT_WORD_INDEX);

	/// Добавляет вариант написания
	ESldError AddVariantProperty(const TListVariantProperty & aVariantProperty, bool aUnique = false);

	/// Устанавливает вариант написания для уже добавленного слова
	ESldError SetCurrentWord(const Int32 aWordIndex, const UInt16* aText, const Int32 aVariantIndex = 0);
	
	/// Раскрывает/скрывает следующий уровень иерархии
	ESldError UncoverHierarchy(const Int32 aWordIndex, EUncoverFolderResult* aResult);

	/// Устанавливает индекс базового списка, в котором нужно проводить поиск
	ESldError SetSearchSourceListIndex(const Int32 aListIndex);

	/// Возвращает количество связанных со словом записей
	ESldError GetSubWordsCount(const Int32 aWordIndex, UInt32* aSubWordsCount);

	/// Проверяет состояние слова и связанных с ним слов
	ESldError CheckSubwordsState(const Int32 aWordIndex, ESubwordsState* aWordState);

	/// Индексирует все слова, содержащиеся в списке в соответсвии с заданным вариантом написания
	ESldError SortListByVariant(const Int32 aVariantIndex);

	/// Объединяет все записи для вспомогательных листов, относящиеся к одному заголовку
	ESldError GroupWordsByHeadwords();

private:
	// Возвращает текщий кастомный лист
	CSldCustomList* customList() { return (CSldCustomList*)m_WordLists[m_CustomListIndex]; }

	/// Списки слов
	sld2::DynArray<ISldList*>	m_WordLists;

	/// Индекс кастомного списка
	Int32					m_CustomListIndex;
};

#endif