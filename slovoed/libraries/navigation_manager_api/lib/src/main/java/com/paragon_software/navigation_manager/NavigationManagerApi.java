package com.paragon_software.navigation_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

import com.paragon_software.article_manager.ArticleManagerAPI;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.screen_opener_api.ScreenType;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.favorites_manager.FavoritesManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.history_manager.HistoryManagerAPI;
import com.paragon_software.native_engine.EngineInformationAPI;
import com.paragon_software.news_manager.NewsManagerAPI;
import com.paragon_software.search_manager.SearchManagerAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;
import com.paragon_software.word_of_day.WotDManagerAPI;

import io.reactivex.Observable;

public abstract class NavigationManagerApi {
    public abstract void registerDictionaryManager(@NonNull DictionaryManagerAPI dictionaryManager);
    public abstract void registerSearchManager(@NonNull SearchManagerAPI searchManager);
    public abstract void registerFavoritesManager(@NonNull FavoritesManagerAPI favoritesManager);
    public abstract void registerHistoryManager(@NonNull HistoryManagerAPI historyManager);
    public abstract void registerNewsManager(@NonNull NewsManagerAPI newsManager);
    public abstract void registerToolbarManager(@NonNull ToolbarManager toolbarManager);
    public abstract void registerEngineInformation(@NonNull EngineInformationAPI engineInformation);
    public abstract void registerWotDManager(@NonNull WotDManagerAPI wotDManager);
    public abstract void registerSettingsManager(@Nullable SettingsManagerAPI settingsManager );
    public abstract void registerHintManager(@Nullable HintManagerAPI hintManager );
    public abstract void registerArticleManager(@Nullable ArticleManagerAPI articleManager);

    public abstract void registerFragment(@NonNull ScreenType screenType, @Nullable Fragment fragment );
    public abstract void registerActivity( @NonNull ScreenType screenType, @Nullable Class activityClass );

    public abstract void registerScreenOpener(ScreenOpenerAPI screenOpener);

    public abstract ScreenOpenerAPI createScreenOpener(String controllerIdFlashcards, boolean useTabletOpener );

    abstract @Nullable ArticleManagerAPI getArticleManager();

    abstract @Nullable Fragment getFragment( @NonNull ScreenType screenType );
    abstract @Nullable Class getActivity( @NonNull ScreenType screenType );

    abstract NavigationControllerApi getController(@NonNull String tag);
    abstract SettingsManagerAPI getSettingsManager();

    abstract @NonNull Observable< Boolean > getTopScreenOverlayStateObservable();
}
