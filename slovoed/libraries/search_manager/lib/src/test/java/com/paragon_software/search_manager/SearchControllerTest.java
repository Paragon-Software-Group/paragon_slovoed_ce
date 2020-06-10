package com.paragon_software.search_manager;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.mock.MockCollectionView;
import com.paragon_software.mock.MockForController_ErrorListener;
import com.paragon_software.mock.MockForController_FontSizeChangeListener;
import com.paragon_software.mock.MockListener;
import com.paragon_software.mock.MockScreenOpener;
import com.paragon_software.mock.MockSearchEngine;
import com.paragon_software.native_engine.GroupHeader;
import com.paragon_software.native_engine.SortType;
import com.paragon_software.screen_opener_api.ScreenType;
import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.toolbar_manager.MockToolbarManager;
import com.paragon_software.toolbar_manager.ToolbarManager;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.junit.MockitoJUnitRunner;

import static com.paragon_software.mock.StaticConsts.DELTA;
import static com.paragon_software.native_engine.SearchType.SEARCH_TYPE_DID_YOU_MEAN;
import static com.paragon_software.native_engine.SearchType.SEARCH_TYPE_FTS;
import static com.paragon_software.native_engine.SearchType.SEARCH_TYPE_WILD_CARD;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Answers.CALLS_REAL_METHODS;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

@RunWith( MockitoJUnitRunner.class )
public class SearchControllerTest
{
  private MockSearchManager searchManager;

  private MockScreenOpener screenOpenerMock;

  private MockToolbarManager toolbarManager;

  private ParagonSearchController paragonSearchController;

  @Before
  public void setUp()
  {
    searchManager = Mockito.spy(MockSearchManager.class);
    screenOpenerMock  = Mockito.spy(MockScreenOpener.class);
    toolbarManager  = Mockito.spy(MockToolbarManager.class);
    paragonSearchController = new ParagonSearchController(searchManager, screenOpenerMock, null, toolbarManager);
  }

  @Test
  public void testSetGetEntryListFontSize()
  {
    assertEquals(paragonSearchController.getEntryListFontSize(), ApplicationSettings.getDefaultFontSize(), DELTA);
    float initialEntryListFontSize = paragonSearchController.getEntryListFontSize();
    float newEntryListFontSize = initialEntryListFontSize;
    if (initialEntryListFontSize + 1 < ApplicationSettings.getMaxFontSize())
    {
      newEntryListFontSize = initialEntryListFontSize + 1;
    }
    else if (initialEntryListFontSize - 1 > ApplicationSettings.getMinFontSize())
    {
      newEntryListFontSize = initialEntryListFontSize - 1;
    }
    if (newEntryListFontSize != initialEntryListFontSize)
    {
      MockForController_FontSizeChangeListener entryListFontSizeChangeListener
              = Mockito.mock(MockForController_FontSizeChangeListener.class,
              Mockito.withSettings().useConstructor(paragonSearchController, 1).defaultAnswer(CALLS_REAL_METHODS));
      paragonSearchController.registerNotifier(entryListFontSizeChangeListener);
      entryListFontSizeChangeListener.setExpectedEntryListFontSize(newEntryListFontSize);
      paragonSearchController.setEntryListFontSize(newEntryListFontSize);
      assertEquals(paragonSearchController.getEntryListFontSize(), newEntryListFontSize, DELTA);
      assertEquals(entryListFontSizeChangeListener.getNumberOfCalls(), 1);
      paragonSearchController.unRegisterNotifier(entryListFontSizeChangeListener);
      paragonSearchController.setEntryListFontSize(initialEntryListFontSize);
      assertEquals(paragonSearchController.getEntryListFontSize(), initialEntryListFontSize, DELTA);
    }
  }

  @Test
  public void testOnErrorListener()
  {
    MockForController_ErrorListener controllerErrorListener
            = Mockito.mock(MockForController_ErrorListener.class, Mockito.withSettings()
            .useConstructor(1).defaultAnswer(CALLS_REAL_METHODS));
    paragonSearchController.registerNotifier(controllerErrorListener);
    ManagerInitException managerInitException = new ManagerInitException("ManagerInitException");
    controllerErrorListener.setExpectedException(managerInitException);
    paragonSearchController.onError(managerInitException);
    assertEquals(controllerErrorListener.getNumberOfCalls(), 1);
  }

