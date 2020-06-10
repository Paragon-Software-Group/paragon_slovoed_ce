package com.paragon_software.favorites_manager;

import android.content.Context;
import android.content.Intent;

import com.paragon_software.analytics_manager.AnalyticsManagerAPI;
import com.paragon_software.analytics_manager.events.AddArticleToFavourites;
import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.native_engine.EngineSerializerAPI;
import com.paragon_software.native_engine.PersistentArticle;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;
import com.paragon_software.utils_slovoed.collections.LimitedLinkedList;
import com.paragon_software.utils_slovoed.directory.Directory;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.DialogFragment;
import androidx.fragment.app.FragmentManager;
import io.reactivex.Observable;
import io.reactivex.Single;
import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.disposables.Disposable;
import io.reactivex.schedulers.Schedulers;
import io.reactivex.subjects.BehaviorSubject;

public class BaseFavoritesManager extends FavoritesManagerAPI implements DictionaryManagerAPI.IDictionaryListObserver
{
  private static final String FAVORITES_DIRECTORY_STRUCTURE_SAVE_KEY = "FAVORITES_DIRECTORY_STRUCTURE_SAVE_KEY";

  /**
   * Key to save article items from root directory.
   */
  static final String FAVORITES_SAVE_KEY = "FAVORITES_SAVE_KEY";
  private final String mFileProviderAuthorities;

  private ScreenOpenerAPI mScreenOpener;
  private EngineSerializerAPI  mEngineSerializer;
  @Nullable
  private SearchEngineAPI      mEngineSearcher;
  private SettingsManagerAPI   mSettingsManager;
  private DictionaryManagerAPI mDictionaryManager;
  @Nullable
  private HintManagerAPI       mHintManager;

  @Nullable
  private ToolbarManager  mToolbarManager;
  @Nullable
  private SoundManagerAPI mSoundManager;

  /**
   * Object used to sort list of favorites articles.
   */
  @Nullable
  private FavoritesSorterAPI mFavoritesSorter;

  @NonNull
  private UIData uiData = new UIData();

  private final List< OnStateChangedListener >         mOnStateChangedListeners         = new ArrayList<>();
  private final List< OnFavoritesListChangesListener > mOnFavoritesListChangesListeners = new ArrayList<>();
  private final List< OnErrorListener >                mOnErrorListeners                = new ArrayList<>();

  private Disposable preloadedFavoritesDisposable = null;

  private final Map< String, FavoritesControllerAPI > mFavoritesControllers = new HashMap<>();
  private final Map< String, FavoritesControllerAddInDirectoryAPI > mFavoritesAddInDirectoryControllers = new HashMap<>();

  private volatile boolean mInTransition = false;
  private final    int     mMaxNumberOfDirWords;

  @NonNull
  private MutableManagerDirectory      mRootDirectory;
  private BehaviorSubject< Directory< ArticleItem > > mRootDirectorySubject;

  @Nullable
  private Class<IExportHtmlBuilder> mExportHtmlBuilderClass;

