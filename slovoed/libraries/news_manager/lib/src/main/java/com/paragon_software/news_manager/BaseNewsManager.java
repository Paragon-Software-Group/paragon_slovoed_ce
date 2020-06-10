package com.paragon_software.news_manager;

import android.content.Context;
import android.database.Cursor;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.screen_opener_api.ScreenType;
import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.settings_manager.OnSettingsSaveListener;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;
import com.paragon_software.toolbar_manager.ToolbarManager;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Callable;
import java.util.concurrent.atomic.AtomicReference;

import io.reactivex.Observable;
import io.reactivex.Single;
import io.reactivex.observers.DisposableSingleObserver;
import io.reactivex.subjects.BehaviorSubject;
import shdd.android.components.news.AdsManager;
import shdd.android.components.news.data.NewsItemDB;
import shdd.android.components.news.storage.NewsStorage;

public class BaseNewsManager extends NewsManagerAPI implements OnSettingsSaveListener {

	protected static String TEST_NEWS_PREF_KEY = "test_mode_news";

	private final Context mContext;
	private final ScreenOpenerAPI mScreenOpener;
	private ToolbarManager mToolbarManager;
	private NewsStorage mNewsStorage;
	private final List<OnNewsListChangesListener> mOnNewsListChangesListeners = new ArrayList<>();
	private Map<String, NewsControllerAPI> mNewsControllers = new HashMap<>();
	private AtomicReference<NewsControllerAPI> mActiveController = new AtomicReference<>();
	private NewsResourcesInit mResourcesInit;
	private NewsItem mShowItem;
	private SettingsManagerAPI mSettingsManager;
	@Nullable
	private HintManagerAPI   mHintManager;

	private final BehaviorSubject<Integer> mUnreadNewsCountSubject = BehaviorSubject.createDefault(0);

	public BaseNewsManager( @NonNull Context context, @NonNull SettingsManagerAPI settingsManagerAPI, @NonNull ToolbarManager toolbarManager,
							@Nullable HintManagerAPI hintManager, @Nullable ScreenOpenerAPI screenOpener, @NonNull NewsNotificationOptions notificationOptions,
							@NonNull String catalogOrProductId, @NonNull Mode mode, @Nullable String pkey ) {
		registerToolbarManager(toolbarManager);
		registerSettingManager(settingsManagerAPI);
		mHintManager = hintManager;
		mScreenOpener = screenOpener;
		mContext = context.getApplicationContext();
		initNewsLibrary(notificationOptions, catalogOrProductId, mode, pkey);
	}

	private void initNewsLibrary(NewsNotificationOptions notificationOptions, String catalogOrProductId, Mode mode, @Nullable String pkey) {
		mResourcesInit = new NewsResourcesInit(mContext, notificationOptions, catalogOrProductId, mode, pkey, getTestModeState());
		AdsManager.getInstance().init(mContext, mResourcesInit);
		AdsManager.getInstance().forceUpdateNewsCheckAlarm(mContext);
		mNewsStorage = getNewsStorage(mContext);
		registerNewsReceiver();
		refreshNews();
	}

	//		Protected access need for tests (mock)
	protected NewsStorage getNewsStorage(Context context) {
		return new NewsStorage(context);
	}

	private void registerToolbarManager(@NonNull ToolbarManager toolbarManager) {
		mToolbarManager = toolbarManager;
	}

	private void registerSettingManager(@NonNull SettingsManagerAPI settingsManagerAPI) {
		mSettingsManager = settingsManagerAPI;
		settingsManagerAPI.registerNotifier(this);
	}

	@Override
	public void registerNotifier(@NonNull Notifier notifier) {
		if (notifier instanceof OnNewsListChangesListener && !mOnNewsListChangesListeners.contains(notifier)) {
			mOnNewsListChangesListeners.add((OnNewsListChangesListener) notifier);
			registerNewsReceiver();
			refreshNews();
			updateUnreadNewsCount();
		}
	}

	@Override
	public void unregisterNotifier(@NonNull Notifier notifier) {
		if (notifier instanceof OnNewsListChangesListener) {
			mOnNewsListChangesListeners.remove(notifier);
			if ( mOnNewsListChangesListeners.isEmpty() )
				unregisterNewsReceiver();
		}
	}

	private void registerNewsReceiver() {
		AdsManager.getInstance().setNewsReceiver(new Runnable() {
			@Override
			public void run() {
				updateNews();
			}
		});
	}

	private void updateNews() {
		updateUnreadNewsCount();
		for (OnNewsListChangesListener notifier : mOnNewsListChangesListeners) {
			notifier.onNewsListChanged();
		}
	}

