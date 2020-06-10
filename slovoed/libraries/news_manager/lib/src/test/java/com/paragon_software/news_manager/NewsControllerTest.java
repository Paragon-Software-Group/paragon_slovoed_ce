package com.paragon_software.news_manager;

import android.content.Context;

import androidx.test.core.app.ApplicationProvider;

import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.hint_manager.HintType;
import com.paragon_software.news_manager.mock.MockOnControllerNewsContentStateListener;
import com.paragon_software.news_manager.mock.MockOnControllerNewsListChangedListener;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.settings_manager.MockSettingsManager;
import com.paragon_software.settings_manager.SettingsManagerAPI;
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

import static com.paragon_software.news_manager.mock.StaticConsts.DELTA;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.BDDMockito.given;
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
public class NewsControllerTest
{
    @Rule
    public MockitoRule rule = MockitoJUnit.rule();

    @Mock
    private ToolbarManager mToolbarManager;
    @Mock
    private HintManagerAPI mHintManager;
    @Mock
    private ScreenOpenerAPI mScreenOpener;
    @Mock
    private NewsNotificationOptions mNewsNotificationOptions;

    private MockBaseNewsManager mockBaseNewsManager;

    private BaseNewsController mNewsController;

    @Before
    public void setUp()
    {
        final Context context = ApplicationProvider.getApplicationContext();

        ApplicationSettings appSettings = Mockito.spy(ApplicationSettings.class);
        SettingsManagerAPI mSettingsManager = Mockito.mock(MockSettingsManager.class
                , withSettings().useConstructor(appSettings).defaultAnswer(CALLS_REAL_METHODS));

        mockBaseNewsManager = Mockito.mock(MockBaseNewsManager.class, Mockito.withSettings()
                .useConstructor(context, mSettingsManager, mToolbarManager, mHintManager
                        , mScreenOpener, mNewsNotificationOptions, "news_id"
                        , NewsManagerAPI.Mode.STANDALONE_MODE, "key")
                .defaultAnswer(Mockito.CALLS_REAL_METHODS));

        mNewsController = new BaseNewsController(mockBaseNewsManager, mToolbarManager
                , mSettingsManager, mHintManager);

        given(appSettings.getEntryListFontSize()).willReturn(ApplicationSettings.getDefaultFontSize());
        given(appSettings.getArticleScale()).willReturn(ApplicationSettings.getDefaultArticleScale());
        given(appSettings.isNewsEnabled()).willReturn(ApplicationSettings.getDefaultReceivedNews());
    }

    @Test
    public void getReadNewsTest()
    {
        int newsListSize = mNewsController.getNewsList().size();
        assertNotEquals(mNewsController.getReadNews().size(), newsListSize);
        mNewsController.markAllAsRead();
        assertEquals(mNewsController.getReadNews().size(), newsListSize);
    }

    @Test
    public void markAllNewsAsReadTest()
    {
        assertNotEquals(mNewsController.getUnReadNews().size(), 0);
        mNewsController.markAllAsRead();
        verify(mockBaseNewsManager, times(1)).markAllAsRead();
        assertEquals(mNewsController.getUnReadNews().size(), 0);
    }

    @Test
    public void refreshNewsTest()
    {
        mNewsController.refreshNews();
        verify(mockBaseNewsManager, times(1)).refreshNews();
    }

    @Test
    public void openNewsItemTest() throws Exception
    {
//        Check default values
        assertNull(mNewsController.getShowItem());
        int unreadNewsCount = mNewsController.getUnReadNews().size();
        assertNotEquals(unreadNewsCount, 0);

//        Find one unread news
        NewsItem unreadNews = null;
        for (NewsItem newsItem: mNewsController.getNewsList())
        {
            if (!newsItem.isRead())
            {
                unreadNews = newsItem;
                break;
            }
        }
        if (unreadNews != null)
        {
//            Checking news controller logic
            mNewsController.openNewsItem(unreadNews);

            assertEquals(unreadNewsCount - 1, mNewsController.getUnReadNews().size());
            assertEquals(unreadNews.getId(), mNewsController.getShowItem().getId());
            verify(mockBaseNewsManager, times(1)).openNewsItem(unreadNews.getId());
        }
        else
        {
            throw new Exception("This test requires at least one unread news item");
        }
    }

    @Test
    public void readNewsCountTest()
    {
        int newsCount = mNewsController.getNewsList().size();
        assertNotEquals(newsCount, 0);

        int unReadNewsCount = mNewsController.getUnReadNews().size();
        int readNewsCount = mNewsController.getReadNews().size();
        int sumOfNews = unReadNewsCount + readNewsCount;

        assertEquals(newsCount, sumOfNews);
    }

