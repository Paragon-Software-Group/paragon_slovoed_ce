package com.paragon_software.history_manager;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.core.app.ShareCompat;
import androidx.fragment.app.FragmentManager;
import androidx.core.content.FileProvider;
import android.view.View;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.article_manager.ShowArticleOptions;
import com.paragon_software.article_manager.SwipeMode;
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
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.toolbar_manager.Notifiers.OnBackActionClick;
import com.paragon_software.toolbar_manager.Notifiers.OnDeleteSelectedActionClick;
import com.paragon_software.toolbar_manager.Notifiers.OnSelectAllActionClick;
import com.paragon_software.toolbar_manager.ToolbarManager;
import com.paragon_software.utils_slovoed.text.LocalizedString;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.disposables.CompositeDisposable;

public class BaseHistoryController extends HistoryControllerAPI implements OnBackActionClick, OnDeleteSelectedActionClick,
                                                                           OnSelectAllActionClick,
                                                                           OnApplicationSettingsSaveListener,
                                                                           OnErrorListener, OnStateChangedListener,
                                                                           OnHistoryListChangedListener
{
  protected static final String HISTORY_MANAGER_EXPORT_FILE_PATH = "history.html";
  protected static final String MIME_TYPE_TEXT = "text/plain";
  protected static final String MIME_TYPE_HTML = "text/html";


  private final @NonNull HistoryManagerAPI mManager;
  private final @NonNull
  ScreenOpenerAPI mScreenOpener;

  private final @Nullable SettingsManagerAPI   mSettingsManager;
  protected final @Nullable ToolbarManager       mToolbarManager;
  private final @Nullable SoundManagerAPI      mSoundManager;
  private final @Nullable DictionaryManagerAPI mDictionaryManager;
  private final @Nullable HintManagerAPI       mHintManager;

  private final @Nullable Class<IExportHTMLBuilder> mExportHtmlBuilderClass;
  private final String mAuthoritiesFileProvider;

  private List< ArticleItem > mHistoryWords  = new ArrayList<>();

  private final List< OnControllerHistoryListChangedListener >       mOnHistoryListChangedListeners                = new ArrayList<>();
  private final List< OnControllerSelectionModeChangedListener >     mOnSelectionModeChangedListeners              = new ArrayList<>();
  private final List< OnControllerExportActionChangeListener> mOnExportActionChangeListener = new ArrayList<>();
  private final List< OnControllerDeleteAllActionChange >            mOnDeleteAllActionChanges                     = new ArrayList<>();
  private final List< OnControllerShowDeleteSelectedDialogListener > mOnShowDeleteSelectedDialogListeners          = new ArrayList<>();
  private final List< OnControllerEntryListFontSizeChangeListener >  mOnControllerEntryListFontSizeChangeListeners = new ArrayList<>();
  private final List< OnControllerTransitionStateChangedListener >   mOnControllerTransitionStateChangedListeners  = new ArrayList<>();
  private final List< OnControllerErrorListener >                    mOnControllerErrorListeners                   = new ArrayList<>();

  private float               mEntryListFontSize = ApplicationSettings.getDefaultFontSize();
  private boolean             mInTransition;
  private boolean             mSelectionMode     = false;
  private List< ArticleItem > mSelectedWords     = new ArrayList<>();

  private boolean mExportActionEnable;
  private int     mExportActionStatus = View.GONE;

  private boolean mDeleteAllActionEnabled;
  private int     mDeleteAllActionStatus = View.GONE;

  private CompositeDisposable mCompositeDisposable = new CompositeDisposable();

  public BaseHistoryController(@NonNull HistoryManagerAPI manager, @NonNull ScreenOpenerAPI screenOpener,
                               @Nullable SettingsManagerAPI settingsManager, @Nullable DictionaryManagerAPI dictionaryManager,
                               @Nullable ToolbarManager toolbarManager, @Nullable SoundManagerAPI soundManager,
                               @Nullable HintManagerAPI hintManager,
                               @Nullable Class<IExportHTMLBuilder> exportHTMLBuilderClass, String authoritiesFileProvider)
  {
    mManager = manager;
    mScreenOpener = screenOpener;
    mToolbarManager = toolbarManager;
    mSettingsManager = settingsManager;
    mSoundManager = soundManager;
    mHintManager = hintManager;
    mDictionaryManager = dictionaryManager;
    mHistoryWords = getFilteredHistoryWords();
    mExportHtmlBuilderClass = exportHTMLBuilderClass;
    mAuthoritiesFileProvider = authoritiesFileProvider;
  }

  @Override
  public void registerNotifier( @NonNull Notifier notifier )
  {
    if (notifier instanceof OnControllerHistoryListChangedListener && !mOnHistoryListChangedListeners.contains(notifier))
    {
      mOnHistoryListChangedListeners.add((OnControllerHistoryListChangedListener) notifier);
    }
    if (notifier instanceof OnControllerSelectionModeChangedListener && !mOnSelectionModeChangedListeners.contains(notifier))
    {
      mOnSelectionModeChangedListeners.add((OnControllerSelectionModeChangedListener) notifier);
    }
    if (notifier instanceof OnControllerShowDeleteSelectedDialogListener && !mOnShowDeleteSelectedDialogListeners.contains(notifier))
    {
      mOnShowDeleteSelectedDialogListeners.add((OnControllerShowDeleteSelectedDialogListener)notifier);
    }
    if (notifier instanceof OnControllerEntryListFontSizeChangeListener && !mOnControllerEntryListFontSizeChangeListeners.contains(notifier))
    {
      mOnControllerEntryListFontSizeChangeListeners.add((OnControllerEntryListFontSizeChangeListener) notifier);
    }
    if (notifier instanceof OnControllerDeleteAllActionChange && !mOnDeleteAllActionChanges.contains(notifier))
    {
      mOnDeleteAllActionChanges.add((OnControllerDeleteAllActionChange)notifier);
    }
    if (notifier instanceof OnControllerExportActionChangeListener && !mOnExportActionChangeListener.contains(notifier))
    {
      mOnExportActionChangeListener.add((OnControllerExportActionChangeListener) notifier);
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
    if (notifier instanceof OnControllerHistoryListChangedListener)
    {
      mOnHistoryListChangedListeners.remove(notifier);
    }
    if (notifier instanceof OnControllerSelectionModeChangedListener)
    {
      mOnSelectionModeChangedListeners.remove(notifier);
    }
    if (notifier instanceof OnControllerShowDeleteSelectedDialogListener)
    {
      mOnShowDeleteSelectedDialogListeners.remove(notifier);
    }
    if (notifier instanceof OnControllerEntryListFontSizeChangeListener)
    {
      mOnControllerEntryListFontSizeChangeListeners.remove(notifier);
    }
    if (notifier instanceof OnControllerDeleteAllActionChange)
    {
      mOnDeleteAllActionChanges.remove(notifier);
    }
    if (notifier instanceof OnControllerExportActionChangeListener)
    {
      mOnExportActionChangeListener.remove(notifier);
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
   * Update list of history words in active controller. Set list of words filtered by specified dictionary.
   */
  private void updateControllerWords()
  {
    setWords(getFilteredHistoryWords());
  }

  /**
   * Get list of history words filtered by specified dictionary.
   * @return List of history words filtered by specified dictionary or list of all history words if
   * dictionary is not specified
   */
  private List< ArticleItem > getFilteredHistoryWords() {
    List< ArticleItem > filteredList = new LinkedList<>();
    if ( mDictionaryManager == null || mDictionaryManager.isSelectAllDictionaries() )
    {
      filteredList = new LinkedList<>(mManager.getWords());
    }
    else
    {
      DictionaryAndDirection dictAndDirection = mDictionaryManager.getDictionaryAndDirectionSelectedByUser();
      for ( ArticleItem item : mManager.getWords() )
      {
        if ( dictAndDirection != null && item.getDictId().equals(dictAndDirection.getDictionaryId()) )
        {
          filteredList.add(item);
        }
      }
    }
    return filteredList;
  }

  /**
   * Update entry list font size parameter for controller. Load this parameter using {@link SettingsManagerAPI}
   * object.
   */
  private void updateControllerEntryListFontSize()
  {
    if (mSettingsManager != null)
    {
      ApplicationSettings applicationSettings = mSettingsManager.loadApplicationSettings();
      setEntryListFontSize(applicationSettings.getEntryListFontSize());
    }
  }

  /**
   * Update toolbar manager state. Show toolbar in selection mode or with dictionary list spinner.
   */
  private void updateToolbar()
  {
      if ( isInSelectionMode() )
      {
          setDeleteAllActionVisibilityStatus(View.GONE);
          setExportAllActionVisibilityStatus(View.GONE);
          if (mToolbarManager != null) {
              mToolbarManager.showSelectionMode(getSelectedWords().size());
          }
      }
      else
      {
          setDefaultToolbarState();
          setDeleteAllActionVisibilityStatus(View.VISIBLE);
          setExportAllActionVisibilityStatus(View.VISIBLE);
          setDeleteAllActionEnable(!getWords().isEmpty() && !mInTransition);
          setExportActionEnable(!getWords().isEmpty() && !mInTransition);
      }
  }

  protected void setDefaultToolbarState() {
      if (mToolbarManager != null) {
          mToolbarManager.showDictionaryListNoDirection();
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

  @Override
  public boolean isInSelectionMode()
  {
    return mSelectionMode;
  }

  @Override
  public void openArticle( int articleIndex, Context context )
  {
    ShowArticleOptions showArticleOptions = new ShowArticleOptions(SwipeMode.A_Z_OF_DICTIONARY);
    mScreenOpener.showArticle(mHistoryWords.get(articleIndex), showArticleOptions, context);
  }

  @NonNull
  @Override
  public List< ArticleItem > getWords()
  {
    return Collections.unmodifiableList(mHistoryWords);
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
  public boolean isInTransition()
  {
    return mInTransition;
  }

  @Override
  public void deleteAllWords()
  {
    mManager.removeWords(getFilteredHistoryWords());
  }

  @Override
  public void deleteSelectedWords()
  {
    if ( isInSelectionMode() && !getSelectedWords().isEmpty())
    {
      mManager.removeWords(getSelectedWords());
      setSelectionMode(false);
    }
  }

  @Override
  public void selectItem( int articleIndex )
  {
    ArticleItem word = mHistoryWords.size() > articleIndex ? mHistoryWords.get(articleIndex) : null;
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
    ArticleItem word = mHistoryWords.get(articleIndex);
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
    if ( mSoundManager != null && articleIndex < mHistoryWords.size()  )
    {
      ArticleItem word = mHistoryWords.get(articleIndex);
      return mSoundManager.itemHasSound(word);
    }
    return false;
  }

  @Override
  public void playSound( int articleIndex )
  {
    if ( mSoundManager != null && articleIndex < mHistoryWords.size() )
    {
      ArticleItem word = mHistoryWords.get(articleIndex);
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
  public int getExportActionVisibilityStatus()
  {
    return mExportActionStatus;
  }

  @Override
  public int getDeleteAllActionVisibilityStatus()
  {
    return mDeleteAllActionStatus;
  }

  @Override
  public boolean isExportActionEnable()
  {
    return mExportActionEnable;
  }

  @Override
  public boolean isDeleteAllActionEnable()
  {
    return mDeleteAllActionEnabled;
  }

  @Override
  public void exportHistory( @NonNull Context context, @NonNull ExportParams params )
  {
    IExportHTMLBuilder builder = createExportHtmlBuilder();
    if( builder == null )
      return;

    if( !mHistoryWords.isEmpty() )
    {
      try
      {
        File file = new File(context.getFilesDir(), HISTORY_MANAGER_EXPORT_FILE_PATH);
        if( file.exists() )
        {
          file.delete();
        }
        file.createNewFile();

        FileOutputStream writer = new FileOutputStream(file, true);
        writer.write(builder
                .addPartOfSpeechColor(params.getPartOfSpeechColor())
                .addHeader(params.getHeader())
                .addContent(mHistoryWords)
                .addFooter(params.getFooter())
                .createHTMLString()
                .getBytes()
        );

        Intent exportIntent = getIntentForExport(MIME_TYPE_HTML);
        exportIntent.putExtra(Intent.EXTRA_SUBJECT, params.getHeader());
        exportIntent.putExtra(Intent.EXTRA_TEXT, params.getExportFileDescription());
        exportIntent.putExtra(
                Intent.EXTRA_STREAM,
                FileProvider.getUriForFile(context, mAuthoritiesFileProvider, file));
        context.startActivity(Intent.createChooser(exportIntent, params.getChooserTitle()));
      } catch (IOException ignore) { }
    }
  }

  @Override
  public void exportHistoryAsString(@NonNull Context context, ExportParams params) {
    IExportHTMLBuilder builder = createExportHtmlBuilder();
    if( builder == null || mHistoryWords.isEmpty() )
      return;

    Intent exportIntent = getIntentForExport(MIME_TYPE_TEXT);
    exportIntent.putExtra(Intent.EXTRA_SUBJECT, params.getHeader());
//    Adding content to message
    String intentContent = builder
            .addPartOfSpeechColor(params.getPartOfSpeechColor())
            .addHeader(params.getHeader())
            .addContent(mHistoryWords)
            .addFooter(params.getFooter())
            .toString()
            .trim();
    exportIntent.putExtra(Intent.EXTRA_TEXT, intentContent);

    context.startActivity(Intent.createChooser(exportIntent, params.getChooserTitle()));
  }

  @Override
  public void exportHistoryAsHtmlString(@NonNull Activity activity, ExportParams params) {
    IExportHTMLBuilder exportHtmlBuilder = createExportHtmlBuilder();
    if( exportHtmlBuilder == null || mHistoryWords.isEmpty() )
      return;

////    Adding content to message
    String intentContent = exportHtmlBuilder
        .addPartOfSpeechColor(params.getPartOfSpeechColor())
        .addHeader(params.getHeader())
        .addContent(mHistoryWords)
        .addFooter(params.getFooter())
        .createHTMLString()
        .trim();

    ShareCompat.IntentBuilder builder = ShareCompat.IntentBuilder.from(activity);
    builder.setType(MIME_TYPE_HTML).setSubject(params.getHeader());
    Intent shareIntent;
    builder.setHtmlText(intentContent);
    shareIntent = builder.getIntent();

    activity.startActivity(Intent.createChooser(shareIntent, params.getChooserTitle()));
  }

  @VisibleForTesting
  protected Intent getIntentForExport(String mimeTypes) {
    Intent exportIntent = new Intent(Intent.ACTION_SEND);
    exportIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    exportIntent.setType(mimeTypes);
    return exportIntent;
  }

  @Override
  void activate()
  {
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
  void deactivate()
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
    mCompositeDisposable.clear();
  }

  void setWords( @NonNull List< ArticleItem > words )
  {
    mHistoryWords = words;
    for ( OnControllerHistoryListChangedListener listener : mOnHistoryListChangedListeners)
    {
      listener.onControllerHistoryListChanged(mHistoryWords);
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
      setSelectedWords(new ArrayList< ArticleItem >());
    }
    updateToolbar();
  }

  void setEntryListFontSize( float entryListFontSize )
  {
    if ( mEntryListFontSize != entryListFontSize )
    {
      mEntryListFontSize = entryListFontSize;
      for ( OnControllerEntryListFontSizeChangeListener listener : mOnControllerEntryListFontSizeChangeListeners )
      {
        listener.onControllerEntryListFontSizeChanged();
      }
    }
  }

  private void setExportActionEnable( boolean enabled )
  {
    if ( mExportActionEnable != enabled )
    {
      mExportActionEnable = enabled;
      notifyExportActionEnabledChanged();
    }
  }

  private void setExportAllActionVisibilityStatus( int status )
  {
    if ( mExportActionStatus != status )
    {
      mExportActionStatus = status;
      notifyExportActionStatusChanged();
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
  IDictionaryIcon getDictionaryIcon(Dictionary.DictionaryId dictionaryId)
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
  LocalizedString getCurrentDictionaryTitle() {
    if (mDictionaryManager != null
        && mDictionaryManager.getDictionaryAndDirectionSelectedByUser()!=null
        && !mDictionaryManager.isSelectAllDictionaries()) {
      Dictionary dictionary = mDictionaryManager
          .getDictionaryById(mDictionaryManager.getDictionaryAndDirectionSelectedByUser().getDictionaryId());
      return dictionary.getTitle();
    }
    return null;
  }

  @Override
  boolean showHintManagerDialog( @NonNull HintType hintType,
                                 @Nullable FragmentManager fragmentManager,
                                 @Nullable HintParams hintParams )
  {
    if ( mHintManager != null )
    {
      return mHintManager.showHintDialog(hintType, fragmentManager, hintParams);
    }
    return false;
  }

  private void notifyExportActionStatusChanged()
  {
    for (final OnControllerExportActionChangeListener listener : mOnExportActionChangeListener)
    {
      listener.onExportActionEnableStatusChange();
    }
  }

  private void notifyExportActionEnabledChanged()
  {
    for (final OnControllerExportActionChangeListener listener : mOnExportActionChangeListener)
    {
      listener.onExportVisibilityStatusChange();
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
  public void onHistoryManagerError( Exception exception )
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
  public void onHistoryStateChanged()
  {
    updateControllerTransitionState();
    updateToolbar();
  }

  @Override
  public void onSelectAllActionClick()
  {
    if ( isInSelectionMode() )
    {
      setSelectedWords(getFilteredHistoryWords());
      updateToolbar();
    }
  }

  @Override
  public void onBackActionClick()
  {
    if ( isInSelectionMode() )
    {
      setSelectionMode(false);
    }
  }

  @Override
  public void onHistoryListChanged( List< ArticleItem > favoritesWords )
  {
    updateState();
  }

  private IExportHTMLBuilder createExportHtmlBuilder()
  {
    if(mExportHtmlBuilderClass == null)
      return null;

    try
    {
      return mExportHtmlBuilderClass.newInstance();
    }
    catch (InstantiationException | IllegalAccessException ignore)
    {
    }

    return null;
  }
}
