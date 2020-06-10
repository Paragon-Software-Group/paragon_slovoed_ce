package com.paragon_software.search_manager;

import android.app.Activity;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryAndDirection;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.favorites_manager.FavoritesManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.history_manager.HistoryManagerAPI;
import com.paragon_software.native_engine.GroupHeader;
import com.paragon_software.native_engine.ScrollResult;
import com.paragon_software.native_engine.SearchAllResult;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.native_engine.SearchType;
import com.paragon_software.native_engine.SortType;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.settings_manager.OnApplicationSettingsSaveListener;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.testmode.TestModeAPI;
import com.paragon_software.toolbar_manager.DirectionView;
import com.paragon_software.toolbar_manager.ToolbarManager;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.disposables.CompositeDisposable;

public class
ParagonSearchManager extends SearchManagerAPI implements OnApplicationSettingsSaveListener, DictionaryManagerAPI.IDictionaryListObserver
{
  private static final int MAX_WORDS = 200;
  private static final String SAVE_SEARCH_REQUEST = "com.paragon_software.search_manager.ParagonSearchManager.SAVE_SEARCH_REQUEST";
  private static final String SAVE_IS_SCROLL_SELECTED = "com.paragon_software.search_manager.ParagonSearchManager.SAVE_IS_SCROLL_SELECTED";

  private       ScreenOpenerAPI                   mScreenOpener;
  private final HashMap<String,SearchController>  mSearchControllers = new HashMap<>();
  private final AtomicReference<SearchController> mActiveController = new AtomicReference<>();
  private       Dictionary.DictionaryId           mCurrentDictionaryId = null;
  private       SearchEngineAPI                   mSearchEngine;
  private       SettingsManagerAPI                mSettingsManager;
  private       SoundManagerAPI                   mSoundManager;
  private       ToolbarManager                    mToolbarManager;
  private       DictionaryManagerAPI              mDictionaryManager;
  private       HistoryManagerAPI                 mHistoryManager;
  private       FavoritesManagerAPI               mFavoritesManager;
  private       TestModeAPI                       mTestMode;
  @Nullable
  private       HintManagerAPI                    mHintManager;

  private final List< OnErrorListener >           mOnErrorListeners = new ArrayList<>();

  private CompositeDisposable mCompositeDisposable = new CompositeDisposable();

  @Override
  public void registerScreenOpener(ScreenOpenerAPI articleManagerAPI )
  {
    mScreenOpener = articleManagerAPI;
  }

  @Override
  public void registerSearchEngine( SearchEngineAPI searchEngine )
  {
    mSearchEngine = searchEngine;
  }

  @Override
  public void registerSettingsManager( SettingsManagerAPI settingsManager )
  {
    mSettingsManager = settingsManager;
    mSettingsManager.registerNotifier(this);
  }

  @Override
  public void registerSoundManager( SoundManagerAPI soundManager )
  {
    mSoundManager = soundManager;
  }

  @Override
  public void registerToolbarManager( ToolbarManager toolbarManager )
  {
    mToolbarManager = toolbarManager;
  }

  @Override
  public void registerDictionaryManager( DictionaryManagerAPI dictionaryManagerAPI )
  {
    mDictionaryManager = dictionaryManagerAPI;
    mDictionaryManager.registerDictionaryListObserver(this);
    mCompositeDisposable.add(mDictionaryManager.getDictionaryAndDirectionChangeObservable()
            .observeOn(AndroidSchedulers.mainThread())
            .subscribe(flag -> {
              if ( mDictionaryManager.isSelectAllDictionaries() )
              {
                onDictionarySelect(null);
              }
              else
              {
                DictionaryAndDirection dictionaryAndDirection = mDictionaryManager.getDictionaryAndDirectionSelectedByUser();
                if ( dictionaryAndDirection != null )
                {
                  onDictionarySelect(dictionaryAndDirection.getDictionaryId());
                  onDirectionSelect(dictionaryAndDirection.getDirection().getLanguageFrom());
                }
              }
            }));
  }

  @Override
  public void registerHistoryManager( HistoryManagerAPI historyManager )
  {
    mHistoryManager = historyManager;
  }

  @Override
  public void registerFavoritesManager( FavoritesManagerAPI favoritesManagerAPI )
  {
    mFavoritesManager = favoritesManagerAPI;
  }

  @Override
  public void registerHintManager( @Nullable HintManagerAPI hintManager )
  {
    mHintManager = hintManager;
  }

  @Override
  public void setControllerSearchText(String uiName, @NonNull String searchText) {
    SearchController controller = getController(uiName);
    if(controller != null)
      controller.setSearchText(searchText);
  }

  void setSelectedDirection(Dictionary.Direction selectedDirection)
  {
    if ( mDictionaryManager != null ) {
      DictionaryAndDirection curDictionaryAndDirection = mDictionaryManager.getDictionaryAndDirectionSelectedByUser();
      if (curDictionaryAndDirection != null) {
        DictionaryAndDirection newDictionaryAndDirection
                = new DictionaryAndDirection(curDictionaryAndDirection.getDictionaryId(), selectedDirection);
        mDictionaryManager.setDictionaryAndDirectionSelectedByUser(newDictionaryAndDirection);
        if (null != mToolbarManager)
        {
          mToolbarManager.setSelectedDirection(selectedDirection);
        }
      }
    }
  }

  @Override
  public void registerNotifier( @NonNull Notifier notifier )
  {
    if (notifier instanceof OnErrorListener && !mOnErrorListeners.contains(notifier))
    {
      mOnErrorListeners.add((OnErrorListener) notifier);
    }
  }

  @Override
  public void unregisterNotifier( @NonNull Notifier notifier )
  {
    if (notifier instanceof OnErrorListener)
    {
      mOnErrorListeners.remove(notifier);
    }
  }

  @Override
  public SearchController getController( String s )
  {
    SearchController controller = mSearchControllers.get(s);
    if (null == controller)
    {
      if (SearchController.CONTROLLER_TYPE_DEFAULT.equals(s))
      {
        controller = new ParagonSearchController(this, mScreenOpener, mHintManager, mToolbarManager);
        mSearchControllers.put(SearchController.CONTROLLER_TYPE_DEFAULT, controller);
      } else if(SearchController.CONTROLLER_TYPE_OALD.equals(s))
      {
        controller = new OaldSearchController(this, mScreenOpener, mHintManager, mToolbarManager);
        mSearchControllers.put(SearchController.CONTROLLER_TYPE_OALD, controller);
      } else if(SearchController.CONTROLLER_TYPE_DUDEN.equals(s))
      {
        controller = new DudenSearchController(this, mScreenOpener, mHintManager, mToolbarManager);
        mSearchControllers.put(SearchController.CONTROLLER_TYPE_DUDEN, controller);
      } else if(SearchController.CONTROLLER_TYPE_BILINGUAL.equals(s))
      {
        controller = new BilingualSearchController(this, mScreenOpener, mHintManager, mToolbarManager);
        mSearchControllers.put(SearchController.CONTROLLER_TYPE_BILINGUAL, controller);
      }
    }
    mActiveController.set(controller);
    onApplicationSettingsSaved(mSettingsManager.loadApplicationSettings());
    return controller;
  }

  @Override
  public void freeController( String uiName )
  {
    if (mActiveController.get() == mSearchControllers.get(uiName))
    {
      mActiveController.set(null);
    }
  }

  @Override
  public void saveSearchRequest(@NonNull String text) {
    try {
      mSettingsManager.save(SAVE_SEARCH_REQUEST, text, true );
    } catch (ManagerInitException e) {
      e.printStackTrace();
    } catch (LocalResourceUnavailableException e) {
      e.printStackTrace();
    }
  }

  @Override
  public void saveIsScrollSelected(boolean isScrollSelected) {
    try {
      mSettingsManager.save(SAVE_IS_SCROLL_SELECTED, isScrollSelected, true );
    } catch (ManagerInitException e) {
      e.printStackTrace();
    } catch (LocalResourceUnavailableException e) {
      e.printStackTrace();
    }
  }

  @Override
  public String restoreSearchRequest() {
    try {
      return mSettingsManager.load(SAVE_SEARCH_REQUEST, "");
    } catch (WrongTypeException e) {
      e.printStackTrace();
    } catch (ManagerInitException e) {
      e.printStackTrace();
    }
    return "";
  }

  @Override
  public boolean restoreIsScrollSelected() {
    try {
      return mSettingsManager.load(SAVE_IS_SCROLL_SELECTED, true);
    } catch (WrongTypeException e) {
      e.printStackTrace();
    } catch (ManagerInitException e) {
      e.printStackTrace();
    }
    return true;
  }

  @Override
  public boolean launchTestMode( @NonNull Activity activity, @NonNull String text )
  {
    if ( null != mTestMode && mTestMode.isTestModeKeyword(text) )
    {
      try
      {
        mTestMode.startTestModeUI(activity);
      }
      catch ( UnsupportedOperationException e )
      {
        return false;
      }
      return true;
    }
    else
    {
      return false;
    }
  }

  @Override
  public void registerTestMode( @NonNull TestModeAPI testMode )
  {
    this.mTestMode = testMode;
  }

  @Override
  ScrollResult scroll(int direction, String word, boolean autoChangeDirection, boolean exactly) {
    Collection<Dictionary.Direction> allDirections = getAllDirections();
    Dictionary.Direction selectedDirection = getSelectedDirection(allDirections, direction);
    return mSearchEngine.scroll(mCurrentDictionaryId, selectedDirection, word, autoChangeDirection ? allDirections : null, exactly);
  }

  @Nullable
  private Collection<Dictionary.Direction> getAllDirections() {
    Collection<Dictionary> dictionaries = getDictionaries();
    Dictionary selectedDictionary = null;
    if (null != dictionaries)
      for (Dictionary dictionary : dictionaries)
        if (dictionary.getId().equals(mCurrentDictionaryId)) {
          selectedDictionary = dictionary;
          break;
        }
    Collection<Dictionary.Direction> allDirections = null;
    if (null != selectedDictionary)
      allDirections = selectedDictionary.getDirections();
    return allDirections;
  }

  @Nullable
  private Dictionary.Direction getSelectedDirection(@Nullable Collection<Dictionary.Direction> allDirections, int directionId) {
    Dictionary.Direction selectedDirection = null;
    if (null != allDirections)
      for (Dictionary.Direction direction : allDirections)
        if (direction.getLanguageFrom() == directionId) {
          selectedDirection = direction;
          break;
        }
    return selectedDirection;
  }

  @Override
  SearchAllResult searchAll(String word)  {
    return mSearchEngine.searchAll(word,MAX_WORDS);
  }

  @Override
  CollectionView<CollectionView<ArticleItem, GroupHeader>, Dictionary.Direction>
    search(int direction, @NonNull String word, boolean autoChangeDirection, @NonNull SearchType searchType, @NonNull SortType sortType, @NonNull Boolean needRunSearch) {
      Collection<Dictionary.Direction> allDirections = getAllDirections();
      Dictionary.Direction selectedDirection = getSelectedDirection(allDirections, direction);
      return mSearchEngine.search(mCurrentDictionaryId, selectedDirection, word, autoChangeDirection ? allDirections : null, searchType, sortType, needRunSearch);
  }

  void playSound( ArticleItem articleItem )
  {
    if (null != mSoundManager)
    {
      mSoundManager.playSound(articleItem);
    }
  }

  boolean hasSound( ArticleItem articleItem )
  {
    if (null != mSoundManager)
    {
      return mSoundManager.itemHasSound(articleItem);
    }
    else
    {
      return false;
    }
  }

  Dictionary.DictionaryId getSelectedDictionary()
  {
    if ( mDictionaryManager.isSelectAllDictionaries() ) {
      return null;
    }
    else {
      DictionaryAndDirection dictionaryAndDirection = mDictionaryManager.getDictionaryAndDirectionSelectedByUser();
      if ( dictionaryAndDirection != null )
        return dictionaryAndDirection.getDictionaryId();
    }
    return null;
  }

  int getSelectedDirection()
  {
    if ( mDictionaryManager.isSelectAllDictionaries() ) {
      return DirectionView.INVALID_DIRECTION;
    }
    else {
      DictionaryAndDirection dictionaryAndDirection = mDictionaryManager.getDictionaryAndDirectionSelectedByUser();
      if ( dictionaryAndDirection != null )
        return dictionaryAndDirection.getDirection().getLanguageFrom();
    }
    return DirectionView.INVALID_DIRECTION;
  }

  List< Dictionary > getDictionaries()
  {
    if (null != mDictionaryManager)
    {
      return mDictionaryManager.getDictionaries();
    }
    else
    {
      return null;
    }
  }

  public void onDictionarySelect( @Nullable Dictionary.DictionaryId dictionaryId )
  {
    mCurrentDictionaryId = dictionaryId;
    SearchController controller = mActiveController.get();
    if (controller instanceof ParagonSearchController)
    {
      ((ParagonSearchController)controller).setSelectedDictionary(dictionaryId);
    }
  }

  public void onDirectionSelect( int direction )
  {
    SearchController controller = mActiveController.get();
    if (controller instanceof ParagonSearchController)
    {
      ((ParagonSearchController)controller).setSelectedDirection(direction);
    }
  }

  void onError(Exception exception) {
    SearchController controller = mActiveController.get();
    if (controller instanceof ParagonSearchController)
    {
      ((ParagonSearchController)controller).onError(exception);
    }
    for ( OnErrorListener listener : mOnErrorListeners )
    {
      listener.onSearchManagerError(exception);
    }
  }

  @Override
  public void onDictionaryListChanged()
  {
    for ( SearchController controller : mSearchControllers.values() )
    {
      controller.setNeedUpdateSearchResults(true);
    }
    SearchController activeController = mActiveController.get();
    if ( activeController instanceof ParagonSearchController )
    {
      ((ParagonSearchController) activeController).dictionaryListChanged();
    }
  }

  @Override
  public void onApplicationSettingsSaved( @NonNull ApplicationSettings applicationSettings )
  {
    SearchController controller = mActiveController.get();
    if ( controller != null )
    {
      controller.setEntryListFontSize(applicationSettings.getEntryListFontSize());
      controller.setShowKeyboardEnabled(applicationSettings.isShowKeyboardForSearchEnabled());
      controller.setShowHighlightingEnabled(applicationSettings.isShowHighlightingEnabled());
    }
  }
}
