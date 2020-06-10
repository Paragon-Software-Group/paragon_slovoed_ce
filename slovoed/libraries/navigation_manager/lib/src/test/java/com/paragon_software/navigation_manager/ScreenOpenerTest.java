package com.paragon_software.navigation_manager;

import android.content.Context;
import android.os.Bundle;
import android.util.Pair;

import androidx.annotation.NonNull;

import com.paragon_software.article_manager.ArticleControllerType;
import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.article_manager.ArticleManagerAPI;
import com.paragon_software.article_manager.ShowArticleOptions;
import com.paragon_software.article_manager.SwipeMode;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.favorites_manager.FavoritesManagerAPI;
import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.screen_opener_api.ScreenType;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;
import org.robolectric.RobolectricTestRunner;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

import io.reactivex.disposables.CompositeDisposable;

import static junit.framework.TestCase.assertSame;
import static org.junit.Assert.assertEquals;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

@RunWith( RobolectricTestRunner.class )
public class ScreenOpenerTest
{
    @Rule
    public MockitoRule rule = MockitoJUnit.rule();

    @Mock
    private ArticleManagerAPI mArticleManager;

    @Mock
    private FavoritesManagerAPI mFavoritesManager;

    @Mock
    private NavigationManagerApi mNavigationManager;

    @Mock
    private Context mContext;

    private static @NonNull
    final Dictionary.DictionaryId DICT1_ID = new Dictionary.DictionaryId("111");

    private ArticleItem mArticleItem = new ArticleItem.Builder(DICT1_ID, 0, 0).build();

    private ShowArticleOptions mOptions = new ShowArticleOptions(SwipeMode.A_Z_OF_DICTIONARY);

    private CompositeDisposable mCompositeDisposable;
    private ScreenOpenerAPI mScreenOpener;
    private ScreenOpenerAPI mScreenOpenerTablet;

    @Before
    public void setUp() {

        mNavigationManager = new NavigationManager();
        mNavigationManager.registerArticleManager(mArticleManager);

        mScreenOpener = mNavigationManager.createScreenOpener(ArticleControllerType.DEFAULT_CONTROLLER, false);
        mScreenOpenerTablet = mNavigationManager.createScreenOpener(ArticleControllerType.DEFAULT_CONTROLLER, true);

        mCompositeDisposable = new CompositeDisposable();
    }

    @Test
    public void testShowArticleActivity() {
        mScreenOpener.showArticleActivity(mArticleItem, mContext);
        mScreenOpenerTablet.showArticleActivity(mArticleItem, mContext);
        verify(mArticleManager, times(2)).showArticleActivity(mArticleItem, ArticleControllerType.DEFAULT_CONTROLLER, mContext);

        assertEquals(ArticleControllerType.DEFAULT_CONTROLLER, mScreenOpener.getArticleControllerId());
        assertEquals(ArticleControllerType.DEFAULT_CONTROLLER, mScreenOpenerTablet.getArticleControllerId());
    }


    @Test
    public void testShowArticle() {
        mScreenOpener.showArticle(mArticleItem, mOptions, mContext);
        verify(mArticleManager, times(1)).showArticleActivity(mArticleItem, mOptions, ArticleControllerType.DEFAULT_CONTROLLER, mContext);
        mScreenOpenerTablet.showArticle(mArticleItem, mOptions, mContext);
        verify(mArticleManager, times(1)).showArticle(mArticleItem, mOptions, ArticleControllerType.DEFAULT_CONTROLLER, mContext);
    }

    @Test
    public void testArticleFromSeparateList() {
        mScreenOpener.showArticleFromSeparateList(new ArrayList<>(), 1, mContext);
        verify(mArticleManager, times(1)).showArticleActivityFromSeparateList(new ArrayList<>(), 1, ArticleControllerType.DEFAULT_CONTROLLER, mContext);
        mScreenOpenerTablet.showArticleFromSeparateList(new ArrayList<>(), 1, mContext);
        verify(mArticleManager, times(1)).showArticleFromSeparateList(new ArrayList<>(), 1, ArticleControllerType.DEFAULT_CONTROLLER, mContext);
    }

    @Test
    public void testOpenScreen() {
        AtomicInteger counter = new AtomicInteger();
        AtomicInteger counterTablet = new AtomicInteger();
        List<ScreenType> test = Arrays.asList(ScreenType.Search, ScreenType.Article
                                                    , ScreenType.Dictionaries, ScreenType.History);
        List< Pair <ScreenType, Bundle> > result = new ArrayList<>();
        List< Pair <ScreenType, Bundle> > resultTablet = new ArrayList<>();
        mCompositeDisposable.addAll(
                mScreenOpener.getScreenOpenerObservable()
                        .subscribe(screenTypePair ->
                        {
                            result.add(screenTypePair);
                            counter.getAndIncrement();
                        }));
        mCompositeDisposable.addAll(
                mScreenOpenerTablet.getScreenOpenerObservable()
                        .subscribe(screenTypePair ->
                        {
                            resultTablet.add(screenTypePair);
                            counterTablet.getAndIncrement();
                        }));

        Bundle tempBundle = new Bundle();
        mScreenOpener.openScreen(test.get(0));
        mScreenOpener.openScreen(test.get(1));
        mScreenOpener.openScreen(test.get(2), null);
        mScreenOpener.openScreen(test.get(3), tempBundle);
        assertSame(counter.get(), counterTablet.get());
        assertSame(4, counter.get());
        assertEquals(result, resultTablet);

        for (int i = 0; i < test.size(); i++)
        {
            Pair<ScreenType, Bundle> pair = result.get(i);
            ScreenType screenTypeTest = test.get(i);
            assertEquals(screenTypeTest, pair.first);
            if (i != 3)
                assertEquals(pair.second, null);
            else
                assertEquals(pair.second, tempBundle);
        }
    }
}
