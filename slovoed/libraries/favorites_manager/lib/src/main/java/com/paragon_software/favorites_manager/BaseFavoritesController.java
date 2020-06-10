package com.paragon_software.favorites_manager;

import android.content.Context;
import android.content.Intent;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.fragment.app.FragmentManager;
import androidx.core.content.FileProvider;
import android.view.View;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.DictionaryAndDirection;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.IDictionaryIcon;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.hint_manager.HintParams;
import com.paragon_software.hint_manager.HintType;
import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.settings_manager.OnApplicationSettingsSaveListener;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.toolbar_manager.Notifiers.OnBackActionClick;
import com.paragon_software.toolbar_manager.Notifiers.OnDeleteSelectedActionClick;
import com.paragon_software.toolbar_manager.Notifiers.OnSelectAllActionClick;
import com.paragon_software.toolbar_manager.ToolbarManager;
import com.paragon_software.utils_slovoed.directory.Directory;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import io.reactivex.Observable;
import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.disposables.CompositeDisposable;
import io.reactivex.disposables.Disposable;
import io.reactivex.subjects.BehaviorSubject;

public class BaseFavoritesController extends FavoritesControllerAPI implements OnBackActionClick,
                                                                               OnDeleteSelectedActionClick,
                                                                               OnSelectAllActionClick,
                                                                               OnApplicationSettingsSaveListener,
                                                                               OnErrorListener, OnStateChangedListener,
                                                                               OnFavoritesListChangesListener
{
  private static final String FAVORITES_SORTING_SAVE_KEY = "FAVORITES_SORTING_SAVE_KEY";
  protected static final String FAVORITE_MANAGER_EXPORT_FILE_PATH = "favourites.html";
  protected static final String MIME_TYPE_TEXT = "text/plain";
  protected static final String MIME_TYPE_HTML = "text/html";

  private final @NonNull FavoritesManagerAPI mManager;
  private final @NonNull
  ScreenOpenerAPI mScreenOpener;

  private final @Nullable SettingsManagerAPI   mSettingsManager;
  protected final @Nullable ToolbarManager       mToolbarManager;
  private final @Nullable SoundManagerAPI      mSoundManager;
  private final @Nullable DictionaryManagerAPI mDictionaryManager;
  private final @Nullable HintManagerAPI       mHintManager;
  private final @Nullable FavoritesSorterAPI   mFavoritesSorter;
  private final @Nullable Class<IExportHtmlBuilder> mExportHtmlBuilderClass;

  private final List< OnControllerFavoritesListChangedListener >     mOnFavoritesListChangedListeners               = new ArrayList<>();
  private final List< OnControllerSelectionModeChangedListener >     mOnSelectionModeChangedListeners               = new ArrayList<>();
  private final List< OnControllerSortingActionChange >              mOnFavoritesSortingActionChanges               = new ArrayList<>();
  private final List< OnControllerDeleteAllActionChange >            mOnDeleteAllActionChanges                      = new ArrayList<>();
  private final List< OnControllerShowDeleteSelectedDialogListener > mOnShowDeleteSelectedDialogListeners           = new ArrayList<>();
  private final List< OnControllerEntryListFontSizeChangeListener >  mOnControllerEntryListFontSizeChangeListeners  = new ArrayList<>();
  private final List< OnControllerTransitionStateChangedListener >   mOnControllerTransitionStateChangedListeners   = new ArrayList<>();
  private final List< OnControllerSortingChangeListener >            mOnControllerFavoritesSortingChangeListeners   = new ArrayList<>();
  private final List< OnControllerShareActionChange >                mOnControllerShareChangeListeners              = new ArrayList<>();
  private final List< OnControllerErrorListener >                    mOnControllerErrorListeners                    = new ArrayList<>();

  private          float               mEntryListFontSize = ApplicationSettings.getDefaultFontSize();
  private          boolean             mInTransition;
  private @NonNull FavoritesSorting    mFavoritesSorting  = FavoritesSorting.BY_DATE_DESCENDING;
  private          boolean             mSelectionMode     = false;
  private          List< ArticleItem > mSelectedWords     = new ArrayList<>();

  private boolean mSortingActionEnabled;
  private int     mSortingActionStatus = View.GONE;
  private boolean mDeleteAllActionEnabled;
  private int     mDeleteAllActionStatus = View.GONE;
  private boolean mShareActionEnable;
  private int     mShareActionStatus = View.GONE;


  @Nullable
  private Disposable                   mRootDirDisposable;
  private BehaviorSubject< Directory< ArticleItem > > mCurrentDirSubject = BehaviorSubject.create();
  private BehaviorSubject< Boolean >   mEditModeSubject = BehaviorSubject.createDefault(false);


  protected String mControllerId;
  private String mFileProviderAuthorities;

  private CompositeDisposable mCompositeDisposable = new CompositeDisposable();

  public BaseFavoritesController( @NonNull FavoritesManagerAPI manager, @NonNull ScreenOpenerAPI screenOpener,
                                  @Nullable SettingsManagerAPI settingsManager, @Nullable DictionaryManagerAPI dictionaryManager,
                                  @Nullable ToolbarManager toolbarManager, @Nullable SoundManagerAPI soundManager, @Nullable HintManagerAPI hintManager,
                                  @Nullable FavoritesSorterAPI favoritesSorter, @Nullable Class<IExportHtmlBuilder> exportHtmlBuilderClass,
                                  @NonNull String controllerId, @Nullable String fileProvider)
  {
    mManager = manager;
    mScreenOpener = screenOpener;
    mToolbarManager = toolbarManager;
    mSettingsManager = settingsManager;
    mSoundManager = soundManager;
    mDictionaryManager = dictionaryManager;
    mHintManager = hintManager;
    mFavoritesSorter = favoritesSorter;
    mExportHtmlBuilderClass = exportHtmlBuilderClass;
    mControllerId = controllerId;
    mFileProviderAuthorities = fileProvider;
    loadParameters();
    subscribeToRootDir();
  }

  @Override
  public void registerNotifier( @NonNull Notifier notifier )
  {
    if (notifier instanceof OnControllerFavoritesListChangedListener && !mOnFavoritesListChangedListeners.contains(notifier))
    {
      mOnFavoritesListChangedListeners.add((OnControllerFavoritesListChangedListener) notifier);
    }
    if (notifier instanceof OnControllerSelectionModeChangedListener && !mOnSelectionModeChangedListeners.contains(notifier))
    {
      mOnSelectionModeChangedListeners.add((OnControllerSelectionModeChangedListener) notifier);
    }
    if (notifier instanceof OnControllerEntryListFontSizeChangeListener && !mOnControllerEntryListFontSizeChangeListeners.contains(notifier))
    {
      mOnControllerEntryListFontSizeChangeListeners.add((OnControllerEntryListFontSizeChangeListener) notifier);
    }
    if (notifier instanceof OnControllerSortingActionChange && !mOnFavoritesSortingActionChanges.contains(notifier))
    {
      mOnFavoritesSortingActionChanges.add((OnControllerSortingActionChange)notifier);
    }
    if (notifier instanceof OnControllerDeleteAllActionChange && !mOnDeleteAllActionChanges.contains(notifier))
    {
      mOnDeleteAllActionChanges.add((OnControllerDeleteAllActionChange)notifier);
    }
    if (notifier instanceof OnControllerShowDeleteSelectedDialogListener && !mOnShowDeleteSelectedDialogListeners.contains(notifier))
    {
      mOnShowDeleteSelectedDialogListeners.add((OnControllerShowDeleteSelectedDialogListener)notifier);
    }
    if (notifier instanceof OnControllerSortingChangeListener
        && !mOnControllerFavoritesSortingChangeListeners.contains(notifier))
    {
      mOnControllerFavoritesSortingChangeListeners.add((OnControllerSortingChangeListener) notifier);
    }
    if (notifier instanceof OnControllerShareActionChange
        && !mOnControllerShareChangeListeners.contains(notifier))
    {
      mOnControllerShareChangeListeners.add((OnControllerShareActionChange) notifier);
    }
    if (notifier instanceof OnControllerTransitionStateChangedListener && !mOnControllerTransitionStateChangedListeners.contains(notifier))
    {
      mOnControllerTransitionStateChangedListeners.add((OnControllerTransitionStateChangedListener) notifier);
    }
    if (notifier instanceof OnControllerErrorListener && !mOnControllerErrorListeners.contains(notifier))
    {
      mOnControllerErrorListeners.add((OnControllerErrorListener) notifier);
    }
  }

  @Override
  public void unregisterNotifier( @NonNull Notifier notifier )
  {
    if (notifier instanceof OnControllerFavoritesListChangedListener)
    {
      mOnFavoritesListChangedListeners.remove(notifier);
    }
    if (notifier instanceof OnControllerSelectionModeChangedListener)
    {
      mOnSelectionModeChangedListeners.remove(notifier);
    }
    if (notifier instanceof OnControllerEntryListFontSizeChangeListener)
    {
      mOnControllerEntryListFontSizeChangeListeners.remove(notifier);
    }
    if (notifier instanceof OnControllerSortingActionChange )
    {
      mOnFavoritesSortingActionChanges.remove(notifier);
    }
    if (notifier instanceof OnControllerDeleteAllActionChange)
    {
      mOnDeleteAllActionChanges.remove(notifier);
    }
    if (notifier instanceof OnControllerShowDeleteSelectedDialogListener)
    {
      mOnShowDeleteSelectedDialogListeners.remove(notifier);
    }
    if (notifier instanceof OnControllerSortingChangeListener )
    {
      mOnControllerFavoritesSortingChangeListeners.remove(notifier);
    }
    if (notifier instanceof OnControllerShareActionChange )
    {
      mOnControllerShareChangeListeners.remove(notifier);
    }
    if (notifier instanceof OnControllerTransitionStateChangedListener)
    {
      mOnControllerTransitionStateChangedListeners.remove(notifier);
    }
    if (notifier instanceof OnControllerErrorListener)
    {
      mOnControllerErrorListeners.remove(notifier);
    }
  }

  /**
   * Update controllers state.
   */
  private void updateState()
  {
    updateControllerWords();
    updateControllerTransitionState();
    updateToolbar();
    updateControllerEntryListFontSize();
  }

  /**
   * Update list of favorites words in active controller. Set list of words sorted by specified sorting type and
   * filtered by specified dictionary.
   */
  private void updateControllerWords()
  {
    Directory< ArticleItem > resDir = mCurrentDirSubject.getValue();
    List<ArticleItem> prevItems = new ArrayList<>(resDir.getItems());
    List<Directory<ArticleItem>> prevDir = new ArrayList<>(resDir.getChildList());
    if ( resDir instanceof ControllerDirectory ) {
      ((ControllerDirectory) resDir).updateControllerWords();
      ((ControllerDirectory) resDir).updateControllerChildList();
    }
    if ( !prevItems.equals(resDir.getItems()) || !prevDir.equals(resDir.getChildList()) )
    {
      mCurrentDirSubject.onNext(resDir);
      for ( OnControllerFavoritesListChangedListener listener : mOnFavoritesListChangedListeners )
      {
        listener.onControllerFavoritesListChanged(getWords());
      }
    }
  }

  /**
   * Update entry list font size parameter for controller. Load this parameter using {@link SettingsManagerAPI}
   * object.
   */
  private void updateControllerEntryListFontSize()
  {
    if ( mSettingsManager != null )
    {
      ApplicationSettings applicationSettings = mSettingsManager.loadApplicationSettings();
      setEntryListFontSize(applicationSettings.getEntryListFontSize());
    }
  }

  private void updateControllerTransitionState()
  {
    if ( mInTransition != mManager.isInTransition())
    {
      mInTransition = mManager.isInTransition();
      for ( OnControllerTransitionStateChangedListener listener : mOnControllerTransitionStateChangedListeners )
      {
        listener.onControllerTransitionStateChanged(mInTransition);
      }
    }
  }

  /**
   * Update toolbar manager state. Show toolbar in selection mode or with dictionary list spinner.
   */
  private void updateToolbar()
  {
    if ( isInSelectionMode() )
    {
      setSortingActionVisibilityStatus(View.GONE);
      setDeleteAllActionVisibilityStatus(View.GONE);
      setShareActionVisibilityStatus(View.GONE);
      if (mToolbarManager != null)
      {
        mToolbarManager.showSelectionMode(getSelectedWords().size());
      }
    }
    else
    {
      setDefaultToolbarState();

      Directory<ArticleItem> rootDirectory = getRootDirectory();

      setSortingActionVisibilityStatus(View.VISIBLE);
      setSortingActionEnable((!getWords().isEmpty() || !getCurrentDirectory().getChildList().isEmpty()) && !mInTransition);
      setDeleteAllActionVisibilityStatus(View.VISIBLE);
      setDeleteAllActionEnable(!getWords().isEmpty() && !mInTransition);
      setShareActionVisibilityStatus(View.VISIBLE);
      setShareActionEnable(rootDirectory != null && (!rootDirectory.getItems().isEmpty() || !rootDirectory.getChildList().isEmpty()) && !mInTransition);
    }
  }

  protected void setDefaultToolbarState() {
    if (mToolbarManager != null) {
      mToolbarManager.showDictionaryListNoDirection();
    }
  }

  @Override
  public boolean isInSelectionMode()
  {
    return mSelectionMode;
  }

  @Override
  public void openArticle( int articleIndex, Context context )
  {
    mScreenOpener.showArticleFromSeparateList( getWords(), articleIndex, context );
  }

  @NonNull
  @Override
  public List< ArticleItem > getWords()
  {
    return mCurrentDirSubject.getValue() != null ? mCurrentDirSubject.getValue().getItems() : new ArrayList<>();
  }

  @NonNull
  @Override
  public List< ArticleItem > getSelectedWords()
  {
    return Collections.unmodifiableList(mSelectedWords);
  }

  @Override
  public float getEntryListFontSize()
  {
    return mEntryListFontSize;
  }

  @Override
  public FavoritesSorting getFavoritesSorting()
  {
    return mFavoritesSorting;
  }

  @Override
  public boolean isInTransition()
  {
    return mInTransition;
  }

  @Override
  public void deleteAllWords()
  {
    mManager.removeWords(getWords(), mCurrentDirSubject.getValue());
  }

  @Override
  public void deleteWord( int articleIndex )
  {
    mManager.removeWord(getWords().get(articleIndex), mCurrentDirSubject.getValue());
  }

  @Override
  public void deleteSelectedWords()
  {
    if ( isInSelectionMode() && !getSelectedWords().isEmpty())
    {
      mManager.removeWords(getSelectedWords(), mCurrentDirSubject.getValue());
      setSelectionMode(false);
    }
  }

  @Override
  public void selectItem( int articleIndex )
  {
    ArticleItem word = getWords().size() > articleIndex ? getWords().get(articleIndex) : null;
    if ( word != null && !mSelectedWords.contains(word) )
    {
      mSelectedWords.add(word);
      for ( OnControllerSelectionModeChangedListener listener : mOnSelectionModeChangedListeners)
      {
        listener.onSelectedListChanged(mSelectedWords);
      }
    }
    setSelectionMode(true);
  }

  @Override
  public void unselectItem( int articleIndex )
  {
    ArticleItem word = getWords().get(articleIndex);
    if (mSelectedWords.contains(word)) {
      mSelectedWords.remove(word);
      for ( OnControllerSelectionModeChangedListener listener : mOnSelectionModeChangedListeners)
      {
        listener.onSelectedListChanged(mSelectedWords);
      }
    }
    setSelectionMode(true);
  }

  @Override
  public boolean hasSound( int articleIndex )
  {
    if ( mSoundManager != null && articleIndex < getWords().size() )
    {
      ArticleItem word = getWords().get(articleIndex);
      return mSoundManager.itemHasSound(word);
    }
    return false;
  }

  @Override
  public void playSound( int articleIndex )
  {
    if ( mSoundManager != null && articleIndex < getWords().size() )
    {
      ArticleItem word = getWords().get(articleIndex);
      mSoundManager.playSound(word);
    }
  }

  @Override
  public void selectionModeOff()
  {
    if (mSelectionMode)
    {
      setSelectionMode(false);
    }
  }

  @Override
  public void selectSorting( int element )
  {
    if (FavoritesSorting.values().length > element)
    {
      changeSorting(FavoritesSorting.values()[element]);
    }
  }

  private void changeSorting( FavoritesSorting sorting )
  {
    if ( sorting != null && mFavoritesSorting != sorting )
    {
      mFavoritesSorting = sorting;
      for ( OnControllerSortingChangeListener listener : mOnControllerFavoritesSortingChangeListeners )
      {
        listener.onFavoritesSortingChanged();
      }
      updateControllerWords();
      saveFavoritesSorting();
    }
  }

  /**
   * Save favorites sorting type parameter using registered {@link SettingsManagerAPI} object.
   */
  private void saveFavoritesSorting()
  {
    if (mSettingsManager != null)
    {
      try
      {
        mSettingsManager.save(FAVORITES_SORTING_SAVE_KEY + mControllerId, mFavoritesSorting, true);
      }
      catch ( ManagerInitException | LocalResourceUnavailableException exception )
      {
        exception.printStackTrace();
      }
    }
  }

  /**
   * Load parameters related to the controller.
   */
  private void loadParameters()
  {
    if (mSettingsManager != null)
    {
      try
      {
        mFavoritesSorting = mSettingsManager.load(FAVORITES_SORTING_SAVE_KEY + mControllerId, FavoritesSorting.BY_DATE_DESCENDING);
      }
      catch ( WrongTypeException | ManagerInitException exception )
      {
        exception.printStackTrace();
      }
    }
  }

  @Override
  public int getSortingActionVisibilityStatus()
  {
    return mSortingActionStatus;
  }

  @Override
  public boolean isSortingActionEnable()
  {
    return mSortingActionEnabled;
  }

  @Override
  public int getDeleteAllActionVisibilityStatus()
  {
    return mDeleteAllActionStatus;
  }

  @Override
  public boolean isDeleteAllActionEnable()
  {
    return mDeleteAllActionEnabled;
  }

  @Override
  public int getShareActionVisibilityStatus()
  {
    return mShareActionStatus;
  }

  @Override
  public boolean isShareActionEnable()
  {
    return mShareActionEnable;
  }

  @NonNull
  @Override
  public Observable< Directory< ArticleItem > > getCurrentDirectoryObservable()
  {
    return mCurrentDirSubject;
  }

  @Nullable
  @Override
  public Directory< ArticleItem > getCurrentDirectory()
  {
    return mCurrentDirSubject.getValue();
  }

  @Override
  public void goToDirectory( @NonNull Directory< ArticleItem > destination )
  {
    if ( mCurrentDirSubject.getValue() != null )
    {
      Directory< ArticleItem > res = mCurrentDirSubject.getValue().findDirectory(destination.getPath());
      if ( res != null && !res.equals(mCurrentDirSubject.getValue()) )
      {
        updateCurDir(new ControllerDirectory(res));
        updateToolbar();
      }
    }
  }

  @Override
  public void goToRootDirectory() {
    Directory<ArticleItem> rootDirectory = this.getRootDirectory();
    if (rootDirectory != null) {
      Directory<ArticleItem> currentDirectory = getCurrentDirectory();
      if (currentDirectory != null && currentDirectory.equals(rootDirectory)) {
        return;
      }
      updateCurDir(new ControllerDirectory(rootDirectory));
      updateToolbar();
    }
  }

  @Override
  public boolean hasChildDirectory( @Nullable String name )
  {
    return mManager.hasChildDirectory(mCurrentDirSubject.getValue(), name);
  }

  @Override
  public boolean addDirectory( @NonNull String name )
  {
    if ( mCurrentDirSubject.getValue() != null )
    {
      return mManager.addDirectory(mCurrentDirSubject.getValue(), name);
    }
    return false;
  }

  @Override
  public  boolean deleteDirectory( @NonNull Directory< ArticleItem > toDelete )
  {
    return mManager.removeDirectory(toDelete);
  }

  @NonNull
  @Override
  public Observable< Boolean > getEditModeStatusObservable()
  {
    return mEditModeSubject;
  }

  @Override
  public void setEditModeStatus( boolean editModeOn )
  {
    if ( !Boolean.valueOf(editModeOn).equals(mEditModeSubject.getValue()) )
    {
      mEditModeSubject.onNext(editModeOn);
      updateToolbar();
    }
  }

  @Override
  public boolean isInEditMode()
  {
    return mEditModeSubject.getValue();
  }

  @Override
  public void exportFavorites( @NonNull Context context, ExportParams params )
  {
    IExportHtmlBuilder builder = createExportHtmlBuilder();
    Directory<ArticleItem> rootDirectory = getRootDirectory();
    if( builder == null || mFavoritesSorter == null || rootDirectory == null)
        return;

    try
    {
      File file = new File(context.getFilesDir(), FAVORITE_MANAGER_EXPORT_FILE_PATH);
      if( file.exists() )
      {
        file.delete();
      }
      file.createNewFile();

      FileOutputStream writer = new FileOutputStream(file, true);
      writer.write(builder
              .addHeader(params.getHeader())
              .addDirectory(rootDirectory)
              .addFooter(params.getFooter())
              .addPartOfSpeechColor(params.getPartOfSpeechColor())
              .addFavoritesSorter(mFavoritesSorter, mFavoritesSorting)
              .createHTMLString()
              .getBytes()
      );

      Intent exportIntent = getIntentForExport(MIME_TYPE_HTML);
      exportIntent.putExtra(Intent.EXTRA_SUBJECT, params.getSubject());
      exportIntent.putExtra(Intent.EXTRA_TEXT, params.getExportFileDescription());
      exportIntent.putExtra(
              Intent.EXTRA_STREAM,
              FileProvider.getUriForFile(context, mFileProviderAuthorities, file));
      context.startActivity(Intent.createChooser(exportIntent, params.getChooserTitle()));
    } catch (IOException ignore) { }
  }

  @Override
  public void exportFavoritesAsString(@NonNull Context context, ExportParams params) {
    IExportHtmlBuilder builder = createExportHtmlBuilder();
    Directory<ArticleItem> rootDirectory = getRootDirectory();
    if( builder == null || mFavoritesSorter == null || rootDirectory == null)
      return;

    Intent exportIntent = getIntentForExport(MIME_TYPE_TEXT);
    exportIntent.putExtra(Intent.EXTRA_SUBJECT, params.getSubject());
//    Adding content to message
    String intentContent = builder
            .addHeader(params.getHeader())
            .addDirectory(rootDirectory)
            .addFooter(params.getFooter())
            .addPartOfSpeechColor(params.getPartOfSpeechColor())
            .addFavoritesSorter(mFavoritesSorter, mFavoritesSorting)
            .toString()
            .trim();
    exportIntent.putExtra(Intent.EXTRA_TEXT, intentContent);

    context.startActivity(Intent.createChooser(exportIntent, params.getChooserTitle()));
  }

  @VisibleForTesting
  protected Intent getIntentForExport(String mimeTypes) {
    Intent exportIntent = new Intent(Intent.ACTION_SEND);
    exportIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    exportIntent.setType(mimeTypes);
    return exportIntent;
  }

  @Nullable
  private IExportHtmlBuilder createExportHtmlBuilder()
  {
      if( mExportHtmlBuilderClass == null )
          return null;

      try {
          return mExportHtmlBuilderClass.newInstance();
      }
      catch (IllegalAccessException | InstantiationException ignore) { }

      return null;
  }

  @Nullable
  private Directory<ArticleItem> getRootDirectory()
  {
    return mManager.getRootDirectoryObservable().blockingFirst();
  }

  @Override
  public void activate()
  {
    subscribeToRootDir();
    mManager.registerNotifier(this);
    if ( mToolbarManager != null)
    {
      mToolbarManager.registerNotifier(this);
    }
    if ( mSettingsManager != null)
    {
      mSettingsManager.registerNotifier(this);
    }
    if (mDictionaryManager != null) {
      mCompositeDisposable.add(mDictionaryManager.getDictionaryAndDirectionChangeObservable()
              .observeOn(AndroidSchedulers.mainThread())
              .subscribe(flag -> updateState()));
    }
    updateState();
  }

  @Override
  public void deactivate()
  {
    mManager.unregisterNotifier(this);
    if ( mToolbarManager != null)
    {
      mToolbarManager.unRegisterNotifier(this);
    }
    if ( mSettingsManager != null)
    {
      mSettingsManager.unregisterNotifier(this);
    }
    if ( mRootDirDisposable != null )
    {
      mRootDirDisposable.dispose();
      mRootDirDisposable = null;
    }
    mCompositeDisposable.clear();
  }

  private void subscribeToRootDir()
  {
    if ( mRootDirDisposable == null )
    {
      mRootDirDisposable = mManager.getRootDirectoryObservable().subscribe(directory -> {
        Directory< ArticleItem > resDir = directory;
        if ( mCurrentDirSubject.getValue() != null )
        {
          Directory< ArticleItem > curDir = directory.findDirectory(mCurrentDirSubject.getValue().getPath());
          // curDir may be not found in root dir (it could been deleted). In this case stay at root dir.
          if ( curDir != null )
          {
            resDir = curDir;
          }
        }
        updateCurDir(new ControllerDirectory(resDir));
        updateToolbar();
      });
    }
  }

  private void updateCurDir( ControllerDirectory newDir )
  {
    // not using equals checking between Directory and ArticleItem object because
    // it's too slow (and ArticleItem equals method is buggy and not complete)
    // and emitting redundant update notifications is much faster
    mCurrentDirSubject.onNext(newDir);
    for ( OnControllerFavoritesListChangedListener listener : mOnFavoritesListChangedListeners )
    {
      listener.onControllerFavoritesListChanged(getWords());
    }
  }

  void setSelectedWords( @NonNull List< ArticleItem > words )
  {
    mSelectedWords.clear();
    mSelectedWords.addAll(words);
    for ( OnControllerSelectionModeChangedListener listener : mOnSelectionModeChangedListeners)
    {
      listener.onSelectedListChanged(mSelectedWords);
    }
  }

  void setSelectionMode( boolean selectionMode )
  {
    if (mSelectionMode != selectionMode)
    {
      mSelectionMode = selectionMode;
      for ( OnControllerSelectionModeChangedListener listener : mOnSelectionModeChangedListeners )
      {
        listener.onSelectionModeChanged(mSelectionMode);
      }
    }
    if (!mSelectionMode && !mSelectedWords.isEmpty()) {
      setSelectedWords(new ArrayList<>());
    }
    updateToolbar();
  }

  void setEntryListFontSize( float entryListFontSize )
  {
    if (mEntryListFontSize != entryListFontSize)
    {
      mEntryListFontSize = entryListFontSize;
      for ( OnControllerEntryListFontSizeChangeListener listener : mOnControllerEntryListFontSizeChangeListeners )
      {
        listener.onControllerEntryListFontSizeChanged();
      }
    }
  }

  private void setSortingActionEnable( boolean enabled )
  {
    if ( mSortingActionEnabled != enabled )
    {
      mSortingActionEnabled = enabled;
      notifyFavoritesSortingEnabledChanged();
    }
  }

  private void setSortingActionVisibilityStatus( int status )
  {
    if ( mSortingActionStatus != status )
    {
      mSortingActionStatus = status;
      notifyFavoritesSortingActionStatusChanged();
    }
  }

  private void setShareActionEnable( boolean enabled )
  {
    if ( mShareActionEnable != enabled )
    {
      mShareActionEnable = enabled;
      notifyFavoritesShareEnabledChanged();
    }
  }

  private void setShareActionVisibilityStatus( int status )
  {
    if ( mShareActionStatus != status )
    {
      mShareActionStatus = status;
      notifyFavoritesShareActionStatusChanged();
    }
  }


  private void setDeleteAllActionEnable( boolean enabled )
  {
    if ( mDeleteAllActionEnabled != enabled )
    {
      mDeleteAllActionEnabled = enabled;
      notifyDeleteAllActionEnabledChanged();
    }
  }

  private void setDeleteAllActionVisibilityStatus( int status )
  {
    if ( mDeleteAllActionStatus != status )
    {
      mDeleteAllActionStatus = status;
      notifyDeleteAllActionStatusChanged();
    }
  }

  @Override
  public IDictionaryIcon getDictionaryIcon(Dictionary.DictionaryId dictionaryId)
  {
    if (mDictionaryManager != null)
    {
      Collection< Dictionary > dictionaries = mDictionaryManager.getDictionaries();
      for ( Dictionary dictionary : dictionaries )
      {
        if ( dictionary.getId().equals(dictionaryId) )
        {
          return dictionary.getIcon();
        }
      }
    }
    return null;
  }

  @Override
  public boolean showHintManagerDialog( @NonNull HintType hintType, @Nullable FragmentManager fragmentManager, @Nullable HintParams hintParams )
  {
    if ( mHintManager != null )
    {
      return mHintManager.showHintDialog(hintType, fragmentManager, hintParams);
    }
    return false;
  }

  private void notifyFavoritesSortingActionStatusChanged()
  {
    for (final OnControllerSortingActionChange listener : mOnFavoritesSortingActionChanges)
    {
      listener.onSortingActionVisibilityStatusChange();
    }
  }

  private void notifyFavoritesSortingEnabledChanged()
  {
    for (final OnControllerSortingActionChange listener : mOnFavoritesSortingActionChanges)
    {
      listener.onSortingActionEnableStatusChange();
    }
  }

  private void notifyFavoritesShareActionStatusChanged()
  {
    for (final OnControllerShareActionChange listener : mOnControllerShareChangeListeners)
    {
      listener.onShareActionVisibilityStatusChange();
    }
  }

  private void notifyFavoritesShareEnabledChanged()
  {
    for (final OnControllerShareActionChange listener : mOnControllerShareChangeListeners)
    {
      listener.onShareActionEnableStatusChange();
    }
  }

  private void notifyDeleteAllActionStatusChanged()
  {
    for (final OnControllerDeleteAllActionChange listener : mOnDeleteAllActionChanges)
    {
      listener.onDeleteAllActionVisibilityStatusChange();
    }
  }

  private void notifyDeleteAllActionEnabledChanged()
  {
    for (final OnControllerDeleteAllActionChange listener : mOnDeleteAllActionChanges)
    {
      listener.onDeleteAllActionEnableStatusChange();
    }
  }

  @Override
  public void onDeleteSelectedActionClick()
  {
    if ( isInSelectionMode() && !getSelectedWords().isEmpty() )
    {
      for ( OnControllerShowDeleteSelectedDialogListener listener : mOnShowDeleteSelectedDialogListeners )
      {
        listener.onControllerShowDeleteSelectedDialog();
      }
    }
  }

  @Override
  public void onApplicationSettingsSaved( @NonNull ApplicationSettings applicationSettings )
  {
    setEntryListFontSize(applicationSettings.getEntryListFontSize());
  }

  @Override
  public void onFavoritesManagerError( Exception exception )
  {
    onError(exception);
  }

  private void onError( Exception exception )
  {
    for ( OnControllerErrorListener listener : mOnControllerErrorListeners)
    {
      listener.onControllerError(exception);
    }
  }

  @Override
  public void onFavoritesStateChanged()
  {
    updateControllerTransitionState();
    updateToolbar();
  }

  @Override
  public void onSelectAllActionClick()
  {
    if ( isInSelectionMode() )
    {
      setSelectedWords(getWords());
      updateToolbar();
    }
  }

  @Override
  public void onBackActionClick()
  {
    if ( isInSelectionMode())
    {
      setSelectionMode(false);
      updateToolbar();
    }
  }

  @Override
  public void onFavoritesListChanged( List< ArticleItem > favoritesWords )
  {
    updateState();
  }

  @VisibleForTesting
  void setWords( @NonNull List< ArticleItem > collection )
  {
    Directory< ArticleItem > resDir = mCurrentDirSubject.getValue();
    if ( resDir instanceof ControllerDirectory )
    {
      ((ControllerDirectory) resDir).setArticleItems(collection);
      ((ControllerDirectory) resDir).mControllerArticleItems = collection;
    }
    mCurrentDirSubject.onNext(resDir);
    for ( OnControllerFavoritesListChangedListener listener : mOnFavoritesListChangedListeners)
    {
      listener.onControllerFavoritesListChanged(getWords());
    }
  }

  @VisibleForTesting
  void setDirs( @NonNull List<Directory< ArticleItem >> collection )
  {
    Directory< ArticleItem > resDir = mCurrentDirSubject.getValue();
    if ( resDir instanceof ControllerDirectory )
    {
      ((ControllerDirectory) resDir).setDirItems(collection);
      ((ControllerDirectory) resDir).mControllerChildList = collection;
    }
    mCurrentDirSubject.onNext(resDir);
    for ( OnControllerFavoritesListChangedListener listener : mOnFavoritesListChangedListeners)
    {
      listener.onControllerFavoritesListChanged(getWords());
    }
  }

  private class ControllerDirectory extends Directory< ArticleItem >
  {
    List< Directory< ArticleItem > > mControllerChildList = new ArrayList<>();

    List< ArticleItem > mControllerArticleItems = new ArrayList<>();

    ControllerDirectory( @NonNull Directory< ArticleItem > toClone )
    {
      super(toClone.getParent(), toClone.getName());
      mChildList = new ArrayList<>(toClone.getChildList());
      mItems = new ArrayList<>(toClone.getItems());
      updateControllerWords();
      updateControllerChildList();
    }

    // Only for tests
    @VisibleForTesting
    void setArticleItems( List< ArticleItem > articleItems )
    {
      mItems = articleItems;
    }

    // Only for tests
    @VisibleForTesting
    void setDirItems( List< Directory<ArticleItem >> dirItem )
    {
      mChildList = dirItem;
    }

    void updateControllerWords()
    {
      mControllerArticleItems = getSortedFavoritesWords(mItems);
    }

    private void updateControllerChildList() {
      mControllerChildList = getSortedFavoritesDirectory(mChildList);
    }

    private List<Directory<ArticleItem>> getSortedFavoritesDirectory(List<Directory<ArticleItem>> directories) {
      List<Directory<ArticleItem>> dirList = new LinkedList<>(directories);
      if (mFavoritesSorter != null && dirList.size() > 1) {
        return mFavoritesSorter.sortDirectories(dirList, mFavoritesSorting);
      } else {
        return dirList;
      }
    }

    @NonNull
    @Override
    public List< Directory< ArticleItem > > getChildList()
    {
      return Collections.unmodifiableList(mControllerChildList);
    }

    @NonNull
    @Override
    public List< ArticleItem > getItems()
    {
      return Collections.unmodifiableList(mControllerArticleItems);
    }

    private List< ArticleItem > getSortedFavoritesWords(List< ArticleItem > words)
    {
      List< ArticleItem > filteredWords = getFilteredFavoritesWords(words);
      if (mFavoritesSorter != null && filteredWords.size() > 1)
      {
        return mFavoritesSorter.sort(filteredWords, mFavoritesSorting);
      }
      else
      {
        return filteredWords;
      }
    }

    /**
     * Get list of favorites words filtered by specified dictionary.
     * @return List of favorites words filtered by specified dictionary or list of all favorites words if
     * dictionary is not specified
     */
    private List< ArticleItem > getFilteredFavoritesWords(List< ArticleItem > words) {
      List< ArticleItem > filteredList = new LinkedList<>();
      if ( mDictionaryManager == null || mDictionaryManager.isSelectAllDictionaries() )
      {
        filteredList = new LinkedList<>(words);
      }
      else
      {
        DictionaryAndDirection dictAndDirection = mDictionaryManager.getDictionaryAndDirectionSelectedByUser();
        for ( ArticleItem item : words )
        {
          if ( dictAndDirection != null && item.getDictId().equals(dictAndDirection.getDictionaryId())  )
          {
            filteredList.add(item);
          }
        }
      }
      return filteredList;
    }

    @Override
    public boolean equals( Object obj )
    {
      if ( !super.equals(obj) )
      {
        return false;
      }
      if ( obj == null || getClass() != obj.getClass() )
      {
        return false;
      }
      ControllerDirectory that = (ControllerDirectory) obj;
      return that.toString().equals(this.toString());
    }

    @Override
    public String toString()
    {
      return super.toString() + " [ ControllerDirectory{" + "mControllerChildList="
          + mControllerChildList + ", mControllerArticleItems=" + mControllerArticleItems + "'} ]";
    }
  }
}