  public BaseFavoritesManager(ScreenOpenerAPI screenOpener, EngineSerializerAPI engineSerializer, SearchEngineAPI engineSearcher,
                              SettingsManagerAPI settingsManager, DictionaryManagerAPI dictionaryManager,
                              ToolbarManager toolbarManager, SoundManagerAPI soundManager, HintManagerAPI hintManager,
                              FavoritesSorterAPI favoritesSorter, Class favoritesActivity,
                              @Nullable DialogFragment addInDirectoryDialogFragment,
                               @Nullable Class<IExportHtmlBuilder> exportHtmlBuilderClass, @Nullable  String authoritiesFileProvider, int maxNumberOfDirWords )
  {
    if ( screenOpener == null || engineSerializer == null || settingsManager == null || dictionaryManager == null ) {
      throw new IllegalArgumentException("The constructor parameters"
                                             + (screenOpener == null ? " ScreenOpenerAPI " : "")
                                             + (engineSerializer == null ? " EngineSerializerAPI " : "")
                                             + (settingsManager == null ? " SettingsManagerAPI " : "")
                                             + (dictionaryManager == null ? " DictionaryManagerAPI " : "")
                                             + "cannot be null!");
    }
    mScreenOpener = screenOpener;
    mEngineSerializer = engineSerializer;
    mEngineSearcher = engineSearcher;
    mSettingsManager = settingsManager;
    mDictionaryManager = dictionaryManager;
    mToolbarManager = toolbarManager;
    mSoundManager = soundManager;
    mHintManager = hintManager;
    mFavoritesSorter = favoritesSorter;
    mExportHtmlBuilderClass = exportHtmlBuilderClass;
    mDictionaryManager.registerDictionaryListObserver(this);
    uiData.mFavoritesActivityClass = favoritesActivity;
    uiData.mAddInDirectoryDialogFragment = addInDirectoryDialogFragment;
    mMaxNumberOfDirWords = maxNumberOfDirWords;
    mRootDirectory = new MutableManagerDirectory(null, FAVORITES_SAVE_KEY);
    mRootDirectorySubject = BehaviorSubject.createDefault(mRootDirectory);
    loadRootDirectoryFromSetting();
    mFileProviderAuthorities = authoritiesFileProvider;
  }

  private void loadRootDirectoryFromSetting()
  {
    try
    {
      PersistentDirectory defSerializableDirectoryForSettings = new PersistentDirectory(mRootDirectory);
      PersistentDirectory loadedDirTmp = mSettingsManager.load(FAVORITES_DIRECTORY_STRUCTURE_SAVE_KEY, defSerializableDirectoryForSettings);
      if ( loadedDirTmp == defSerializableDirectoryForSettings )
      {
        // empty Favorites means that user was not used Favorites before
        // try to load predefined Favorites
        loadPredefinedFavorites();
      }
      else
      {
        mRootDirectory = new MutableManagerDirectory(null, loadedDirTmp.getName(), loadedDirTmp.getChildList());
        mRootDirectory.deepLoadItems();
      }
    }
    catch ( WrongTypeException | ManagerInitException exception )
    {
      exception.printStackTrace();
    }
  }

  private void saveRootDirectoryStructure()
  {
    try
    {
      PersistentDirectory persistentDirectory = new PersistentDirectory(mRootDirectory);
      mSettingsManager.save(FAVORITES_DIRECTORY_STRUCTURE_SAVE_KEY, persistentDirectory, true);
    }
    catch ( ManagerInitException | LocalResourceUnavailableException exception )
    {
      onError(exception);
    }
  }

  private void saveDeepRootDirectoryItemsAsynchronously()
  {
    Single.fromCallable(() ->
                        {
                          try
                          {
                            mRootDirectory.saveDeepDirectoryItems();
                          }
                          catch ( LocalResourceUnavailableException exception )
                          {
                            exception.printStackTrace();
                          }
                          return true;
                        })
          .subscribeOn(Schedulers.computation())
          .subscribe();
  }

  private void loadPredefinedFavorites()
  {
    if ( mEngineSearcher != null && mDictionaryManager.getDictionaryAndDirectionSelectedByUser() != null )
    {
      Dictionary.DictionaryId dictId = mDictionaryManager.getDictionaryAndDirectionSelectedByUser().getDictionaryId();
      if ( preloadedFavoritesDisposable != null && !preloadedFavoritesDisposable.isDisposed() )
      {
        preloadedFavoritesDisposable.dispose();
      }
      updateState(true);
      preloadedFavoritesDisposable =
          mEngineSearcher.getPreloadedFavorites(dictId).observeOn(AndroidSchedulers.mainThread()).subscribe(
              predefinedFavorites -> {
                mRootDirectory = new MutableManagerDirectory(null, FAVORITES_SAVE_KEY, predefinedFavorites.getChildList(),
                                                             predefinedFavorites.getItems());
                notifyRootChange(true);
                saveRootDirectoryStructure();
                saveDeepRootDirectoryItemsAsynchronously();
                updateState(false);
              },
              throwable -> updateState(false));
    }
  }

