package com.paragon_software.history_manager;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Iterator;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.native_engine.EngineSerializerAPI;
import com.paragon_software.native_engine.PersistentArticle;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;
import com.paragon_software.utils_slovoed.collections.LimitedLinkedList;

public class BaseHistoryManager extends HistoryManagerAPI implements
                                                          DictionaryManagerAPI.IDictionaryListObserver,
                                                          EngineSerializerAPI.OnDeserializedArticlesReadyCallback
{
  private static final String HISTORY_SAVE_KEY = "HISTORY_SAVE_KEY";

  @NonNull
  private final UIData mUIData = new UIData();
  private final String mAuthoritiesFileProvider;

  private ScreenOpenerAPI mScreenOpener;
  private EngineSerializerAPI   mEngineSerializer;
  private SettingsManagerAPI    mSettingsManager;
  private DictionaryManagerAPI  mDictionaryManager;

  private @Nullable  ToolbarManager        mToolbarManager;
  private @Nullable  SoundManagerAPI       mSoundManager;
  private @Nullable  HintManagerAPI        mHintManager;

  private @Nullable  Class<IExportHTMLBuilder>    mExportHtmlBuilderClass;

  private final List< OnStateChangedListener >         mOnStateChangedListeners         = new ArrayList<>();
  private final List< OnHistoryListChangedListener >   mOnHistoryListChangesListeners   = new ArrayList<>();
  private final List< OnErrorListener >                mOnErrorListeners                = new ArrayList<>();

  private final Map< String, HistoryControllerAPI >     mHistoryControllers   = new HashMap<>();

  private volatile boolean mInTransition = false;
  private final LinkedList< ArticleItem > mHistoryWords;

  public BaseHistoryManager(ScreenOpenerAPI screenOpener, EngineSerializerAPI engineSerializer,
                            SettingsManagerAPI settingsManager, DictionaryManagerAPI dictionaryManager,
                            @Nullable HintManagerAPI hintManager,
                            String authoritiesFileProvider, int maxNumberOfWords) {
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
    mSettingsManager = settingsManager;
    mDictionaryManager = dictionaryManager;
    mDictionaryManager.registerDictionaryListObserver(this);
    mHintManager = hintManager;
    mAuthoritiesFileProvider = authoritiesFileProvider;
    mHistoryWords = new LimitedLinkedList<>(maxNumberOfWords);
    loadWordsFromSettings(new PersistentArticle[0]);
  }

  void registerToolbarManager( @Nullable ToolbarManager toolbarManager )
  {
    mToolbarManager = toolbarManager;
  }

  void registerSoundManager( @Nullable SoundManagerAPI soundManager )
  {
    mSoundManager = soundManager;
  }

  void registerUI( @Nullable Class historyActivity )
  {
    mUIData.historyActivityClass = historyActivity;
  }

  void registerExportHTMLBuilderClass(@Nullable Class<IExportHTMLBuilder> exportHTMLBuilderClass )
  {
    mExportHtmlBuilderClass = exportHTMLBuilderClass;
  }

  private < T extends Serializable > void loadWordsFromSettings( T[] defSerializableArticlesForSettings )
  {
    try
    {
      Serializable[] loadedWordsTmp = mSettingsManager.load(HISTORY_SAVE_KEY, defSerializableArticlesForSettings);
      if ( loadedWordsTmp == defSerializableArticlesForSettings )
      {
        // try to load words located by old key, migrate data to location with new key and clear data in old location
        loadedWordsTmp = mSettingsManager.load(getClass().getCanonicalName(), defSerializableArticlesForSettings);
        if ( loadedWordsTmp != defSerializableArticlesForSettings )
        {
          mSettingsManager.save(HISTORY_SAVE_KEY, loadedWordsTmp, true);
          mSettingsManager.save(getClass().getCanonicalName(), new PersistentArticle[0], true);
        }
      }
      final Serializable[] loadedWords = loadedWordsTmp;
      if ( loadedWords != null && loadedWords.length != 0 )
      {
        updateState(true);
        mEngineSerializer.deserializeArticleItems(loadedWords, BaseHistoryManager.this, true);
      }
    }
    catch ( WrongTypeException | ManagerInitException | LocalResourceUnavailableException exception )
    {
      exception.printStackTrace();
    }
  }

  private Serializable[] serializeArticleItems( List<ArticleItem> items )
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
    if (notifier instanceof OnStateChangedListener && !mOnStateChangedListeners.contains(notifier))
    {
      mOnStateChangedListeners.add((OnStateChangedListener) notifier);
    }
    if (notifier instanceof OnHistoryListChangedListener && !mOnHistoryListChangesListeners.contains(notifier))
    {
      mOnHistoryListChangesListeners.add((OnHistoryListChangedListener) notifier);
    }
    if (notifier instanceof OnErrorListener && !mOnErrorListeners.contains(notifier))
    {
      mOnErrorListeners.add((OnErrorListener) notifier);
    }
  }

  @Override
  public void unregisterNotifier( @NonNull Notifier notifier )
  {
    if (notifier instanceof OnStateChangedListener)
    {
      mOnStateChangedListeners.remove(notifier);
    }
    if (notifier instanceof OnHistoryListChangedListener)
    {
      mOnHistoryListChangesListeners.remove(notifier);
    }
    if (notifier instanceof OnErrorListener)
    {
      mOnErrorListeners.remove(notifier);
    }
  }

  @NonNull
  @Override
  public List< ArticleItem > getWords()
  {
    return Collections.unmodifiableList(mHistoryWords);
  }

  @Override
  public boolean addWord( @NonNull ArticleItem articleItem )
  {
    boolean toAdd = !mInTransition && !articleItem.isAdditional()
        && !articleItem.isSpecialAdditionalInteractive() && !articleItem.isMorphologyArticle();
    if ( toAdd )
    {
      updateState(true);
      try
      {
        LimitedLinkedList< ArticleItem > wordsToSave = (LimitedLinkedList) mHistoryWords.clone();
        wordsToSave.remove(articleItem);
        wordsToSave.add(0, articleItem);
        mSettingsManager.save(HISTORY_SAVE_KEY, serializeArticleItems(wordsToSave), true);
        mHistoryWords.remove(articleItem);
        mHistoryWords.add(0, articleItem);
        notifyHistoryListChange();
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
    return toAdd;
  }

  @Override
  public boolean addWords( @NonNull List< ArticleItem > words )
  {
    List< ArticleItem > wordsToAdd = new ArrayList<>(words);
    Iterator< ArticleItem > iterator = wordsToAdd.iterator();
    while ( iterator.hasNext() )
      if ( iterator.next().isAdditional() || iterator.next().isMorphologyArticle())
        iterator.remove();
    boolean toAdd = false;
    for ( ArticleItem item : wordsToAdd )
    {
      if ( !mHistoryWords.contains(item) )
      {
        toAdd = true;
        break;
      }
    }
    toAdd = !mInTransition && toAdd;
    if ( toAdd )
    {
      updateState(true);
      try
      {
        LimitedLinkedList< ArticleItem > wordsToSave = (LimitedLinkedList) mHistoryWords.clone();
        wordsToSave.removeAll(wordsToAdd);
        wordsToSave.addAll(0, wordsToAdd);
        mSettingsManager.save(HISTORY_SAVE_KEY, serializeArticleItems(wordsToSave), true);
        mHistoryWords.removeAll(words);
        mHistoryWords.addAll(0, wordsToAdd);
        notifyHistoryListChange();
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
    return toAdd;
  }

  @Override
  public boolean removeWords( @NonNull List< ArticleItem > words )
  {
    boolean toRemove = false;
    for ( ArticleItem item : words )
    {
      if ( mHistoryWords.contains(item) )
      {
        toRemove = true;
        break;
      }
    }
    toRemove = !mInTransition && toRemove;
    if ( toRemove )
    {
      updateState(true);
      try
      {
        LimitedLinkedList< ArticleItem > wordsToSave = (LimitedLinkedList) mHistoryWords.clone();
        wordsToSave.removeAll(words);
        mSettingsManager.save(HISTORY_SAVE_KEY, serializeArticleItems(wordsToSave), true);
        mHistoryWords.removeAll(words);
        notifyHistoryListChange();
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
  public boolean removeAllWords()
  {
    boolean toRemove = !mInTransition && !mHistoryWords.isEmpty();
    if ( toRemove )
    {
      updateState(true);
      try
      {
        LimitedLinkedList< ArticleItem > wordsToSave = (LimitedLinkedList) mHistoryWords.clone();
        wordsToSave.clear();
        mSettingsManager.save(HISTORY_SAVE_KEY, serializeArticleItems(wordsToSave), true);
        mHistoryWords.clear();
        notifyHistoryListChange();
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

  @Override
  public void selectionModeOff()
  {
    for ( HistoryControllerAPI controller : mHistoryControllers.values() )
    {
      controller.selectionModeOff();
    }
  }

  @Override
  public boolean showHistoryScreen( @Nullable Context context )
  {
    if ( mUIData.historyActivityClass != null && Activity.class.isAssignableFrom(mUIData.historyActivityClass) && context != null )
    {
      Intent intent = new Intent(context, mUIData.historyActivityClass);
      context.startActivity(intent);
      return true;
    }
    return false;
  }

  @NonNull
  @Override
  HistoryControllerAPI getController( @NonNull String uiName )
  {
    HistoryControllerAPI controller = mHistoryControllers.get(uiName);
    if (controller == null)
    {
      if ( HistoryControllerType.BILINGUAL_CONTROLLER.equals(uiName) ) {
        controller = new BilingualHistoryController(this, mScreenOpener, mSettingsManager,
                mDictionaryManager, mToolbarManager, mSoundManager,
                mHintManager, mExportHtmlBuilderClass, mAuthoritiesFileProvider);
      } else {
        controller = new BaseHistoryController(this, mScreenOpener, mSettingsManager,
                mDictionaryManager, mToolbarManager, mSoundManager,
                mHintManager, mExportHtmlBuilderClass, mAuthoritiesFileProvider);
      }
      mHistoryControllers.put(uiName, controller);
    }
    controller.activate();
    return controller;
  }

  @Override
  void freeController( @NonNull String uiName )
  {
    if ( mHistoryControllers.get(uiName) != null )
    {
      mHistoryControllers.get(uiName).deactivate();
    }
  }

  private void updateState( boolean inTransition )
  {
    mInTransition = inTransition;
    for ( OnStateChangedListener listener : mOnStateChangedListeners )
    {
      listener.onHistoryStateChanged();
    }
  }

  private void notifyHistoryListChange()
  {
    for ( OnHistoryListChangedListener listener : mOnHistoryListChangesListeners )
    {
      listener.onHistoryListChanged(mHistoryWords);
    }
  }

  /**
   * Notify listeners about an error.
   * @param exception Exception that has occurred
   */
  private void onError( Exception exception )
  {
    for ( OnErrorListener listener : mOnErrorListeners )
    {
      listener.onHistoryManagerError(exception);
    }
  }

  @Override
  public void onDictionaryListChanged()
  {
    mHistoryWords.clear();
    notifyHistoryListChange();
    loadWordsFromSettings(new PersistentArticle[0]);
  }

  @Override
  public void onDeserializedArticlesReady( @NonNull ArticleItem[] articles )
  {
    mHistoryWords.clear();
    Collections.addAll(mHistoryWords, articles);
    notifyHistoryListChange();
  }

  @Override
  public void onDeserializationStatusChanged( boolean inProgress )
  {
    updateState(inProgress);
  }

  /**
   * Class to store registered UI data of the history manager.
   */
  private static class UIData
  {
    @Nullable
    Class historyActivityClass = null;
  }
}
