#pragma once

#include "VersionInfo.h"
#include "sld_Types.h"
#include "sld2_Error.h"
#include "Articles.h"
#include "Styles.h"
#include "Log.h"
#include "WordLists.h"
#include "ImageManager.h"
#include "VideoManager.h"
#include "SoundManager.h"
#include "SceneManager.h"
#include "MeshManager.h"
#include "AbstractItemManager.h"
#include "MaterialManager.h"
#include "XmlTagContent.h"
#include "SortTablesManager.h"
#include "MetadataManager.h"
#include "MorphoDataManager.h"
#include "CSSDataManager.h"

#include <vector>
#include <wchar.h>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>

#include <stdio.h>
#include <list>
#include <sstream>
#include <set>
#include <map>
#include <iterator>

using namespace std;

class ArticlesLoader;
class CListLoader;

/**
	Класс хранящий информацию по проекту словаря.
	Проект словаря загружается из файла.
*/
class SlovoEdProject
{
public:

	/// Конструктор
	SlovoEdProject(void);
	/// Деструктор
	~SlovoEdProject(void);

	/// Загружаем файл проекта
	int Load(wstring aFileName);
	
	/// Возвращает имя файла для чтения/сохранения информации о версии словарной базы
	wstring GetVersionInfoFilename(void);
	/// Загружает информацию о версии словарной базы из отдельного файла
	int LoadVersionInfo(const wstring &aFileName);
	/// Сохраняет информацию о версии словарной базы в отдельный файл
	int SaveVersionInfo(const wstring &aFileName);
	/// Возвращает указатель на структуру с информацией о версии словарной базы
	const TDictionaryVersionInfo* GetDictionaryVersionInfo(void);

	/// Возвращает количество загруженых списков слов.
	UInt32 GetNumberOfLists() const;

	///	Возвращает объект списка слов по его номеру среди прочих загруженых списков слов
	CWordList* GetWordList(UInt32 index);
	const CWordList* GetWordList(UInt32 index) const;

	/// Добавляет новый лист в словарь
	int AddWordList(CWordList&& aList) { return m_WordLists.AddList(std::move(aList)); }

	/// Возвращает указатель на объект с статьями
	CArticles *GetArticles(void);

	/// Возвращает указатель на объект для работы с таблицами сортировки
	CSortTablesManager* GetSortTablesManager(void);

	/// Возвращает указатель на объект для работы с базами морфологии
	CMorphoDataManager* GetMorphoDataManager(void);

	/// Возвращает указатель на менеджер метаданных
	MetadataManager* GetMetadataManager();

	/// Возвращает указатель на менеджер css метаданных
	CSSDataManager* GetCSSDataManager();

	/// Возвращает структуру заголовка словаря.
	const TDictionaryHeader *GetDictionaryHeader();

	/// Возвращает структуру с дополнительной информацией учебника
	const TAdditionalInfo *GetAdditionalInfo();

	/// Возвращает аннотацию к учебнику
	wstring GetAnnotation() const;
	
	/// Возвращает количество наборов локализованных строк (названий)
	UInt32 LocalizedNamesCount(void) const;
	/// Возвращает указатель на набор локализованных строк (названий) по номеру
	TLocalizedNames* GetLocalizedNames(UInt32 aIndex);

	/// Возвращает количество записей с локализованными строчками
	UInt32 GetStringsCount();

	/// Возвращает указатель на локализованные строчки по номеру
	const TLocalizedNames * GetStrings(UInt32 aIndex);

	/// Возвращает менеджер стилей
	TSlovoedStyleManager * GetStyles(void);

	/// Возвращает менеджер картинок
	CImageManager* GetImageManager(void);

	/// Возвращает менеджер видео
	CVideoManager* GetVideoManager(void);
	
	/// Возвращает менеджер озвучек
	CSoundManager* GetSoundManager(void);

	/// Возвращает менеджер сцены
	CSceneManager* GetSceneManager(void);

	/// Возвращает менеджер моделей
	CMeshManager* GetMeshManager(void);

	/// Возвращает менеджер материалов
	CMaterialManager* GetMaterialManager(void);

	/// Возвращает менеджер материалов
	CAbstractItemManager* GetAbstractItemManager(void);

	/// Устанавливает максимальный размер типа перевода
	int SetMaxWordtypeSize(UInt32 aMaxWordtypeSize);

	/// Устанавливает максимальный размер строчки в переводе
	int SetMaxBufferSize(UInt32 aMaxBufferSize);
	
	/// Устанавливает тип содержимого проекта
	void SetDatabaseType(ESlovoEdContainerDatabaseTypeEnum aDatabaseType);
	/// Получает тип содержимого проекта
	ESlovoEdContainerDatabaseTypeEnum GetDatabaseType(void);