  private Serializable[] serializeArticleItems( List< ArticleItem > items )
  {
    List< Serializable > convertList = new ArrayList<>();
    if ( items != null )
    {
      for ( ArticleItem article : items )
      {
        convertList.add(mEngineSerializer.serializeArticleItem(article));
      }
    }
    return convertList.toArray(new Serializable[0]);
  }

  @Override
  public void registerNotifier( @NonNull Notifier notifier )
  {
    if ( notifier instanceof OnStateChangedListener && !mOnStateChangedListeners.contains(notifier) )
    {
      mOnStateChangedListeners.add((OnStateChangedListener) notifier);
    }
    if ( notifier instanceof OnFavoritesListChangesListener && !mOnFavoritesListChangesListeners.contains(notifier) )
    {
      mOnFavoritesListChangesListeners.add((OnFavoritesListChangesListener) notifier);
    }
    if ( notifier instanceof OnErrorListener && !mOnErrorListeners.contains(notifier) )
    {
      mOnErrorListeners.add((OnErrorListener) notifier);
    }
  }

  @Override
  public void unregisterNotifier( @NonNull Notifier notifier )
  {
    if ( notifier instanceof OnStateChangedListener )
    {
      mOnStateChangedListeners.remove(notifier);
    }
    if ( notifier instanceof OnFavoritesListChangesListener )
    {
      mOnFavoritesListChangesListeners.remove(notifier);
    }
    if ( notifier instanceof OnErrorListener )
    {
      mOnErrorListeners.remove(notifier);
    }
  }

  @NonNull
  @Override
  public List< ArticleItem > getWords()
  {
    return mRootDirectory.getItems();
  }

  @Override
  public boolean hasWord( @Nullable ArticleItem articleItem )
  {
    return hasWord(articleItem, mRootDirectory);
  }

  @Override
  public boolean addWord( @NonNull ArticleItem articleItem )
  {
    return addWord(articleItem, mRootDirectory);
  }

  @Override
  public boolean addWordsForTestmode( @NonNull List< ArticleItem > words )
  {
    boolean toAdd = false;
    for ( ArticleItem item : words )
    {
      if ( !mRootDirectory.hasItem(item) )
      {
        toAdd = true;
        break;
      }
    }
    toAdd = !mInTransition && toAdd;
    if ( toAdd )
    {
      updateState(true);
      mRootDirectory.getMutableArticleItems().removeAll(words);
      mRootDirectory.getMutableArticleItems().addAll(0, words);
      // NOTE: It is only for testmode
      mRootDirectory.distributeItemsInChildDirsForTestmode(words);
      saveDeepRootDirectoryItemsAsynchronously();
      notifyRootChange(true);
      updateState(false);
    }
    return toAdd;
  }

  @Override
  public boolean removeWord( @NonNull ArticleItem articleItem )
  {
    return removeWord(articleItem, mRootDirectory);
  }

  @Override
  public boolean removeWords( @NonNull List< ArticleItem > words )
  {
    return removeWords(words, mRootDirectory);
  }

  @Override
  public boolean removeAllWords()
  {
    return removeAllWords(mRootDirectory);
  }

  @NonNull
  @Override
  Observable< Directory< ArticleItem > > getRootDirectoryObservable()
  {
    return mRootDirectorySubject;
  }

  @Override
  boolean hasChildDirectory( @Nullable Directory< ArticleItem > parent, @Nullable String childDirName )
  {
    if ( parent != null && childDirName != null )
    {
      Directory< ArticleItem > foundParent = mRootDirectory.findDirectory(parent.getPath());
      if ( foundParent != null )
      {
        for ( Directory< ArticleItem > childDir : foundParent.getChildList() )
        {
          if ( childDir.getName().equals(childDirName) )
            return true;
        }
      }
    }
    return false;
  }