  @Test
  public void checkRegisterOnDictionarySelectListener()
  {
    MockListener listener = Mockito.spy(MockListener.class);
    assertNull(listener.getSelectedDictionary());

    paragonSearchController.registerNotifier(listener);
    assertNull(listener.getSelectedDictionary());

    Dictionary.DictionaryId dictionaryId_1 = new Dictionary.DictionaryId("dict_id_1");
    Dictionary.DictionaryId dictionaryId_2 = new Dictionary.DictionaryId("dict_id_2");
    Dictionary.DictionaryId dictionaryId_3 = new Dictionary.DictionaryId("dict_id_3");

    paragonSearchController.setSelectedDictionary(dictionaryId_1);
    assertEquals(listener.getSelectedDictionary(),dictionaryId_1);

    paragonSearchController.setSelectedDictionary(dictionaryId_2);
    assertEquals(listener.getSelectedDictionary(),dictionaryId_2);

    paragonSearchController.unRegisterNotifier(listener);
    paragonSearchController.setSelectedDictionary(dictionaryId_3);
    assertEquals(listener.getSelectedDictionary(),dictionaryId_2);
  }

  @Test
  public void checkRegisterOnDirectionSelectListener()
  {
    MockListener listener = Mockito.spy(MockListener.class);
    assertEquals(listener.getSelectedDirection(),-1);

    paragonSearchController.registerNotifier(listener);
    assertEquals(listener.getSelectedDirection(),-1);

    int direction_1 = 1;
    int direction_2 = 2;

    paragonSearchController.setSelectedDirection(direction_1);
    assertEquals(listener.getSelectedDirection(), direction_1);

    paragonSearchController.setSelectedDirection(direction_2);
    assertEquals(listener.getSelectedDirection(), direction_2);

    paragonSearchController.unRegisterNotifier(listener);
    paragonSearchController.setSelectedDirection(direction_1);
    assertEquals(listener.getSelectedDirection(), direction_2);
  }

  @Test
  public void checkOnDictionaryListChangedListener()
  {
    MockListener listener = Mockito.spy(MockListener.class);
    assertEquals(listener.getDictionaryListChangedCount(),0);

    paragonSearchController.registerNotifier(listener);
    assertEquals(listener.getDictionaryListChangedCount(),0);

    paragonSearchController.dictionaryListChanged();
    assertEquals(listener.getDictionaryListChangedCount(),1);

    paragonSearchController.dictionaryListChanged();
    assertEquals(listener.getDictionaryListChangedCount(),2);

    paragonSearchController.unRegisterNotifier(listener);
    paragonSearchController.dictionaryListChanged();
    assertEquals(listener.getDictionaryListChangedCount(),2);

  }

  @Test
  public void checkSetSelectedSearch()
  {
    paragonSearchController.setSelectedSearch(SEARCH_TYPE_FTS);
    assertEquals(paragonSearchController.getSelectedSearch(),SEARCH_TYPE_FTS);

    paragonSearchController.setSelectedSearch(SEARCH_TYPE_WILD_CARD);
    assertEquals(paragonSearchController.getSelectedSearch(),SEARCH_TYPE_WILD_CARD);

    paragonSearchController.setSelectedSearch(SEARCH_TYPE_DID_YOU_MEAN);
    assertEquals(paragonSearchController.getSelectedSearch(),SEARCH_TYPE_DID_YOU_MEAN);

    paragonSearchController.setSelectedSearch(null);
  }

  @Test
  public void checkSetToolbarFtsMode()
  {
    assertTrue(toolbarManager.mShowAllDictionaries);

    paragonSearchController.setSelectedSearch(null);
    assertFalse(toolbarManager.mShowAllDictionaries);

    paragonSearchController.setSelectedSearch(SEARCH_TYPE_WILD_CARD);
    assertTrue(toolbarManager.mShowAllDictionaries);

    paragonSearchController.setSelectedSearch(SEARCH_TYPE_FTS);
    assertTrue(toolbarManager.mShowAllDictionaries);
  }

