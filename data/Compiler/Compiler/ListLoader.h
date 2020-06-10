#pragma once

#include "WordList.h"

#include "SoundContainer.h"
#include "ImageContainer.h"
#include "SceneContainer.h"
#include "VideoContainer.h"

#include "PugiXML/pugixml.hpp"

/// Класс который производит считывание каталога/списка слов и конвертацию его в более удобный вид
class CListLoader
{
	using VariantsMap = std::map<UInt32, ListVariantProperty>;

public:
	CListLoader() {}

	// Производит загрузку параметров из xml узла файла проекта
	int Load(pugi::xml_node aNode, UInt32 aDictLangCodeFrom);

	/// Добавляет элемент в каталог/список слов по куску XML-файла
	// TODO: make private
	int LoadListElement(pugi::xml_node aNode, UInt32 aDepth);

	/// Возвращает xml-узел с форматированными заголовками
	pugi::xml_node GetStylizedVariantsNode() { return m_StylizedVariants.root(); }

	/// Возвращает ссылку на контейнер картинок
	const CImageContainer& GetImageContainer() const { return m_ImageContainer; }

	/// Возвращает ссылку на контейнер 3d сцен
	const CSceneContainer& GetSceneContainer() const { return m_SceneContainer; }

	/// Возвращает ссылку на контейнер озвучки
	const CSoundContainer& GetSoundContainer() const { return m_SoundContainer; }

	/// Возвращает ссылку на контейнер видео
	const CVideoContainer& GetVideoContainer() const { return m_VideoContainer; }

	/// Доступ к списку загружаемому данным объектом
	CWordList* operator->() { return &m_list; }
	const CWordList* operator->() const { return &m_list; }

	/// Возвращает загруженный список
	CWordList& list() { return m_list; }

private:

	/// Устанавливает тип назначения списка.
	int ParseType(const std::wstring &aType);

	/// Устанавливает уровни каталога, на которых будет производиться поиск
	int ParseSearchRange(const std::wstring &aString);

	/// Устанавливает тип алфавита в списке
	int ParseAlphabetType(const std::wstring &aString);

	/// Склеивает все текстовое содержимое xml-узла в одну строку
	static std::wstring GetNotFormattedNodeValue(pugi::xml_node aNode);

	/// Добавляет блок свойств списка слов полнотекстового поиска по куску XML-файла
	int LoadFullTextSearchOptions(pugi::xml_node aNode, bool aFTSList);

	/// Добавляем блок локализованных строк для списка слов
	int LoadLocalizedListStrings(pugi::xml_node aNode);

	/// Добавляем свойства варианта написания из XML-файла
	int LoadVariantProperty(pugi::xml_node aNode, VariantsMap &aVariants, unsigned long aNumber = ULONG_MAX);

	/// Добавляет свойства вариантов напсиания из XML-файла
	int LoadVariantProperties(pugi::xml_node aNode, VariantsMap &aVariants);

	/// Функция загрузки файла со списком
	int LoadFile(const std::wstring& aFileName);

	/// Добавить новый уровень.
	int LoadLevel(pugi::xml_node aNode, UInt32 aDepth);

	/// Добавить статью - это верхний общий уровнь.
	int LoadArticle(pugi::xml_node aNode, UInt32 aDepth);

private:

	/// xml-узел с форматированными заголовками
	struct TStylizedVariants {
		pugi::xml_document doc;

		TStylizedVariants() { doc.load(L"<Articles/>"); }
		TStylizedVariants(const TStylizedVariants &rhs) { doc.append_copy(rhs.doc.root().first_child()); }
		pugi::xml_node root() { return doc.root(); }
	} m_StylizedVariants;

	/// Контейнер картинок
	CImageContainer m_ImageContainer;

	/// Контейнер сцен
	CSceneContainer m_SceneContainer;

	/// Контейнер озвучек
	CSoundContainer m_SoundContainer;

	/// Контейнер видео
	CVideoContainer m_VideoContainer;

	CWordList m_list;
};