  @Override
  boolean addDirectory( @NonNull Directory< ArticleItem > parent, @NonNull String name )
  {
    if ( !mInTransition )
    {
      MutableManagerDirectory foundParent = mRootDirectory.findMutableDirectory(parent);
      if ( foundParent != null )
      {
        MutableManagerDirectory newDir = new MutableManagerDirectory(foundParent, name);
        Directory< ArticleItem > foundExistingChildDir = foundParent.findDirectory(newDir.getPath());
        if ( foundExistingChildDir == null )
        {
          foundParent.getMutableChildList().add(0, newDir);
          saveRootDirectoryStructure();
          notifyFullRootDirectoryChange();
          return true;
        }
      }
    }
    return false;
  }

  @Override
  boolean removeDirectory( @NonNull Directory< ArticleItem > toDelete )
  {
    if ( !mInTransition )
    {
      MutableManagerDirectory foundToDelete = mRootDirectory.findMutableDirectory(toDelete);
      if ( foundToDelete != null && foundToDelete.getParent() instanceof MutableManagerDirectory )
      {
        try
        {
          foundToDelete.getMutableArticleItems().clear();
          foundToDelete.saveDirectoryItems();
          ( (MutableManagerDirectory) foundToDelete.getParent() ).getMutableChildList().remove(foundToDelete);
          saveRootDirectoryStructure();
          notifyFullRootDirectoryChange();
        }
        catch ( ManagerInitException | LocalResourceUnavailableException exception )
        {
          onError(exception);
        }
        return true;
      }
    }
    return false;
  }

  @Override
  boolean hasWord( @Nullable ArticleItem word, @Nullable Directory< ArticleItem > directory )
  {
    MutableManagerDirectory curDirectory = mRootDirectory.findMutableDirectory(directory);
    if ( curDirectory != null )
    {
     return curDirectory.hasItem(word);
    }
    return false;
  }

  @Override
  boolean addWord( @NonNull ArticleItem word, @Nullable Directory< ArticleItem > directory )
  {
    MutableManagerDirectory curDirectory = mRootDirectory.findMutableDirectory(directory);
    boolean toAdd = !mInTransition && curDirectory != null && !curDirectory.hasItem(word);
    if ( toAdd )
    {
      updateState(true);
      try
      {
        curDirectory.getMutableArticleItems().add(0, word);
        curDirectory.saveDirectoryItems();
        notifyRootChange(curDirectory.getParent() == null);
      }
      catch ( ManagerInitException | LocalResourceUnavailableException exception )
      {
        onError(exception);
      }
      finally
      {
        updateState(false);
      }
      AnalyticsManagerAPI.get().logEvent(new AddArticleToFavourites());
    }
    return toAdd;
  }

  @Override
  boolean removeWord( @NonNull ArticleItem word, @Nullable Directory< ArticleItem > directory )
  {
    MutableManagerDirectory curDirectory = mRootDirectory.findMutableDirectory(directory);
    boolean toRemove = !mInTransition && curDirectory != null && curDirectory.hasItem(word);
    if ( toRemove )
    {
      updateState(true);
      try
      {
        curDirectory.getMutableArticleItems().remove(word);
        curDirectory.saveDirectoryItems();
        notifyRootChange(curDirectory.getParent() == null);
      }
      catch ( ManagerInitException | LocalResourceUnavailableException exception )
      {
        onError(exception);
      }
      finally
      {
        updateState(false);
      }
    }
    return toRemove;
  }

  @Override
  boolean removeWords( @NonNull List< ArticleItem > words, @Nullable Directory< ArticleItem > directory )
  {
    MutableManagerDirectory curDirectory = mRootDirectory.findMutableDirectory(directory);
    boolean toRemove = false;
    if ( curDirectory != null )
    {
      for ( ArticleItem item : words )
      {
        if ( curDirectory.hasItem(item) )
        {
          toRemove = true;
          break;
        }
      }
    }
    toRemove = !mInTransition && toRemove;
    if ( toRemove )
    {
      updateState(true);
      try
      {
        curDirectory.getMutableArticleItems().removeAll(words);
        curDirectory.saveDirectoryItems();
        notifyRootChange(curDirectory.getParent() == null);
      }
      catch ( ManagerInitException | LocalResourceUnavailableException exception )
      {
        onError(exception);
      }
      finally
      {
        updateState(false);
      }
    }
    return toRemove;
  }

