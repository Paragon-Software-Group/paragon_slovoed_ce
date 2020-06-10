package com.paragon_software.news_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.FragmentManager;

import java.util.List;

import com.paragon_software.hint_manager.HintParams;
import com.paragon_software.hint_manager.HintType;

abstract class NewsControllerAPI {

	abstract void registerNotifier(@NonNull Notifier notifier);

	abstract void unregisterNotifier(@NonNull Notifier notifier);

	abstract void showNewsContent(boolean visibility);

	@NonNull
	public abstract List<NewsItem> getNewsList();

	@NonNull
	abstract List<NewsItem> getReadNews();

	@NonNull
	abstract List<NewsItem> getUnReadNews();

	abstract void markAllAsRead();

	abstract void refreshNews();

	abstract void openNewsItem(NewsItem newsItem);

	abstract boolean isContentVisibility();

	abstract NewsItem getShowItem();

	public abstract float getListFontSize();

	public abstract float getArticleScale();

	public abstract boolean isNewsEnabled();

	abstract boolean showHintManagerDialog( @NonNull HintType hintType, @Nullable FragmentManager fragmentManager, @Nullable HintParams hintParams );

	interface Notifier {
	}
}
