#ifndef _SLD_SDC_READ_MY_H_
#define _SLD_SDC_READ_MY_H_

#include "SldPlatform.h"
#include "SldError.h"
#include "SldTypes.h"
#include "SldDefines.h"
#include "SldDynArray.h"
#include "ISDCFile.h"

// simple_list
namespace sld2 {

// a kernel/bsd style intrusive list node
struct list_node
{
	list_node *next;
	list_node *prev;

	list_node();
};

// "initializes" the list to an "empty" state
static inline void make_empty(list_node *node) {
	node->next = node;
	node->prev = node;
}

// removes the list_node from the parent list
static inline void unlink(list_node *node) {
	node->prev->next = node->next;
	node->next->prev = node->prev;
	make_empty(node);
}

inline list_node::list_node() {
	make_empty(this);
}

// the list head (different type to enforce type safety a bit)
struct list_head : public list_node {};

} // namespace sld2

/// Класс предназначеный для чтения данных из контейнера.
class CSDCReadMy
{
	struct ResourceStruct
	{
		// refcount
		int refcnt;

		/// размер данных
		UInt32 size;
		/// тип
		UInt32 type;
		/// индекс среди ресурсов такого же типа
		UInt32 index;
		/// данные ресурса
		void *data;

		// backref to the reader
		CSDCReadMy &reader;
		// list handling
		sld2::list_node link;

		ResourceStruct(CSDCReadMy &reader_);
		~ResourceStruct();

		void clear();

		void ref() { refcnt++; }
		ResourceStruct* unref()
		{
			refcnt--;
			if (refcnt > 0)
				return this;
			reader.CloseResource(this);
			return nullptr;
		}
	};

public:
	/// Класс представляет из себя "хэндл" загруженого ресурса
	class Resource
	{
	public:
		Resource() : ptr_(nullptr) {}
		Resource(const Resource &h) : ptr_(h.ptr_) { ref(); }
		Resource(Resource&& h) : ptr_(h.ptr_) { h.ptr_ = nullptr; }

		~Resource() { unref(); }

		Resource& operator=(const Resource &h)
		{
			unref();
			ptr_ = h.ptr_;
			ref();
			return *this;
		}

		Resource& operator=(Resource&& h)
		{
			unref();
			ptr_ = h.ptr_;
			h.ptr_ = nullptr;
			return *this;
		}

		// checks if this handle is "pointing" to a valid opened resource
		bool empty() const { return ptr_ == nullptr; }

		// returns a pointer to the resource data
		const UInt8* ptr() const { return empty() ? nullptr : (const UInt8*)ptr_->data; }

		// returns the size of the resource data
		UInt32 size() const { return empty() ? 0 : ptr_->size; }

		// returns resource type
		UInt32 type() const { return empty() ? 0 : ptr_->type; }

		// returns resource index (among the resources of the same type)
		UInt32 index() const { return empty() ? 0 : ptr_->index; }

		// returns resource data in the form of a span
		sld2::Span<const UInt8> data() const {
			return empty() ? nullptr : sld2::make_span((const UInt8*)ptr_->data, ptr_->size);
		}

		// helper to easily see if we have a resource loaded
		explicit operator bool() const { return !empty(); }

	protected:
		explicit Resource(ResourceStruct *ptr) : ptr_(ptr) {}

		void ref() { if (ptr_) ptr_->ref(); }
		void unref() { if (ptr_) ptr_ = ptr_->unref(); }

	private:
		ResourceStruct *ptr_;
	};

	/// Класс представляет из себя "хэндл" *только что* загруженого ресурса
	/// По сути Resource + статус загрузки (если eOK - значит ресурс загружен успешно)
	class ResourceHandle : protected Resource
	{
		friend class CSDCReadMy;
	public:
		ResourceHandle() : error_(eMemoryNullPointer) {}

		// returns the underlying loading status code
		ESldError error() const { return error_; }

		// helper to see if we successfuly loaded the resource
		explicit operator bool() const { return error_ == eOK; }

		// returns the underlying resource handle
		const Resource& resource() const { return *this; }