    @Test
    public void newsListChangesTest()
    {
        MockOnControllerNewsListChangedListener listChangesListener
                = Mockito.mock(MockOnControllerNewsListChangedListener.class, withSettings()
                .useConstructor(2).defaultAnswer(CALLS_REAL_METHODS));
        MockOnControllerNewsListChangedListener listChangesListener2
                = Mockito.mock(MockOnControllerNewsListChangedListener.class, withSettings()
                .useConstructor(3).defaultAnswer(CALLS_REAL_METHODS));

        mNewsController.registerNotifier(listChangesListener);
        mNewsController.registerNotifier(listChangesListener2);
        mNewsController.onNewsListChanged();
        verify(listChangesListener, times(1)).onControllerNewsListChanged();
        assertEquals(listChangesListener.getNumberOfCalls(), 1);
        mNewsController.unregisterNotifier(listChangesListener);

        mNewsController.onNewsListChanged();
        mNewsController.onNewsListChanged();
        verify(listChangesListener2, times(3)).onControllerNewsListChanged();
        verify(listChangesListener, times(1)).onControllerNewsListChanged();
        mNewsController.unregisterNotifier(listChangesListener2);

//        Simulation of changes in the manager
        mockBaseNewsManager.markAllAsRead();
        mNewsController.registerNotifier(listChangesListener);
        mockBaseNewsManager.markAllAsRead();
        verify(listChangesListener, times(2)).onControllerNewsListChanged();
        assertEquals(listChangesListener.getNumberOfCalls(), 2);
    }

    @Test
    public void newsContentStateTest()
    {
        MockOnControllerNewsContentStateListener contentStateListener
                = Mockito.mock(MockOnControllerNewsContentStateListener.class, withSettings()
                .useConstructor(1, mNewsController).defaultAnswer(CALLS_REAL_METHODS));

        mNewsController.registerNotifier(contentStateListener);

        boolean newsContentVisible = true;
        mNewsController.showNewsContent(newsContentVisible);
        verify(contentStateListener, times(1)).onNewsContentStateListener(newsContentVisible);
        assertEquals(contentStateListener.getNumberOfCalls(), 1);
        mNewsController.unregisterNotifier(contentStateListener);
        mNewsController.showNewsContent(false);
        verify(contentStateListener, times(1)).onNewsContentStateListener(newsContentVisible);

//        Simulation of changes in the manager
        mockBaseNewsManager.markAllAsRead();
        mNewsController.registerNotifier(contentStateListener);
        mockBaseNewsManager.markAllAsRead();
        verify(contentStateListener, times(1)).onNewsContentStateListener(newsContentVisible);
        assertEquals(contentStateListener.getNumberOfCalls(), 1);
    }

    @Test
    public void showNewsContentTest()
    {
        MockOnControllerNewsListChangedListener listChangesListener
                = Mockito.mock(MockOnControllerNewsListChangedListener.class, withSettings()
                .useConstructor(2).defaultAnswer(CALLS_REAL_METHODS));

        MockOnControllerNewsContentStateListener contentStateListener
                = Mockito.mock(MockOnControllerNewsContentStateListener.class, withSettings()
                .useConstructor(2, mNewsController).defaultAnswer(CALLS_REAL_METHODS));

        mNewsController.registerNotifier(listChangesListener);
        mNewsController.registerNotifier(contentStateListener);

        mNewsController.showNewsContent(true);

        verify(mToolbarManager, times(1)).showHomeAsUp(true);
        verify(mToolbarManager).registerNotifier(any(ToolbarManager.Notifier.class));

        verify(listChangesListener, times(1)).onControllerNewsListChanged();
        verify(contentStateListener, times(1)).onNewsContentStateListener(true);

        mNewsController.showNewsContent(false);

        verify(mToolbarManager, times(1)).showHomeAsUp(false);
        verify(mToolbarManager).unRegisterNotifier(any(ToolbarManager.Notifier.class));

        verify(listChangesListener, times(2)).onControllerNewsListChanged();
        verify(contentStateListener, times(1)).onNewsContentStateListener(false);
    }

    @Test
    public void getListFontSizeTest()
    {
        assertEquals(mNewsController.getListFontSize(), ApplicationSettings.getDefaultFontSize(), DELTA);
    }

    @Test
    public void  getArticleScaleTest()
    {
        assertEquals(mNewsController.getArticleScale(), ApplicationSettings.getDefaultArticleScale(), DELTA);
    }

    @Test
    public void isNewsEnabledTest()
    {
        assertTrue(mNewsController.isNewsEnabled());
    }

    @Test
    public void showDialogTest()
    {
        mNewsController.showHintManagerDialog(HintType.News, null, null);
        verify(mHintManager, times(1)).showHintDialog(HintType.News, null, null);
    }
}