  @Test
  public void checkSrcoll()
  {
    assertNull(searchManager.mSearchText);

    String SEARCH_TEXT_1 = "TEXT_1";
    String SEARCH_TEXT_2 = "TEXT_2";

    int DIRECTION_1 = 1;
    int DIRECTION_2 = 2;

    searchManager.mSelectedDirection = DIRECTION_1;

    paragonSearchController.scroll(SEARCH_TEXT_1,false, false);
    assertEquals(searchManager.mSearchText,SEARCH_TEXT_1);
    assertEquals(searchManager.mSearchDirecton,DIRECTION_1);

    searchManager.mSelectedDirection = DIRECTION_2;

    paragonSearchController.scroll(SEARCH_TEXT_1,false, false);
    assertEquals(searchManager.mSearchText,SEARCH_TEXT_1);
    assertEquals(searchManager.mSearchDirecton,DIRECTION_2);

    searchManager.mSelectedDirection = DIRECTION_1;

    paragonSearchController.scroll(SEARCH_TEXT_2,false, false);
    assertEquals(searchManager.mSearchText,SEARCH_TEXT_2);
    assertEquals(searchManager.mSearchDirecton,DIRECTION_1);
  }

  @Test
  public void checkSearchAll()
  {
    assertNull(searchManager.mSearchText);

    String SEARCH_TEXT_1 = "TEXT_1";
    String SEARCH_TEXT_2 = "TEXT_2";

    paragonSearchController.searchAll(SEARCH_TEXT_1);
    assertEquals(searchManager.mSearchAllText,SEARCH_TEXT_1);

    paragonSearchController.searchAll(SEARCH_TEXT_2);
    assertEquals(searchManager.mSearchAllText,SEARCH_TEXT_2);
  }

  @Test
  public void checkFtsSearch()
  {
    assertNull(searchManager.mSearchText);

    String SEARCH_TEXT_1 = "TEXT_1";
    String SEARCH_TEXT_2 = "TEXT_2";

    int DIRECTION_1 = 1;
    int DIRECTION_2 = 2;

    searchManager.mSelectedDirection = DIRECTION_1;

    paragonSearchController.search(SEARCH_TEXT_1,false, SEARCH_TYPE_FTS, SortType.Full);
    assertEquals(searchManager.mSearch,SEARCH_TEXT_1);
    assertEquals(searchManager.mSearchDirecton,DIRECTION_1);

    searchManager.mSelectedDirection = DIRECTION_2;

    paragonSearchController.search(SEARCH_TEXT_1,false, SEARCH_TYPE_FTS, SortType.Full);
    assertEquals(searchManager.mSearch,SEARCH_TEXT_1);
    assertEquals(searchManager.mSearchDirecton,DIRECTION_2);


    searchManager.mSelectedDirection = DIRECTION_1;

    paragonSearchController.search(SEARCH_TEXT_2,false, SEARCH_TYPE_FTS, SortType.Full);
    assertEquals(searchManager.mSearch,SEARCH_TEXT_2);
    assertEquals(searchManager.mSearchDirecton,DIRECTION_1);
  }