	/// Проверяет, относится ли база к ЛЭУ (Учебник, книга, ...)
	bool IsETBDatabase();
	
	/// Устанавливает тип источника озвучки
	void SetSoundSourceType(EMediaSourceTypeEnum aType);
	/// Получает тип источника озвучки
	EMediaSourceTypeEnum GetSoundSourceType(void);
	
	/// Устанавливает тип источника картинок
	void SetImageSourceType(EMediaSourceTypeEnum aType);
	/// Получает тип источника картинок
	EMediaSourceTypeEnum GetImageSourceType(void);

	/// Устанавливает тип источника видео
	void SetVideoSourceType(EMediaSourceTypeEnum aType);
	/// Получает тип источника видео
	EMediaSourceTypeEnum GetVideoSourceType(void);

	/// Устанавливает тип источника сцен
	void SetSceneSourceType(EMediaSourceTypeEnum aType);
	/// Получает тип источника сцен
	EMediaSourceTypeEnum GetSceneSourceType(void);

	/// Устанавливает тип источника материалов
	void SetMaterialSourceType(EMediaSourceTypeEnum aType);
	/// Получает тип источника материалов
	EMediaSourceTypeEnum GetMaterialSourceType(void);

	/// Устанавливает тип источника материалов
	void SetMeshSourceType(EMediaSourceTypeEnum aType);
	/// Получает тип источника материалов
	EMediaSourceTypeEnum GetMeshSourceType(void);
	
	/// Устанавливает метод сжатия, одинаковый для статей и всех списков слов
	int SetCompressionMethod(wstring aMethod);
	CompressionConfig GetCompressionMethod() const { return m_compressionConfig; }

	/// Устанавливает флаг того, что это демо-база
	void SetInApp(UInt32 aInAppFlag) { m_IsInApp = aInAppFlag; }
	/// Проверяет полная это база или нет
	UInt32 GetInApp(void);

	/// Возвращает массив имен внешних html-файлов
	map<wstring, wstring>& GetHtmlSourceNames() { return m_ListEntryId_HtmlSourceName; }

	/// Возвращает java script файл для статей
	const std::wstring& GetJavaScriptFileContents() const { return m_javaScriptFileContents; }

private:
	
	/// Делает постобработку загруженного проекта
	int PostProcessProject(ArticlesLoader &aArticlesLoader);

	// Posprocesses a list before adding it
	int PostProcessList(CListLoader &aList, std::vector<CWordList> &aSimpleSortedLists, ArticlesLoader &aArticlesLoader);

	/// Постобработка метаданных
	int PostProcessMetadata(ArticlesLoader &aArticlesLoader);

	/// Постобработка метаданных в листах
	int PostProcessListsResources();

	/// Постобработка таблиц сортировки
	int PostProcessSortTables(void);

	/// Постобоработка и проверка стилей
	int PostProcessStyles();

	/// Создает и возвращает структуру дополнительной информации
	TAdditionalInfo *AdditionalInfo();

	/// Подготавливает информацию о страницах
	int DoPrepagePages();

public:

	/// ключ - имя КЭСа
	static map<wstring, UInt32> m_ExternContent[eExternContentTypeMax];
private:


	/// Добавляем локализованные строки
	int AddStrings(pugi::xml_node aNode);
	/// Добавляем статьи
	int AddArticles(pugi::xml_node aNode, const wstring& aRoot, ArticlesLoader &aArticlesLoader);
	/// Добавляем список слов
	int AddList(pugi::xml_node aNode, std::vector<CWordList> &aSimpleSortedLists, ArticlesLoader &aArticlesLoader);
	/// Добавляем описание стиля
	int AddStyle(pugi::xml_node aNode, CImageContainer &aImages);
	/// Устанавливаем данные варианта стиля
	int SetStyleVariantData(TSlovoedStyleVariant* aStyleVariant, pugi::xml_node aNode, CImageContainer &aImages);
	/// Добавляем файл с таблицей сортировки
	int AddSortFile(pugi::xml_node aNode, wstring aRoot);
	/// Добавляем файл с вспомогательной таблицей сортировки
	int AddSecondarySortFile(pugi::xml_node aNode, const wstring& aRoot);
	/// Добавляем файл с базой морфологии
	int AddMorphoBase(pugi::xml_node aNode);

