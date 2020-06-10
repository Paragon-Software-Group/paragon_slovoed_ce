package com.paragon_software.news_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.FragmentManager;

import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.hint_manager.HintParams;
import com.paragon_software.hint_manager.HintType;
import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.settings_manager.OnApplicationSettingsSaveListener;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.toolbar_manager.Notifiers.OnBackActionClick;
import com.paragon_software.toolbar_manager.ToolbarManager;

import java.util.ArrayList;
import java.util.List;

public class BaseNewsController extends NewsControllerAPI implements OnBackActionClick, OnNewsListChangesListener, OnApplicationSettingsSaveListener
{

  private final NewsManagerAPI                            mManager;
  private final ToolbarManager                            mToolbarManager;
  private final List<OnControllerNewsListChangedListener> mOnNewsListChangedListeners = new ArrayList<>();
  private final List<OnNewsContentStateListener>          mOnContentStateListeners = new ArrayList<>();
  private final SettingsManagerAPI                        mSettingsManager;
  @Nullable
  private       HintManagerAPI                            mHintManager;
  private       float                                     mEntryListFontSize = ApplicationSettings.getDefaultFontSize();
  private       float                                     mArticleScale = ApplicationSettings.getDefaultArticleScale();
  private       boolean                                   mIsNewsEnabled = ApplicationSettings.getDefaultReceivedNews();


  private boolean mContentVisibility = false;

  BaseNewsController(NewsManagerAPI newsManager, ToolbarManager toolbarManager, SettingsManagerAPI settingsManager,
                     @Nullable HintManagerAPI hintManager) {
    mManager = newsManager;
    mToolbarManager = toolbarManager;
    mSettingsManager = settingsManager;
    mSettingsManager.registerNotifier(this);
    mHintManager = hintManager;

    if (mSettingsManager != null)
      updateControllerSettings(settingsManager.loadApplicationSettings());
  }


  @Override
  public void registerNotifier(@NonNull Notifier notifier) {
    if (notifier instanceof OnControllerNewsListChangedListener && !mOnNewsListChangedListeners.contains(notifier))
    {
      mOnNewsListChangedListeners.add((OnControllerNewsListChangedListener) notifier);
      mManager.registerNotifier(this);
    }
    if (notifier instanceof OnNewsContentStateListener && !mOnContentStateListeners.contains(notifier))
    {
      mOnContentStateListeners.add((OnNewsContentStateListener) notifier);
      mManager.registerNotifier(this);
    }
  }

  @Override
  public void unregisterNotifier(@NonNull Notifier notifier) {
    if (notifier instanceof OnControllerNewsListChangedListener)
    {
      mOnNewsListChangedListeners.remove(notifier);
      mManager.unregisterNotifier(this);
    }
    if(notifier instanceof OnNewsContentStateListener)
    {
      mOnContentStateListeners.remove(notifier);
    }
  }

  @Override
  public void showNewsContent(boolean visibility)
  {
  	mContentVisibility = visibility;
    for (OnNewsContentStateListener listener:mOnContentStateListeners)
    {
      listener.onNewsContentStateListener(visibility);
    }
    showNewsContentToolbarUpdate();

    onNewsListChanged();
  }

  protected void showNewsContentToolbarUpdate() {
    mToolbarManager.showHomeAsUp(mContentVisibility);
    if (mContentVisibility)
      mToolbarManager.registerNotifier(this);
    else
      mToolbarManager.unRegisterNotifier(this);
  }

  @NonNull
  @Override
  public List<NewsItem> getNewsList() {
    return mManager.getNewsItems();
  }

  @NonNull
  @Override
  public List<NewsItem> getReadNews() {
    return mManager.getReadNews();
  }

  @NonNull
  @Override
  public List<NewsItem> getUnReadNews() {
    return mManager.getUnReadNews();
  }

  @Override
  public void markAllAsRead() {
    mManager.markAllAsRead();
  }

  @Override
  public void refreshNews() {
		mManager.refreshNews();
	}

  @Override
  void openNewsItem(NewsItem newsItem) {
    mManager.openNewsItem(newsItem.getId());
    showNewsContent(true);
  }

  @Override
  public boolean isContentVisibility() {
  	return mContentVisibility;
  }

  @Override
  public NewsItem getShowItem() {
	return mManager.getShowItem();
  }

  @Override
  public void onBackActionClick() {
    showNewsContent(false);
  }

  @Override
  public void onNewsListChanged() {

    for (OnControllerNewsListChangedListener listener : mOnNewsListChangedListeners) {
      listener.onControllerNewsListChanged();
    }
  }

  @Override
  public float getListFontSize()
  {
    return mEntryListFontSize;
  }

  @Override
  public float getArticleScale()
  {
    return mArticleScale;
  }

  @Override
  public boolean isNewsEnabled()
  {
    return mIsNewsEnabled;
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

  /**
   * Update entry list font size parameter for controller. Load this parameter using {@link SettingsManagerAPI}
   * object.
   */
  private void updateControllerSettings( @NonNull ApplicationSettings applicationSettings )
  {
    mEntryListFontSize = applicationSettings.getEntryListFontSize();
    mArticleScale = applicationSettings.getArticleScale();
    mIsNewsEnabled = applicationSettings.isNewsEnabled();
  }

  @Override
  public void onApplicationSettingsSaved( @NonNull ApplicationSettings applicationSettings ) {
    updateControllerSettings(applicationSettings);
  }
}
