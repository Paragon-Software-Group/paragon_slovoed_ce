package com.paragon_software.news_manager;

import android.content.Context;

import androidx.test.core.app.ApplicationProvider;

import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.news_manager.mock.MockForManager_OnNewsListChangesListener;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.settings_manager.MockSettingsManager;
import com.paragon_software.settings_manager.SettingsManagerAPI;
import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;
import com.paragon_software.settings_manager.exceptions.ManagerInitException;
import com.paragon_software.settings_manager.exceptions.WrongTypeException;
import com.paragon_software.toolbar_manager.ToolbarManager;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;

import java.util.List;

import static com.paragon_software.news_manager.BaseNewsManager.TEST_NEWS_PREF_KEY;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.CALLS_REAL_METHODS;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.withSettings;

/**
 * Created by Ivan Kuzmin on 19.02.2020;
 * 3van@mail.ru;
 * Copyright © 2020 Example. All rights reserved.
 */

@RunWith( RobolectricTestRunner.class )
public class NewsManagerTest
{
    @Rule
    public MockitoRule rule = MockitoJUnit.rule();

    @Mock
    private ToolbarManager mToolbarManager;
    @Mock
    private ScreenOpenerAPI mScreenOpener;
    @Mock
    private HintManagerAPI mHintManager;
    @Mock
    private NewsNotificationOptions mNewsNotificationOptions;

//    Need mock for set mock NewsStorage
    private MockBaseNewsManager mNewsManager;
    private SettingsManagerAPI mSettingsManager;

    @Before
    public void setUp()
    {
        final Context context = ApplicationProvider.getApplicationContext();

        ApplicationSettings appSettings = Mockito.spy(ApplicationSettings.class);
        mSettingsManager = Mockito.mock(MockSettingsManager.class
                , withSettings().useConstructor(appSettings).defaultAnswer(CALLS_REAL_METHODS));

        mNewsManager = Mockito.mock(MockBaseNewsManager.class, Mockito.withSettings()
                .useConstructor(context, mSettingsManager, mToolbarManager, mHintManager
                        , mScreenOpener, mNewsNotificationOptions, "news_id"
                        , NewsManagerAPI.Mode.STANDALONE_MODE, "key")
                .defaultAnswer(Mockito.CALLS_REAL_METHODS));
    }

    @Test
    public void getReadNewsTest()
    {
        int newsListSize = mNewsManager.getNewsItems().size();
        assertNotEquals(mNewsManager.getReadNews().size(), newsListSize);
        mNewsManager.markAllAsRead();
        assertEquals(mNewsManager.getReadNews().size(), newsListSize);
    }

    @Test
    public void markAllNewsAsReadTest()
    {
        List<NewsItem> newsList = mNewsManager.getNewsItems();
        assertNotNull(newsList);
        assertTrue(newsList.size() > 1);
        List<NewsItem> unReadNews = mNewsManager.getUnReadNews();
        assertNotEquals(unReadNews.size(), 0);
        mNewsManager.markAllAsRead();
        List<NewsItem> unReadNews1 = mNewsManager.getUnReadNews();
        assertEquals(unReadNews1.size(), 0);
    }

    @Test
    public void openNewsItemTest() throws Exception
    {
//        Check default values
        assertNull(mNewsManager.getShowItem());
        int unreadNewsCount = mNewsManager.getUnReadNews().size();
        assertNotEquals(unreadNewsCount, 0);

//        Find one unread news
        NewsItem unreadNews = null;
        for (NewsItem newsItem: mNewsManager.getNewsItems())
        {
            if (!newsItem.isRead())
            {
                unreadNews = newsItem;
                break;
            }
        }
        if (unreadNews != null)
        {
//            Checking news manager logic
            mNewsManager.openNewsItem(unreadNews.getId());

            assertEquals(unreadNewsCount - 1, mNewsManager.getUnReadNews().size());
            assertEquals(unreadNews.getId(), mNewsManager.getShowItem().getId());
            verify(mNewsManager, times(1)).openNewsItem(unreadNews.getId());
        }
        else
        {
            throw new Exception("This test requires at least one unread news item");
        }
    }

    @Test
    public void readNewsCountTest()
    {
        int newsCount = mNewsManager.getNewsItems().size();
        assertNotEquals(newsCount, 0);

        int unReadNewsCount = mNewsManager.getUnReadNews().size();
        int readNewsCount = mNewsManager.getReadNews().size();
        int sumOfNews = unReadNewsCount + readNewsCount;

        assertEquals(newsCount, sumOfNews);
    }

    @Test
    public void newsListChangesListenerTest()
    {
        MockForManager_OnNewsListChangesListener listChangesListener
                = Mockito.mock(MockForManager_OnNewsListChangesListener.class, withSettings()
                .useConstructor(1).defaultAnswer(CALLS_REAL_METHODS));
        mNewsManager.registerNotifier(listChangesListener);

//        Simulation of changes in the list
        mNewsManager.markAllAsRead();
        verify(listChangesListener, times(1)).onNewsListChanged();
        assertEquals(listChangesListener.getNumberOfCalls(), 1);
        mNewsManager.unregisterNotifier(listChangesListener);
        mNewsManager.markAllAsRead();
        verify(listChangesListener, times(1)).onNewsListChanged();
    }

    @Test
    public void getDefaultControllerTest()
    {
        NewsControllerAPI test = mNewsManager.getController(NewsControllerType.DEFAULT_CONTROLLER);
        assertNotNull(test);
    }

    @Test
    public void changeTestModeStateTest()
            throws ManagerInitException, LocalResourceUnavailableException, WrongTypeException
    {
//        Check default value
        assertFalse(mNewsManager.getTestModeState());
//        NumberOfInvocations is 2, because the first one occurs
//        in BaseNewsManager.initNewsLibrary during initialization.
        verify(mSettingsManager, times(2))
                .load(eq(TEST_NEWS_PREF_KEY), any(Boolean.class));

        boolean isTestMode = true;
        mNewsManager.changeTestModeState(isTestMode);
        assertTrue(mNewsManager.getTestModeState());
        verify(mSettingsManager, times(1))
                .save(eq(TEST_NEWS_PREF_KEY), eq(isTestMode), any(Boolean.class));

        isTestMode = false;
        mNewsManager.changeTestModeState(isTestMode);
        assertFalse(mNewsManager.getTestModeState());
        verify(mSettingsManager, times(1))
                .save(eq(TEST_NEWS_PREF_KEY), eq(isTestMode), any(Boolean.class));
    }
}
