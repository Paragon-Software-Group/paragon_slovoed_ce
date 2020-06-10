package com.paragon_software.news_manager;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.news_manager.mock.MockNewsStorage;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.toolbar_manager.ToolbarManager;

import java.util.ArrayList;
import java.util.List;

import shdd.android.components.news.storage.NewsStorage;

/**
 * Created by Ivan Kuzmin on 19.02.2020;
 * 3van@mail.ru;
 * Copyright © 2020 Example. All rights reserved.
 */

public abstract class MockBaseNewsManager extends BaseNewsManager
{
    public MockBaseNewsManager(@NonNull Context context,
                               @NonNull SettingsManagerAPI settingsManagerAPI,
                               @NonNull ToolbarManager toolbarManager,
                               @Nullable HintManagerAPI hintManager,
                               @Nullable ScreenOpenerAPI screenOpener,
                               @NonNull NewsNotificationOptions notificationOptions,
                               @NonNull String catalogOrProductId,
                               @NonNull Mode mode,
                               @Nullable String pkey)
    {
        super(context, settingsManagerAPI, toolbarManager, hintManager, screenOpener
                , notificationOptions, catalogOrProductId, mode, pkey);
    }

    @Override
    protected NewsStorage getNewsStorage(Context context) {
        return new MockNewsStorage(context, getMockNewsList());
    }

    @NonNull
    private List<NewsItem> getMockNewsList()
    {
        List<NewsItem> mockNewsList = new ArrayList<>(11);

        for (int i = 0; i < 10; i++) {
            mockNewsList.add(new MockNewsItem(i));
        }
        mockNewsList.add(new MockNewsItem(10, true));

        return mockNewsList;
    }
}