	private void updateUnreadNewsCount() {
		Single.fromCallable(new Callable<Integer>() {
			@Override
			public Integer call() {
				return mNewsStorage.getUnreadCount();
			}
		}).subscribe(new DisposableSingleObserver<Integer>() {
			@Override
			public void onSuccess(Integer unreadNewsCount) {
				mUnreadNewsCountSubject.onNext(unreadNewsCount);
				dispose();
			}

			@Override
			public void onError(Throwable e) {
				dispose();
			}
		});
	}

	private void unregisterNewsReceiver() {
		AdsManager.getInstance().setNewsReceiver(null);
	}

	private List<NewsItem> getNews() {
		List<NewsItem> news = new ArrayList<>();
		try (Cursor cursor = mNewsStorage.getContainerNewsCursor())
		{
		for (cursor.moveToFirst(); !cursor.isAfterLast(); cursor.moveToNext()) {
			news.add(NewsItem.createFrom(cursor));
		}
	}catch(Exception e) {
		e.printStackTrace();
	}
	return news;
	}

	@NonNull
	@Override
	List<NewsItem> getNewsItems() {
		return getNews();
	}

	@Override
	NewsItem getShowItem() {
		return mShowItem;
	}

	@NonNull
	@Override
	List<NewsItem> getReadNews() {
		List<NewsItem> read = new ArrayList<>();
		for (NewsItem newsItem : getNews()) {
			if (newsItem.isRead()) {
				read.add(newsItem);
			}
		}
		return read;
	}

	@NonNull
	@Override
	List<NewsItem> getUnReadNews() {
		List<NewsItem> unread = new ArrayList<>();
		for (NewsItem newsItem : getNews()) {
			if (!newsItem.isRead()) {
				unread.add(newsItem);
			}
		}
		return unread;
	}

	@Override
	public Observable<Integer> getUnreadNewsCount() {
		return mUnreadNewsCountSubject;
	}

	@Override
	public void refreshNews() {
		AdsManager.getInstance().forcePullAdsAndNews(mContext);
	}

	@Override
	public void openNewsItem(int newsId) {
		mScreenOpener.openScreen(ScreenType.NewsItem);
		try (Cursor cursor = mNewsStorage.getContainerNewsCursorById(newsId)) {
			NewsItemDB newsItemDB;
			cursor.moveToFirst();
			newsItemDB = NewsItemDB.createFrom(cursor);
			if (!newsItemDB.isRead()) {
				newsItemDB.setIsRead(true);
				mNewsStorage.updateNews(newsItemDB);
				updateNews();
			}
			mShowItem = NewsItem.createFrom(cursor);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	@Override
	void markAllAsRead() {
		mNewsStorage.markAllAsRead();
		updateNews();
	}

	@NonNull
	@Override
	NewsControllerAPI getController(@NonNull String uiName) {
		NewsControllerAPI controller = mNewsControllers.get(uiName);
		if (controller == null) {
			if (NewsControllerType.BILINGUAL_NEWS_CONTROLLER.equals(uiName)) {
				controller = new BilingualNewsController(this, mToolbarManager, mSettingsManager, mHintManager);
			} else {
				controller = new BaseNewsController(this, mToolbarManager, mSettingsManager, mHintManager);
			}
			mNewsControllers.put(uiName, controller);
		}
		mActiveController.set(controller);
		return mActiveController.get();
	}


	@Override
	void freeController(@NonNull String uiName) {

		if (mActiveController.get() == mNewsControllers.get(uiName)) {
			mActiveController.set(null);
		}
	}

	@Override
	public void changeTestModeState(boolean isTestMode) {
		try
		{
			mSettingsManager.save(TEST_NEWS_PREF_KEY, isTestMode, false);
			mResourcesInit.setIsTestMode(isTestMode);
			AdsManager.getInstance().forceReRegister(mContext);
		}
		catch ( ManagerInitException | LocalResourceUnavailableException exception )
		{
			exception.printStackTrace();
		}
	}

	@Override
	public boolean getTestModeState()
	{
		try
		{
			return mSettingsManager.load(TEST_NEWS_PREF_KEY, false);
		}
		catch ( WrongTypeException | ManagerInitException exception )
		{
			return false;
		}
	}

	@Override
	public void onSettingsSaved(String name, Serializable data) {
		if (data instanceof ApplicationSettings) {
			AdsManager.getInstance().settingsChanged(mContext);
			updateNews();
		}
	}
}
