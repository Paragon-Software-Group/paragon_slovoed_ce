package com.paragon_software.news_manager;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;

import java.util.List;

import io.reactivex.Observable;

public abstract class NewsManagerAPI {
	public interface Factory
	{
		NewsManagerAPI create( @NonNull Context context,  @NonNull NewsNotificationOptions notificationOptions,
							   @NonNull String catalogOrProductId, @NonNull Mode mode, @Nullable String pkey );

		Factory registerSettingsManager( @Nullable SettingsManagerAPI settingManager );
		Factory registerToolbarManager( @Nullable ToolbarManager toolbarManager );
		Factory registerHintManager( @Nullable HintManagerAPI hintManager );
        Factory registerScreenOpener(@Nullable ScreenOpenerAPI screenOpener);
    }

	public enum Mode {
		STANDALONE_MODE,
		CONTAINER_MODE;
	}

	public static String INTENT_KEY_ADS_FROM_NOTIFICATION = "ads_from_notification";
	public static String INTENT_KEY_ADS_ID = "ads_id";

	public abstract void registerNotifier(@NonNull Notifier notifier);

	public abstract void unregisterNotifier(@NonNull Notifier notifier);

	@NonNull
	abstract List<NewsItem> getNewsItems();

	abstract NewsItem getShowItem();

	@NonNull
	abstract List<NewsItem> getReadNews();

	@NonNull
	abstract List<NewsItem> getUnReadNews();

	public abstract Observable<Integer> getUnreadNewsCount();

	public abstract void refreshNews();

	public abstract void openNewsItem(int newsId);

	abstract void markAllAsRead();

	@NonNull
	abstract NewsControllerAPI getController(@NonNull String uiName);

	abstract void freeController(@NonNull String uiName);

	public abstract void changeTestModeState(boolean isTestMode);
	public abstract boolean getTestModeState();

	protected interface Notifier {
	}
}