  @Override
  boolean removeAllWords( @Nullable Directory< ArticleItem > directory )
  {
    MutableManagerDirectory curDirectory = mRootDirectory.findMutableDirectory(directory);
    boolean toRemove = !mInTransition && curDirectory != null && !curDirectory.getItems().isEmpty();
    if ( toRemove )
    {
      updateState(true);
      try
      {
        curDirectory.getMutableArticleItems().clear();
        curDirectory.saveDirectoryItems();
        notifyRootChange(curDirectory.getParent() == null);
      }
      catch ( ManagerInitException | LocalResourceUnavailableException exception )
      {
        onError(exception);
      }
      finally
      {
        updateState(false);
      }
    }
    return toRemove;
  }

  @Override
  public boolean isInTransition()
  {
    return mInTransition;
  }

  @NonNull
  @Override
  FavoritesControllerAPI getController( @NonNull String uiName )
  {
    FavoritesControllerAPI controller = mFavoritesControllers.get(uiName);
    if ( controller == null )
    {
      if ( uiName.contains(FavoritesControllerType.BILINGUAL_CONTROLLER) ) {
        controller = new BilingualFavoritesController(this, mScreenOpener, mSettingsManager, mDictionaryManager, mToolbarManager,
                        mSoundManager, mHintManager, mFavoritesSorter, mExportHtmlBuilderClass, uiName, mFileProviderAuthorities);
      } else {
        controller = new BaseFavoritesController(this, mScreenOpener, mSettingsManager, mDictionaryManager, mToolbarManager,
                        mSoundManager, mHintManager, mFavoritesSorter, mExportHtmlBuilderClass, uiName, mFileProviderAuthorities);
      }
      mFavoritesControllers.put(uiName, controller);
    }
    controller.activate();
    return controller;
  }

  @Override
  void freeController( @NonNull String uiName )
  {
    if ( mFavoritesControllers.get(uiName) != null )
    {
      mFavoritesControllers.get(uiName).deactivate();
    }
  }

  @NonNull
  @Override
  FavoritesControllerAddInDirectoryAPI getAddInDirectoryController( @NonNull String uiName )
  {
    FavoritesControllerAddInDirectoryAPI controller = mFavoritesAddInDirectoryControllers.get(uiName);
    if ( controller == null )
    {
      controller = new BaseFavoritesControllerAddInDirectory(this, uiName);
      mFavoritesAddInDirectoryControllers.put(uiName, controller);
    }
    controller.setArticleItemToSave(uiData.mToAddInDirectoryItem);
    controller.activate();
    return controller;
  }

  @Override
  void freeAddInDirectoryController( @NonNull String uiName )
  {
    if ( mFavoritesAddInDirectoryControllers.get(uiName) != null )
    {
      mFavoritesAddInDirectoryControllers.get(uiName).deactivate();
    }
  }

  @Override
  public void selectionModeOff()
  {
    for ( FavoritesControllerAPI controller : mFavoritesControllers.values() )
    {
      controller.selectionModeOff();
    }
  }

  @Override
  public boolean showFavoritesScreen(Context context)
  {
    if ( context != null && uiData.mFavoritesActivityClass != null )
    {
      Intent intent = new Intent(context, uiData.mFavoritesActivityClass);
      context.startActivity(intent);
      return true;
    }
    return false;
  }

  @Override
  public boolean showAddArticleInDirectoryScreen( @Nullable FragmentManager fragmentManager,
                                                  @Nullable ArticleItem itemToAdd )
  {
    if ( fragmentManager != null && itemToAdd != null && uiData.mAddInDirectoryDialogFragment != null )
    {
      uiData.mToAddInDirectoryItem = itemToAdd;
      uiData.mAddInDirectoryDialogFragment.show(fragmentManager,
                                                uiData.mAddInDirectoryDialogFragment.getClass().getSimpleName());
      return true;
    }
    return false;
  }

