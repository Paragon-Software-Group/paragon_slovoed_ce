#pragma once

#include <string>
#include <vector>
#include <map>

#include "sld2_Error.h"
#include "sld_Types.h"
#include "MetadataManager.h" // for MetadataRef
#include "CSSDataManager.h" // mostly for InvalidCSSBlockIndex
#include "util.h" // for RangeImpl

using namespace std;

/// Структура описывающая один блок в статье
struct BlockType
{
	enum {
		/// list_entry_id для ссылок
		ListEntryId = 0,
		/// ссылка для флеш карточек
		FlashCardListEntryIdFront,
		FlashCardListEntryIdBack,
		/// список слов для слайдшоу
		SlideShowList,
		/// имя элементарного объекта
		AtomicObjectId,
		/// идентификатор статьи для ссылки напрямую на article
		ArticleId,
		/// маркеры списков
		ListMarkers,

		TextType_COUNT
	};

	struct Metadata
	{
		friend struct BlockType;
	public:
		/// "Ссылка" на метаданные
		MetadataRef ref;

		/// индекс css block'a
		uint32_t cssBlockIndex;

	private:
		/// строковые данные блока
		std::vector<std::wstring> stringData;

		/// индексы строковых данных блока
		UInt8 stringIndexes[TextType_COUNT];

		UInt8 textIndex(int aTextType) const
		{
			assert(aTextType < TextType_COUNT);
			return stringIndexes[aTextType];
		}

	public:
		Metadata() : cssBlockIndex(InvalidCSSBlockIndex) {
			memset(stringIndexes, 0xff, sizeof(stringIndexes));
		}
	};

public:

	// тип метаданных блока
	ESldStyleMetaTypeEnum metaType;

	/// Номер стиля
	UInt16 styleNumber;

	/// Флаг того, что блок является закрывающим метатегом (table, tr, td ...)
	bool isClosing;

	/// Текст
	std::wstring text;

	/// Указатель на метаданные открывающего блока
	// XXX: make private?
	Metadata *metadata_;

	/// Добавляет текст определенного типа
	void AddText(int aTextType, sld::wstring_ref aText, bool aClear = false);

	/// Возвращает длину текста определенного типа
	size_t TextLength(int aTextType) const
	{
		if (sld2_unlikely(!metadata_)) return 0;
		const UInt8 index = metadata_->textIndex(aTextType);
		return index != 0xFF ? metadata_->stringData[index].length() : 0;
	}

	/// Возвращает содержимое буфера определенного типа
	const std::wstring& GetText(int aTextType) const
	{
		static const std::wstring dummy;
		if (sld2_unlikely(!metadata_)) return dummy;
		const UInt8 index = metadata_->textIndex(aTextType);
		return index != 0xFF ? metadata_->stringData[index] : dummy;
	}

	/// Возвращает "ссылку" на метаданные
	MetadataRef metadata() const { return metadata_ ? metadata_->ref : MetadataRef(); }

	/// Возвращает индекс CSS блока
	UInt32 cssBlockIndex() const { return metadata_ ? metadata_->cssBlockIndex : InvalidCSSBlockIndex; }

	/// Конструктор
	BlockType(ESldStyleMetaTypeEnum aType, UInt16 aStyleNum, std::wstring&& aText, bool aIsClosing) :
		metaType(aType), styleNumber(aStyleNum), isClosing(aIsClosing), text(std::move(aText)), metadata_(nullptr)
	{}
};

/// Класс хранящий информацию о статье
class CArticle
{
	friend class ArticlesLoader;
public:

	/// Конструктор
	CArticle() {}

	// non-copyable
	CArticle(const CArticle&) = delete;
	CArticle& operator=(const CArticle&) = delete;

	/// Устанавливаем идентификатор статьи.
	void setID(const std::wstring &text);
	/// Возвращает идентификатор статьи
	const wstring& GetID() const { return m_id; };

	/// Возвращает количество блоков в статье
	UInt32 GetBlockCount(void) const;
	
	/// Возвращает блок данных по указанному номеру
	BlockType* GetBlock(UInt32 aNumber);

	struct PageIndex
	{
		uint32_t pageNum;
		wstring labelId;
	};
	const vector<PageIndex>& GetPages() const { return m_Pages; }

	bool removeBlock(const BlockType *aBlock);

public:

	/// Map соответствия тематик блоков switch к количеству состояний в них (для проверок)
	static std::map<UInt32, UInt32> m_ThematicToStatesNumberMap;

private:

	/// Идентификатор статьи по которому будет происходить привязка из списка слов.
	wstring				m_id;

	/// Блоки с обработанными данными
	vector<BlockType*>	m_blocks;

	/// Список страниц, размеченных в статье
	vector<PageIndex>		m_Pages;

public:

	auto blocks() -> sld::DereferencingRange<decltype(m_blocks.begin())>
	{
		return{ m_blocks.begin(), m_blocks.end() };
	}

	auto blocks() const -> sld::DereferencingRange<decltype(m_blocks.begin())>
	{
		return{ m_blocks.begin(), m_blocks.end() };
	}
};
