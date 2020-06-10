package com.paragon_software.navigation_manager;

import android.os.Bundle;
import android.util.Pair;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;

import com.paragon_software.screen_opener_api.ScreenType;

import static com.paragon_software.utils_slovoed.static_consts.BundleKeys.BUNDLE_KEY_IS_OPEN_FREE_PREVIEW;

/**
 * Created by Ivan Kuzmin on 17.04.2020;
 * 3van@mail.ru;
 * Copyright © 2020 Example. All rights reserved.
 */

public abstract class BaseNavigationUi implements NavigationUiApi
{
    @NonNull
    protected final NavigationControllerApi mNavigationController;

    protected BaseNavigationUi(@NonNull String tag)
    {
        this.mNavigationController = NavigationHolder.getNavigationManager().getController(tag);
    }

    public BaseNavigationUi(@NonNull NavigationControllerApi mNavigationController)
    {
        this.mNavigationController = mNavigationController;
    }

    @Override
    public boolean onBackPressed(@NonNull FragmentActivity activity)
    {
        return false;
    }

    @Nullable
    protected ScreenType getCurrentScreen()
    {
        return mNavigationController.getCurrentScreen();
    }

    protected abstract void openScreen( Pair< ScreenType, Bundle> screenTypePair );

    protected abstract void openScreen( @NonNull ScreenType screenType,
                                        FragmentActivity mainActivity, Fragment newFragment,
                                        Class newActivityClass, @Nullable Bundle bundle );

    protected void setSelectedItem(@NonNull ScreenType screenType )
    {
        mNavigationController.setCurrentScreen(screenType);
    }

    protected boolean needOpenFragment(ScreenType screenType)
    {
        return !(ScreenType.WotDItem.equals(screenType)
                || ScreenType.NewsItem.equals(screenType)
                || ScreenType.DictionaryDescription.equals(screenType));
    }

    protected boolean needOpenActivity(ScreenType screenType)
    {
        return ScreenType.WotDItem.equals(screenType)
                || ScreenType.NewsItem.equals(screenType)
                || ScreenType.SettingsGeneral.equals(screenType)
                || ScreenType.SettingsMyView.equals(screenType)
                || ScreenType.SettingsNotifications.equals(screenType)
                || ScreenType.DictionaryDescription.equals(screenType)
                || ScreenType.Quiz.equals(screenType)
                || ScreenType.QuizAchievement.equals(screenType)
                || ScreenType.QuizStatistic.equals(screenType)
                || ScreenType.Flashcards.equals(screenType);
    }

    protected ScreenType checkFirstRun( ScreenType screenType, Bundle bundle )
    {
        if (ScreenType.Search.equals(screenType))
        {
            if (bundle == null || !bundle.getBoolean(BUNDLE_KEY_IS_OPEN_FREE_PREVIEW, false))
                screenType = checkAndGetSearchScreenAvailable();
        }
        return screenType;
    }

    protected ScreenType checkAndGetSearchScreenAvailable()
    {
        ScreenType currentScreen = ScreenType.Dictionaries;

        if (mNavigationController.isHaveMyDictionaries()) {
            if (mNavigationController.isSelectedUserDictionaryDownloaded())
                currentScreen = ScreenType.Search;
            else
                currentScreen = ScreenType.Download;
        }
        return currentScreen;
    }
}