  @Test
  public void checkWildCardSearch()
  {
    assertNull(searchManager.mSearchText);

    String SEARCH_TEXT_1 = "TEXT_1";
    String SEARCH_TEXT_2 = "TEXT_2";

    int DIRECTION_1 = 1;
    int DIRECTION_2 = 2;

    searchManager.mSelectedDirection = DIRECTION_1;

    paragonSearchController.search(SEARCH_TEXT_1,false, SEARCH_TYPE_WILD_CARD, SortType.Full);
    assertEquals(searchManager.mSearch,SEARCH_TEXT_1);
    assertEquals(searchManager.mSearchDirecton,DIRECTION_1);

    searchManager.mSelectedDirection = DIRECTION_2;

    paragonSearchController.search(SEARCH_TEXT_1,false, SEARCH_TYPE_WILD_CARD, SortType.Full);
    assertEquals(searchManager.mSearch,SEARCH_TEXT_1);
    assertEquals(searchManager.mSearchDirecton,DIRECTION_2);


    searchManager.mSelectedDirection = DIRECTION_1;

    paragonSearchController.search(SEARCH_TEXT_2,false, SEARCH_TYPE_WILD_CARD, SortType.Full);
    assertEquals(searchManager.mSearch,SEARCH_TEXT_2);
    assertEquals(searchManager.mSearchDirecton,DIRECTION_1);
  }

  @Test
  public void checkDidYouMeanSearch()
  {
    assertNull(searchManager.mSearchText);

    String SEARCH_TEXT_1 = "TEXT_1";
    String SEARCH_TEXT_2 = "TEXT_2";

    int DIRECTION_1 = 1;
    int DIRECTION_2 = 2;

    searchManager.mSelectedDirection = DIRECTION_1;

    paragonSearchController.search(SEARCH_TEXT_1,false, SEARCH_TYPE_DID_YOU_MEAN, SortType.Full);
    assertEquals(searchManager.mSearch,SEARCH_TEXT_1);
    assertEquals(searchManager.mSearchDirecton,DIRECTION_1);

    searchManager.mSelectedDirection = DIRECTION_2;

    paragonSearchController.search(SEARCH_TEXT_1,false, SEARCH_TYPE_DID_YOU_MEAN, SortType.Full);
    assertEquals(searchManager.mSearch,SEARCH_TEXT_1);
    assertEquals(searchManager.mSearchDirecton,DIRECTION_2);


    searchManager.mSelectedDirection = DIRECTION_1;

    paragonSearchController.search(SEARCH_TEXT_2,false, SEARCH_TYPE_DID_YOU_MEAN, SortType.Full);
    assertEquals(searchManager.mSearch,SEARCH_TEXT_2);
    assertEquals(searchManager.mSearchDirecton,DIRECTION_1);
  }


  @Test
  public void checkNeedUpdateSearchResults()
  {
    Dictionary.DictionaryId DICT_ID_1 = new Dictionary.DictionaryId("dict_id_1");
    Dictionary.DictionaryId DICT_ID_2 = new Dictionary.DictionaryId("dict_id_2");
    Dictionary.DictionaryId DICT_ID_3 = new Dictionary.DictionaryId("dict_id_3");

    paragonSearchController.setNeedUpdateSearchResults(false);
    assertFalse(paragonSearchController.needUpdateSearchResults());

    searchManager.mSelectedDictionary = DICT_ID_1;
    paragonSearchController.setSelectedDictionary(DICT_ID_1);
    assertFalse(paragonSearchController.needUpdateSearchResults());

    searchManager.mSelectedDictionary = DICT_ID_2;
    assertTrue(paragonSearchController.needUpdateSearchResults());
    assertEquals(paragonSearchController.getSelectedDictionary(),DICT_ID_2);

    searchManager.mSelectedDictionary = DICT_ID_3;
    assertTrue(paragonSearchController.needUpdateSearchResults());
    assertEquals(paragonSearchController.getSelectedDictionary(),DICT_ID_3);
    assertFalse(paragonSearchController.needUpdateSearchResults());

    paragonSearchController.setNeedUpdateSearchResults(true);
    assertTrue(paragonSearchController.needUpdateSearchResults());
  }

  @Test
  public void checkSearchText()
  {
    String SEARCH_TEXT_1 = "search_text_1";
    String SEARCH_TEXT_2 = "search_text_2";

    paragonSearchController.setSearchText(SEARCH_TEXT_1);
    assertEquals(paragonSearchController.getSearchText(),SEARCH_TEXT_1);

    paragonSearchController.setSearchText(SEARCH_TEXT_2);
    assertEquals(paragonSearchController.getSearchText(),SEARCH_TEXT_2);
  }