  private void updateState(boolean inTransition )
  {
    if ( mInTransition != inTransition )
    {
      mInTransition = inTransition;
      for ( OnStateChangedListener listener : mOnStateChangedListeners )
      {
        listener.onFavoritesStateChanged();
      }
    }
  }

  private void updateStateUsingRootDirectory()
  {
    updateState(mRootDirectory.isDeepLoading());
  }

  private void notifyRootChange( boolean isRootItemsChanged )
  {
    notifyFullRootDirectoryChange();
    if ( isRootItemsChanged )
    {
      // support for deprecated notify methods
      for ( OnFavoritesListChangesListener listener : mOnFavoritesListChangesListeners )
      {
        listener.onFavoritesListChanged(mRootDirectory.getItems());
      }
    }
  }

  private void notifyFullRootDirectoryChange()
  {
    mRootDirectorySubject.onNext(mRootDirectory);
  }

  /**
   * Notify listeners about an error.
   *
   * @param exception Exception that has occurred
   */
  private void onError( Exception exception )
  {
    for ( OnErrorListener listener : mOnErrorListeners )
    {
      listener.onFavoritesManagerError(exception);
    }
  }

  @Override
  public void onDictionaryListChanged()
  {
    mRootDirectory.deepClearItems();
    notifyRootChange(true);
    loadRootDirectoryFromSetting();
  }

  private class MutableManagerDirectory extends Directory< ArticleItem > implements EngineSerializerAPI.OnDeserializedArticlesReadyCallback
  {
    private boolean isLoading;

    MutableManagerDirectory( @Nullable MutableManagerDirectory parent, @NonNull String name )
    {
      this(parent, name, new ArrayList<>(), new ArrayList<>());
    }

    MutableManagerDirectory( @Nullable MutableManagerDirectory parent, @NonNull String name,
                             @NonNull List< Directory< ArticleItem > > childList, List< ArticleItem > items )
    {
      super(parent, name);
      mItems = new LimitedLinkedList<>(mMaxNumberOfDirWords);
      mItems.addAll(items);
      List< Directory< ArticleItem > > list = new ArrayList<>();
      for ( Directory< ArticleItem > dir : childList )
      {
        list.add(new MutableManagerDirectory(this, dir.getName(), dir.getChildList(), dir.getItems()));
      }
      this.mChildList = list;
    }

    /**
     * Constructor for PersistentDirectory
     */
    MutableManagerDirectory( @Nullable MutableManagerDirectory parent, @NonNull String name,
                             @NonNull List< PersistentDirectory > childList )
    {
      super(parent, name);
      this.mItems = new LimitedLinkedList<>(mMaxNumberOfDirWords);
      List< Directory< ArticleItem > > list = new ArrayList<>();
      for ( PersistentDirectory dir : childList )
      {
        list.add(new MutableManagerDirectory(this, dir.getName(), dir.getChildList()));
      }
      this.mChildList = list;
    }

    void deepClearItems()
    {
      mItems.clear();
      for ( Directory childDir : mChildList )
      {
        if ( childDir instanceof MutableManagerDirectory )
        {
          ( (MutableManagerDirectory) childDir ).deepClearItems();
        }
      }
    }

    @NonNull
    List< ArticleItem > getMutableArticleItems()
    {
      return mItems;
    }

    @NonNull
    List< Directory< ArticleItem > > getMutableChildList()
    {
      return mChildList;
    }

    void saveDeepDirectoryItems() throws ManagerInitException, LocalResourceUnavailableException
    {
      saveDirectoryItems();
      for ( Directory< ArticleItem > childDir : getChildList() )
      {
        if ( childDir instanceof MutableManagerDirectory )
        {
          ( (MutableManagerDirectory) childDir ).saveDeepDirectoryItems();
        }
      }
    }