	/// Добавляем свойства словаря
	int AddDictionaryFeatures(pugi::xml_node aNode);
	/// Устанавливает произвольное свойство словаря
	int SetFeature(ESldFeatures aFeature);
	/// Устанавливает произвольный тип содержимого
	int SetContentType(const ESldContentType aType);
	/// Устанавливает свойство словаря SkeletalMode
	int SetSkeletalMode(UInt32 aValue);
	/// Устанавливает свойство словаря Taboo
	int SetTaboo(UInt32 aValue);
	/// Устанавливает свойство словаря Stress
	int SetStress(UInt32 aValue);
	/// Устанавливает свойство словаря DarkTheme
	int SetDarkTheme(UInt32 aValue);

	/// Возвращает минимальную версию ядра, которая нужна для поддержки всех фич словаря
	UInt32 GetDictionaryVersion();

	/// Обработка листов, слинкованных с Atomic-ами
	int ProcessAtomicReferenceLists(CWordList &aAtomicList);
	int DoPreparePages();

	static vector<wstring> initKeyWords();
	
private:

	/// список ключевых слов
	static const vector<wstring> KeyWords;

	/// Таблица локализованных названий
	vector<TLocalizedNames> m_names;

	/// Заголовок словаря с основными данными
	TDictionaryHeader m_header;

	/// Структура с дополнительной информацией по базе
	TAdditionalInfo *m_pAdditionalInfo;

	/// Аннотация к базе
	wstring m_Annotation;
	
	/// Информация о версии словарной базы
	TDictionaryVersionInfo m_VersionInfo;

	/// Массив статей которые вообще есть в словаре.
	CArticles m_Articles;
	
	/// Объект для управления и доступа ко всем стилям словаря
	TSlovoedStyleManager m_Styles;
	
	/// Объект для управления всеми картинками словаря
	CImageManager m_ImageManager;

	/// Объект для управления всеми видео словаря
	CVideoManager m_VideoManager;
	
	/// Объект для управления всеми озвучками словаря
	CSoundManager m_SoundManager;

	/// Объект для управления всеми сценами словаря
	CSceneManager m_SceneManager;

	/// Объект для управления всеми моделями словаря
	CMeshManager m_MeshManager;

	/// Объект для управления всеми абстрактными объектами словаря
	CAbstractItemManager m_AbstractItemManager;

	/// Объект для управления всеми 3d материалами словаря
	CMaterialManager m_MaterialManager;

	/// Списки слов
	CWordListManager m_WordLists;

	/// Объект для работы с таблицами сортировки
	CSortTablesManager m_SortTablesManager;

	/// Тип содержимого проекта
	ESlovoEdContainerDatabaseTypeEnum m_DatabaseType;
	
	/// Тип источника озвучки
	EMediaSourceTypeEnum m_SoundSourceType;
	
	/// Тип источника картинок
	EMediaSourceTypeEnum m_ImageSourceType;

	/// Тип источника видео
	EMediaSourceTypeEnum m_VideoSourceType;

	/// Тип источника трехмерных сцен
	EMediaSourceTypeEnum m_SceneSourceType;

	/// Тип источника моделей
	EMediaSourceTypeEnum m_MeshSourceType;

	/// Тип источника материалов
	EMediaSourceTypeEnum m_MaterialSourceType;

	/// Объект для работы с базами морфологии
	CMorphoDataManager m_MorphoDataManager;

	// Флаг того, что в базе есть switch-блоки с тематиками
	UInt32 m_IsHasSwitchThematics;

	/// Флаг того, что это демо-база
	UInt32				m_IsInApp;

	/// Массив имен внешних html-файлов
	map<wstring, wstring> m_ListEntryId_HtmlSourceName;

	/// Менеджер метаданных
	MetadataManager m_MetadataManager;

	/// Переопределенный метод сжатия
	// Так же используется как дефолтный метод сжатия для "ресурсов" без явно заданных настроек
	CompressionConfig m_compressionConfig;

	/// Менеджер css метаданных
	CSSDataManager m_CSSDataManager;

	/// Флаг того, что база мультиязычная (код языка списка не должен совпадать с кодами языка базы)
	bool			m_IsMultiLanguageBase;

	/// Содержимое java scrupt файла
	std::wstring	m_javaScriptFileContents;

public:

	auto lists()       -> decltype(m_WordLists.lists()) { return m_WordLists.lists(); }
	auto lists() const -> decltype(m_WordLists.lists()) { return m_WordLists.lists(); }

	auto articles()       -> decltype(m_Articles.articles()) { return m_Articles.articles(); }
	auto articles() const -> decltype(m_Articles.articles()) { return m_Articles.articles(); }

	auto styles()       -> decltype(m_Styles.styles()) { return m_Styles.styles(); }
	auto styles() const -> decltype(m_Styles.styles()) { return m_Styles.styles(); }
};