  @Test
  public void checkGetArticles()
  {
    MockSearchEngine searchEngine = Mockito.spy(MockSearchEngine.class);
    searchManager.registerSearchEngine(searchEngine);

    assertNull(paragonSearchController.getArticles());

    paragonSearchController.scroll("test",false, false);
    assertNotNull(paragonSearchController.getArticles());
  }

  @Test
  public void checkGetMorphoArticles()
  {
    MockSearchEngine searchEngine = Mockito.spy(MockSearchEngine.class);
    searchManager.registerSearchEngine(searchEngine);

    assertNull(paragonSearchController.getMorphoArticles());

    paragonSearchController.scroll("test",false, false);
    assertNotNull(paragonSearchController.getMorphoArticles());
  }

  @Test
  public void checkGetSearchAllResults()
  {
    MockSearchEngine searchEngine = Mockito.spy(MockSearchEngine.class);
    searchManager.registerSearchEngine(searchEngine);

    assertNull(paragonSearchController.getSearchAllArticles());

    paragonSearchController.searchAll("test");
    assertNotNull(paragonSearchController.getSearchAllArticles());
  }

  @Test
  public void checkGetFtsArticles()
  {
    MockSearchEngine searchEngine = Mockito.spy(MockSearchEngine.class);
    searchManager.registerSearchEngine(searchEngine);

    assertNull(paragonSearchController.getSpecialSearchArticles());

    paragonSearchController.search("test",false, SEARCH_TYPE_FTS, SortType.Full);
    assertNotNull(paragonSearchController.getSpecialSearchArticles());
  }

  @Test
  public void checkGetWildCardArticles()
  {
    MockSearchEngine searchEngine = Mockito.spy(MockSearchEngine.class);
    searchManager.registerSearchEngine(searchEngine);

    assertNull(paragonSearchController.getSpecialSearchArticles());

    paragonSearchController.search("test",true, SEARCH_TYPE_WILD_CARD, SortType.Full);
    assertNotNull(paragonSearchController.getSpecialSearchArticles());
  }

  @Test
  public void checkGetDidYouMeanArticles()
  {
    MockSearchEngine searchEngine = Mockito.spy(MockSearchEngine.class);
    searchManager.registerSearchEngine(searchEngine);

    assertNull(paragonSearchController.getSpecialSearchArticles());

    paragonSearchController.search("test", true, SEARCH_TYPE_DID_YOU_MEAN, SortType.Full);
    assertNotNull(paragonSearchController.getSpecialSearchArticles());
  }

  @Test
  public void checkSetSearchScrollState()
  {
    ScrollState normal = new ScrollState(1,1);
    ScrollState dictFilter = new ScrollState(2,2);
    ScrollState specialSearch = new ScrollState(3,3);
    ScrollState expanded = new ScrollState(4,4);

    paragonSearchController.setNormalSearchScrollState(normal);
    paragonSearchController.setDictionaryFilterScrollState(dictFilter);
    paragonSearchController.setSpecialSearchScrollState(specialSearch);
    paragonSearchController.setExpandedSpecialSearchGroupScrollState(expanded);

    assertEquals(paragonSearchController.getNormalSearchScrollState(),normal);
    assertEquals(paragonSearchController.getDictionaryFilterScrollState(),dictFilter);
    assertEquals(paragonSearchController.getSpecialSearchScrollState(), specialSearch);
    assertEquals(paragonSearchController.getExpandedSpecialSearchGroupScrollState(),expanded);
  }

  @Test
  public void checkSetDictionaryId()
  {
    assertNull(paragonSearchController.getSearchDictionaryId());
    Dictionary.DictionaryId dictId= new Dictionary.DictionaryId("test");

    paragonSearchController.setSelectedDictionary(dictId);
    assertEquals(paragonSearchController.getSearchDictionaryId(),dictId);
  }

  @Test
  public void checkGetDictionaries()
  {
    assertEquals(paragonSearchController.getDictionaries(), searchManager.mDictionaries);
  }

