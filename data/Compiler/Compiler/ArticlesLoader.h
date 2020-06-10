#pragma once

#include <string>

#include "PugiXML/pugixml.hpp"

#include "AbstractItemContainer.h"
#include "ImageContainer.h"
#include "SceneContainer.h"
#include "SoundContainer.h"
#include "VideoContainer.h"
#include "MyProcessTimer.h"

class CArticle;
class CArticles;
class CSSDataManager;
class CWordList;
class MetadataManager;
class TSlovoedStyleManager;

// Загрузчик статей
class ArticlesLoader
{
public:
	ArticlesLoader(CArticles &aArticles, MetadataManager &aMetadata, CSSDataManager &aCSS, TSlovoedStyleManager &aStyles, CWordList &aAtomicList)
		: m_articles(aArticles), m_metadata(aMetadata), m_css(aCSS), m_styles(aStyles), m_atomicList(aAtomicList), m_error(0)
	{
		m_timer.SetFilename("XmlParsingTimeArticles.time");
	}

	/// Производит загрузку статей из файла
	int Load(const std::wstring &aFileName);

	/// Производит загрузку статей из xml-дерева
	int Load(pugi::xml_node aNode);

	/// Возвращает ссылку на контейнер картинок
	CImageContainer& GetImageContainer() { return m_imageContainer; }

	/// Возвращает ссылку на контейнер озвучек
	CSoundContainer& GetSoundContainer() { return m_soundContainer; }

	/// Возвращает ссылку на контейнер видео
	CVideoContainer& GetVideoContainer() { return m_videoContainer; }

	/// Возвращает ссылку на контейнер сцен
	CSceneContainer& GetSceneContainer() { return m_sceneContainer; }

	/// Возвращает ссылку на контейнер абстрактных объектов
	CAbstractItemContainer& GetAbstractItemContainer() { return m_abstractItemContainer; }

	/// Возвращает код ошибки
	int GetError() const { return m_error; }

	struct ParserContext;

private:

	/// Загружает одну статью из ветви XML-файла и добавляет ее в список статей
	int LoadArticle(pugi::xml_node aNode, ParserContext &aCtx);

private:

	/// Ссылка на загружаемый объект со статьями
	CArticles &m_articles;

	/// Ссылки на объекты необходимые для загрузки статей
	MetadataManager &m_metadata;
	CSSDataManager &m_css;
	TSlovoedStyleManager &m_styles;
	CWordList &m_atomicList;

	/// Контейнер картинок
	CImageContainer m_imageContainer;

	/// Контейнер озвучек
	CSoundContainer m_soundContainer;

	/// Контейнер видео
	CVideoContainer m_videoContainer;

	/// Контейнер сцен
	CSceneContainer m_sceneContainer;

	/// Контейнер абстрактных объектов
	CAbstractItemContainer m_abstractItemContainer;

	/// Для замеров времени работы различных этапов
	CMyProcessTimer m_timer;

	/// Код ошибки для отложенного завершения программы
	int m_error;
};