    // NOTE: It is only for testmode
    void distributeItemsInChildDirsForTestmode( @NonNull List< ArticleItem > articleItems )
    {
      if ( !mChildList.isEmpty() && !articleItems.isEmpty() )
      {
        int i = 0;
        int subListLength = articleItems.size() / mChildList.size();
        for ( Directory childDir : mChildList )
        {
          if ( childDir instanceof MutableManagerDirectory )
          {
            List< ArticleItem > subArticleItems =
                articleItems.subList(i * subListLength, ( i + 1 ) * subListLength);
            ( (MutableManagerDirectory) childDir ).getMutableArticleItems().addAll(subArticleItems);
            ( (MutableManagerDirectory) childDir ).distributeItemsInChildDirsForTestmode(subArticleItems);
          }
          i++;
        }
      }
    }

    void saveDirectoryItems() throws ManagerInitException, LocalResourceUnavailableException
    {
      mSettingsManager.save(createDirectoryItemsKey(this), serializeArticleItems(getItems()), true);
    }

    void deepLoadItems()
    {
      try
      {
        Serializable[] defSerializableArticlesForSettings = new PersistentArticle[0];
        String key = createDirectoryItemsKey(this);
        Serializable[] loadedWordsTmp = mSettingsManager.load(key, defSerializableArticlesForSettings);

        // it is needed to restore data that previously may been saved by old key
        if ( loadedWordsTmp == defSerializableArticlesForSettings && FAVORITES_SAVE_KEY.equals(key) )
        {
          // try to load words located by old key, migrate data to location with new key and clear data in old location
          loadedWordsTmp = mSettingsManager
              .load(BaseFavoritesManager.this.getClass().getCanonicalName(), defSerializableArticlesForSettings);
          if ( loadedWordsTmp != defSerializableArticlesForSettings )
          {
            mSettingsManager.save(key, loadedWordsTmp, true);
            mSettingsManager
                .save(BaseFavoritesManager.this.getClass().getCanonicalName(), new PersistentArticle[0], true);
          }
        }

        final Serializable[] loadedWords = loadedWordsTmp;
        if ( loadedWords.length != 0 )
        {
          isLoading = true;
          updateStateUsingRootDirectory();
          mEngineSerializer.deserializeArticleItems(loadedWords, this, true);
        }
      }
      catch ( WrongTypeException | ManagerInitException | LocalResourceUnavailableException exception )
      {
        exception.printStackTrace();
      }
      for ( Directory childDir : mChildList )
      {
        if ( childDir instanceof MutableManagerDirectory )
        {
          ( (MutableManagerDirectory) childDir ).deepLoadItems();
        }
      }
    }

    @Nullable
    MutableManagerDirectory findMutableDirectory( @Nullable Directory directory )
    {
      MutableManagerDirectory curDirectory = null;
      if ( directory != null )
      {
        Directory foundDir = findDirectory(directory.getPath());
        if ( foundDir instanceof MutableManagerDirectory )
          curDirectory = (MutableManagerDirectory) foundDir;
      }
      return curDirectory;
    }

    boolean isDeepLoading()
    {
      boolean deepLoading = isLoading;
      for ( Directory childDir : mChildList )
      {
        if ( deepLoading )
        {
          break;
        }
        if ( childDir instanceof MutableManagerDirectory )
          deepLoading = ((MutableManagerDirectory) childDir).isDeepLoading();
      }
      return deepLoading;
    }

    @Override
    public void onDeserializedArticlesReady( @NonNull ArticleItem[] articles )
    {
      mItems.clear();
      Collections.addAll(mItems, articles);
      notifyRootChange(getParent() == null);
    }

    @Override
    public void onDeserializationStatusChanged( boolean inProgress )
    {
      isLoading = inProgress;
      updateStateUsingRootDirectory();
    }
  }

  // !!! NOTE: DO NOT MODIFY this method, it's value is used as key to save directory article items
  static private String createDirectoryItemsKey( @Nullable Directory directory )
  {
    return directory == null ? "" : createDirectoryItemsKey(directory.getParent()) + directory.getName();
  }

  private class UIData
  {
    @Nullable
    private Class          mFavoritesActivityClass;
    @Nullable
    private DialogFragment mAddInDirectoryDialogFragment;
    @Nullable
    private ArticleItem    mToAddInDirectoryItem;
  }
}