  @Test
  public void checkGetFilterDictionaryIdList()
  {
    MockSearchEngine searchEngine = Mockito.spy(MockSearchEngine.class);
    searchManager.registerSearchEngine(searchEngine);

    assertNull(paragonSearchController.getFilterDictionaryIdList());
    paragonSearchController.searchAll("test");
    assertNotNull(paragonSearchController.getFilterDictionaryIdList());
  }

  @Test
  public void checkSetSelectedDirection()
  {
    assertNull(searchManager.mAutoSelectedDirection);

    Dictionary.Direction direction = new Dictionary.Direction(0,0,null);
    paragonSearchController.setSelectedDirection(direction);

    assertEquals(searchManager.mAutoSelectedDirection,direction);

  }

  @Test
  public void checkOpenArticle()
  {
    assertNull(paragonSearchController.getActiveArticle());
    assertNull(screenOpenerMock.mArticleItem);

    ArticleItem item = new ArticleItem.Builder
            (new Dictionary.DictionaryId("test"), 0, 0).build();

    paragonSearchController.openArticle(item);
    assertEquals(paragonSearchController.getActiveArticle(),item);
    assertEquals(screenOpenerMock.mArticleItem,item);
  }

  @Test
  public void checkPlaySound()
  {
    assertNull(searchManager.mPlaySoundArticle);

    ArticleItem item = new ArticleItem.Builder(new Dictionary.DictionaryId("test"),
            0,
            0).
            build();

    paragonSearchController.playSound(item);
    assertEquals(searchManager.mPlaySoundArticle,item);
  }

  @Test
  public void checkItemHasSound()
  {
    ArticleItem item = new ArticleItem.Builder(new Dictionary.DictionaryId("test"),
            0,
            0).
            build();

    searchManager.mItemHasSound = true;
    assertTrue(paragonSearchController.itemHasSound(item));

    searchManager.mItemHasSound = false;
    assertFalse(paragonSearchController.itemHasSound(item));
  }

  @Test
  public void checkSetSpecialSearchGroupExpanded()
  {
    assertFalse(paragonSearchController.isSpecialSearchGroupExpanded());

    paragonSearchController.setSpecialSearchGroupExpanded(true);
    assertTrue(paragonSearchController.isSpecialSearchGroupExpanded());

    paragonSearchController.setSpecialSearchGroupExpanded(false);
    assertFalse(paragonSearchController.isSpecialSearchGroupExpanded());
  }

  @Test
  public void checkSetExpandedSpecialSearchGroup()
  {
    assertNull(paragonSearchController.getExpandedSpecialSearchGroup());

    CollectionView<ArticleItem, GroupHeader> expanded = Mockito.spy(MockCollectionView.class);

    paragonSearchController.setExpandedSpecialSearchGroup(expanded);
    assertEquals(paragonSearchController.getExpandedSpecialSearchGroup(),expanded);
  }

  @Test
  public void checkSetExpandedGroupHeader()
  {
    assertNull(paragonSearchController.getExpandedGroupHeader());
    String label = "test";
    paragonSearchController.setExpandedGroupHeader(label);
    assertEquals(paragonSearchController.getExpandedGroupHeader(),label);
  }

  @Test
  public void checkSetFilterDictionaryId()
  {
    assertNull(paragonSearchController.getFilterDictionaryId());

    Dictionary.DictionaryId dictionaryId = new Dictionary.DictionaryId("test");
    paragonSearchController.setFilterDictionaryId(dictionaryId);
    assertEquals(paragonSearchController.getFilterDictionaryId(),dictionaryId);

  }

  @Test
  public void testOpenScreen() {
    paragonSearchController.openScreen(ScreenType.Article);
    verify(screenOpenerMock).openScreen(eq(ScreenType.Article));
    paragonSearchController.openScreen(ScreenType.Download);
    verify(screenOpenerMock).openScreen(eq(ScreenType.Download));
  }

  @Test
  public void testTopScreenOverlayObservable() {
    paragonSearchController.getTopScreenOverlayStateObservable();
    verify(screenOpenerMock, times(1)).getTopScreenOverlayStateObservable();
  }
}