		// checks if this handle is "pointing" to a successfully loaded resource
		bool empty() const { return error_ != eOK || Resource::empty(); }

		// accessors to the underlying resource
		using Resource::ptr;
		using Resource::size;
		using Resource::type;
		using Resource::index;
		using Resource::data;

	private:
		ResourceHandle(ESldError error) : error_(error) {}
		explicit ResourceHandle(ResourceStruct *ptr) : Resource(ptr), error_(eOK) {}

		ESldError error_;
	};

public:
	/// Конструктор
	CSDCReadMy(void);

	/// Деструктор
	~CSDCReadMy(void);

	/// Открываем контейнер
	ESldError Open(ISDCFile *aFile);

	/// Закрываем контейнер.
	void Close();

	/// Получаем ресурс по его типу и номеру
	ResourceHandle GetResource(UInt32 aResType, UInt32 aResIndex);

	/// Получаем данные ресурса по его типу и номеру без выделения памяти
	ESldError GetResourceData(void* aData, UInt32 aResType, UInt32 aResIndex, UInt32 *aDataSize);

	/// Получаем данные ресурса по его типу и номеру без выделения памяти
	ESldError GetResourceData(void* aData, UInt32 aResType, UInt32 aResIndex, UInt32 aDataSize)
	{
		return GetResourceData(aData, aResType, aResIndex, &aDataSize);
	}

	/// Получает сдвиг от начала файла до ресурса с заданным типом и номером
	ESldError GetResourceShiftAndSize(UInt32 *aShift, UInt32 *aSize, UInt32 aResType, UInt32 aResIndex) const;

	/// Возвращает свойство базы по заданному ключу
	bool GetPropertyByKey(const UInt16* aKey, UInt16** aValue);
	/// Возвращает количество дополнительных свойств базы
	UInt32 GetNumberOfProperty() const;
	/// Возвращает ключ и значение свойства базы по индексу свойства
	SDCError GetPropertyByIndex(UInt32 aPropertyIndex, UInt16** aKey, UInt16** aValue);

	/// Возвращает CRC файла для заданного хидера и выходного потока
	static SDCError GetFileCRC(const SlovoEdContainerHeader *aHeader, ISDCFile* aFileData, UInt32* aFileCRC);

	/// Возвращает тип содержимого в контейнере
	UInt32 GetDatabaseType(void) const;

	/// Проверяет полная это база или нет
	UInt32 IsInApp(void) const { return m_Header.IsInApp; }

	/// Проверяем целостность контейнера.
	SDCError CheckData(void);

	/// Получает указатель на текущий файл контейнера
	ISDCFile* GetFileData();

private:

	/// Возвращает количество ресурсов в открытом контейнере.
	UInt32 GetNumberOfResources() const;

	/// Получает индекс ресурса в таблице расположения ресурсов по его типу и номеру
	UInt32 GetResourceIndexInTable(UInt32 aResType, UInt32 aResIndex) const;

	/// Освобождает данные ресурса если его рефкаунт падает до 0
	void CloseResource(ResourceStruct *aResource);

	static inline ResourceStruct* to_resource(sld2::list_node *node) {
		return sld2_container_of(node, ResourceStruct, &ResourceStruct::link);
	}

private:

	/// Открытый файл контейнера
	ISDCFile								*m_FileData;

	/// Заголовок контейнера
	SlovoEdContainerHeader					m_Header;

	/// Таблица расположения ресурсов
	SlovoEdContainerResourcePosition*		m_resTable;

	/// Список загружнных активных (используемых) ресурсов
	sld2::list_head							m_loadedResources;

	/// Список неактивных ресурсов
	sld2::list_head							m_freeList;

	/// Буфер для хранения текущего свойства
	TBaseProperty*							m_Property;

	/// Кэш для чтения запакованых ресурсов
	sld2::DynArray<UInt8>					m_compressedData;
};

// ResourceHandle equality comparsion operators to simplify error checking
inline bool operator==(const CSDCReadMy::ResourceHandle &res, ESldError error) { return res.error() == error; }
inline bool operator!=(const CSDCReadMy::ResourceHandle &res, ESldError error) { return res.error() != error; }

#endif
