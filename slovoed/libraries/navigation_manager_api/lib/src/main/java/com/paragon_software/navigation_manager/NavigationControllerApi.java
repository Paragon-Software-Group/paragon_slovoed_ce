package com.paragon_software.navigation_manager;

import android.content.Context;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.util.Pair;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.hint_manager.HintParams;
import com.paragon_software.hint_manager.HintType;
import com.paragon_software.native_engine.EngineVersion;
import com.paragon_software.screen_opener_api.ScreenType;
import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.word_of_day.WotDItem;

import java.util.List;

import io.reactivex.Observable;

public interface NavigationControllerApi {
    interface Notifier {}

    interface OnNewsListChangedListener extends Notifier {
        void onNewsListChanged();
    }

    interface OnDictionariesChangedListener extends Notifier {
        void onDictionariesChanged();
    }

    void registerNotifier(@NonNull Notifier notifier);
    void unregisterNotifier(@NonNull Notifier notifier);
    @Nullable Fragment getFragment( @NonNull ScreenType screenType );
    @Nullable Class getActivity( @NonNull ScreenType screenType );

    @NonNull
    List<Dictionary> getDictionaries();
    @NonNull
    List<Dictionary> getMyDictionaries();
    boolean isHaveMyDictionaries();
    boolean isUserCoreLoggedIn();
    void openUserCoreLoginDialog(FragmentManager fragmentManager);
    void userCoreLogout(Context context);
    boolean isSelectedUserDictionaryDownloaded();
    void openDictionaryForSearch( @NonNull Context context,
                                  @NonNull final Dictionary.DictionaryId dictionaryId,
                                  @Nullable Dictionary.Direction direction,
                                  @Nullable String text);
    boolean isFreePreview();
    void setIsFreePreview(boolean freePreview);
    void setDictionaryAndDirectionSelectedByUser( @NonNull final Dictionary.DictionaryId dictionaryId,
                                                  @Nullable Dictionary.Direction direction );
    Observable< Integer > getUnreadNewsCount();
    Observable< Integer > getNewWotDCount();
    void openWotDItemScreen( @NonNull WotDItem wotDItem );
    void openNewsItemScreen(int newsIdToShow);
    String getDrawerTitle();
    Bitmap getDrawerIcon();
    void showDictionaryListInToolbar();
    void showDictionaryListInToolbar(@Nullable Dictionary.DictionaryId selected, @Nullable Dictionary.Direction direction);
    void updateDictionaryListInToolbar();
    void showTitleAndDirectionInToolbar(@NonNull String title);
    void showTitleInToolbar(@NonNull String title);
    @Nullable
    EngineVersion getEngineVersion();
    void setSearchText(@NonNull String searchUI, @NonNull String searchText);
    void historySelectionModeOff();
    void favoritesSelectionModeOff();
    boolean isTrialExpired();

    @NonNull
    Observable< Boolean > getFullScreenViewStateObservable();
    void setFullScreenViewState(boolean state);
    void clickFullScreenViewAction();

    @NonNull
    Observable< Boolean > getTopScreenOverlayStateObservable();
    void setTopScreenOverlayState(boolean state);

    @NonNull
    Observable< Float > getEntryListFontSizeObservable();

    String getArticleControllerId();

    Observable< Pair< HintType, HintParams > > getNeedToShowHintObservable();
    boolean showHintManagerDialog(@NonNull HintType hintType, @Nullable FragmentManager fragmentManager, @Nullable HintParams hintParams );

    Observable< Pair< ScreenType, Bundle > > getScreenOpenerObservable();
    void openScreen( @NonNull ScreenType screenType );
    void openScreen( @NonNull ScreenType screenType, @Nullable Bundle bundle );

    @Nullable
    ApplicationSettings getApplicationSettings();

    void saveTabletColumnWidth(float widthPercent);

    void restorePurchases(@NonNull Context context);

    void setCurrentScreen(@NonNull ScreenType currentScreen);
    @Nullable
    ScreenType getCurrentScreen();

    @NonNull
    Observable< Boolean > getChangeDictionaryDirectionObservable();
}
